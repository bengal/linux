/*
 * Pin controller and GPIO driver for Amlogic Meson SoCs
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

#include <linux/gpio.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/spinlock.h>
#include <linux/types.h>

/**
 * struct meson_pmx_group - a pinmux group
 *
 * @name:	group name
 * @pins:	pins in the group
 * @num_pins:	number of pins in the group
 * @is_gpio:	flag set when the group is a single GPIO group
 * @reg:	register offset for the group in the domain mux registers
 * @bit		bit index enabling the group
 * @domain:	pin domain this group belongs to
 */
struct meson_pmx_group {
	const char *name;
	const unsigned int *pins;
	unsigned int num_pins;
	bool is_gpio;
	unsigned int reg;
	unsigned int bit;
	struct meson_domain *domain;
};

/**
 * struct meson_pmx_func - a pinmux function
 *
 * @name:	function name
 * @groups:	groups in the function
 * @num_groups:	number of groups in the function
 */
struct meson_pmx_func {
	const char *name;
	const char **groups;
	unsigned int num_groups;
};

/**
 * struct meson_reg_offset
 *
 * @reg:	register offset
 * @bit:	bit index
 */
struct meson_reg_offset {
	unsigned int reg;
	unsigned int bit;
};

enum {
	REG_PULLEN,
	REG_PULL,
	REG_DIR,
	REG_OUT,
	REG_IN,
	NUM_REG,
};

/**
 * struct meson bank
 *
 * @name:	bank name
 * @first:	first pin of the bank
 * @last:	last pin of the bank
 * @regs:	couples of <reg offset, bit index> controlling the
 *		functionalities of the bank pins (pull, direction, value)
 *
 * A bank represents a set of pins controlled by a contiguous set of
 * bits in the domain registers.
 */
struct meson_bank {
	const char *name;
	unsigned int first;
	unsigned int last;
	struct meson_reg_offset regs[NUM_REG];
};

/**
 * struct meson_domain_data - domain platform data
 *
 * @name:	label for the domain
 * @pin_names:	names of the pins in the domain
 * @banks:	set of banks belonging to the domain
 * @funcs:	available pinmux functions
 * @groups:	available pinmux groups
 * @num_pins:	number of pins in the domain
 * @num_banks:	number of banks in the domain
 * @num_funcs:	number of available pinmux functions
 * @num_groups:	number of available pinmux groups
 *
 */
struct meson_domain_data {
	const char *name;
	const char **pin_names;
	struct meson_bank *banks;
	struct meson_pmx_func *funcs;
	struct meson_pmx_group *groups;
	unsigned int num_pins;
	unsigned int num_banks;
	unsigned int num_funcs;
	unsigned int num_groups;
};

/**
 * struct meson_domain
 *
 * @reg_mux:	registers for mux settings
 * @reg_pullen:	registers for pull-enable settings
 * @reg_pull:	registers for pull settings
 * @reg_gpio:	registers for gpio settings
 * @mux_size:	size of mux register range (in words)
 * @pullen_size:size of pull-enable register range
 * @pull_size:	size of pull register range
 * @gpio_size:	size of gpio register range
 * @chip:	gpio chip associated with the domain
 * @data;	platform data for the domain
 * @node:	device tree node for the domain
 * @gpio_range:	gpio range that maps domain gpios to the pin controller
 * @lock:	spinlock for accessing domain registers
 *
 * A domain represents a set of banks controlled by the same set of
 * registers. Typically there is a domain for the normal banks and
 * another one for the Always-On bus.
 */
struct meson_domain {
	void __iomem *reg_mux;
	void __iomem *reg_pullen;
	void __iomem *reg_pull;
	void __iomem *reg_gpio;

	unsigned int mux_size;
	unsigned int pullen_size;
	unsigned int pull_size;
	unsigned int gpio_size;

	struct gpio_chip chip;
	struct meson_domain_data *data;
	struct device_node *of_node;
	struct pinctrl_gpio_range gpio_range;

	spinlock_t lock;
};

struct meson_pinctrl {
	struct device *dev;
	struct pinctrl_dev *pcdev;
	struct pinctrl_desc desc;

	struct pinctrl_pin_desc *pins;
	struct meson_domain *domains;
	struct meson_pmx_func *funcs;
	struct meson_pmx_group *groups;

	unsigned int num_pins;
	unsigned int num_domains;
	unsigned int num_funcs;
	unsigned int num_groups;
};

#define PINS_END	0xffff

#define GROUP(_name, _reg, _bit, _pins...)				\
	{								\
		.name = #_name,						\
		.pins = (const unsigned int[]) { _pins, PINS_END },	\
		.is_gpio = false,					\
		.reg = _reg,						\
		.bit = _bit,						\
	}

#define GPIO_GROUP(_gpio)						\
	{								\
		.name = #_gpio,						\
		.pins = (const unsigned int[]){ _gpio, PINS_END },	\
		.is_gpio = true,					\
	}

#define FUNCTION(_name, _groups...)					\
	{								\
		.name = _name,						\
		.groups = (const char *[]) { _groups, NULL },		\
		.num_groups = 0,					\
	}

#define BANK(n, f, l, per, peb, pr, pb, dr, db, or, ob, ir, ib)		\
	{								\
		.name	= n,						\
		.first	= f,						\
		.last	= l,						\
		.regs	= {						\
			[REG_PULLEN]	= { per, peb },			\
			[REG_PULL]	= { pr, pb },			\
			[REG_DIR]	= { dr, db },			\
			[REG_OUT]	= { or, ob },			\
			[REG_IN]	= { ir, ib },			\
		},							\
	 }

#define MESON_PIN(x) PINCTRL_PIN(x, #x)

extern struct meson_domain_data meson8_domain_data[];
