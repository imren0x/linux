// SPDX-License-Identifier: GPL-2.0-only
/*
 * Battery charger driver for the Silicon Mitus SM5703
 *
 * The register programming sequence is based on the vendor driver in the
 * Samsung Galaxy J3/J5/J7 downstream kernels.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/devm-helpers.h>
#include <linux/extcon.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/mfd/sm5703.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/property.h>
#include <linux/regmap.h>

#define SM5703_FASTCHG_MIN_UA		100000
#define SM5703_FASTCHG_MAX_UA		2500000
#define SM5703_FASTCHG_STEP_UA		50000

#define SM5703_VBUSLIMIT_MIN_UA		100000
#define SM5703_VBUSLIMIT_MAX_UA		2100000
#define SM5703_VBUSLIMIT_STEP_UA	50000

#define SM5703_BATREG_MIN_UV		4120000
#define SM5703_BATREG_MAX_UV		4430000
#define SM5703_BATREG_STEP_UV		10000

#define SM5703_TOPOFF_MIN_UA		100000
#define SM5703_TOPOFF_MAX_UA		475000
#define SM5703_TOPOFF_STEP_UA		25000

#define SM5703_TOPOFF_TIMER_45M		0x3
#define SM5703_FREQSEL_1P5MHZ		0x2
#define SM5703_AICLTH_4500MV		0x2
#define SM5703_OTGCURRENT_1P2A		0x3

#define SM5703_DEFAULT_USB_UA		500000
#define SM5703_DEFAULT_DCP_UA		1000000

struct sm5703_charger {
	struct device *dev;
	struct regmap *regmap;
	struct power_supply *psy;
	struct gpio_desc *chgen;
	struct extcon_dev *edev;
	struct notifier_block extcon_nb;
	struct work_struct extcon_work;
	struct mutex lock;

	u32 fast_charge_ua;
	u32 float_uv;
	u32 term_ua;
	u32 sdp_ua;
	u32 dcp_ua;
};

static int sm5703_charger_set_input_limit(struct sm5703_charger *chg, u32 ua)
{
	ua = clamp(ua, SM5703_VBUSLIMIT_MIN_UA + SM5703_VBUSLIMIT_STEP_UA,
		   SM5703_VBUSLIMIT_MAX_UA);

	return regmap_update_bits(chg->regmap, SM5703_REG_VBUSCNTL,
				  SM5703_VBUSCNTL_VBUSLIMIT,
				  (ua - SM5703_VBUSLIMIT_MIN_UA) /
				  SM5703_VBUSLIMIT_STEP_UA);
}

static int sm5703_charger_get_input_limit(struct sm5703_charger *chg, int *ua)
{
	unsigned int val;
	int ret;

	ret = regmap_read(chg->regmap, SM5703_REG_VBUSCNTL, &val);
	if (ret)
		return ret;

	*ua = SM5703_VBUSLIMIT_MIN_UA +
	      (val & SM5703_VBUSCNTL_VBUSLIMIT) * SM5703_VBUSLIMIT_STEP_UA;
	return 0;
}

static int sm5703_charger_set_fast_current(struct sm5703_charger *chg, u32 ua)
{
	ua = clamp(ua, SM5703_FASTCHG_MIN_UA, SM5703_FASTCHG_MAX_UA);

	return regmap_update_bits(chg->regmap, SM5703_REG_CHGCNTL2,
				  SM5703_CHGCNTL2_FASTCHG,
				  (ua - SM5703_FASTCHG_MIN_UA) /
				  SM5703_FASTCHG_STEP_UA);
}

static int sm5703_charger_get_fast_current(struct sm5703_charger *chg, int *ua)
{
	unsigned int val;
	int ret;

	ret = regmap_read(chg->regmap, SM5703_REG_CHGCNTL2, &val);
	if (ret)
		return ret;

	*ua = SM5703_FASTCHG_MIN_UA +
	      (val & SM5703_CHGCNTL2_FASTCHG) * SM5703_FASTCHG_STEP_UA;
	return 0;
}

static int sm5703_charger_set_float_voltage(struct sm5703_charger *chg, u32 uv)
{
	uv = clamp(uv, SM5703_BATREG_MIN_UV, SM5703_BATREG_MAX_UV);

	return regmap_update_bits(chg->regmap, SM5703_REG_CHGCNTL3,
				  SM5703_CHGCNTL3_BATREG,
				  (uv - SM5703_BATREG_MIN_UV) /
				  SM5703_BATREG_STEP_UV);
}

static int sm5703_charger_get_float_voltage(struct sm5703_charger *chg, int *uv)
{
	unsigned int val;
	int ret;

	ret = regmap_read(chg->regmap, SM5703_REG_CHGCNTL3, &val);
	if (ret)
		return ret;

	*uv = SM5703_BATREG_MIN_UV +
	      (val & SM5703_CHGCNTL3_BATREG) * SM5703_BATREG_STEP_UV;
	return 0;
}

static int sm5703_charger_set_term_current(struct sm5703_charger *chg, u32 ua)
{
	ua = clamp(ua, SM5703_TOPOFF_MIN_UA, SM5703_TOPOFF_MAX_UA);

	return regmap_update_bits(chg->regmap, SM5703_REG_CHGCNTL4,
				  SM5703_CHGCNTL4_TOPOFF,
				  FIELD_PREP(SM5703_CHGCNTL4_TOPOFF,
					     (ua - SM5703_TOPOFF_MIN_UA) /
					     SM5703_TOPOFF_STEP_UA));
}

static int sm5703_charger_set_enabled(struct sm5703_charger *chg, bool enable)
{
	int ret;

	if (chg->chgen)
		gpiod_set_value_cansleep(chg->chgen, enable);

	ret = regmap_update_bits(chg->regmap, SM5703_REG_CNTL,
				 SM5703_CNTL_OPERATION_MODE,
				 enable ? SM5703_CNTL_MODE_CHARGING_ON :
					  SM5703_CNTL_MODE_CHARGING_OFF);
	return ret;
}

static int sm5703_charger_hw_init(struct sm5703_charger *chg)
{
	struct regmap *regmap = chg->regmap;
	int ret;

	/* Enable autoset, auto input current limiting and set the thresholds */
	ret = regmap_update_bits(regmap, SM5703_REG_CNTL, SM5703_CNTL_AUTOSET,
				 SM5703_CNTL_AUTOSET);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, SM5703_REG_CHGCNTL5,
				 SM5703_CHGCNTL5_AICLEN | SM5703_CHGCNTL5_AICLTH |
				 SM5703_CHGCNTL5_TOPOFF_TIMER,
				 SM5703_CHGCNTL5_AICLEN |
				 FIELD_PREP(SM5703_CHGCNTL5_AICLTH,
					    SM5703_AICLTH_4500MV) |
				 FIELD_PREP(SM5703_CHGCNTL5_TOPOFF_TIMER,
					    SM5703_TOPOFF_TIMER_45M));
	if (ret)
		return ret;

	/* Do not stop the charger automatically at the end of charge */
	ret = regmap_update_bits(regmap, SM5703_REG_CHGCNTL4,
				 SM5703_CHGCNTL4_AUTOSTOP, 0);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, SM5703_REG_CHGCNTL6,
				 SM5703_CHGCNTL6_FREQSEL,
				 FIELD_PREP(SM5703_CHGCNTL6_FREQSEL,
					    SM5703_FREQSEL_1P5MHZ));
	if (ret)
		return ret;

	/* The regulation voltage must be correct before the charger is on */
	ret = sm5703_charger_set_float_voltage(chg, chg->float_uv);
	if (ret)
		return ret;

	ret = sm5703_charger_set_fast_current(chg, chg->fast_charge_ua);
	if (ret)
		return ret;

	ret = sm5703_charger_set_term_current(chg, chg->term_ua);
	if (ret)
		return ret;

	ret = sm5703_charger_set_input_limit(chg, chg->sdp_ua);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, SM5703_REG_OTGCURRENTCNTL,
				 SM5703_OTGCURRENT, SM5703_OTGCURRENT_1P2A);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, SM5703_REG_Q3LIMITCNTL,
				 SM5703_Q3LIMIT_1X, SM5703_Q3LIMIT_1X);
	if (ret)
		return ret;

	return sm5703_charger_set_enabled(chg, true);
}

static int sm5703_charger_get_status(struct sm5703_charger *chg)
{
	unsigned int status3, status5;
	int ret;

	ret = regmap_read(chg->regmap, SM5703_REG_STATUS5, &status5);
	if (ret)
		return POWER_SUPPLY_STATUS_UNKNOWN;
	ret = regmap_read(chg->regmap, SM5703_REG_STATUS3, &status3);
	if (ret)
		return POWER_SUPPLY_STATUS_UNKNOWN;

	if (!(status5 & SM5703_STATUS5_VBUSOK))
		return POWER_SUPPLY_STATUS_DISCHARGING;
	if (status3 & (SM5703_STATUS3_DONE | SM5703_STATUS3_TOPOFF))
		return POWER_SUPPLY_STATUS_FULL;
	if (status3 & SM5703_STATUS3_CHGON)
		return POWER_SUPPLY_STATUS_CHARGING;

	return POWER_SUPPLY_STATUS_NOT_CHARGING;
}

static int sm5703_charger_get_health(struct sm5703_charger *chg)
{
	unsigned int status5;

	if (regmap_read(chg->regmap, SM5703_REG_STATUS5, &status5))
		return POWER_SUPPLY_HEALTH_UNKNOWN;

	if (status5 & SM5703_STATUS5_VBUSOK)
		return POWER_SUPPLY_HEALTH_GOOD;
	if (status5 & SM5703_STATUS5_VBUSOVP)
		return POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	if (status5 & SM5703_STATUS5_VBUSUVLO)
		return POWER_SUPPLY_HEALTH_UNDERVOLTAGE;

	return POWER_SUPPLY_HEALTH_UNKNOWN;
}

static enum power_supply_property sm5703_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
};

static int sm5703_charger_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct sm5703_charger *chg = power_supply_get_drvdata(psy);
	unsigned int status5;
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = sm5703_charger_get_status(chg);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = sm5703_charger_get_health(chg);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		ret = regmap_read(chg->regmap, SM5703_REG_STATUS5, &status5);
		val->intval = !ret && (status5 & SM5703_STATUS5_VBUSOK);
		ret = 0;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		if (sm5703_charger_get_status(chg) == POWER_SUPPLY_STATUS_CHARGING)
			val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
		else
			val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
		ret = sm5703_charger_get_fast_current(chg, &val->intval);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
		ret = sm5703_charger_get_float_voltage(chg, &val->intval);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		ret = sm5703_charger_get_input_limit(chg, &val->intval);
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = "SM5703";
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = "Silicon Mitus";
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

static int sm5703_charger_set_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       const union power_supply_propval *val)
{
	struct sm5703_charger *chg = power_supply_get_drvdata(psy);
	int ret;

	mutex_lock(&chg->lock);
	switch (psp) {
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
		ret = sm5703_charger_set_fast_current(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		ret = sm5703_charger_set_input_limit(chg, val->intval);
		break;
	default:
		ret = -EINVAL;
	}
	mutex_unlock(&chg->lock);

	return ret;
}

static int sm5703_charger_property_is_writeable(struct power_supply *psy,
						enum power_supply_property psp)
{
	return psp == POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT ||
	       psp == POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT;
}

static const struct power_supply_desc sm5703_charger_desc = {
	.name = "sm5703-charger",
	.type = POWER_SUPPLY_TYPE_USB,
	.properties = sm5703_charger_props,
	.num_properties = ARRAY_SIZE(sm5703_charger_props),
	.get_property = sm5703_charger_get_property,
	.set_property = sm5703_charger_set_property,
	.property_is_writeable = sm5703_charger_property_is_writeable,
};

static void sm5703_charger_extcon_work(struct work_struct *work)
{
	struct sm5703_charger *chg =
		container_of(work, struct sm5703_charger, extcon_work);
	u32 ua = chg->sdp_ua;
	int ret;

	if (extcon_get_state(chg->edev, EXTCON_USB_HOST) > 0) {
		/* OTG: the VBUS regulator takes over, charging stays off */
		power_supply_changed(chg->psy);
		return;
	}

	if (extcon_get_state(chg->edev, EXTCON_CHG_USB_DCP) > 0 ||
	    extcon_get_state(chg->edev, EXTCON_CHG_USB_FAST) > 0 ||
	    extcon_get_state(chg->edev, EXTCON_CHG_USB_SLOW) > 0 ||
	    extcon_get_state(chg->edev, EXTCON_CHG_USB_CDP) > 0 ||
	    extcon_get_state(chg->edev, EXTCON_CHG_USB_ACA) > 0)
		ua = chg->dcp_ua;

	/* Give the chip some time to settle after the cable event */
	msleep(100);

	mutex_lock(&chg->lock);
	ret = sm5703_charger_set_input_limit(chg, ua);
	if (!ret)
		ret = sm5703_charger_set_enabled(chg, true);
	mutex_unlock(&chg->lock);
	if (ret)
		dev_err(chg->dev, "Failed to update input limit: %d\n", ret);

	power_supply_changed(chg->psy);
}

static int sm5703_charger_extcon_notifier(struct notifier_block *nb,
					  unsigned long event, void *param)
{
	struct sm5703_charger *chg =
		container_of(nb, struct sm5703_charger, extcon_nb);

	schedule_work(&chg->extcon_work);

	return NOTIFY_OK;
}

static irqreturn_t sm5703_charger_irq(int irq, void *data)
{
	struct sm5703_charger *chg = data;

	power_supply_changed(chg->psy);

	return IRQ_HANDLED;
}

static int sm5703_charger_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct power_supply_battery_info *info;
	struct power_supply_config psy_cfg = {};
	struct sm5703_charger *chg;
	int i, irq, ret;

	chg = devm_kzalloc(dev, sizeof(*chg), GFP_KERNEL);
	if (!chg)
		return -ENOMEM;

	chg->dev = dev;
	mutex_init(&chg->lock);
	platform_set_drvdata(pdev, chg);

	chg->regmap = dev_get_regmap(dev->parent, NULL);
	if (!chg->regmap)
		return dev_err_probe(dev, -ENODEV, "Failed to get regmap\n");

	chg->chgen = devm_gpiod_get_optional(dev, "enable", GPIOD_OUT_LOW);
	if (IS_ERR(chg->chgen))
		return dev_err_probe(dev, PTR_ERR(chg->chgen),
				     "Failed to get enable GPIO\n");

	chg->sdp_ua = SM5703_DEFAULT_USB_UA;
	chg->dcp_ua = SM5703_DEFAULT_DCP_UA;
	device_property_read_u32(dev, "siliconmitus,usb-input-current-microamp",
				 &chg->sdp_ua);
	device_property_read_u32(dev, "siliconmitus,dcp-input-current-microamp",
				 &chg->dcp_ua);

	psy_cfg.fwnode = dev_fwnode(dev);
	psy_cfg.drv_data = chg;

	chg->psy = devm_power_supply_register(dev, &sm5703_charger_desc,
					      &psy_cfg);
	if (IS_ERR(chg->psy))
		return dev_err_probe(dev, PTR_ERR(chg->psy),
				     "Failed to register power supply\n");

	ret = power_supply_get_battery_info(chg->psy, &info);
	if (ret)
		return dev_err_probe(dev, ret, "Missing battery info\n");

	chg->fast_charge_ua = info->constant_charge_current_max_ua;
	chg->float_uv = info->constant_charge_voltage_max_uv;
	chg->term_ua = info->charge_term_current_ua;
	power_supply_put_battery_info(chg->psy, info);

	if (chg->fast_charge_ua <= 0 || chg->float_uv <= 0)
		return dev_err_probe(dev, -EINVAL,
				     "Incomplete battery information\n");
	if (chg->term_ua <= 0)
		chg->term_ua = SM5703_TOPOFF_MIN_UA;

	ret = sm5703_charger_hw_init(chg);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to initialize charger\n");

	for (i = 0; i < pdev->num_resources; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0)
			continue;

		ret = devm_request_threaded_irq(dev, irq, NULL,
						sm5703_charger_irq,
						IRQF_ONESHOT, "sm5703-charger",
						chg);
		if (ret)
			dev_warn(dev, "Failed to request IRQ %d: %d\n", irq, ret);
	}

	/* Extcon is only needed to select the right input current limit */
	chg->edev = extcon_get_edev_by_phandle(dev, 0);
	if (IS_ERR(chg->edev)) {
		if (PTR_ERR(chg->edev) == -EPROBE_DEFER)
			return -EPROBE_DEFER;

		dev_warn(dev, "No extcon device found, using USB input limit\n");
		return 0;
	}

	ret = devm_work_autocancel(dev, &chg->extcon_work,
				   sm5703_charger_extcon_work);
	if (ret)
		return ret;

	chg->extcon_nb.notifier_call = sm5703_charger_extcon_notifier;
	ret = devm_extcon_register_notifier_all(dev, chg->edev, &chg->extcon_nb);
	if (ret)
		return dev_err_probe(dev, ret,
				     "Failed to register extcon notifier\n");

	/* Handle a cable that is already plugged in */
	schedule_work(&chg->extcon_work);

	return 0;
}

static const struct platform_device_id sm5703_charger_id[] = {
	{ "sm5703-charger", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(platform, sm5703_charger_id);

static const struct of_device_id sm5703_charger_of_match[] = {
	{ .compatible = "siliconmitus,sm5703-charger", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sm5703_charger_of_match);

static struct platform_driver sm5703_charger_driver = {
	.driver = {
		.name = "sm5703-charger",
		.of_match_table = sm5703_charger_of_match,
	},
	.probe = sm5703_charger_probe,
	.id_table = sm5703_charger_id,
};
module_platform_driver(sm5703_charger_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 charger driver");
MODULE_LICENSE("GPL");
