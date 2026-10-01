#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/err.h>

#include "gf_spi.h"

#define gf_dbg(fmt, args...) do { \
					pr_warn("gf:" fmt, ##args);\
		} while (0)


/*
 * The GPIOs are only requested once the fingerprint HAL asks for them
 * (GF_IOC_ENABLE_GPIO), and are given back on GF_IOC_RELEASE_GPIO, as other
 * sensor drivers sharing the board may want them while probing their sensor.
 */
static int gf3208_request_named_gpio(struct gf_dev *gf_dev, const char *label,
				     enum gpiod_flags flags,
				     struct gpio_desc **gpio)
{
	struct device *dev = &gf_dev->spi->dev;
	struct gpio_desc *desc;

	desc = gpiod_get(dev, label, flags);
	if (IS_ERR(desc)) {
		dev_err(dev, "failed to get '%s' gpio: %ld\n", label,
			PTR_ERR(desc));
		return PTR_ERR(desc);
	}
	*gpio = desc;
	return 0;
}

/*GPIO pins reference.*/
int gf_parse_dts(struct gf_dev *gf_dev)
{
	int rc = 0;

	/*get reset resource*/
	rc = gf3208_request_named_gpio(gf_dev, "reset", GPIOD_OUT_LOW,
				       &gf_dev->reset_gpio);
	if (rc) {
		gf_dbg("Failed to request RESET GPIO. rc = %d\n", rc);
		return -EPERM;
	}

	/*get irq resourece*/
	rc = gf3208_request_named_gpio(gf_dev, "irq", GPIOD_IN,
				       &gf_dev->irq_gpio);
	if (rc) {
		gf_dbg("Failed to request IRQ GPIO. rc = %d\n", rc);
		return -EPERM;
	}

	return rc;
}

void gf_cleanup(struct gf_dev *gf_dev)
{
	gf_dbg("[info]  enter%s\n", __func__);

	if (gf_dev->irq_gpio) {
		gpiod_put(gf_dev->irq_gpio);
		gf_dev->irq_gpio = NULL;
		gf_dbg("remove irq_gpio success\n");
	}

	if (gf_dev->reset_gpio) {
		gpiod_put(gf_dev->reset_gpio);
		gf_dev->reset_gpio = NULL;
		gf_dbg("remove reset_gpio success\n");
	}
}

/*power management*/
int gf_power_on(struct gf_dev *gf_dev)
{
	int rc = 0;
	msleep(10);
	pr_info("---- power on ok ----\n");

	return rc;
}

int gf_power_off(struct gf_dev *gf_dev)
{
	int rc = 0;
	pr_info("---- power off ----\n");
	return rc;
}

static int hw_reset(struct  gf_dev *gf_dev)
{
	int irq_gpio;
	struct device *dev = &gf_dev->spi->dev;

	if (!gf_dev->reset_gpio || !gf_dev->irq_gpio)
		return -ENODEV;

	/*
	 * Downstream did this by switching between the "reset" and "active"
	 * pinctrl states, which drove the pin low and high.
	 */
	gpiod_set_value_cansleep(gf_dev->reset_gpio, 1);
	mdelay(3);
	gpiod_set_value_cansleep(gf_dev->reset_gpio, 0);

	irq_gpio = gpiod_get_value_cansleep(gf_dev->irq_gpio);
	dev_info(dev, "IRQ after reset %d\n", irq_gpio);
	return 0;
}


/********************************************************************
 *CPU output low level in RST pin to reset GF. This is the MUST action for GF.
 *Take care of this function. IO Pin driver strength / glitch and so on.
 ********************************************************************/
int gf_hw_reset(struct gf_dev *gf_dev, unsigned int delay_ms)
{
	if (gf_dev == NULL) {
		pr_info("Input buff is NULL.\n");
		return -EPERM;
	}
	hw_reset(gf_dev);
	mdelay(delay_ms);
	return 0;
}

int gf_irq_num(struct gf_dev *gf_dev)
{
	if (gf_dev == NULL) {
		pr_info("Input buff is NULL.\n");
		return -EPERM;
	} else {
		return gpiod_to_irq(gf_dev->irq_gpio);
	}
}

