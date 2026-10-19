// SPDX-License-Identifier: GPL-2.0-only
/*
 * MFD core driver for the Silicon Mitus SM5703.
 *
 * The SM5703 comprises a switching charger, two USB LDOs, three LDOs,
 * a buck converter and a flash LED driver.
 */

#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mfd/core.h>
#include <linux/mfd/sm5703.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regmap.h>

#define SM5703_REGMAP_IRQ(_irq, _reg, _bit) \
	[_irq] = { .reg_offset = (_reg), .mask = BIT(_bit) }

static const struct regmap_irq sm5703_irqs[] = {
	SM5703_REGMAP_IRQ(SM5703_IRQ_AICL, 0, 0),
	SM5703_REGMAP_IRQ(SM5703_IRQ_BATOVP, 0, 1),
	SM5703_REGMAP_IRQ(SM5703_IRQ_LOWBATT, 0, 2),
	SM5703_REGMAP_IRQ(SM5703_IRQ_VBUSLIMIT, 0, 3),
	SM5703_REGMAP_IRQ(SM5703_IRQ_DISLIMIT, 0, 4),
	SM5703_REGMAP_IRQ(SM5703_IRQ_VSYSOLP, 0, 5),
	SM5703_REGMAP_IRQ(SM5703_IRQ_OTGFAIL, 0, 6),
	SM5703_REGMAP_IRQ(SM5703_IRQ_THEMREG, 1, 0),
	SM5703_REGMAP_IRQ(SM5703_IRQ_THEMSHDN, 1, 1),
	SM5703_REGMAP_IRQ(SM5703_IRQ_VSYSNG, 1, 2),
	SM5703_REGMAP_IRQ(SM5703_IRQ_VSYSOK, 1, 3),
	SM5703_REGMAP_IRQ(SM5703_IRQ_NOBAT, 1, 4),
	SM5703_REGMAP_IRQ(SM5703_IRQ_PRETMROFF, 1, 5),
	SM5703_REGMAP_IRQ(SM5703_IRQ_FASTTMROFF, 1, 6),
	SM5703_REGMAP_IRQ(SM5703_IRQ_CHGON, 2, 0),
	SM5703_REGMAP_IRQ(SM5703_IRQ_Q4FULLON, 2, 1),
	SM5703_REGMAP_IRQ(SM5703_IRQ_TOPOFF, 2, 2),
	SM5703_REGMAP_IRQ(SM5703_IRQ_DONE, 2, 3),
	SM5703_REGMAP_IRQ(SM5703_IRQ_CHGRSTF, 2, 4),
	SM5703_REGMAP_IRQ(SM5703_IRQ_FLEDSHORT, 3, 0),
	SM5703_REGMAP_IRQ(SM5703_IRQ_FLEDOPEN, 3, 1),
	SM5703_REGMAP_IRQ(SM5703_IRQ_BOOSTPOK_NG, 3, 2),
	SM5703_REGMAP_IRQ(SM5703_IRQ_BOOSTPOK, 3, 3),
	SM5703_REGMAP_IRQ(SM5703_IRQ_ABSTMRON, 3, 4),
};

static const struct regmap_irq_chip sm5703_irq_chip = {
	.name = "sm5703",
	.status_base = SM5703_REG_INT1,
	.mask_base = SM5703_REG_INTMSK1,
	.num_regs = 4,
	.irqs = sm5703_irqs,
	.num_irqs = ARRAY_SIZE(sm5703_irqs),
};

static const struct regmap_config sm5703_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = SM5703_MAX_REGISTER,
};

static const struct resource sm5703_charger_resources[] = {
	DEFINE_RES_IRQ_NAMED(SM5703_IRQ_AICL, "aicl"),
	DEFINE_RES_IRQ_NAMED(SM5703_IRQ_NOBAT, "nobat"),
	DEFINE_RES_IRQ_NAMED(SM5703_IRQ_CHGON, "chgon"),
	DEFINE_RES_IRQ_NAMED(SM5703_IRQ_TOPOFF, "topoff"),
	DEFINE_RES_IRQ_NAMED(SM5703_IRQ_DONE, "done"),
	DEFINE_RES_IRQ_NAMED(SM5703_IRQ_OTGFAIL, "otgfail"),
};

static const struct mfd_cell sm5703_devs[] = {
	{ .name = "sm5703-regulator", },
	{
		.name = "sm5703-charger",
		.of_compatible = "siliconmitus,sm5703-charger",
		.resources = sm5703_charger_resources,
		.num_resources = ARRAY_SIZE(sm5703_charger_resources),
	},
};

static int sm5703_probe(struct i2c_client *i2c)
{
	struct device *dev = &i2c->dev;
	struct regmap_irq_chip_data *irq_data;
	struct regmap *regmap;
	struct gpio_desc *mrstb;
	unsigned int id;
	int ret;

	regmap = devm_regmap_init_i2c(i2c, &sm5703_regmap_config);
	if (IS_ERR(regmap))
		return dev_err_probe(dev, PTR_ERR(regmap),
				     "Failed to initialize regmap\n");

	/* Keep MRSTB high to indicate that the system is alive */
	mrstb = devm_gpiod_get_optional(dev, "mrstb", GPIOD_OUT_HIGH);
	if (IS_ERR(mrstb))
		return dev_err_probe(dev, PTR_ERR(mrstb),
				     "Failed to get mrstb GPIO\n");

	ret = regmap_read(regmap, SM5703_REG_DEVICE_ID, &id);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to read device ID\n");
	dev_dbg(dev, "Device ID: %#x\n", id);

	ret = devm_regmap_add_irq_chip(dev, regmap, i2c->irq,
				       IRQF_ONESHOT, 0,
				       &sm5703_irq_chip, &irq_data);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to add IRQ chip\n");

	return devm_mfd_add_devices(dev, PLATFORM_DEVID_NONE, sm5703_devs,
				    ARRAY_SIZE(sm5703_devs), NULL, 0,
				    regmap_irq_get_domain(irq_data));
}

static const struct of_device_id sm5703_of_match[] = {
	{ .compatible = "siliconmitus,sm5703" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sm5703_of_match);

static struct i2c_driver sm5703_driver = {
	.driver = {
		.name = "sm5703",
		.of_match_table = sm5703_of_match,
	},
	.probe = sm5703_probe,
};
module_i2c_driver(sm5703_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 MFD core driver");
MODULE_LICENSE("GPL");
