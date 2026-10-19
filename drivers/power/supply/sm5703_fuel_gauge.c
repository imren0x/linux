// SPDX-License-Identifier: GPL-2.0-only
/*
 * Fuel gauge driver for the Silicon Mitus SM5703
 *
 * The SM5703 fuel gauge is a voltage tracking gauge that needs a set of
 * battery specific tables and tuning parameters, which are provided by the
 * device tree. The initialization and calibration sequences are based on the
 * vendor driver in the Samsung Galaxy J3/J5/J7 downstream kernels.
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/math.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/power_supply.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>

#define SM5703_FG_REG_DEVICE_ID		0x00
#define SM5703_FG_REG_CNTL		0x01
#define SM5703_FG_REG_SOC		0x05
#define SM5703_FG_REG_OCV		0x06
#define SM5703_FG_REG_VOLTAGE		0x07
#define SM5703_FG_REG_CURRENT		0x08
#define SM5703_FG_REG_TEMPERATURE	0x09
#define SM5703_FG_REG_FG_OP_STATUS	0x10
#define SM5703_FG_REG_TOPOFFSOC		0x12
#define SM5703_FG_REG_PARAM_CTRL	0x13
#define SM5703_FG_REG_PARAM_RUN_UPDATE	0x14
#define SM5703_FG_REG_VIT_PERIOD	0x1a
#define SM5703_FG_REG_MIX_RATE		0x1b
#define SM5703_FG_REG_MIX_INIT_BLANK	0x1c
#define SM5703_FG_REG_RCE0		0x20
#define SM5703_FG_REG_DTCD		0x23
#define SM5703_FG_REG_RS		0x24
#define SM5703_FG_REG_RS_MIX_FACTOR	0x25
#define SM5703_FG_REG_RS_MAX		0x26
#define SM5703_FG_REG_RS_MIN		0x27
#define SM5703_FG_REG_RS_MAN		0x29
#define SM5703_FG_REG_VOLT_CAL		0x2b
#define SM5703_FG_REG_CURR_CAL		0x2c
#define SM5703_FG_REG_IOCV_MAN		0x2e
#define SM5703_FG_REG_END_V_IDX		0x2f
#define SM5703_FG_REG_IOCV_B_L_MIN	0x30
#define SM5703_FG_REG_IOCV_B_L_MAX	0x37
#define SM5703_FG_REG_IOCV_B_S_MIN	0x40
#define SM5703_FG_REG_IOCV_B_S_MAX	0x43
#define SM5703_FG_REG_MISC		0x90
#define SM5703_FG_REG_TABLE_START	0xa0
#define SM5703_FG_MAX_REGISTER		0xcf

#define SM5703_FG_PARAM_UNLOCK_CODE	0x3700
#define SM5703_FG_PARAM_LOCK_CODE	0x0000
#define SM5703_FG_TABLE_LEN		16
#define SM5703_FG_NUM_TABLES		3
#define SM5703_FG_SW_RESET_CODE		0x0008

#define SM5703_FG_CNTL_MIX_MODE		BIT(15)
#define SM5703_FG_CNTL_TEMP_MEASURE	BIT(14)
#define SM5703_FG_CNTL_TOPOFF_SOC	BIT(13)
#define SM5703_FG_CNTL_MANUAL_OCV	BIT(10)
#define SM5703_FG_CNTL_RS_MAN_MODE	BIT(11)
#define SM5703_FG_CNTL_DEFAULT		0x2008

#define SM5703_FG_INIT_CHECK_MASK	0x00ff
#define SM5703_FG_INIT_DONE		0x0007

#define SM5703_FG_POLL_INTERVAL		msecs_to_jiffies(10000)

struct sm5703_fg_params {
	u32 table[SM5703_FG_NUM_TABLES][SM5703_FG_TABLE_LEN];
	u32 rce[3];
	u32 dtcd;
	u32 rs[4];	/* rs, mix factor, max, min */
	u32 vit_period;
	u32 mix[2];	/* mix rate, init blank */
	u32 topoff_soc[2];	/* enable, soc */
	u32 volt_cal;
	u32 curr_cal;
	u32 temp_std;
	u32 temp_offset;
	s32 temp_offset_cal;
	s32 charge_offset_cal;
	u32 temp_poff[4];	/* normal/low temperature voltage + offset */
	u32 ext_temp_cal[8];
	u32 min_charge_curr;
};

struct sm5703_fg {
	struct device *dev;
	struct regmap *regmap;
	struct power_supply *psy;
	struct power_supply *charger;
	struct power_supply_battery_info *info;
	struct delayed_work work;
	struct mutex lock;
	struct sm5703_fg_params p;

	bool initialized;
	int iocv_error_count;
	int prev_voltage;
	int voltage;
	int ocv;
	int current_now;
	int temperature;
	int soc;
};

static int sm5703_fg_read(struct sm5703_fg *fg, unsigned int reg)
{
	unsigned int val;
	int ret;

	ret = regmap_read(fg->regmap, reg, &val);
	return ret ? ret : (int)val;
}

/* Voltage/current/OCV registers: 3 integer bits and 8 fractional bits */
static int sm5703_fg_to_milli(unsigned int raw)
{
	return ((raw & 0x0700) >> 8) * 1000 + ((raw & 0x00ff) * 1000) / 256;
}

static int sm5703_fg_get_voltage(struct sm5703_fg *fg)
{
	int raw = sm5703_fg_read(fg, SM5703_FG_REG_VOLTAGE);

	return raw < 0 ? raw : sm5703_fg_to_milli(raw);
}

static int sm5703_fg_get_ocv(struct sm5703_fg *fg)
{
	int raw = sm5703_fg_read(fg, SM5703_FG_REG_OCV);

	return raw < 0 ? raw : sm5703_fg_to_milli(raw);
}

static int sm5703_fg_get_current(struct sm5703_fg *fg, int *ma)
{
	int raw = sm5703_fg_read(fg, SM5703_FG_REG_CURRENT);

	if (raw < 0)
		return raw;

	*ma = sm5703_fg_to_milli(raw);
	if (raw & 0x8000)
		*ma = -*ma;
	return 0;
}

/* Returns the temperature in 0.1 degrees Celsius */
static int sm5703_fg_get_temperature(struct sm5703_fg *fg, int *temp)
{
	int raw = sm5703_fg_read(fg, SM5703_FG_REG_TEMPERATURE);

	if (raw < 0)
		return raw;

	*temp = ((raw & 0x7f00) >> 8) * 10 + ((raw & 0x00ff) * 10) / 256;
	if (raw & 0x8000)
		*temp = -*temp;
	return 0;
}

/* Returns the state of charge in 0.1 percent */
static int sm5703_fg_get_soc(struct sm5703_fg *fg)
{
	int raw = sm5703_fg_read(fg, SM5703_FG_REG_SOC);

	if (raw < 0)
		return raw;

	return ((raw & 0xff00) >> 8) * 10 + ((raw & 0x00ff) * 10) / 256;
}

static bool sm5703_fg_charger_online(struct sm5703_fg *fg)
{
	union power_supply_propval val;

	if (!fg->charger)
		return false;
	if (power_supply_get_property(fg->charger, POWER_SUPPLY_PROP_ONLINE, &val))
		return false;

	return val.intval;
}

static int sm5703_fg_calculate_iocv(struct sm5703_fg *fg)
{
	int i, val, max = 0, min = 0, sum = 0, l_avg, s_avg = 0, l_minmax;

	for (i = SM5703_FG_REG_IOCV_B_L_MIN; i <= SM5703_FG_REG_IOCV_B_L_MAX; i++) {
		val = sm5703_fg_read(fg, i);
		if (val < 0)
			return val;
		if (i == SM5703_FG_REG_IOCV_B_L_MIN) {
			max = min = sum = val;
			continue;
		}
		max = max(max, val);
		min = min(min, val);
		sum += val;
	}

	sum -= max + min;
	l_minmax = max - min;
	l_avg = sum / (SM5703_FG_REG_IOCV_B_L_MAX - SM5703_FG_REG_IOCV_B_L_MIN - 1);

	val = sm5703_fg_read(fg, SM5703_FG_REG_END_V_IDX);
	if (val >= 0 && (val & 0x0030) == 0x0030) {
		for (i = SM5703_FG_REG_IOCV_B_S_MIN; i <= SM5703_FG_REG_IOCV_B_S_MAX; i++) {
			val = sm5703_fg_read(fg, i);
			if (val < 0)
				return val;
			if (i == SM5703_FG_REG_IOCV_B_S_MIN) {
				max = min = sum = val;
				continue;
			}
			max = max(max, val);
			min = min(min, val);
			sum += val;
		}
		sum -= max + min;
		s_avg = sum / (SM5703_FG_REG_IOCV_B_S_MAX - SM5703_FG_REG_IOCV_B_S_MIN - 1);
	}

	if ((abs(l_avg - s_avg) > 0x29 && l_minmax < 0x200) || s_avg == 0)
		return l_avg;

	return s_avg;
}

static int sm5703_fg_reg_init(struct sm5703_fg *fg, bool surge)
{
	struct regmap *regmap = fg->regmap;
	struct sm5703_fg_params *p = &fg->p;
	unsigned int val;
	int i, j, ret;

	/* Unlock the parameters */
	ret = regmap_write(regmap, SM5703_FG_REG_PARAM_CTRL,
			   SM5703_FG_PARAM_UNLOCK_CODE);
	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(p->rce); i++)
		regmap_write(regmap, SM5703_FG_REG_RCE0 + i, p->rce[i]);
	regmap_write(regmap, SM5703_FG_REG_DTCD, p->dtcd);
	regmap_write(regmap, SM5703_FG_REG_RS, p->rs[0]);
	regmap_write(regmap, SM5703_FG_REG_VIT_PERIOD, p->vit_period);

	/* Write the battery tables */
	regmap_write(regmap, SM5703_FG_REG_PARAM_CTRL,
		     SM5703_FG_PARAM_UNLOCK_CODE | (SM5703_FG_TABLE_LEN - 1));
	for (i = 0; i < SM5703_FG_NUM_TABLES; i++)
		for (j = 0; j < SM5703_FG_TABLE_LEN; j++)
			regmap_write(regmap,
				     SM5703_FG_REG_TABLE_START + (i << 4) + j,
				     p->table[i][j]);

	regmap_write(regmap, SM5703_FG_REG_RS_MIX_FACTOR, p->rs[1]);
	regmap_write(regmap, SM5703_FG_REG_RS_MAX, p->rs[2]);
	regmap_write(regmap, SM5703_FG_REG_RS_MIN, p->rs[3]);
	regmap_write(regmap, SM5703_FG_REG_MIX_RATE, p->mix[0]);
	regmap_write(regmap, SM5703_FG_REG_MIX_INIT_BLANK, p->mix[1]);
	regmap_write(regmap, SM5703_FG_REG_VOLT_CAL, p->volt_cal);
	regmap_write(regmap, SM5703_FG_REG_CURR_CAL, p->curr_cal);
	regmap_write(regmap, SM5703_FG_REG_TOPOFFSOC, p->topoff_soc[1]);

	/* Enable the gauge */
	ret = regmap_read(regmap, SM5703_FG_REG_CNTL, &val);
	if (ret)
		return ret;
	val &= ~SM5703_FG_CNTL_TOPOFF_SOC;
	val |= SM5703_FG_CNTL_MIX_MODE | SM5703_FG_CNTL_TEMP_MEASURE |
	       SM5703_FG_CNTL_MANUAL_OCV;
	if (p->topoff_soc[0])
		val |= SM5703_FG_CNTL_TOPOFF_SOC;
	ret = regmap_write(regmap, SM5703_FG_REG_CNTL, val);
	if (ret)
		return ret;

	/* Lock the parameters again */
	regmap_write(regmap, SM5703_FG_REG_PARAM_CTRL,
		     SM5703_FG_PARAM_LOCK_CODE | (SM5703_FG_TABLE_LEN - 1));

	if (surge)
		val = (fg->ocv << 8) / 125;
	else
		val = sm5703_fg_calculate_iocv(fg);

	return regmap_write(regmap, SM5703_FG_REG_IOCV_MAN, val);
}

static int sm5703_fg_hw_init(struct sm5703_fg *fg, bool surge)
{
	unsigned int val;
	int ret;

	fg->initialized = false;
	fg->iocv_error_count = 0;

	ret = regmap_read(fg->regmap, SM5703_FG_REG_DEVICE_ID, &val);
	if (ret)
		return ret;

	fg->ocv = sm5703_fg_get_ocv(fg);

	ret = regmap_read(fg->regmap, SM5703_FG_REG_CNTL, &val);
	if (ret)
		return ret;
	val &= ~SM5703_FG_CNTL_TOPOFF_SOC;
	if (fg->p.topoff_soc[0])
		val |= SM5703_FG_CNTL_TOPOFF_SOC;
	regmap_write(fg->regmap, SM5703_FG_REG_CNTL, val);

	ret = regmap_read(fg->regmap, SM5703_FG_REG_FG_OP_STATUS, &val);
	if (ret)
		return ret;
	if ((val & SM5703_FG_INIT_CHECK_MASK) != SM5703_FG_INIT_DONE) {
		ret = sm5703_fg_reg_init(fg, surge);
		if (ret)
			return ret;
	}

	fg->voltage = fg->prev_voltage = sm5703_fg_get_voltage(fg);
	sm5703_fg_get_temperature(fg, &fg->temperature);
	fg->ocv = sm5703_fg_get_ocv(fg);
	fg->initialized = true;

	return 0;
}

/* The gauge forgot its configuration (e.g. brown-out): reset and reload */
static void sm5703_fg_abnormal_reset_check(struct sm5703_fg *fg)
{
	int status, cntl;

	status = sm5703_fg_read(fg, SM5703_FG_REG_FG_OP_STATUS);
	if (status < 0 ||
	    (status & SM5703_FG_INIT_CHECK_MASK) == SM5703_FG_INIT_DONE)
		return;

	cntl = sm5703_fg_read(fg, SM5703_FG_REG_CNTL);
	if (cntl != SM5703_FG_CNTL_DEFAULT)
		return;

	dev_warn(fg->dev, "Fuel gauge was reset, reinitializing\n");
	regmap_write(fg->regmap, SM5703_FG_REG_MISC, SM5703_FG_SW_RESET_CODE);
	msleep(200);
	sm5703_fg_hw_init(fg, true);
}

static void sm5703_fg_set_rs_manual(struct sm5703_fg *fg, unsigned int rs,
				    bool manual)
{
	unsigned int cntl;

	if (manual) {
		regmap_write(fg->regmap, SM5703_FG_REG_RS_MAN, rs);
		regmap_write(fg->regmap, SM5703_FG_REG_PARAM_RUN_UPDATE, 0);
		regmap_write(fg->regmap, SM5703_FG_REG_PARAM_RUN_UPDATE, 1);
	}

	if (regmap_read(fg->regmap, SM5703_FG_REG_CNTL, &cntl))
		return;

	cntl |= SM5703_FG_CNTL_MIX_MODE;
	if (manual)
		cntl |= SM5703_FG_CNTL_RS_MAN_MODE;
	else
		cntl &= ~SM5703_FG_CNTL_RS_MAN_MODE;
	regmap_write(fg->regmap, SM5703_FG_REG_CNTL, cntl);
}

/* Handle IOCV errors and the low voltage power-off protection */
static void sm5703_fg_vbatocv_check(struct sm5703_fg *fg, bool ta_exist)
{
	struct sm5703_fg_params *p = &fg->p;
	u32 poff = fg->temperature / 10 > 15 ? p->temp_poff[0] : p->temp_poff[2];
	u32 poff_offset = fg->temperature / 10 > 15 ? p->temp_poff[1] : p->temp_poff[3];

	if (abs(fg->current_now) < 40 ||
	    (ta_exist && abs(fg->current_now) < (int)p->min_charge_curr)) {
		if (abs(fg->ocv - fg->voltage) > 30)
			fg->iocv_error_count++;
		fg->iocv_error_count = min(fg->iocv_error_count, 6);
	} else {
		fg->iocv_error_count = 0;
	}

	if (fg->iocv_error_count > 5) {
		if (abs(fg->prev_voltage - fg->voltage) > 15)
			fg->iocv_error_count = 0;
		else
			sm5703_fg_set_rs_manual(fg, p->rs[0], true);
	} else if (fg->prev_voltage < (int)poff && fg->voltage < (int)poff &&
		   !ta_exist) {
		if (fg->prev_voltage < (int)(poff - poff_offset) &&
		    fg->voltage < (int)(poff - poff_offset))
			sm5703_fg_set_rs_manual(fg, p->rs[0] >> 1, true);
		else
			sm5703_fg_set_rs_manual(fg, p->rs[0], true);
	} else {
		sm5703_fg_set_rs_manual(fg, 0, false);
	}

	fg->prev_voltage = fg->voltage;
}

static void sm5703_fg_update_curr_cal(struct sm5703_fg *fg, bool ta_exist)
{
	struct sm5703_fg_params *p = &fg->p;
	int temp = fg->temperature / 10;
	int curr_cal, temp_cal, gap;

	curr_cal = p->curr_cal;
	if (ta_exist)
		curr_cal += p->charge_offset_cal << 8;

	temp_cal = (int)(p->temp_std - temp) / (int)p->temp_offset;
	curr_cal += (temp_cal * p->temp_offset_cal) << 8;

	gap = temp - (int)p->temp_std;
	if (p->ext_temp_cal[0] && gap > 0 && p->ext_temp_cal[1]) {
		curr_cal += ((gap / (int)p->ext_temp_cal[1]) *
			     (int)p->ext_temp_cal[ta_exist ? 2 : 3]) << 8;
	} else if (p->ext_temp_cal[4] && gap < 0 && p->ext_temp_cal[5]) {
		curr_cal += (((-gap) / (int)p->ext_temp_cal[5]) *
			     (int)p->ext_temp_cal[ta_exist ? 6 : 7]) << 8;
	}

	regmap_write(fg->regmap, SM5703_FG_REG_CURR_CAL, curr_cal & 0xffff);
}

static void sm5703_fg_update(struct sm5703_fg *fg)
{
	bool ta_exist;
	int val;

	mutex_lock(&fg->lock);

	if (!fg->charger)
		fg->charger = power_supply_get_by_reference(dev_fwnode(fg->dev),
							    "power-supplies");
	if (IS_ERR_OR_NULL(fg->charger))
		fg->charger = NULL;

	val = sm5703_fg_get_voltage(fg);
	if (val >= 0)
		fg->voltage = val;
	sm5703_fg_get_current(fg, &fg->current_now);
	val = sm5703_fg_get_ocv(fg);
	if (val >= 0)
		fg->ocv = val;
	sm5703_fg_get_temperature(fg, &fg->temperature);

	ta_exist = sm5703_fg_charger_online(fg) && fg->current_now > 30;

	sm5703_fg_abnormal_reset_check(fg);
	sm5703_fg_vbatocv_check(fg, ta_exist);
	sm5703_fg_update_curr_cal(fg, ta_exist);

	val = sm5703_fg_get_soc(fg);
	if (val >= 0 && val != fg->soc) {
		fg->soc = val;
		power_supply_changed(fg->psy);
	}

	mutex_unlock(&fg->lock);
}

static void sm5703_fg_work(struct work_struct *work)
{
	struct sm5703_fg *fg = container_of(work, struct sm5703_fg, work.work);

	sm5703_fg_update(fg);
	schedule_delayed_work(&fg->work, SM5703_FG_POLL_INTERVAL);
}

static enum power_supply_property sm5703_fg_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_OCV,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
};

static int sm5703_fg_get_status(struct sm5703_fg *fg)
{
	union power_supply_propval val;

	if (!fg->charger ||
	    power_supply_get_property(fg->charger, POWER_SUPPLY_PROP_STATUS, &val))
		return POWER_SUPPLY_STATUS_UNKNOWN;

	return val.intval;
}

static int sm5703_fg_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	struct sm5703_fg *fg = power_supply_get_drvdata(psy);

	mutex_lock(&fg->lock);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = sm5703_fg_get_status(fg);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = fg->voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		val->intval = fg->ocv * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = fg->info->voltage_max_design_uv;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = fg->current_now * 1000;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = clamp(DIV_ROUND_CLOSEST(fg->soc, 10), 0, 100);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = fg->temperature;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = "SM5703";
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = "Silicon Mitus";
		break;
	default:
		mutex_unlock(&fg->lock);
		return -EINVAL;
	}
	mutex_unlock(&fg->lock);

	return 0;
}

static const struct power_supply_desc sm5703_fg_desc = {
	.name = "sm5703-fuel-gauge",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = sm5703_fg_props,
	.num_properties = ARRAY_SIZE(sm5703_fg_props),
	.get_property = sm5703_fg_get_property,
};

static const struct regmap_config sm5703_fg_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,
	.val_format_endian = REGMAP_ENDIAN_LITTLE,
	.max_register = SM5703_FG_MAX_REGISTER,
};

static int sm5703_fg_read_array(struct device *dev, const char *name,
				u32 *out, size_t len)
{
	int ret = device_property_read_u32_array(dev, name, out, len);

	if (ret)
		dev_err_probe(dev, ret, "Failed to read %s\n", name);
	return ret;
}

static int sm5703_fg_parse_dt(struct sm5703_fg *fg)
{
	struct device *dev = fg->dev;
	struct sm5703_fg_params *p = &fg->p;
	static const char * const tables[] = {
		"siliconmitus,discharge-table",
		"siliconmitus,charge-table",
		"siliconmitus,q-table",
	};
	u32 val;
	int i, ret;

	/* Defaults for optional properties */
	p->temp_poff[0] = 3350;
	p->temp_poff[1] = 70;
	p->temp_poff[2] = 3300;
	p->temp_poff[3] = 70;
	p->min_charge_curr = 100;
	p->temp_std = 25;
	p->temp_offset = 10;

	for (i = 0; i < SM5703_FG_NUM_TABLES; i++) {
		ret = sm5703_fg_read_array(dev, tables[i], p->table[i],
					   SM5703_FG_TABLE_LEN);
		if (ret)
			return ret;
	}

	ret = sm5703_fg_read_array(dev, "siliconmitus,rce", p->rce, 3);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,rs", p->rs, 4);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,mix", p->mix, 2);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,topoff-soc", p->topoff_soc, 2);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,dtcd", &p->dtcd, 1);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,vit-period", &p->vit_period, 1);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,volt-cal", &p->volt_cal, 1);
	if (ret)
		return ret;
	ret = sm5703_fg_read_array(dev, "siliconmitus,curr-cal", &p->curr_cal, 1);
	if (ret)
		return ret;

	device_property_read_u32(dev, "siliconmitus,temp-std", &p->temp_std);
	device_property_read_u32(dev, "siliconmitus,temp-offset", &p->temp_offset);
	device_property_read_u32(dev, "siliconmitus,min-charge-current-milliamp",
				 &p->min_charge_curr);
	device_property_read_u32_array(dev, "siliconmitus,temp-poff",
				       p->temp_poff, 4);
	device_property_read_u32_array(dev, "siliconmitus,ext-temp-cal",
				       p->ext_temp_cal, 8);
	if (!device_property_read_u32(dev, "siliconmitus,temp-offset-cal", &val))
		p->temp_offset_cal = val;
	if (!device_property_read_u32(dev, "siliconmitus,charge-offset-cal", &val))
		p->charge_offset_cal = val;

	if (!p->temp_offset)
		return dev_err_probe(dev, -EINVAL, "Invalid temp-offset\n");

	return 0;
}

static void sm5703_fg_put_charger(void *data)
{
	struct sm5703_fg *fg = data;

	if (fg->charger)
		power_supply_put(fg->charger);
}

static int sm5703_fg_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct power_supply_config psy_cfg = {};
	struct sm5703_fg *fg;
	int ret;

	fg = devm_kzalloc(dev, sizeof(*fg), GFP_KERNEL);
	if (!fg)
		return -ENOMEM;

	fg->dev = dev;
	mutex_init(&fg->lock);
	i2c_set_clientdata(client, fg);

	fg->regmap = devm_regmap_init_i2c(client, &sm5703_fg_regmap_config);
	if (IS_ERR(fg->regmap))
		return dev_err_probe(dev, PTR_ERR(fg->regmap),
				     "Failed to initialize regmap\n");

	ret = sm5703_fg_parse_dt(fg);
	if (ret)
		return ret;

	psy_cfg.fwnode = dev_fwnode(dev);
	psy_cfg.drv_data = fg;

	fg->psy = devm_power_supply_register(dev, &sm5703_fg_desc, &psy_cfg);
	if (IS_ERR(fg->psy))
		return dev_err_probe(dev, PTR_ERR(fg->psy),
				     "Failed to register power supply\n");

	ret = power_supply_get_battery_info(fg->psy, &fg->info);
	if (ret)
		return dev_err_probe(dev, ret, "Missing battery info\n");

	ret = devm_add_action_or_reset(dev, sm5703_fg_put_charger, fg);
	if (ret)
		return ret;

	ret = sm5703_fg_hw_init(fg, false);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to initialize gauge\n");

	INIT_DELAYED_WORK(&fg->work, sm5703_fg_work);
	sm5703_fg_update(fg);
	schedule_delayed_work(&fg->work, SM5703_FG_POLL_INTERVAL);

	return 0;
}

static void sm5703_fg_remove(struct i2c_client *client)
{
	struct sm5703_fg *fg = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&fg->work);
}

static const struct of_device_id sm5703_fg_of_match[] = {
	{ .compatible = "siliconmitus,sm5703-fuel-gauge" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sm5703_fg_of_match);

static struct i2c_driver sm5703_fg_driver = {
	.driver = {
		.name = "sm5703-fuel-gauge",
		.of_match_table = sm5703_fg_of_match,
	},
	.probe = sm5703_fg_probe,
	.remove = sm5703_fg_remove,
};
module_i2c_driver(sm5703_fg_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 fuel gauge driver");
MODULE_LICENSE("GPL");
