// SPDX-License-Identifier: GPL-2.0-only
/*
 * Power supply driver for the TI LM3632 LCD bias and backlight boost
 *
 * The brightness is controlled by the display panel (e.g. via DCS commands
 * that generate a PWM signal for the LM3632), so this driver only needs to
 * power up the LM3632 in the right order and program its initial register
 * configuration.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>

#define LM3632_BIAS_MICROVOLT	5500000

struct lm3632 {
	struct i2c_client *client;
	struct gpio_desc *en;
	struct gpio_desc *enp;
	struct gpio_desc *enn;
	u8 *init_seq;
	int init_seq_len;
};

/*
 * The GPIOs are requested "as is" so that a display that was already
 * powered up by the bootloader keeps working until it is handed over. They
 * might still be inputs then, so always switch the direction when changing them.
 */
static void lm3632_set(struct gpio_desc *gpio, int value)
{
	gpiod_direction_output(gpio, value);
}

static int lm3632_enable(struct regulator_dev *rdev)
{
	struct lm3632 *lm = rdev_get_drvdata(rdev);
	int i, ret;

	lm3632_set(lm->en, 1);
	msleep(5);

	for (i = 0; i < lm->init_seq_len; i += 2) {
		ret = i2c_smbus_write_byte_data(lm->client, lm->init_seq[i],
						lm->init_seq[i + 1]);
		if (ret < 0) {
			dev_err(&lm->client->dev,
				"Failed to write register %#x: %d\n",
				lm->init_seq[i], ret);
			lm3632_set(lm->en, 0);
			return ret;
		}
	}

	lm3632_set(lm->enp, 1);
	msleep(5);
	lm3632_set(lm->enn, 1);

	return 0;
}

static int lm3632_disable(struct regulator_dev *rdev)
{
	struct lm3632 *lm = rdev_get_drvdata(rdev);

	lm3632_set(lm->enn, 0);
	msleep(5);
	lm3632_set(lm->enp, 0);
	lm3632_set(lm->en, 0);
	msleep(5);

	return 0;
}

static int lm3632_is_enabled(struct regulator_dev *rdev)
{
	struct lm3632 *lm = rdev_get_drvdata(rdev);

	return gpiod_get_value_cansleep(lm->en) > 0 &&
	       gpiod_get_value_cansleep(lm->enn) > 0;
}

static const struct regulator_ops lm3632_ops = {
	.enable = lm3632_enable,
	.disable = lm3632_disable,
	.is_enabled = lm3632_is_enabled,
};

static const struct regulator_desc lm3632_desc = {
	.name = "lm3632",
	.type = REGULATOR_VOLTAGE,
	.ops = &lm3632_ops,
	.n_voltages = 1,
	.fixed_uV = LM3632_BIAS_MICROVOLT,
	.owner = THIS_MODULE,
};

static int lm3632_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct regulator_config config = { };
	struct regulator_dev *rdev;
	struct lm3632 *lm;
	int len;

	lm = devm_kzalloc(dev, sizeof(*lm), GFP_KERNEL);
	if (!lm)
		return -ENOMEM;

	lm->client = client;

	lm->en = devm_gpiod_get(dev, "enable", GPIOD_ASIS);
	if (IS_ERR(lm->en))
		return dev_err_probe(dev, PTR_ERR(lm->en),
				     "Failed to get enable GPIO\n");

	lm->enp = devm_gpiod_get(dev, "enp", GPIOD_ASIS);
	if (IS_ERR(lm->enp))
		return dev_err_probe(dev, PTR_ERR(lm->enp),
				     "Failed to get enp GPIO\n");

	lm->enn = devm_gpiod_get(dev, "enn", GPIOD_ASIS);
	if (IS_ERR(lm->enn))
		return dev_err_probe(dev, PTR_ERR(lm->enn),
				     "Failed to get enn GPIO\n");

	len = device_property_count_u8(dev, "ti,init-sequence");
	if (len > 0) {
		if (len % 2)
			return dev_err_probe(dev, -EINVAL,
					     "Invalid ti,init-sequence length\n");

		lm->init_seq = devm_kcalloc(dev, len, sizeof(*lm->init_seq),
					    GFP_KERNEL);
		if (!lm->init_seq)
			return -ENOMEM;

		device_property_read_u8_array(dev, "ti,init-sequence",
					      lm->init_seq, len);
		lm->init_seq_len = len;
	}

	config.dev = dev;
	config.of_node = dev->of_node;
	config.driver_data = lm;
	config.init_data = of_get_regulator_init_data(dev, dev->of_node,
						      &lm3632_desc);

	rdev = devm_regulator_register(dev, &lm3632_desc, &config);
	if (IS_ERR(rdev))
		return dev_err_probe(dev, PTR_ERR(rdev),
				     "Failed to register regulator\n");

	return 0;
}

static const struct of_device_id lm3632_of_match[] = {
	{ .compatible = "ti,lm3632" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, lm3632_of_match);

static struct i2c_driver lm3632_driver = {
	.driver = {
		.name = "lm3632-regulator",
		.of_match_table = lm3632_of_match,
	},
	.probe = lm3632_probe,
};
module_i2c_driver(lm3632_driver);

MODULE_DESCRIPTION("TI LM3632 LCD bias and backlight boost power supply driver");
MODULE_LICENSE("GPL");
