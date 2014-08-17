/*
 * SMP initialization for Amlogic Meson SoCs
 *
 * Copyright (C) 2014 Beniamino Galvani <b.galvani@gmail.com>
 *
 * Based on code
 *   Copyright (C) 2011-2012 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <asm/smp_scu.h>
#include <asm/smp_plat.h>

static void __iomem *scu_base_addr;
static void __iomem *sram_base_addr;
static void __iomem *hhi_base_addr;
static void __iomem *pwr_a9_cntl_addr;

#define HHI_CPU_CLK_CNTL	0xb0
#define PWR_A9_CNTL0		0x00
#define PWR_A9_CNTL1		0x04
#define SRAM_CPU_CONTROL	0x1ff80
#define SRAM_CPU_CONTROL_ADDR	0x1ff84

void meson_secondary_startup(void);

static void set_reg_mask(void __iomem *addr, int val, int start, int len)
{
	uint32_t data;

	data = readl(addr);
	data &= ~(((1UL << len) - 1) << start);
	data |= val << start;
	writel(data, addr);
}

static void __iomem *find_and_map(const char *comp)
{
	struct device_node *node;
	void __iomem *addr;

	node = of_find_compatible_node(NULL, NULL, comp);
	if (!node) {
		pr_err("meson smp: couldn't find %s node\n", comp);
		return NULL;
	}

	addr = of_iomap(node, 0);
	if (!addr) {
		pr_err("meson smp: couldn't map %s\n", comp);
		return NULL;
	}

	return addr;
}

static void __init meson8_smp_prepare_cpus(unsigned int max_cpus)
{
	scu_base_addr = find_and_map("arm,cortex-a9-scu");
	if (!scu_base_addr)
		return;

	sram_base_addr = find_and_map("meson,meson8-smp-sram");
	if (!sram_base_addr)
		return;

	hhi_base_addr = find_and_map("meson,meson8-hhi");
	if (!hhi_base_addr)
		return;

	pwr_a9_cntl_addr = find_and_map("meson,meson8-pwr-a9-cntl");
	if (!pwr_a9_cntl_addr)
		return;

	scu_enable(scu_base_addr);
}

static int meson8_smp_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	/* Power on the CPU in the SCU */
	set_reg_mask(scu_base_addr + 8, 0, cpu << 3, 2);
	/* Enable reset */
	set_reg_mask(hhi_base_addr + HHI_CPU_CLK_CNTL, 1, cpu + 24, 1);
	/* Power on CPU */
	set_reg_mask(pwr_a9_cntl_addr + PWR_A9_CNTL1, 0, (cpu + 1) << 1, 2);

	udelay(10);

	/* Disable isolation */
	set_reg_mask(pwr_a9_cntl_addr + PWR_A9_CNTL0, 0, cpu, 1);
	/* Disable reset */
	set_reg_mask(hhi_base_addr + HHI_CPU_CLK_CNTL, 0, cpu + 24, 1);

	/* Write entry point into SRAM */
	writel((const uint32_t)virt_to_phys(meson_secondary_startup),
	       sram_base_addr + SRAM_CPU_CONTROL_ADDR + ((cpu - 1) << 2));
	set_reg_mask(sram_base_addr + SRAM_CPU_CONTROL, 1, cpu, 1);
	set_reg_mask(sram_base_addr + SRAM_CPU_CONTROL, 1, 0, 1);

	return 0;
}

static struct smp_operations meson8_smp_ops __initdata = {
	.smp_prepare_cpus	= meson8_smp_prepare_cpus,
	.smp_boot_secondary	= meson8_smp_boot_secondary,
};

CPU_METHOD_OF_DECLARE(meson_smp, "amlogic,meson8-smp", &meson8_smp_ops);
