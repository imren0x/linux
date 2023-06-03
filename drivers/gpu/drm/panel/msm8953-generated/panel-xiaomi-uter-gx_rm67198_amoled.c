// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2025 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct gx_rm67198_amoled {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data gx_rm67198_amoled_supplies[] = {
	{ .supply = "vsn" },
	{ .supply = "vsp" },
};

static inline
struct gx_rm67198_amoled *to_gx_rm67198_amoled(struct drm_panel *panel)
{
	return container_of(panel, struct gx_rm67198_amoled, panel);
}

static void gx_rm67198_amoled_reset(struct gx_rm67198_amoled *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int gx_rm67198_amoled_on(struct gx_rm67198_amoled *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x4c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x85);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0xe6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x93);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x93);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x74, 0x2f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0xcc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x41, 0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x64);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x64);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0xca);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9b, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xae, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaf, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbc, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbd, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbe, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbf, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0xf6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7d, 0x35);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x86, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa7, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x42, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x8d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0xbd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x63);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x38);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x69, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6b, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6d, 0xfe);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6e, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x71, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x73, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x74, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x76, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x77, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7c, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7e, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7f, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x80, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x81, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x82, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x85, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x86, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x87, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x88, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x89, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8a, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8b, 0xaf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8c, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8d, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8e, 0xce);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8f, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x90, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0xe8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x99, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 58);

	return dsi_ctx.accum_err;
}

static int gx_rm67198_amoled_off(struct gx_rm67198_amoled *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 16000, 17000);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 64);

	return dsi_ctx.accum_err;
}

static int gx_rm67198_amoled_prepare(struct drm_panel *panel)
{
	struct gx_rm67198_amoled *ctx = to_gx_rm67198_amoled(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(gx_rm67198_amoled_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	gx_rm67198_amoled_reset(ctx);

	ret = gx_rm67198_amoled_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(gx_rm67198_amoled_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int gx_rm67198_amoled_unprepare(struct drm_panel *panel)
{
	struct gx_rm67198_amoled *ctx = to_gx_rm67198_amoled(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = gx_rm67198_amoled_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(gx_rm67198_amoled_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode gx_rm67198_amoled_mode = {
	.clock = (1080 + 26 + 2 + 36) * (1920 + 8 + 4 + 4) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 26,
	.hsync_end = 1080 + 26 + 2,
	.htotal = 1080 + 26 + 2 + 36,
	.vdisplay = 1920,
	.vsync_start = 1920 + 8,
	.vsync_end = 1920 + 8 + 4,
	.vtotal = 1920 + 8 + 4 + 4,
	.width_mm = 68,
	.height_mm = 121,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int gx_rm67198_amoled_get_modes(struct drm_panel *panel,
				       struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &gx_rm67198_amoled_mode);
}

static const struct drm_panel_funcs gx_rm67198_amoled_panel_funcs = {
	.prepare = gx_rm67198_amoled_prepare,
	.unprepare = gx_rm67198_amoled_unprepare,
	.get_modes = gx_rm67198_amoled_get_modes,
};

static int gx_rm67198_amoled_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct gx_rm67198_amoled *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct gx_rm67198_amoled, panel,
				   &gx_rm67198_amoled_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(gx_rm67198_amoled_supplies),
					    gx_rm67198_amoled_supplies,
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
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	ctx->panel.prepare_prev_first = true;

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void gx_rm67198_amoled_remove(struct mipi_dsi_device *dsi)
{
	struct gx_rm67198_amoled *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id gx_rm67198_amoled_of_match[] = {
	{ .compatible = "xiaomi,uter-gx_rm67198_amoled" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, gx_rm67198_amoled_of_match);

static struct mipi_dsi_driver gx_rm67198_amoled_driver = {
	.probe = gx_rm67198_amoled_probe,
	.remove = gx_rm67198_amoled_remove,
	.driver = {
		.name = "panel-gx-rm67198-amoled",
		.of_match_table = gx_rm67198_amoled_of_match,
	},
};
module_mipi_dsi_driver(gx_rm67198_amoled_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for guoxian rm67198 amoled 1080p cmd mode dsi panel");
MODULE_LICENSE("GPL");
