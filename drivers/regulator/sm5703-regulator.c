// SPDX-License-Identifier: GPL-2.0-only
/*
 * Regulator driver for the Silicon Mitus SM5703 LDOs, buck converter,
 * USB LDOs and USB OTG VBUS supply.
 */

#include <linux/mfd/sm5703.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>

enum sm5703_regulators {
	SM5703_BUCK,
	SM5703_LDO1,
	SM5703_LDO2,
	SM5703_LDO3,
	SM5703_USBLDO1,
	SM5703_USBLDO2,
	SM5703_VBUS,
	SM5703_MAX_REGULATORS,
};

#define SM5703_USBLDO_MICROVOLT		4800000
#define SM5703_VBUS_MICROVOLT		5000000

static const unsigned int sm5703_ldo_voltagemap[] = {
	1500000, 1800000, 2600000, 2800000, 3000000, 3300000, 3300000, 3300000,
};

static const unsigned int sm5703_buck_voltagemap[] = {
	1000000, 1000000, 1000000, 1000000,
	1000000, 1000000, 1000000, 1000000,
	1000000, 1000000, 1000000, 1100000,
	1200000, 1300000, 1400000, 1500000,
	1600000, 1700000, 1800000, 1900000,
	2000000, 2100000, 2200000, 2300000,
	2400000, 2500000, 2600000, 2700000,
	2800000, 2900000, 3000000, 3000000,
};

static int sm5703_vbus_enable(struct regulator_dev *rdev)
{
	struct regmap *regmap = rdev_get_regmap(rdev);
	int ret;

	ret = regmap_update_bits(regmap, SM5703_REG_FLEDCNTL6,
				 SM5703_FLEDCNTL6_BSTOUT, SM5703_BSTOUT_5P0);
	if (ret)
		return ret;

	return regmap_update_bits(regmap, SM5703_REG_CNTL,
				  SM5703_CNTL_OPERATION_MODE,
				  SM5703_CNTL_MODE_USB_OTG);
}

static int sm5703_vbus_disable(struct regulator_dev *rdev)
{
	struct regmap *regmap = rdev_get_regmap(rdev);
	int ret;

	ret = regmap_update_bits(regmap, SM5703_REG_CNTL,
				 SM5703_CNTL_OPERATION_MODE,
				 SM5703_CNTL_MODE_CHARGING_ON);
	if (ret)
		return ret;

	return regmap_update_bits(regmap, SM5703_REG_FLEDCNTL6,
				  SM5703_FLEDCNTL6_BSTOUT, SM5703_BSTOUT_4P5);
}

static int sm5703_vbus_is_enabled(struct regulator_dev *rdev)
{
	unsigned int val;
	int ret;

	ret = regmap_read(rdev_get_regmap(rdev), SM5703_REG_CNTL, &val);
	if (ret)
		return ret;

	return (val & SM5703_CNTL_OPERATION_MODE) == SM5703_CNTL_MODE_USB_OTG;
}

static const struct regulator_ops sm5703_regulator_ops = {
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
	.list_voltage		= regulator_list_voltage_table,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
};

static const struct regulator_ops sm5703_regulator_ops_fixed = {
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};

static const struct regulator_ops sm5703_vbus_ops = {
	.enable			= sm5703_vbus_enable,
	.disable		= sm5703_vbus_disable,
	.is_enabled		= sm5703_vbus_is_enabled,
};

#define SM5703_USBLDO(_name, _id)					\
	[SM5703_USBLDO ## _id] = {					\
		.name = _name,						\
		.of_match = _name,					\
		.regulators_node = "regulators",			\
		.type = REGULATOR_VOLTAGE,				\
		.id = SM5703_USBLDO ## _id,				\
		.ops = &sm5703_regulator_ops_fixed,			\
		.n_voltages = 1,					\
		.fixed_uV = SM5703_USBLDO_MICROVOLT,			\
		.enable_reg = SM5703_REG_CNTL,				\
		.enable_mask = SM5703_CNTL_EN_USBLDO ## _id,		\
		.owner = THIS_MODULE,					\
	}

#define SM5703_VBUS(_name)						\
	[SM5703_VBUS] = {						\
		.name = _name,						\
		.of_match = _name,					\
		.regulators_node = "regulators",			\
		.type = REGULATOR_VOLTAGE,				\
		.id = SM5703_VBUS,					\
		.ops = &sm5703_vbus_ops,				\
		.n_voltages = 1,					\
		.fixed_uV = SM5703_VBUS_MICROVOLT,			\
		.owner = THIS_MODULE,					\
	}

#define SM5703_BUCK_DESC(_name)						\
	[SM5703_BUCK] = {						\
		.name = _name,						\
		.of_match = _name,					\
		.regulators_node = "regulators",			\
		.type = REGULATOR_VOLTAGE,				\
		.id = SM5703_BUCK,					\
		.ops = &sm5703_regulator_ops,				\
		.n_voltages = ARRAY_SIZE(sm5703_buck_voltagemap),	\
		.volt_table = sm5703_buck_voltagemap,			\
		.vsel_reg = SM5703_REG_BUCK,				\
		.vsel_mask = SM5703_BUCK_VOLT_MASK,			\
		.enable_reg = SM5703_REG_BUCK,				\
		.enable_mask = SM5703_BUCK_EN,				\
		.owner = THIS_MODULE,					\
	}

#define SM5703_LDO_DESC(_name, _id)					\
	[SM5703_LDO ## _id] = {						\
		.name = _name,						\
		.of_match = _name,					\
		.regulators_node = "regulators",			\
		.type = REGULATOR_VOLTAGE,				\
		.id = SM5703_LDO ## _id,				\
		.ops = &sm5703_regulator_ops,				\
		.n_voltages = ARRAY_SIZE(sm5703_ldo_voltagemap),	\
		.volt_table = sm5703_ldo_voltagemap,			\
		.vsel_reg = SM5703_REG_LDO ## _id,			\
		.vsel_mask = SM5703_LDO_VOLT_MASK,			\
		.enable_reg = SM5703_REG_LDO ## _id,			\
		.enable_mask = SM5703_LDO_EN,				\
		.owner = THIS_MODULE,					\
	}

static const struct regulator_desc sm5703_regulators_desc[SM5703_MAX_REGULATORS] = {
	SM5703_BUCK_DESC("buck"),
	SM5703_LDO_DESC("ldo1", 1),
	SM5703_LDO_DESC("ldo2", 2),
	SM5703_LDO_DESC("ldo3", 3),
	SM5703_USBLDO("usbldo1", 1),
	SM5703_USBLDO("usbldo2", 2),
	SM5703_VBUS("vbus"),
};

static int sm5703_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct regulator_config config = { };
	struct regulator_dev *rdev;
	int i;

	config.dev = dev->parent;
	config.regmap = dev_get_regmap(dev->parent, NULL);
	if (!config.regmap)
		return dev_err_probe(dev, -ENODEV, "Failed to get regmap\n");

	for (i = 0; i < SM5703_MAX_REGULATORS; i++) {
		rdev = devm_regulator_register(dev, &sm5703_regulators_desc[i],
					       &config);
		if (IS_ERR(rdev))
			return dev_err_probe(dev, PTR_ERR(rdev),
					     "Failed to register %s\n",
					     sm5703_regulators_desc[i].name);
	}

	return 0;
}

static const struct platform_device_id sm5703_regulator_id[] = {
	{ "sm5703-regulator", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(platform, sm5703_regulator_id);

static struct platform_driver sm5703_regulator_driver = {
	.driver = {
		.name = "sm5703-regulator",
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.probe = sm5703_regulator_probe,
	.id_table = sm5703_regulator_id,
};
module_platform_driver(sm5703_regulator_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 LDO/Buck/USB regulator driver");
MODULE_LICENSE("GPL");
