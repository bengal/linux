/*
 * Pin controller and GPIO data for Amlogic Meson8
 *
 * Copyright (C) 2014 Beniamino Galvani <b.galvani@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <dt-bindings/gpio/meson8-gpio.h>

#include "pinctrl-meson.h"

const char *meson8_pin_names[] = {
	"GPIOX_0", "GPIOX_1", "GPIOX_2", "GPIOX_3", "GPIOX_4",
	"GPIOX_5", "GPIOX_6", "GPIOX_7", "GPIOX_8", "GPIOX_9",
	"GPIOX_10", "GPIOX_11", "GPIOX_12", "GPIOX_13", "GPIOX_14",
	"GPIOX_15", "GPIOX_16", "GPIOX_17", "GPIOX_18", "GPIOX_19",
	"GPIOX_20", "GPIOX_21",

	"GPIOY_0", "GPIOY_1", "GPIOY_2", "GPIOY_3", "GPIOY_4",
	"GPIOY_5", "GPIOY_6", "GPIOY_7", "GPIOY_8", "GPIOY_9",
	"GPIOY_10", "GPIOY_11", "GPIOY_12", "GPIOY_13", "GPIOY_14",
	"GPIOY_15", "GPIOY_16",

	"GPIODV_0", "GPIODV_1", "GPIODV_2", "GPIODV_3", "GPIODV_4",
	"GPIODV_5", "GPIODV_6", "GPIODV_7", "GPIODV_8", "GPIODV_9",
	"GPIODV_10", "GPIODV_11", "GPIODV_12", "GPIODV_13", "GPIODV_14",
	"GPIODV_15", "GPIODV_16", "GPIODV_17", "GPIODV_18", "GPIODV_19",
	"GPIODV_20", "GPIODV_21", "GPIODV_22", "GPIODV_23", "GPIODV_24",
	"GPIODV_25", "GPIODV_26", "GPIODV_27", "GPIODV_28", "GPIODV_29",

	"GPIOH_0", "GPIOH_1", "GPIOH_2", "GPIOH_3", "GPIOH_4",
	"GPIOH_5", "GPIOH_6", "GPIOH_7", "GPIOH_8", "GPIOH_9",

	"GPIOZ_0", "GPIOZ_1", "GPIOZ_2", "GPIOZ_3", "GPIOZ_4",
	"GPIOZ_5", "GPIOZ_6", "GPIOZ_7", "GPIOZ_8", "GPIOZ_9",
	"GPIOZ_10", "GPIOZ_11", "GPIOZ_12", "GPIOZ_13", "GPIOZ_14",

	"CARD_0", "CARD_1", "CARD_2", "CARD_3", "CARD_4",
	"CARD_5", "CARD_6",

	"BOOT_0", "BOOT_1", "BOOT_2", "BOOT_3", "BOOT_4",
	"BOOT_5", "BOOT_6", "BOOT_7", "BOOT_8", "BOOT_9",
	"BOOT_10", "BOOT_11", "BOOT_12", "BOOT_13", "BOOT_14",
	"BOOT_15", "BOOT_16", "BOOT_17", "BOOT_18",
};

const char *meson8_ao_pin_names[] = {
	"GPIOAO_0", "GPIOAO_1", "GPIOAO_2", "GPIOAO_3",
	"GPIOAO_4", "GPIOAO_5", "GPIOAO_6", "GPIOAO_7",
	"GPIOAO_8", "GPIOAO_9", "GPIOAO_10", "GPIOAO_11",
	"GPIOAO_12", "GPIOAO_13", "GPIO_BSD_EN", "GPIO_TEST_N",
};

struct meson_pmx_group meson8_groups[] = {
	GPIO_GROUP(GPIOX_0),
	GPIO_GROUP(GPIOX_1),
	GPIO_GROUP(GPIOX_2),
	GPIO_GROUP(GPIOX_3),
	GPIO_GROUP(GPIOX_4),
	GPIO_GROUP(GPIOX_5),
	GPIO_GROUP(GPIOX_6),
	GPIO_GROUP(GPIOX_7),
	GPIO_GROUP(GPIOX_8),
	GPIO_GROUP(GPIOX_9),
	GPIO_GROUP(GPIOX_10),
	GPIO_GROUP(GPIOX_11),
	GPIO_GROUP(GPIOX_12),
	GPIO_GROUP(GPIOX_13),
	GPIO_GROUP(GPIOX_14),
	GPIO_GROUP(GPIOX_15),
	GPIO_GROUP(GPIOX_16),
	GPIO_GROUP(GPIOX_17),
	GPIO_GROUP(GPIOX_18),
	GPIO_GROUP(GPIOX_19),
	GPIO_GROUP(GPIOX_20),
	GPIO_GROUP(GPIOX_21),
	GPIO_GROUP(GPIOY_0),
	GPIO_GROUP(GPIOY_1),
	GPIO_GROUP(GPIOY_2),
	GPIO_GROUP(GPIOY_3),
	GPIO_GROUP(GPIOY_4),
	GPIO_GROUP(GPIOY_5),
	GPIO_GROUP(GPIOY_6),
	GPIO_GROUP(GPIOY_7),
	GPIO_GROUP(GPIOY_8),
	GPIO_GROUP(GPIOY_9),
	GPIO_GROUP(GPIOY_10),
	GPIO_GROUP(GPIOY_11),
	GPIO_GROUP(GPIOY_12),
	GPIO_GROUP(GPIOY_13),
	GPIO_GROUP(GPIOY_14),
	GPIO_GROUP(GPIOY_15),
	GPIO_GROUP(GPIOY_16),
	GPIO_GROUP(GPIODV_0),
	GPIO_GROUP(GPIODV_1),
	GPIO_GROUP(GPIODV_2),
	GPIO_GROUP(GPIODV_3),
	GPIO_GROUP(GPIODV_4),
	GPIO_GROUP(GPIODV_5),
	GPIO_GROUP(GPIODV_6),
	GPIO_GROUP(GPIODV_7),
	GPIO_GROUP(GPIODV_8),
	GPIO_GROUP(GPIODV_9),
	GPIO_GROUP(GPIODV_10),
	GPIO_GROUP(GPIODV_11),
	GPIO_GROUP(GPIODV_12),
	GPIO_GROUP(GPIODV_13),
	GPIO_GROUP(GPIODV_14),
	GPIO_GROUP(GPIODV_15),
	GPIO_GROUP(GPIODV_16),
	GPIO_GROUP(GPIODV_17),
	GPIO_GROUP(GPIODV_18),
	GPIO_GROUP(GPIODV_19),
	GPIO_GROUP(GPIODV_20),
	GPIO_GROUP(GPIODV_21),
	GPIO_GROUP(GPIODV_22),
	GPIO_GROUP(GPIODV_23),
	GPIO_GROUP(GPIODV_24),
	GPIO_GROUP(GPIODV_25),
	GPIO_GROUP(GPIODV_26),
	GPIO_GROUP(GPIODV_27),
	GPIO_GROUP(GPIODV_28),
	GPIO_GROUP(GPIODV_29),
	GPIO_GROUP(GPIOH_0),
	GPIO_GROUP(GPIOH_1),
	GPIO_GROUP(GPIOH_2),
	GPIO_GROUP(GPIOH_3),
	GPIO_GROUP(GPIOH_4),
	GPIO_GROUP(GPIOH_5),
	GPIO_GROUP(GPIOH_6),
	GPIO_GROUP(GPIOH_7),
	GPIO_GROUP(GPIOH_8),
	GPIO_GROUP(GPIOH_9),
	GPIO_GROUP(GPIOZ_0),
	GPIO_GROUP(GPIOZ_1),
	GPIO_GROUP(GPIOZ_2),
	GPIO_GROUP(GPIOZ_3),
	GPIO_GROUP(GPIOZ_4),
	GPIO_GROUP(GPIOZ_5),
	GPIO_GROUP(GPIOZ_6),
	GPIO_GROUP(GPIOZ_7),
	GPIO_GROUP(GPIOZ_8),
	GPIO_GROUP(GPIOZ_9),
	GPIO_GROUP(GPIOZ_10),
	GPIO_GROUP(GPIOZ_11),
	GPIO_GROUP(GPIOZ_12),
	GPIO_GROUP(GPIOZ_13),
	GPIO_GROUP(GPIOZ_14),
	GPIO_GROUP(CARD_0),
	GPIO_GROUP(CARD_1),
	GPIO_GROUP(CARD_2),
	GPIO_GROUP(CARD_3),
	GPIO_GROUP(CARD_4),
	GPIO_GROUP(CARD_5),
	GPIO_GROUP(CARD_6),
	GPIO_GROUP(BOOT_0),
	GPIO_GROUP(BOOT_1),
	GPIO_GROUP(BOOT_2),
	GPIO_GROUP(BOOT_3),
	GPIO_GROUP(BOOT_4),
	GPIO_GROUP(BOOT_5),
	GPIO_GROUP(BOOT_6),
	GPIO_GROUP(BOOT_7),
	GPIO_GROUP(BOOT_8),
	GPIO_GROUP(BOOT_9),
	GPIO_GROUP(BOOT_10),
	GPIO_GROUP(BOOT_11),
	GPIO_GROUP(BOOT_12),
	GPIO_GROUP(BOOT_13),
	GPIO_GROUP(BOOT_14),
	GPIO_GROUP(BOOT_15),
	GPIO_GROUP(BOOT_16),
	GPIO_GROUP(BOOT_17),
	GPIO_GROUP(BOOT_18),

	/* BANK X */

	/* SD A */
	GROUP(sd_d0_a,		8,	5,	GPIOX_0),
	GROUP(sd_d1_a,		8,	4,	GPIOX_1),
	GROUP(sd_d2_a,		8,	3,	GPIOX_2),
	GROUP(sd_d3_a,		8,	2,	GPIOX_3),
	GROUP(sd_clk_a,		8,	1,	GPIOX_8),
	GROUP(sd_cmd_a,		8,	0,	GPIOX_9),
	/* SDXC A */
	GROUP(sdxc_d0_a,	5,	14,	GPIOX_0),
	GROUP(sdxc_d13_a,	5,	13,	GPIOX_1, GPIOX_2, GPIOX_3),
	GROUP(sdxc_d47_a,	5,	12,	GPIOX_4, GPIOX_5, GPIOX_6, GPIOX_7),
	GROUP(sdxc_clk_a,	5,	11,	GPIOX_8),
	GROUP(sdxc_cmd_a,	5,	10,	GPIOX_9),
	/* PCM A */
	GROUP(pcm_out_a,	3,	30,	GPIOX_4),
	GROUP(pcm_in_a,		3,	29,	GPIOX_5),
	GROUP(pcm_fs_a,		3,	28,	GPIOX_6),
	GROUP(pcm_clk_a,	3,	27,	GPIOX_7),
	/* UART A (0) */
	GROUP(uart_tx_a0,	4,	17,	GPIOX_4),
	GROUP(uart_rx_a0,	4,	16,	GPIOX_5),
	GROUP(uart_cts_a0,	4,	15,	GPIOX_6),
	GROUP(uart_rts_a0,	4,	14,	GPIOX_7),
	/* UART A (1) */
	GROUP(uart_tx_a1,	4,	13,	GPIOX_12),
	GROUP(uart_rx_a1,	4,	12,	GPIOX_13),
	GROUP(uart_cts_a1,	4,	11,	GPIOX_14),
	GROUP(uart_rts_a1,	4,	10,	GPIOX_15),
	/* UART B (0) */
	GROUP(uart_tx_b0,	4,	9,	GPIOX_16),
	GROUP(uart_rx_b0,	4,	8,	GPIOX_17),
	GROUP(uart_cts_b0,	4,	7,	GPIOX_18),
	GROUP(uart_rts_b0,	4,	6,	GPIOX_19),
	/* ISO 7816 */
	GROUP(iso7816_det,	4,	21,	GPIOX_16),
	GROUP(iso7816_reset,	4,	20,	GPIOX_17),
	GROUP(iso7816_clk,	4,	19,	GPIOX_18),
	GROUP(iso7816_data,	4,	18,	GPIOX_19),
	/* I2C D (0) */
	GROUP(i2c_sda_d0,	4,	5,	GPIOX_16),
	GROUP(i2c_sck_d0,	4,	4,	GPIOX_17),
	/* XTAL */
	GROUP(xtal_32k_out,	3,	22,	GPIOX_10),
	GROUP(xtal_24m_out,	3,	23,	GPIOX_11),

	/* BANK Y */

	/* UART C */
	GROUP(uart_tx_c,	1,	19,	GPIOY_0),
	GROUP(uart_rx_c,	1,	18,	GPIOY_1),
	GROUP(uart_cts_c,	1,	17,	GPIOY_2),
	GROUP(uart_rts_c,	1,	16,	GPIOY_3),
	/* PCM B */
	GROUP(pcm_out_b,	4,	25,	GPIOY_4),
	GROUP(pcm_in_b,		4,	24,	GPIOY_5),
	GROUP(pcm_fs_b,		4,	23,	GPIOY_6),
	GROUP(pcm_clk_b,	4,	22,	GPIOY_7),
	/* I2C C (0) */
	GROUP(i2c_sda_c0,	1,	15,	GPIOY_0),
	GROUP(i2c_sck_c0,	1,	14,	GPIOY_1),

	/* BANK DV */

	/* DVIN */
	GROUP(dvin_rgb,		0,	6,	GPIODV_0, GPIODV_1, GPIODV_2, GPIODV_3,
						GPIODV_4, GPIODV_5, GPIODV_6, GPIODV_7,
						GPIODV_8, GPIODV_9, GPIODV_10, GPIODV_11,
						GPIODV_12, GPIODV_13, GPIODV_14, GPIODV_15,
						GPIODV_16, GPIODV_17, GPIODV_18, GPIODV_19,
						GPIODV_20, GPIODV_21, GPIODV_22, GPIODV_23),
	GROUP(dvin_vs,		0,	9,	GPIODV_24),
	GROUP(dvin_hs,		0,	8,	GPIODV_25),
	GROUP(dvin_clk,		0,	7,	GPIODV_26),
	GROUP(dvin_de,		0,	10,	GPIODV_27),
	/* ENC */
	GROUP(enc_0,		7,	0,	GPIODV_0),
	GROUP(enc_1,		7,	1,	GPIODV_1),
	GROUP(enc_2,		7,	2,	GPIODV_2),
	GROUP(enc_3,		7,	3,	GPIODV_3),
	GROUP(enc_4,		7,	4,	GPIODV_4),
	GROUP(enc_5,		7,	5,	GPIODV_5),
	GROUP(enc_6,		7,	6,	GPIODV_6),
	GROUP(enc_7,		7,	7,	GPIODV_7),
	GROUP(enc_8,		7,	8,	GPIODV_8),
	GROUP(enc_9,		7,	9,	GPIODV_9),
	GROUP(enc_10,		7,	10,	GPIODV_10),
	GROUP(enc_11,		7,	11,	GPIODV_11),
	GROUP(enc_12,		7,	12,	GPIODV_12),
	GROUP(enc_13,		7,	13,	GPIODV_13),
	GROUP(enc_14,		7,	14,	GPIODV_14),
	GROUP(enc_15,		7,	15,	GPIODV_15),
	GROUP(enc_16,		7,	16,	GPIODV_16),
	GROUP(enc_17,		7,	17,	GPIODV_17),
	/* UART B (1) */
	GROUP(uart_tx_b1,	6,	23,	GPIODV_24),
	GROUP(uart_rx_b1,	6,	22,	GPIODV_25),
	GROUP(uart_cts_b1,	6,	21,	GPIODV_26),
	GROUP(uart_rts_b1,	6,	20,	GPIODV_27),
	/* VGA */
	GROUP(vga_vs,		0,	21,	GPIODV_24),
	GROUP(vga_hs,		0,	20,	GPIODV_25),

	/* BANK H */

	/* HDMI */
	GROUP(hdmi_hpd,		1,	26,	GPIOH_0),
	GROUP(hdmi_sda,		1,	25,	GPIOH_1),
	GROUP(hdmi_scl,		1,	24,	GPIOH_2),
	GROUP(hdmi_cec,		1,	23,	GPIOH_3),
	/* SPI (0) */
	GROUP(spi_ss0_0,	9,	13,	GPIOH_3),
	GROUP(spi_miso_0,	9,	12,	GPIOH_4),
	GROUP(spi_mosi_0,	9,	11,	GPIOH_5),
	GROUP(spi_sclk_0,	9,	10,	GPIOH_6),
	/* I2C D (1) */
	GROUP(i2c_sda_d1,	4,	3,	GPIOH_7),
	GROUP(i2c_sck_d1,	4,	2,	GPIOH_8),

	/* BANK Z */

	/* SPI (1) */
	GROUP(spi_ss0_1,	8,	16,	GPIOZ_9),
	GROUP(spi_ss1_1,	8,	12,	GPIOZ_10),
	GROUP(spi_sclk_1,	8,	15,	GPIOZ_11),
	GROUP(spi_mosi_1,	8,	14,	GPIOZ_12),
	GROUP(spi_miso_1,	8,	13,	GPIOZ_13),
	GROUP(spi_ss2_1,	8,	17,	GPIOZ_14),
	/* ETHERNET */
	GROUP(eth_tx_clk_50m,	6,	15,	GPIOZ_4),
	GROUP(eth_tx_en,	6,	14,	GPIOZ_5),
	GROUP(eth_txd1,		6,	13,	GPIOZ_6),
	GROUP(eth_txd0,		6,	12,	GPIOZ_7),
	GROUP(eth_rx_clk_in,	6,	10,	GPIOZ_8),
	GROUP(eth_rx_dv,	6,	11,	GPIOZ_9),
	GROUP(eth_rxd1,		6,	8,	GPIOZ_10),
	GROUP(eth_rxd0,		6,	7,	GPIOZ_11),
	GROUP(eth_mdio,		6,	6,	GPIOZ_12),
	GROUP(eth_mdc,		6,	5,	GPIOZ_13),
	/* I2C A (0) */
	GROUP(i2c_sda_a0,	5,	31,	GPIOZ_0),
	GROUP(i2c_sck_a0,	5,	30,	GPIOZ_1),
	/* I2C B */
	GROUP(i2c_sda_b,	5,	27,	GPIOZ_2),
	GROUP(i2c_sck_b,	5,	26,	GPIOZ_3),
	/* I2C C (1) */
	GROUP(i2c_sda_c1,	5,	25,	GPIOZ_4),
	GROUP(i2c_sck_c1,	5,	24,	GPIOZ_5),
	/* I2C A (1) */
	GROUP(i2c_sda_a1,	5,	9,	GPIOZ_0),
	GROUP(i2c_sck_a1,	5,	8,	GPIOZ_1),
	/* I2C A (2) */
	GROUP(i2c_sda_a2,	5,	7,	GPIOZ_0),
	GROUP(i2c_sck_a2,	5,	6,	GPIOZ_1),

	/* BANK BOOT */

	/* SD C */
	GROUP(sd_d0_c,		6,	29,	BOOT_0),
	GROUP(sd_d1_c,		6,	28,	BOOT_1),
	GROUP(sd_d2_c,		6,	27,	BOOT_2),
	GROUP(sd_d3_c,		6,	26,	BOOT_3),
	GROUP(sd_cmd_c,		6,	25,	BOOT_16),
	GROUP(sd_clk_c,		6,	24,	BOOT_17),
	/* SDXC C */
	GROUP(sdxc_d0_c,	4,	30,	BOOT_0),
	GROUP(sdxc_d13_c,	4,	29,	BOOT_1, BOOT_2, BOOT_3),
	GROUP(sdxc_d47_c,	4,	28,	BOOT_4,	BOOT_5, BOOT_6, BOOT_7),
	GROUP(sdxc_cmd_c,	4,	27,	BOOT_16),
	GROUP(sdxc_clk_c,	4,	26,	BOOT_17),
	/* NAND */
	GROUP(nand_io,		2,	26,	BOOT_0, BOOT_1, BOOT_2, BOOT_3,
						BOOT_4, BOOT_5, BOOT_6, BOOT_7),
	GROUP(nand_io_ce0,	2,	25,	BOOT_8),
	GROUP(nand_io_ce1,	2,	24,	BOOT_9),
	GROUP(nand_io_rb0,	2,	17,	BOOT_10),
	GROUP(nand_ale,		2,	21,	BOOT_11),
	GROUP(nand_cle,		2,	20,	BOOT_12),
	GROUP(nand_wen_clk,	2,	19,	BOOT_13),
	GROUP(nand_ren_clk,	2,	18,	BOOT_14),
	GROUP(nand_dqs,		2,	27,	BOOT_15),
	GROUP(nand_ce2,		2,	23,	BOOT_16),
	GROUP(nand_ce3,		2,	22,	BOOT_17),
	/* NOR */
	GROUP(nor_d,		5,	1,	BOOT_11),
	GROUP(nor_q,		5,	3,	BOOT_12),
	GROUP(nor_c,		5,	2,	BOOT_13),
	GROUP(nor_cs,		5,	0,	BOOT_18),

	/* BANK CARD */

	/* SD B */
	GROUP(sd_d1_b,		2,	14,	CARD_0),
	GROUP(sd_d0_b,		2,	15,	CARD_1),
	GROUP(sd_clk_b,		2,	11,	CARD_2),
	GROUP(sd_cmd_b,		2,	10,	CARD_3),
	GROUP(sd_d3_b,		2,	12,	CARD_4),
	GROUP(sd_d2_b,		2,	13,	CARD_5),
	/* SDXC B */
	GROUP(sdxc_d13_b,	2,	6,	CARD_0, CARD_4, CARD_5),
	GROUP(sdxc_d0_b,	2,	7,	CARD_1),
	GROUP(sdxc_clk_b,	2,	5,	CARD_2),
	GROUP(sdxc_cmd_b,	2,	4,	CARD_3),
};

struct meson_pmx_group meson8_ao_groups[] = {
	GPIO_GROUP(GPIOAO_0),
	GPIO_GROUP(GPIOAO_1),
	GPIO_GROUP(GPIOAO_2),
	GPIO_GROUP(GPIOAO_3),
	GPIO_GROUP(GPIOAO_4),
	GPIO_GROUP(GPIOAO_5),
	GPIO_GROUP(GPIOAO_6),
	GPIO_GROUP(GPIOAO_7),
	GPIO_GROUP(GPIOAO_8),
	GPIO_GROUP(GPIOAO_9),
	GPIO_GROUP(GPIOAO_10),
	GPIO_GROUP(GPIOAO_11),
	GPIO_GROUP(GPIOAO_12),
	GPIO_GROUP(GPIOAO_13),
	GPIO_GROUP(GPIO_BSD_EN),
	GPIO_GROUP(GPIO_TEST_N),
	/* UART AO A */
	GROUP(uart_tx_ao_a,	0,	12,	GPIOAO_0),
	GROUP(uart_rx_ao_a,	0,	11,	GPIOAO_1),
	GROUP(uart_cts_ao_a,	0,	10,	GPIOAO_2),
	GROUP(uart_rts_ao_a,	0,	9,	GPIOAO_3),
	/* REMOTE */
	GROUP(remote_input,	0,	0,	GPIOAO_7),
	/* I2C SLAVE AO */
	GROUP(i2c_slave_sck_ao,	0,	2,	GPIOAO_4),
	GROUP(i2c_slave_sda_ao,	0,	1,	GPIOAO_5),
	/* UART AO B (0) */
	GROUP(uart_tx_ao_b0,	0,	26,	GPIOAO_0),
	GROUP(uart_rx_ao_b0,	0,	25,	GPIOAO_1),
	/* UART AO B (1) */
	GROUP(uart_tx_ao_b1,	0,	24,	GPIOAO_4),
	GROUP(uart_rx_ao_b1,	0,	23,	GPIOAO_5),
	/* I2C MST AO */
	GROUP(i2c_mst_sck_ao,	0,	6,	GPIOAO_4),
	GROUP(i2c_mst_sda_ao,	0,	5,	GPIOAO_5),
};

struct meson_pmx_func meson8_functions[] = {
	{
		.name = "gpio",
		.groups = meson8_pin_names,
		.num_groups = ARRAY_SIZE(meson8_pin_names),
	},
	FUNCTION("sd_a",	"sd_d0_a", "sd_d1_a", "sd_d2_a", "sd_d3_a",
				"sd_clk_a", "sd_cmd_a"),
	FUNCTION("sdxc_a",	"sdxc_d0_a", "sdxc_d13_a", "sdxc_d47_a",
				"sdxc_clk_a", "sdxc_cmd_a"),
	FUNCTION("pcm_a",	"pcm_out_a", "pcm_in_a", "pcm_fs_a", "pcm_clk_a"),

	FUNCTION("uart_a",	"uart_tx_a0", "uart_rx_a0", "uart_cts_a0", "uart_rts_a0",
				"uart_tx_a1", "uart_rx_a1", "uart_cts_a1", "uart_rts_a1"),
	FUNCTION("uart_b",	"uart_tx_b0", "uart_rx_b0", "uart_cts_b0", "uart_rts_b0",
				"uart_tx_b1", "uart_rx_b1", "uart_cts_b1", "uart_rts_b1"),
	FUNCTION("iso7816",	"iso7816_det", "iso7816_reset", "iso7816_clk",
				"iso7816_data"),
	FUNCTION("i2c_d",	"i2c_sda_d0", "i2c_sck_d0", "i2c_sda_d1", "i2c_sck_d1"),
	FUNCTION("xtal",	"xtal_32k_out", "xtal_24m_out"),
	FUNCTION("uart_c",	"uart_tx_c", "uart_rx_c", "uart_cts_c", "uart_rts_c"),
	FUNCTION("pcm_b",	"pcm_out_b", "pcm_in_b", "pcm_fs_b", "pcm_clk_b"),
	FUNCTION("i2c_c",	"i2c_sda_c0", "i2c_sck_c0", "i2c_sda_c1", "i2c_sck_c1"),
	FUNCTION("dvin",	"dvin_rgb", "dvin_vs", "dvin_hs", "dvin_clk", "dvin_de"),
	FUNCTION("enc",		"enc_0", "enc_1", "enc_2", "enc_3", "enc_4", "enc_5",
				"enc_6", "enc_7", "enc_8", "enc_9", "enc_10", "enc_11",
				"enc_12", "enc_13", "enc_14", "enc_15", "enc_16", "enc_17"),
	FUNCTION("vga",		"vga_vs", "vga_hs"),
	FUNCTION("hdmi",	"hdmi_hpd", "hdmi_sda", "hdmi_scl", "hdmi_cec"),
	FUNCTION("spi",		"spi_ss0_0", "spi_miso_0", "spi_mosi_0", "spi_sclk_0",
				"spi_ss0_1", "spi_ss1_1", "spi_sclk_1", "spi_mosi_1",
				"spi_miso_1", "spi_ss2_1"),
	FUNCTION("ethernet",	"eth_tx_clk_50m", "eth_tx_en", "eth_txd1",
				"eth_txd0", "eth_rx_clk_in", "eth_rx_dv",
				"eth_rxd1", "eth_rxd0", "eth_mdio", "eth_mdc"),
	FUNCTION("i2c_a",	"i2c_sda_a0", "i2c_sck_a0", "i2c_sda_a1", "i2c_sck_a1",
				"i2c_sda_a2", "i2c_sck_a2"),
	FUNCTION("i2c_b",	"i2c_sda_b", "i2c_sck_b"),
	FUNCTION("sd_c",	"sd_d0_c", "sd_d1_c" "sd_d2_c", "sd_d3_c",
				"sd_cmd_c", "sd_clk_c"),
	FUNCTION("sdxc_c",	"sdxc_d0_c", "sdxc_d13_c", "sdxc_d47_c", "sdxc_cmd_c",
				"sdxc_clk_c"),
	FUNCTION("nand",	"nand_io", "nand_io_ce0", "nand_io_ce1",
				"nand_io_rb0", "nand_ale", "nand_cle",
				"nand_wen_clk", "nand_ren_clk", "nand_dqs",
				"nand_ce2", "nand_ce3"),
	FUNCTION("nor",		"nor_d", "nor_q", "nor_c", "nor_cs"),
	FUNCTION("sd_b",	"sd_d1_b", "sd_d0_b", "sd_clk_b", "sd_cmd_b",
				"sd_d3_b", "sd_d2_b"),
	FUNCTION("sdxc_b",	"sdxc_d13_b", "sdxc_d0_b", "sdxc_clk_b", "sdxc_cmd_b"),
};

struct meson_pmx_func meson8_ao_functions[] = {
	{
		.name = "gpio",
		.groups = meson8_ao_pin_names,
		.num_groups = ARRAY_SIZE(meson8_ao_pin_names),
	},
	FUNCTION("uart_ao",	"uart_tx_ao_a", "uart_rx_ao_a",
				"uart_cts_ao_a", "uart_rts_ao_a"),
	FUNCTION("remote",	"remote_input"),
	FUNCTION("i2c_slave_ao", "i2c_slave_sck_ao", "i2c_slave_sda_ao"),
	FUNCTION("uart_ao_b",	"uart_tx_ao_b0", "uart_rx_ao_b0",
				"uart_tx_ao_b1", "uart_rx_ao_b1"),
	FUNCTION("i2c_mst_ao",	"i2c_mst_sck_ao", "i2c_mst_sda_ao"),
};


struct meson_bank meson8_banks[] = {
	/*   name    first     last         pullen  pull     dir     out     in  */
	BANK("X",    GPIOX_0,  GPIOX_21,    4,  0,  4,  0,  0,  0,  1,  0,  2,  0),
	BANK("Y",    GPIOY_0,  GPIOY_16,    3,  0,  3,  0,  3,  0,  4,  0,  5,  0),
	BANK("DV",   GPIODV_0, GPIODV_29,   0,  0,  0,  0,  7,  0,  8,  0,  9,  0),
	BANK("H",    GPIOH_0,  GPIOH_9,     1, 16,  1, 16,  9, 19, 10, 19, 11, 19),
	BANK("Z",    GPIOZ_0,  GPIOZ_14,    1,  0,  1,  0,  3, 17,  4, 17,  5, 17),
	BANK("CARD", CARD_0,   CARD_6,      2, 20,  2, 20,  0, 22,  1, 22,  2, 22),
	BANK("BOOT", BOOT_0,   BOOT_18,     2,  0,  2,  0,  9,  0, 10,  0, 11,  0),
};

struct meson_bank meson8_ao_banks[] = {
	/*   name    first     last         pullen  pull     dir     out     in  */
	BANK("AO",   GPIOAO_0, GPIO_TEST_N, 0,  0,  0, 16,  0,  0,  0, 16,  1,  0),
};

struct meson_domain_data meson8_domain_data[] = {
	{
		.name		= "banks",
		.banks		= meson8_banks,
		.pin_names	= meson8_pin_names,
		.funcs		= meson8_functions,
		.groups		= meson8_groups,
		.num_pins	= ARRAY_SIZE(meson8_pin_names),
		.num_banks	= ARRAY_SIZE(meson8_banks),
		.num_funcs	= ARRAY_SIZE(meson8_functions),
		.num_groups	= ARRAY_SIZE(meson8_groups),
	},
	{
		.name		= "ao-bank",
		.banks		= meson8_ao_banks,
		.pin_names	= meson8_ao_pin_names,
		.funcs		= meson8_ao_functions,
		.groups		= meson8_ao_groups,
		.num_pins	= ARRAY_SIZE(meson8_ao_pin_names),
		.num_banks	= ARRAY_SIZE(meson8_ao_banks),
		.num_funcs	= ARRAY_SIZE(meson8_ao_functions),
		.num_groups	= ARRAY_SIZE(meson8_ao_groups),
	},
	{ },
};
