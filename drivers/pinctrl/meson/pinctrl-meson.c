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

#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>

#include "../core.h"
#include "../pinctrl-utils.h"
#include "pinctrl-meson.h"

static void meson_domain_set_bit(struct meson_domain *domain,
				 void __iomem *addr, unsigned int bit,
				 unsigned int value)
{
	unsigned long flags;
	unsigned int data;

	spin_lock_irqsave(&domain->lock, flags);
	data = readl(addr);

	if (value)
		data |= BIT(bit);
	else
		data &= ~BIT(bit);

	writel(data, addr);
	spin_unlock_irqrestore(&domain->lock, flags);
}

static struct meson_domain *meson_pinctrl_get_domain(struct meson_pinctrl *pc,
						     int pin)
{
	struct meson_domain *domain;
	int i, j;

	for (i = 0; i < pc->num_domains; i++) {
		domain = &pc->domains[i];
		for (j = 0; j < domain->data->num_banks; j++) {
			if (pin >= domain->data->banks[j].first &&
			    pin < domain->data->banks[j].last)
				return domain;
		}
	}

	return NULL;
}

static int meson_pinctrl_calc_regnum_bit(struct meson_domain *domain,
					 unsigned pin, int reg_type,
					 unsigned int *reg_num,
					 unsigned int *bit)
{
	struct meson_bank *bank;
	int i, found = 0;

	for (i = 0; i < domain->data->num_banks; i++) {
		bank = &domain->data->banks[i];
		if (pin >= bank->first && pin <= bank->last) {
			found = 1;
			break;
		}
	}

	if (!found)
		return 1;

	*reg_num = bank->regs[reg_type].reg;
	*bit = bank->regs[reg_type].bit + pin - bank->first;

	return 0;
}

static void *meson_get_mux_reg(struct meson_domain *domain,
			       unsigned int reg_num)
{
	if (reg_num < domain->mux_size)
		return domain->reg_mux + 4 * reg_num;

	return NULL;
}

static int meson_get_groups_count(struct pinctrl_dev *pcdev)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	return pc->num_groups;
}

static const char *meson_get_group_name(struct pinctrl_dev *pcdev,
					unsigned selector)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	return pc->groups[selector].name;
}

static int meson_get_group_pins(struct pinctrl_dev *pcdev, unsigned selector,
				const unsigned **pins, unsigned *num_pins)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	*pins = pc->groups[selector].pins;
	*num_pins = pc->groups[selector].num_pins;

	return 0;
}

static void meson_pin_dbg_show(struct pinctrl_dev *pcdev, struct seq_file *s,
			       unsigned offset)
{
	seq_printf(s, " %s", dev_name(pcdev->dev));
}

static const struct pinctrl_ops meson_pctrl_ops = {
	.get_groups_count	= meson_get_groups_count,
	.get_group_name		= meson_get_group_name,
	.get_group_pins		= meson_get_group_pins,
	.dt_node_to_map		= pinconf_generic_dt_node_to_map_group,
	.dt_free_map		= pinctrl_utils_dt_free_map,
	.pin_dbg_show		= meson_pin_dbg_show,
};

static void meson_pmx_disable_other_groups(struct meson_pinctrl *pc,
					   unsigned int pin, int sel_group)
{
	struct meson_pmx_group *group;
	struct meson_domain *domain;
	void __iomem *reg;
	int i, j;

	for (i = 0; i < pc->num_groups; i++) {
		group = &pc->groups[i];

		if (group->is_gpio || i == sel_group)
			continue;

		for (j = 0; j < group->num_pins; j++) {
			if (group->pins[j] == pin) {
				domain = group->domain;
				reg = meson_get_mux_reg(domain, group->reg);
				if (reg)
					meson_domain_set_bit(domain, reg,
							     group->bit, 0);
				break;
			}
		}
	}
}

static int meson_pmx_enable(struct pinctrl_dev *pcdev, unsigned func_num,
			    unsigned group_num)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);
	struct meson_pmx_func *func = &pc->funcs[func_num];
	struct meson_pmx_group *group = &pc->groups[group_num];
	void __iomem *reg;
	int i;

	dev_dbg(pc->dev, "enable function %s, group %s\n", func->name,
		group->name);

	reg = meson_get_mux_reg(group->domain, group->reg);
	if (!reg)
		return -EINVAL;

	/* Disable other groups using the same pins */
	for (i = 0; i < group->num_pins; i++)
		meson_pmx_disable_other_groups(pc, group->pins[i], group_num);

	/*
	 * Function 0 (GPIO) is the default one and doesn't need any
	 * additional settings
	 */
	if (func_num)
		meson_domain_set_bit(group->domain, reg, group->bit, 1);

	return 0;
}

static int meson_pmx_request_gpio(struct pinctrl_dev *pcdev,
				  struct pinctrl_gpio_range *range,
				  unsigned offset)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	meson_pmx_disable_other_groups(pc, offset, -1);

	return 0;
}

static int meson_pmx_get_funcs_count(struct pinctrl_dev *pcdev)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	return pc->num_funcs;
}

static const char *meson_pmx_get_func_name(struct pinctrl_dev *pcdev,
					   unsigned selector)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	return pc->funcs[selector].name;
}

static int meson_pmx_get_groups(struct pinctrl_dev *pcdev, unsigned selector,
				const char * const **groups,
				unsigned * const num_groups)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);

	*groups = pc->funcs[selector].groups;
	*num_groups = pc->funcs[selector].num_groups;

	return 0;
}

static const struct pinmux_ops meson_pmx_ops = {
	.enable = meson_pmx_enable,
	.get_functions_count = meson_pmx_get_funcs_count,
	.get_function_name = meson_pmx_get_func_name,
	.get_function_groups = meson_pmx_get_groups,
	.gpio_request_enable = meson_pmx_request_gpio,
};

static int meson_pinctrl_calc_reg_bit(struct meson_domain *domain,
				      unsigned int pin, int reg_type,
				      void **reg, unsigned int *bit)
{
	unsigned int reg_num;
	int ret;

	*reg = NULL;

	ret = meson_pinctrl_calc_regnum_bit(domain, pin, reg_type,
					    &reg_num, bit);
	if (ret)
		return -EINVAL;

	if (reg_type == REG_PULLEN) {
		if (reg_num < domain->pullen_size)
			*reg = domain->reg_pullen + 4 * reg_num;
	} else {
		if (reg_num < domain->pull_size)
			*reg = domain->reg_pull + 4 * reg_num;
	}

	return *reg ? 0 : -EINVAL;
}

static int meson_pinconf_set(struct pinctrl_dev *pcdev, unsigned int pin,
			     unsigned long *configs, unsigned num_configs)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);
	void __iomem *pullen_reg, __iomem *pull_reg;
	unsigned int pullen_bit, pull_bit;
	enum pin_config_param param;
	struct meson_domain *domain;
	int i, ret;
	u16 arg;

	domain = meson_pinctrl_get_domain(pc, pin);
	if (!domain)
		return -EINVAL;

	ret = meson_pinctrl_calc_reg_bit(domain, pin, REG_PULL,
					 &pull_reg, &pull_bit);
	if (ret)
		return -EINVAL;

	ret = meson_pinctrl_calc_reg_bit(domain, pin, REG_PULLEN,
					 &pullen_reg, &pullen_bit);
	if (ret)
		return -EINVAL;

	for (i = 0; i < num_configs; i++) {
		param = pinconf_to_config_param(configs[i]);
		arg = pinconf_to_config_argument(configs[i]);

		switch (param) {
		case PIN_CONFIG_BIAS_DISABLE:
			dev_dbg(pc->dev, "pin %d bias-disable\n", pin);
			meson_domain_set_bit(domain, pullen_reg, pullen_bit, 0);
			break;
		case PIN_CONFIG_BIAS_PULL_UP:
			dev_dbg(pc->dev, "pin %d pull-up\n", pin);
			meson_domain_set_bit(domain, pullen_reg, pullen_bit, 1);
			meson_domain_set_bit(domain, pull_reg, pull_bit, 1);
			break;
		case PIN_CONFIG_BIAS_PULL_DOWN:
			dev_dbg(pc->dev, "pin %d pull-down\n", pin);
			meson_domain_set_bit(domain, pullen_reg, pullen_bit, 1);
			meson_domain_set_bit(domain, pull_reg, pull_bit, 0);
			break;
		default:
			return -ENOTSUPP;
		}
	}

	return 0;
}

static int meson_pinconf_get_pull(struct meson_pinctrl *pc, unsigned int pin)
{
	struct meson_domain *domain;
	unsigned int bit;
	int ret, conf;
	void *reg;

	domain = meson_pinctrl_get_domain(pc, pin);
	if (!domain)
		return -EINVAL;

	ret = meson_pinctrl_calc_reg_bit(domain, pin, REG_PULLEN, &reg, &bit);
	if (ret) {
		dev_err(pc->dev, "can't find register for pin %u\n", pin);
		return -EINVAL;
	}

	if (!(readl(reg) & BIT(bit))) {
		conf = PIN_CONFIG_BIAS_DISABLE;
	} else {
		ret = meson_pinctrl_calc_reg_bit(domain, pin, REG_PULL,
						 &reg, &bit);
		if (ret) {
			dev_err(pc->dev, "can't find register for pin %u\n",
				pin);
			return -EINVAL;
		}

		if (!(readl(reg) & BIT(bit)))
			conf = PIN_CONFIG_BIAS_PULL_DOWN;
		else
			conf = PIN_CONFIG_BIAS_PULL_UP;
	}

	return conf;
}

static int meson_pinconf_get(struct pinctrl_dev *pcdev, unsigned int pin,
			     unsigned long *config)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);
	enum pin_config_param param = pinconf_to_config_param(*config);
	u16 arg;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_UP:
		if (meson_pinconf_get_pull(pc, pin) == param)
			arg = 1;
		else
			return -EINVAL;
		break;
	default:
		return -ENOTSUPP;
	}

	*config = pinconf_to_config_packed(param, arg);

	return 0;
}

static int meson_pinconf_group_set(struct pinctrl_dev *pcdev,
				   unsigned int num_group,
				   unsigned long *configs, unsigned num_configs)
{
	struct meson_pinctrl *pc = pinctrl_dev_get_drvdata(pcdev);
	struct meson_pmx_group *group = &pc->groups[num_group];
	int i;

	dev_dbg(pc->dev, "set pinconf for group %s\n", group->name);

	for (i = 0; i < group->num_pins; i++) {
		meson_pinconf_set(pcdev, group->pins[i], configs,
				  num_configs);
	}

	return 0;
}

static int meson_pinconf_group_get(struct pinctrl_dev *pcdev,
				   unsigned int group, unsigned long *config)
{
	return -ENOSYS;
}

static const struct pinconf_ops meson_pinconf_ops = {
	.pin_config_get		= meson_pinconf_get,
	.pin_config_set		= meson_pinconf_set,
	.pin_config_group_get	= meson_pinconf_group_get,
	.pin_config_group_set	= meson_pinconf_group_set,
	.is_generic		= true,
};

static inline struct meson_domain *to_meson_domain(struct gpio_chip *chip)
{
	return container_of(chip, struct meson_domain, chip);
}

static int meson_gpio_request(struct gpio_chip *chip, unsigned gpio)
{
	return pinctrl_request_gpio(chip->base + gpio);
}

static void meson_gpio_free(struct gpio_chip *chip, unsigned gpio)
{
	pinctrl_free_gpio(chip->base + gpio);
}

static int meson_gpio_calc_reg_bit(struct meson_domain *domain, unsigned gpio,
				   int reg_type, void **reg, unsigned int *bit)
{
	unsigned int reg_num;
	int ret;

	ret = meson_pinctrl_calc_regnum_bit(domain, gpio, reg_type,
					    &reg_num, bit);

	if (ret)
		return -EINVAL;

	if (reg_num >= domain->gpio_size)
		return -EINVAL;

	*reg = domain->reg_gpio + 4 * reg_num;

	return 0;
}

static int meson_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	struct meson_domain *domain = to_meson_domain(chip);
	void __iomem *addr;
	unsigned int bit;
	int ret;

	ret = meson_gpio_calc_reg_bit(domain, chip->base + gpio, REG_DIR,
				      &addr, &bit);
	if (ret)
		return ret;

	meson_domain_set_bit(domain, addr, bit, 1);

	return 0;
}

static int meson_gpio_direction_output(struct gpio_chip *chip, unsigned gpio,
				       int value)
{
	struct meson_domain *domain = to_meson_domain(chip);
	void __iomem *addr;
	unsigned int bit;
	int ret;

	ret = meson_gpio_calc_reg_bit(domain, chip->base + gpio, REG_DIR,
				      &addr, &bit);
	if (ret)
		return ret;

	meson_domain_set_bit(domain, addr, bit, 0);

	return 0;
}

static void meson_gpio_set(struct gpio_chip *chip, unsigned gpio, int value)
{
	struct meson_domain *domain = to_meson_domain(chip);
	void __iomem *addr;
	unsigned int bit;

	if (meson_gpio_calc_reg_bit(domain, chip->base + gpio, REG_OUT,
				    &addr, &bit))
		return;

	meson_domain_set_bit(domain, addr, bit, value);
}

static int meson_gpio_get(struct gpio_chip *chip, unsigned gpio)
{
	struct meson_domain *domain = to_meson_domain(chip);
	void __iomem *addr;
	unsigned int bit;

	if (meson_gpio_calc_reg_bit(domain, chip->base + gpio, REG_IN,
				    &addr, &bit))
		return 0;

	return (readl(addr) >> bit) & 1;
}

static const struct of_device_id meson_pinctrl_dt_match[] = {
	{
		.compatible = "amlogic,meson8-pinctrl",
		.data = meson8_domain_data,
	},
	{ },
};
MODULE_DEVICE_TABLE(of, meson_pinctrl_dt_match);

static int meson_gpio_of_xlate(struct gpio_chip *chip,
			       const struct of_phandle_args *gpiospec,
			       u32 *flags)
{
	unsigned gpio = gpiospec->args[0];

	if (gpio < chip->base || gpio >= chip->base + chip->ngpio)
		return -EINVAL;

	if (flags)
		*flags = gpiospec->args[1];

	return gpio - chip->base;
}

static struct meson_domain_data *get_domain_data(struct device_node *node,
						 struct meson_domain_data *data)
{
	while (data->name) {
		if (!strcmp(node->name, data->name))
			return data;
		data++;
	}

	return NULL;
}

static int meson_pinctrl_prepare_data(struct meson_pinctrl *pc)
{
	struct meson_domain_data *data;
	int i, j, pin = 0, func = 0, group = 0;

	/* Copy pin definitions from domains to pinctrl device */
	pc->pins = devm_kzalloc(pc->dev, pc->num_pins *
				sizeof(struct pinctrl_pin_desc), GFP_KERNEL);

	for (i = 0; i < pc->num_domains; i++) {
		data = pc->domains[i].data;
		for (j = 0; j < data->num_pins; j++) {
			pc->pins[pin].number = pin;
			pc->pins[pin++].name = data->pin_names[j];
		}
	}

	pc->num_groups = 0;
	pc->num_funcs = 0;

	for (i = 0; i < pc->num_domains; i++) {
		data = pc->domains[i].data;
		pc->num_groups += data->num_groups;
		pc->num_funcs += data->num_funcs;
	}

	/* Copy group and function definitions from domains to pinctrl */
	pc->groups = devm_kzalloc(pc->dev, pc->num_groups *
				  sizeof(struct meson_pmx_group), GFP_KERNEL);
	pc->funcs = devm_kzalloc(pc->dev, pc->num_funcs *
				  sizeof(struct meson_pmx_func), GFP_KERNEL);
	if (!pc->groups || !pc->funcs)
		return -ENOMEM;

	for (i = 0; i < pc->num_domains; i++) {
		data = pc->domains[i].data;

		for (j = 0; j < data->num_groups; j++) {
			memcpy(&pc->groups[group], &data->groups[j],
			       sizeof(struct meson_pmx_group));
			pc->groups[group++].domain = &pc->domains[i];
		}

		for (j = 0; j < data->num_funcs; j++) {
			memcpy(&pc->funcs[func++], &data->funcs[j],
			       sizeof(struct meson_pmx_func));
		}
	}

	/* Count pins in groups */
	for (i = 0; i < pc->num_groups; i++) {
		for (j = 0; ; j++) {
			if (pc->groups[i].pins[j] == PINS_END) {
				pc->groups[i].num_pins = j;
				break;
			}
		}
	}

	/* Count groups in functions */
	for (i = 0; i < pc->num_funcs; i++) {
		for (j = 0; ; j++) {
			if (!pc->funcs[i].groups[j]) {
				pc->funcs[i].num_groups = j;
				break;
			}
		}
	}

	return 0;
}

static void __iomem *meson_map_resource(struct meson_pinctrl *pc,
					struct device_node *node,
					char *name, unsigned int *size)
{
	struct resource res;
	int i;

	i = of_property_match_string(node, "reg-names", name);
	if (of_address_to_resource(node, i, &res))
		return NULL;

	*size = resource_size(&res) / 4;
	return devm_ioremap_resource(pc->dev, &res);
}

static int meson_pinctrl_parse_dt(struct meson_pinctrl *pc,
				  struct device_node *node,
				  struct meson_domain_data *data)
{
	struct device_node *np;
	struct meson_domain *domain;
	int i;

	for_each_child_of_node(node, np) {
		if (!of_find_property(np, "gpio-controller", NULL))
			continue;
		pc->num_domains++;
	}

	pc->domains = devm_kzalloc(pc->dev, pc->num_domains *
				   sizeof(struct meson_domain),
				   GFP_KERNEL);
	if (!pc->domains)
		return -ENOMEM;

	i = 0;
	for_each_child_of_node(node, np) {
		if (!of_find_property(np, "gpio-controller", NULL))
			continue;

		domain = &pc->domains[i];
		domain->reg_mux = meson_map_resource(pc, np, "mux",
						     &domain->mux_size);
		if (!domain->reg_mux) {
			dev_err(pc->dev, "mux registers not found\n");
			return -ENODEV;
		}

		domain->reg_pull = meson_map_resource(pc, np, "pull",
						      &domain->pull_size);
		if (!domain->reg_pull) {
			dev_err(pc->dev, "pull registers not found\n");
			return -ENODEV;
		}

		domain->reg_pullen = meson_map_resource(pc, np, "pull-enable",
							&domain->pullen_size);
		if (!domain->reg_pullen) {
			/* Use pull region if pull-enable one is not present */
			domain->reg_pullen = domain->reg_pull;
			domain->pullen_size = domain->pull_size;
		}

		domain->reg_gpio = meson_map_resource(pc, np, "gpio",
						      &domain->gpio_size);
		if (!domain->reg_gpio) {
			dev_err(pc->dev, "gpio registers not found\n");
			return -ENODEV;
		}

		domain->data = get_domain_data(np, data);
		if (!domain->data) {
			dev_err(pc->dev, "domain data not found for node %s\n",
				np->name);
			return -ENODEV;
		}

		domain->of_node = np;
		pc->num_pins += domain->data->num_pins;
		i++;
	}

	meson_pinctrl_prepare_data(pc);

	return 0;
}

static int meson_gpiolib_register(struct meson_pinctrl *pc)
{
	struct meson_domain *domain;
	unsigned int base = 0;
	int i, ret;

	for (i = 0; i < pc->num_domains; i++) {
		domain = &pc->domains[i];

		domain->chip.label = domain->data->name;
		domain->chip.dev = pc->dev;
		domain->chip.request = meson_gpio_request;
		domain->chip.free = meson_gpio_free;
		domain->chip.direction_input = meson_gpio_direction_input;
		domain->chip.direction_output = meson_gpio_direction_output;
		domain->chip.get = meson_gpio_get;
		domain->chip.set = meson_gpio_set;
		domain->chip.base = base;
		domain->chip.ngpio = domain->data->num_pins;
		domain->chip.names = domain->data->pin_names;
		domain->chip.can_sleep = false;
		domain->chip.of_node = domain->of_node;
		domain->chip.of_gpio_n_cells = 2;
		domain->chip.of_xlate = meson_gpio_of_xlate;

		ret = gpiochip_add(&domain->chip);
		if (ret < 0) {
			dev_err(pc->dev, "can't add gpio chip %s\n",
				domain->data->name);
			goto fail;
		}

		domain->gpio_range.name = domain->data->name;
		domain->gpio_range.id = i;
		domain->gpio_range.base = base;
		domain->gpio_range.pin_base = base;
		domain->gpio_range.npins = domain->data->num_pins;
		domain->gpio_range.gc = &domain->chip;

		pinctrl_add_gpio_range(pc->pcdev, &domain->gpio_range);
		base += domain->data->num_pins;
	}

	return 0;
fail:
	for (i--; i >= 0; i--)
		gpiochip_remove(&pc->domains[i].chip);

	return ret;
}

static int meson_pinctrl_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct device *dev = &pdev->dev;
	struct meson_domain_data *data;
	struct meson_pinctrl *pc;
	int i, j, ret;

	pc = devm_kzalloc(dev, sizeof(struct meson_pinctrl), GFP_KERNEL);
	if (!pc)
		return -ENOMEM;

	match = of_match_node(meson_pinctrl_dt_match, pdev->dev.of_node);
	data = (struct meson_domain_data *)match->data;
	pc->dev = dev;

	ret = meson_pinctrl_parse_dt(pc, pdev->dev.of_node, data);
	if (ret)
		return ret;

	/* FIXME */
	for (i = 0; i < pc->num_domains; i++)
		for (j = 0; j < pc->domains[i].mux_size; j++)
			writel(0, pc->domains[i].reg_mux + j * 4);
	/* END */

	pc->desc.name		= "pinctrl-meson";
	pc->desc.owner		= THIS_MODULE;
	pc->desc.pctlops	= &meson_pctrl_ops;
	pc->desc.pmxops		= &meson_pmx_ops;
	pc->desc.confops	= &meson_pinconf_ops;
	pc->desc.pins		= pc->pins;
	pc->desc.npins		= pc->num_pins;

	pc->pcdev = pinctrl_register(&pc->desc, pc->dev, pc);
	if (!pc->pcdev) {
		dev_err(pc->dev, "can't register pinctrl device");
		return -EINVAL;
	}

	ret = meson_gpiolib_register(pc);
	if (ret) {
		pinctrl_unregister(pc->pcdev);
		return ret;
	}

	return 0;
}

static struct platform_driver meson_pinctrl_driver = {
	.probe		= meson_pinctrl_probe,
	.driver = {
		.name	= "meson-pinctrl",
		.of_match_table = meson_pinctrl_dt_match,
	},
};

static int __init meson_pinctrl_drv_register(void)
{
	return platform_driver_register(&meson_pinctrl_driver);
}
postcore_initcall(meson_pinctrl_drv_register);

MODULE_AUTHOR("Beniamino Galvani <b.galvani@gmail.com>");
MODULE_DESCRIPTION("Amlogic Meson pinctrl driver");
MODULE_LICENSE("GPL v2");
