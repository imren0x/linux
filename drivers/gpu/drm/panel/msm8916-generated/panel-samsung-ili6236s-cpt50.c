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

struct ili6236s_cpt50 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data ili6236s_cpt50_supplies[] = {
	{ .supply = "vdd3" },
	{ .supply = "vci" },
	{ .supply = "vbias" },
};

static inline struct ili6236s_cpt50 *to_ili6236s_cpt50(struct drm_panel *panel)
{
	return container_of_const(panel, struct ili6236s_cpt50, panel);
}

static void ili6236s_cpt50_reset(struct ili6236s_cpt50 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int ili6236s_cpt50_on(struct ili6236s_cpt50 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x42, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0xae);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0xae);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8e, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x76, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0xa4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xdb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x45);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x67);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0x45);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x67);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3c, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0xef);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x57, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x69, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6b, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6e, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x71, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x73, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x74, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x76, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa0, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa1, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0x1b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa5, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa6, 0x1e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa7, 0x1e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa8, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x2d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x38);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0x39);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xae, 0x39);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaf, 0x2e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x15);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x1f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x35);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x3f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x3e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x35);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x3b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x80, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0xfc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0xbc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0xfc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0xfc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2c, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2e, 0xbc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0xe4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0xe8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0xec);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0xfc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x5c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0xbc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0xcc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0xd8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0xf9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x69, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6e, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x94);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x71, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x3f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x77, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0xf9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x61, 0x36, 0x00);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);

	return dsi_ctx.accum_err;
}

static int ili6236s_cpt50_off(struct ili6236s_cpt50 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 150);

	return dsi_ctx.accum_err;
}

static int ili6236s_cpt50_prepare(struct drm_panel *panel)
{
	struct ili6236s_cpt50 *ctx = to_ili6236s_cpt50(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(ili6236s_cpt50_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	ili6236s_cpt50_reset(ctx);

	ret = ili6236s_cpt50_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(ili6236s_cpt50_supplies), ctx->supplies);
		return ret;
	}

	/* The panel starts with the brightness at 0, so apply it now */
	if (panel->backlight)
		backlight_update_status(panel->backlight);

	return 0;
}

static int ili6236s_cpt50_unprepare(struct drm_panel *panel)
{
	struct ili6236s_cpt50 *ctx = to_ili6236s_cpt50(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = ili6236s_cpt50_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(ili6236s_cpt50_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode ili6236s_cpt50_mode = {
	.clock = (720 + 18 + 10 + 80) * (1280 + 16 + 4 + 18) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 18,
	.hsync_end = 720 + 18 + 10,
	.htotal = 720 + 18 + 10 + 80,
	.vdisplay = 1280,
	.vsync_start = 1280 + 16,
	.vsync_end = 1280 + 16 + 4,
	.vtotal = 1280 + 16 + 4 + 18,
	.width_mm = 62,
	.height_mm = 106,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int ili6236s_cpt50_get_modes(struct drm_panel *panel,
				    struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &ili6236s_cpt50_mode);
}

static const struct drm_panel_funcs ili6236s_cpt50_panel_funcs = {
	.prepare = ili6236s_cpt50_prepare,
	.unprepare = ili6236s_cpt50_unprepare,
	.get_modes = ili6236s_cpt50_get_modes,
};

static int ili6236s_cpt50_bl_update_status(struct backlight_device *bl)
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

static const struct backlight_ops ili6236s_cpt50_bl_ops = {
	.update_status = ili6236s_cpt50_bl_update_status,
};

static struct backlight_device *
ili6236s_cpt50_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &ili6236s_cpt50_bl_ops, &props);
}

static int ili6236s_cpt50_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct ili6236s_cpt50 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct ili6236s_cpt50, panel,
				   &ili6236s_cpt50_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(ili6236s_cpt50_supplies),
					    ili6236s_cpt50_supplies,
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
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			  MIPI_DSI_MODE_NO_EOT_PACKET;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = ili6236s_cpt50_create_backlight(dsi);
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

static void ili6236s_cpt50_remove(struct mipi_dsi_device *dsi)
{
	struct ili6236s_cpt50 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id ili6236s_cpt50_of_match[] = {
	{ .compatible = "samsung,ili6236s-cpt50" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ili6236s_cpt50_of_match);

static struct mipi_dsi_driver ili6236s_cpt50_driver = {
	.probe = ili6236s_cpt50_probe,
	.remove = ili6236s_cpt50_remove,
	.driver = {
		.name = "panel-samsung-ili6236s-cpt50",
		.of_match_table = ili6236s_cpt50_of_match,
	},
};
module_mipi_dsi_driver(ili6236s_cpt50_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for ss_dsi_panel_ILI6236S_CPT50_HD");
MODULE_LICENSE("GPL");
