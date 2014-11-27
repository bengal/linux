#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/io.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "irqchip.h"

#define REG_EDGE_POL	0x00
#define REG_GPIO_SEL0	0x04
#define REG_GPIO_SEL1	0x08
#define REG_FILTER	0x0c

#define REG_EDGE_POL_MASK(x)	(BIT(x) | BIT(16 + (x)))
#define REG_EDGE_POL_LEVEL(x)	0
#define REG_EDGE_POL_EDGE(x)	BIT(x)
#define REG_EDGE_POL_HIGH(x)	0
#define REG_EDGE_POL_LOW(x)	BIT(16 + (x))

struct meson_irqchip_data {
	void __iomem		*regs;
	spinlock_t		lock;
	int			num_gic_irqs;
	struct of_phandle_args	*gic_irqs;
	unsigned long		gic_irq_map;
};

static int meson_irq_set_type(struct irq_data *data, unsigned int type)
{
	return 0;
}

static struct irq_chip meson_irq_chip = {
	.name			= "meson-irq",
	.irq_mask		= irq_chip_mask_parent,
	.irq_unmask		= irq_chip_unmask_parent,
	.irq_eoi		= irq_chip_eoi_parent,
	.irq_set_type		= meson_irq_set_type,
	.irq_retrigger		= irq_chip_retrigger_hierarchy,
	.irq_set_affinity	= irq_chip_set_affinity_parent,
};

static int meson_map_free_gic_irq(struct irq_domain *domain,
				  irq_hw_number_t hwirq)
{
	struct meson_irqchip_data *chip = domain->host_data;
	int index, reg;
	u32 val;

	index = find_first_bit(&chip->gic_irq_map, BITS_PER_LONG);
	if (index == BITS_PER_LONG) {
		pr_err("meson-irq: no GIC interrupt found");
		return -ENOSPC;
	}

	pr_debug("meson-irq: found free GIC interrupt %d\n", index);
	clear_bit(index, &chip->gic_irq_map);

	/* Setup pin */
	reg = index < 4 ? REG_GPIO_SEL0 : REG_GPIO_SEL1;
	val = readl(chip->regs + reg);
	val &= ~(0xff << (index % 4) * 8);
	val |= hwirq << (index % 4) * 8;
	writel(val, chip->regs + reg);

	/* Set default trigger type */
	val = readl(chip->regs + REG_EDGE_POL);
	val &= ~REG_EDGE_POL_MASK(index);
	val |= REG_EDGE_POL_LEVEL(x) | REG_EDGE_POL_HIGH(x);
	writel(val, chip->regs + REG_EDGE_POL);

	/* Set filter */
	val = readl(chip->regs + REG_FILTER);
	val |= 7 << index * 4;
	writel(val, chip->regs + REG_FILTER);

	return index;
}

static int meson_irq_domain_alloc(struct irq_domain *domain, unsigned int virq,
				  unsigned int nr_irqs, void *arg)
{
	struct meson_irqchip_data *chip = domain->host_data;
	struct of_phandle_args *irq_data = arg;
	struct of_phandle_args gic_data;
	irq_hw_number_t hwirq;
	int index, ret, i;

	if (irq_data->args_count != 3)
		return -EINVAL;

	hwirq = irq_data->args[1];

	for (i = 0; i < nr_irqs; i++) {
		index = meson_map_free_gic_irq(domain, hwirq);
		if (index < 0)
			return index;

		irq_domain_set_hwirq_and_chip(domain, virq + i,
					      hwirq + i,
					      &meson_irq_chip,
					      domain->host_data);
		gic_data = chip->gic_irqs[index];
		ret = irq_domain_alloc_irqs_parent(domain, virq, nr_irqs,
						   &gic_data);
	}

	return 0;
}

static void meson_irq_activate(struct irq_domain *domain,
			       struct irq_data *irq_data)
{

}

static void meson_irq_deactivate(struct irq_domain *domain,
				 struct irq_data *irq_data)
{

}

static struct irq_domain_ops meson_irq_domain_ops = {
	.alloc		= meson_irq_domain_alloc,
	.free		= irq_domain_free_irqs_common,
	.activate	= meson_irq_activate,
	.deactivate	= meson_irq_deactivate,
};

static int __init meson_irq_of_init(struct device_node *node,
				     struct device_node *parent)
{
	struct irq_domain *domain, *domain_parent;
	struct meson_irqchip_data *chip;
	int i, ret;

	domain_parent = irq_find_host(parent);
	if (!domain_parent) {
		pr_err("meson-irq: interrupt-parent not found\n");
		return -EINVAL;
	}

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->regs = of_io_request_and_map(node, 0, "irq-meson");
	if (IS_ERR(chip->regs)) {
		ret = PTR_ERR(chip->regs);
		pr_err("meson-irq: can't request and map registers: %d\n", ret);
		return ret;
	}

	chip->num_gic_irqs = of_irq_count(node);
	if (!chip->num_gic_irqs) {
		pr_err("meson-irq: no parent interrupt specified\n");
		return -EINVAL;
	}

	chip->gic_irqs = kzalloc(sizeof(struct of_phandle_args) *
				 chip->num_gic_irqs, GFP_KERNEL);
	if (!chip->gic_irqs)
		return -ENOMEM;

	for (i = 0; i < chip->num_gic_irqs; i++)
		of_irq_parse_one(node, i, &chip->gic_irqs[i]);

	chip->gic_irq_map = BIT(chip->num_gic_irqs) - 1;
	domain = irq_domain_add_hierarchy(domain_parent, 0, 136,
					  node, &meson_irq_domain_ops,
					  chip);
	return 0;
}

IRQCHIP_DECLARE(meson_irq, "amlogic,meson-irq", meson_irq_of_init);
