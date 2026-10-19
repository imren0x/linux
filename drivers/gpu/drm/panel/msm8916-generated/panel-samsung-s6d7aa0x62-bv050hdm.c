// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/mod_devicetable.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct s6d7aa0x62_bv050hdm {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data s6d7aa0x62_bv050hdm_supplies[] = {
	{ .supply = "vdd3" },
	{ .supply = "vci" },
	{ .supply = "vbias" },
};

static inline
struct s6d7aa0x62_bv050hdm *to_s6d7aa0x62_bv050hdm(struct drm_panel *panel)
{
	return container_of_const(panel, struct s6d7aa0x62_bv050hdm, panel);
}

static void s6d7aa0x62_bv050hdm_reset(struct s6d7aa0x62_bv050hdm *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int s6d7aa0x62_bv050hdm_on(struct s6d7aa0x62_bv050hdm *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x00, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x14, 0x22, 0x2f, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				     0x03, 0x19, 0x19, 0x11, 0x03, 0x05, 0x18,
				     0x18, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbc, 0x00, 0x4e, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x80, 0x80, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x40, 0x00, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1,
				     0x03, 0x10, 0x1c, 0xa0, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee,
				     0xa5, 0x00, 0x98, 0x00, 0xa5, 0x00, 0x98,
				     0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2,
				     0x02, 0x10, 0x10, 0x44, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf3,
				     0x01, 0x93, 0x20, 0x22, 0x80, 0x05, 0x25,
				     0x3c, 0x26, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf4,
				     0x02, 0x02, 0x10, 0x26, 0x10, 0x02, 0x03,
				     0x03, 0x03, 0x10, 0x16, 0x03, 0x00, 0x0c,
				     0x0c, 0x03, 0x04, 0x05, 0x13, 0x1e, 0x09,
				     0x0a, 0x05, 0x05, 0x01, 0x04, 0x02, 0x61,
				     0x74, 0x75, 0x72, 0x83, 0x80, 0x80, 0x00,
				     0x00, 0x01, 0x01, 0x28, 0x04, 0x03, 0x28,
				     0x01, 0xd1, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf5,
				     0x92, 0x23, 0x2d, 0x5f, 0xbb, 0x8b, 0x55,
				     0x0f, 0x33, 0x33, 0x03, 0x59, 0x54, 0x52,
				     0x45, 0x57, 0x60, 0x10, 0x60, 0x80, 0x27,
				     0x26, 0x52, 0x25, 0x6f, 0x1b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7,
				     0x01, 0x1b, 0x08, 0x0c, 0x09, 0x0d, 0x01,
				     0x01, 0x01, 0x04, 0x06, 0x01, 0x01, 0x00,
				     0x00, 0x1a, 0x01, 0x1b, 0x0a, 0x0e, 0x0b,
				     0x0f, 0x01, 0x01, 0x01, 0x05, 0x07, 0x01,
				     0x01, 0x00, 0x00, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf6,
				     0x60, 0x25, 0x05, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfd,
				     0x16, 0x10, 0x11, 0x23, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe,
				     0x00, 0x02, 0x03, 0x21, 0x80, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef,
				     0x34, 0x12, 0x98, 0xba, 0x10, 0x80, 0x24,
				     0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x44,
				     0xa0, 0x82, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd,
				     0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
				     0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfa,
				     0x08, 0x3f, 0x0a, 0x0c, 0x00, 0x02, 0x06,
				     0x03, 0x04, 0x0c, 0x11, 0x16, 0x19, 0x1a,
				     0x1e, 0x25, 0x2f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb,
				     0x08, 0x3f, 0x0a, 0x0c, 0x00, 0x01, 0x05,
				     0x03, 0x03, 0x0b, 0x11, 0x16, 0x18, 0x1a,
				     0x1e, 0x25, 0x2e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0x5a, 0x5a);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);

	return dsi_ctx.accum_err;
}

static int s6d7aa0x62_bv050hdm_off(struct s6d7aa0x62_bv050hdm *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 150);

	return dsi_ctx.accum_err;
}

static int s6d7aa0x62_bv050hdm_prepare(struct drm_panel *panel)
{
	struct s6d7aa0x62_bv050hdm *ctx = to_s6d7aa0x62_bv050hdm(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(s6d7aa0x62_bv050hdm_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	s6d7aa0x62_bv050hdm_reset(ctx);

	ret = s6d7aa0x62_bv050hdm_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(s6d7aa0x62_bv050hdm_supplies), ctx->supplies);
		return ret;
	}

	/* The panel starts with the brightness at 0, so apply it now */
	if (panel->backlight)
		backlight_update_status(panel->backlight);

	return 0;
}

static int s6d7aa0x62_bv050hdm_unprepare(struct drm_panel *panel)
{
	struct s6d7aa0x62_bv050hdm *ctx = to_s6d7aa0x62_bv050hdm(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = s6d7aa0x62_bv050hdm_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(s6d7aa0x62_bv050hdm_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode s6d7aa0x62_bv050hdm_mode = {
	.clock = (720 + 104 + 42 + 154) * (1280 + 16 + 4 + 12) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 104,
	.hsync_end = 720 + 104 + 42,
	.htotal = 720 + 104 + 42 + 154,
	.vdisplay = 1280,
	.vsync_start = 1280 + 16,
	.vsync_end = 1280 + 16 + 4,
	.vtotal = 1280 + 16 + 4 + 12,
	.width_mm = 62,
	.height_mm = 106,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int s6d7aa0x62_bv050hdm_get_modes(struct drm_panel *panel,
					 struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &s6d7aa0x62_bv050hdm_mode);
}

static const struct drm_panel_funcs s6d7aa0x62_bv050hdm_panel_funcs = {
	.prepare = s6d7aa0x62_bv050hdm_prepare,
	.unprepare = s6d7aa0x62_bv050hdm_unprepare,
	.get_modes = s6d7aa0x62_bv050hdm_get_modes,
};

static int s6d7aa0x62_bv050hdm_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	unsigned long mode_flags = dsi->mode_flags;
	u8 brightness = backlight_get_brightness(bl);
	ssize_t ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	ret = mipi_dsi_dcs_write(dsi, MIPI_DCS_SET_DISPLAY_BRIGHTNESS,
				 &brightness, sizeof(brightness));
	dsi->mode_flags = mode_flags;

	return ret < 0 ? ret : 0;
}

static const struct backlight_ops s6d7aa0x62_bv050hdm_bl_ops = {
	.update_status = s6d7aa0x62_bv050hdm_bl_update_status,
};

static struct backlight_device *
s6d7aa0x62_bv050hdm_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &s6d7aa0x62_bv050hdm_bl_ops, &props);
}

static int s6d7aa0x62_bv050hdm_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct s6d7aa0x62_bv050hdm *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct s6d7aa0x62_bv050hdm, panel,
				   &s6d7aa0x62_bv050hdm_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(s6d7aa0x62_bv050hdm_supplies),
					    s6d7aa0x62_bv050hdm_supplies,
					    &ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_MODE_VIDEO_NO_HBP;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = s6d7aa0x62_bv050hdm_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void s6d7aa0x62_bv050hdm_remove(struct mipi_dsi_device *dsi)
{
	struct s6d7aa0x62_bv050hdm *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id s6d7aa0x62_bv050hdm_of_match[] = {
	{ .compatible = "samsung,s6d7aa0x62-bv050hdm" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, s6d7aa0x62_bv050hdm_of_match);

static struct mipi_dsi_driver s6d7aa0x62_bv050hdm_driver = {
	.probe = s6d7aa0x62_bv050hdm_probe,
	.remove = s6d7aa0x62_bv050hdm_remove,
	.driver = {
		.name = "panel-samsung-s6d7aa0x62-bv050hdm",
		.of_match_table = s6d7aa0x62_bv050hdm_of_match,
	},
};
module_mipi_dsi_driver(s6d7aa0x62_bv050hdm_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for ss_dsi_panel_S6D7AA0X62_BV050HDM_HD");
MODULE_LICENSE("GPL");
