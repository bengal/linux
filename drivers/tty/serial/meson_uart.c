/*
 *  Based on meson_uart.c, by AMLOGIC, INC.
 *
 * Copyright (C) 2014 Carlo Caione <carlo@caione.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/clk.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

/* Register offsets */
#define AML_UART_WFIFO			0x00
#define AML_UART_RFIFO			0x04
#define AML_UART_CONTROL		0x08
#define AML_UART_STATUS			0x0c
#define AML_UART_MISC			0x10
#define AML_UART_REG5			0x14

/* AML_UART_CONTROL bits */
#define AML_UART_TX_EN			BIT(12)
#define AML_UART_RX_EN			BIT(13)
#define AML_UART_TX_RST			BIT(22)
#define AML_UART_RX_RST			BIT(23)
#define AML_UART_CLR_ERR		BIT(24)
#define AML_UART_RX_INT_EN		BIT(27)
#define AML_UART_TX_INT_EN		BIT(28)
#define AML_UART_DATA_LEN_MASK		(0x03 << 20)
#define AML_UART_DATA_LEN_8BIT		(0x00 << 20)
#define AML_UART_DATA_LEN_7BIT		(0x01 << 20)
#define AML_UART_DATA_LEN_6BIT		(0x02 << 20)
#define AML_UART_DATA_LEN_5BIT		(0x03 << 20)

/* AML_UART_STATUS bits */
#define AML_UART_PARITY_ERR		BIT(16)
#define AML_UART_FRAME_ERR		BIT(17)
#define AML_UART_OVERFLOW_ERR		BIT(18)
#define AML_UART_RX_EMPTY		BIT(20)
#define AML_UART_TX_FULL		BIT(21)
#define AML_UART_TX_EMPTY		BIT(22)

/* AML_UART_CONTROL bits */
#define AML_UART_TWO_WIRE_EN		BIT(15)
#define AML_UART_PARITY_TYPE		BIT(18)
#define AML_UART_PARITY_EN		BIT(19)
#define AML_UART_CLEAR_ERR		BIT(24)
#define AML_UART_STOP_BIN_LEN_MASK	(0x03 << 16)
#define AML_UART_STOP_BIN_1SB		(0x00 << 16)
#define AML_UART_STOP_BIN_2SB		(0x01 << 16)

/* AML_UART_MISC bits */
#define AML_UART_XMIT_IRQ(c)		(((c) & 0xff) << 8)
#define AML_UART_RECV_IRQ(c)		((c) & 0xff)

/* AML_UART_REG5 bits */
#define AML_UART_BAUD_MASK		0x7fffff
#define AML_UART_BAUD_USE		BIT(23)

#define AML_UART_PORT_NUM		6
#define AML_UART_DEV_NAME		"ttyAML"


static struct uart_driver meson_uart_driver;

struct meson_uart_port {
	struct uart_port port;
	struct work_struct tqueue;
	int mem_size;
	int count;
	int rx_cnt;
	int rx_error;
};

static struct meson_uart_port *meson_ports[AML_UART_PORT_NUM];

static void meson_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int meson_uart_get_mctrl(struct uart_port *port)
{
	return TIOCM_CTS;
}

static unsigned int meson_uart_tx_empty(struct uart_port *port)
{
	u32 val;

	val = readl(port->membase + AML_UART_STATUS);
	return (val & AML_UART_TX_EMPTY) ? TIOCSER_TEMT : 0;
}

static void meson_uart_stop_tx(struct uart_port *port)
{
	u32 val;

	val = readl(port->membase + AML_UART_CONTROL);
	val &= ~AML_UART_TX_EN;
	writel(val, port->membase + AML_UART_CONTROL);
}

static void meson_uart_stop_rx(struct uart_port *port)
{
	u32 val;

	val = readl(port->membase + AML_UART_CONTROL);
	val &= ~AML_UART_RX_EN;
	writel(val, port->membase + AML_UART_CONTROL);
}

static void meson_uart_shutdown(struct uart_port *port)
{
	unsigned long flags;
	u32 val;

	free_irq(port->irq, port);

	spin_lock_irqsave(&port->lock, flags);

	val = readl(port->membase + AML_UART_CONTROL);
	val &= ~(AML_UART_RX_EN | AML_UART_TX_EN);
	val &= ~(AML_UART_RX_INT_EN | AML_UART_TX_INT_EN);
	writel(val, port->membase + AML_UART_CONTROL);

	spin_unlock_irqrestore(&port->lock, flags);
}

static void meson_uart_start_tx(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;
	unsigned int ch;

	if (port->x_char) {
		writel(port->x_char, port->membase + AML_UART_WFIFO);
		port->x_char = 0;
		goto out;
	}

	if (uart_circ_empty(xmit) || uart_tx_stopped(port))
		goto out;

	while (!uart_circ_empty(xmit)) {
		if (!(readl(port->membase + AML_UART_STATUS)
					& AML_UART_TX_FULL)) {
			ch = xmit->buf[xmit->tail];
			writel(ch, port->membase + AML_UART_WFIFO);
			xmit->tail = (xmit->tail+1) & (SERIAL_XMIT_SIZE - 1);
		} else {
			break;
		}
	}

out:
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);
}

static void meson_receive_chars(struct meson_uart_port *mup)
{
	struct tty_port *tport = &mup->port.state->port;
	struct uart_port *port = &mup->port;
	u32 status, mode, rx;

	status = readl(port->membase + AML_UART_STATUS);
	if (status & AML_UART_OVERFLOW_ERR) {
		mup->rx_error |= AML_UART_OVERFLOW_ERR;
		mode = readl(port->membase + AML_UART_CONTROL);
		mode |= AML_UART_CLEAR_ERR;
		writel(mode, port->membase + AML_UART_CONTROL);
	} else if (status & AML_UART_FRAME_ERR) {
		mup->rx_error |= AML_UART_FRAME_ERR;
		mode = readl(port->membase + AML_UART_CONTROL);
		mode |= AML_UART_CLEAR_ERR;
		writel(mode, port->membase + AML_UART_CONTROL);
	} else if (status & AML_UART_PARITY_ERR) {
		mup->rx_error |= AML_UART_PARITY_ERR;
		mode = readl(port->membase + AML_UART_CONTROL);
		mode |= AML_UART_CLEAR_ERR;
		writel(mode, port->membase + AML_UART_CONTROL);
	}

	do {
		rx = readl(port->membase + AML_UART_RFIFO);
		rx &= 0xff;

		tty_insert_flip_char(tport, rx, TTY_NORMAL);

		mup->rx_cnt++;

	} while (!(readl(port->membase + AML_UART_STATUS) & AML_UART_RX_EMPTY));
}

static void meson_bh_receive_chars(struct meson_uart_port *mup)
{
	struct tty_port *tport = &mup->port.state->port;
	unsigned long flag = TTY_NORMAL;

	if (mup->rx_error & AML_UART_OVERFLOW_ERR)
		flag = TTY_OVERRUN;
	else if (mup->rx_error & AML_UART_FRAME_ERR)
		flag = TTY_FRAME;
	else if (mup->rx_error & AML_UART_PARITY_ERR)
		flag = TTY_PARITY;

	mup->rx_error = 0;
	if (mup->rx_cnt) {
		mup->rx_cnt = 0;
		tty_flip_buffer_push(tport);
	}
}

static void meson_uart_workqueue(struct work_struct *work)
{
	struct meson_uart_port *mup = container_of(work,
						   struct meson_uart_port,
						   tqueue);
	struct uart_port *port = &mup->port;
	u32 val;

	if (mup->rx_cnt > 0)
		meson_bh_receive_chars(mup);

	val = readl(port->membase + AML_UART_STATUS);
	if (val & AML_UART_FRAME_ERR) {
		val &= ~AML_UART_FRAME_ERR;
		writel(val, port->membase + AML_UART_STATUS);
	}

	if (val & AML_UART_OVERFLOW_ERR) {
		val &= ~AML_UART_OVERFLOW_ERR;
		writel(val, port->membase + AML_UART_STATUS);
	}
}

static irqreturn_t meson_uart_interrupt(int irq, void *dev_id)
{
	struct meson_uart_port *mup = (struct meson_uart_port *)dev_id;
	struct uart_port *port = &mup->port;

	spin_lock(&port->lock);

	if (!(readl(port->membase + AML_UART_STATUS) & AML_UART_RX_EMPTY))
		meson_receive_chars(mup);

	if (!(readl(port->membase + AML_UART_STATUS) & AML_UART_TX_FULL))
		meson_uart_start_tx(port);

	spin_unlock(&port->lock);

	schedule_work(&mup->tqueue);

	return IRQ_HANDLED;
}

static const char *meson_uart_type(struct uart_port *port)
{
	return (port->type == PORT_MESON) ? "meson_uart" : NULL;
}

static int meson_uart_startup(struct uart_port *port)
{
	struct meson_uart_port *mup = meson_ports[port->line];
	u32 val;
	int ret = 0;

	INIT_WORK(&mup->tqueue, meson_uart_workqueue);

	mup->count = 0;
	mup->rx_cnt = 0;
	mup->rx_error = 0;

	val = readl(port->membase + AML_UART_CONTROL);
	val |= (AML_UART_RX_RST | AML_UART_TX_RST | AML_UART_CLR_ERR);
	writel(val, port->membase + AML_UART_CONTROL);

	val &= ~(AML_UART_RX_RST | AML_UART_TX_RST | AML_UART_CLR_ERR);
	writel(val, port->membase + AML_UART_CONTROL);

	val |= (AML_UART_RX_EN | AML_UART_TX_EN);
	writel(val, port->membase + AML_UART_CONTROL);

	val |= (AML_UART_RX_INT_EN | AML_UART_TX_INT_EN);
	writel(val, port->membase + AML_UART_CONTROL);

	val = (AML_UART_RECV_IRQ(1) | AML_UART_XMIT_IRQ(port->fifosize / 2));
	writel(val, port->membase + AML_UART_MISC);

	ret = request_irq(port->irq, meson_uart_interrupt, 0,
			  meson_uart_type(port), mup);

	return ret;
}

static void meson_uart_change_speed(struct uart_port *up, unsigned long baud)
{
	u32 val;

	while (!(readl(up->membase + AML_UART_STATUS) & AML_UART_TX_EMPTY))
		cpu_relax();

	val = readl(up->membase + AML_UART_REG5);
	val &= ~AML_UART_BAUD_MASK;
	val = ((up->uartclk * 10 / (baud * 4) + 5) / 10) - 1;
	val |= AML_UART_BAUD_USE;
	writel(val, up->membase + AML_UART_REG5);
}

static void meson_uart_set_termios(struct uart_port *port,
				   struct ktermios *termios,
				   struct ktermios *old)
{
	unsigned int cflags, baud;
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&port->lock, flags);

	cflags = termios->c_cflag;
	val = readl(port->membase + AML_UART_CONTROL);

	val &= ~AML_UART_DATA_LEN_MASK;
	switch (cflags & CSIZE) {
	case CS8:
		val |= AML_UART_DATA_LEN_8BIT;
		break;
	case CS7:
		val |= AML_UART_DATA_LEN_7BIT;
		break;
	case CS6:
		val |= AML_UART_DATA_LEN_6BIT;
		break;
	case CS5:
		val |= AML_UART_DATA_LEN_5BIT;
		break;
	}

	if (cflags & PARENB)
		val |= AML_UART_PARITY_EN;
	else
		val &= ~AML_UART_PARITY_EN;

	if (cflags & PARODD)
		val |= AML_UART_PARITY_TYPE;
	else
		val &= ~AML_UART_PARITY_TYPE;

	val &= ~AML_UART_STOP_BIN_LEN_MASK;
	if (cflags & CSTOPB)
		val |= AML_UART_STOP_BIN_2SB;
	else
		val &= ~AML_UART_STOP_BIN_1SB;

	if (cflags & CRTSCTS)
		val &= ~AML_UART_TWO_WIRE_EN;
	else
		val |= AML_UART_TWO_WIRE_EN;

	writel(val, port->membase + AML_UART_CONTROL);

	baud = uart_get_baud_rate(port, termios, old, 9600, 115200);
	meson_uart_change_speed(port, baud);

	uart_update_timeout(port, termios->c_cflag, baud);
	spin_unlock_irqrestore(&port->lock, flags);
}

static int meson_uart_verify_port(struct uart_port *port,
				  struct serial_struct *ser)
{
	int ret = 0;

	if (port->type != PORT_MESON)
		ret = -EINVAL;
	if (port->irq != ser->irq)
		ret = -EINVAL;
	if (ser->baud_base < 9600)
		ret = -EINVAL;
	return ret;
}

static void meson_uart_release_port(struct uart_port *port)
{
	if (port->flags & UPF_IOREMAP) {
		iounmap(port->membase);
		port->membase = NULL;
	}
}

static int meson_uart_request_port(struct uart_port *port)
{
	struct meson_uart_port *mup = meson_ports[port->line];

	if (!devm_request_mem_region(port->dev, port->mapbase, mup->mem_size,
				     dev_name(port->dev))) {
		dev_err(port->dev, "Memory region busy\n");
		return -EBUSY;
	}

	if (port->flags & UPF_IOREMAP) {
		port->membase = devm_ioremap_nocache(port->dev,
						     port->mapbase,
						     mup->mem_size);
		if (port->membase == NULL)
			return -ENOMEM;
	}

	return 0;
}

static void meson_uart_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE) {
		port->type = PORT_MESON;
		meson_uart_request_port(port);
	}
}

static struct uart_ops meson_uart_ops = {
	.set_mctrl      = meson_uart_set_mctrl,
	.get_mctrl      = meson_uart_get_mctrl,
	.tx_empty	= meson_uart_tx_empty,
	.start_tx	= meson_uart_start_tx,
	.stop_tx	= meson_uart_stop_tx,
	.stop_rx	= meson_uart_stop_rx,
	.startup	= meson_uart_startup,
	.shutdown	= meson_uart_shutdown,
	.set_termios	= meson_uart_set_termios,
	.type		= meson_uart_type,
	.config_port	= meson_uart_config_port,
	.request_port	= meson_uart_request_port,
	.release_port	= meson_uart_release_port,
	.verify_port	= meson_uart_verify_port,
};

#ifdef CONFIG_SERIAL_MESON_CONSOLE

static void meson_console_putchar(struct uart_port *port, int ch)
{
	if (!port->membase)
		return;

	while (readl(port->membase + AML_UART_STATUS) & AML_UART_TX_FULL)
		cpu_relax();
	writel(ch, port->membase + AML_UART_WFIFO);
}

static void meson_serial_console_write(struct console *co, const char *s,
				       u_int count)
{
	struct meson_uart_port *mup;
	struct uart_port *port;
	unsigned long flags;
	int locked;

	mup = meson_ports[co->index];
	if (!mup)
		return;

	port = &mup->port;

	local_irq_save(flags);
	if (port->sysrq) {
		locked = 0;
	} else if (oops_in_progress) {
		locked = spin_trylock(&port->lock);
	} else {
		spin_lock(&port->lock);
		locked = 1;
	}

	uart_console_write(port, s, count, meson_console_putchar);

	if (locked)
		spin_unlock(&port->lock);
	local_irq_restore(flags);
}

static int meson_serial_console_setup(struct console *co, char *options)
{
	struct meson_uart_port *mup;
	struct uart_port *up;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (co->index < 0 || co->index >= AML_UART_PORT_NUM)
		return -EINVAL;

	mup = meson_ports[co->index];
	if (!mup)
		return -ENODEV;

	up = &mup->port;
	if (!up->membase)
		return -ENODEV;

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(up, co, baud, parity, bits, flow);
}

static struct console meson_serial_console = {
	.name		= AML_UART_DEV_NAME,
	.write		= meson_serial_console_write,
	.device		= uart_console_device,
	.setup		= meson_serial_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &meson_uart_driver,
};

static int __init meson_serial_console_init(void)
{
	register_console(&meson_serial_console);
	return 0;
}
console_initcall(meson_serial_console_init);

#define MESON_SERIAL_CONSOLE	(&meson_serial_console)
#else
#define MESON_SERIAL_CONSOLE	NULL
#endif

static struct uart_driver meson_uart_driver = {
	.owner		= THIS_MODULE,
	.driver_name	= "meson_uart",
	.dev_name	= AML_UART_DEV_NAME,
	.nr		= AML_UART_PORT_NUM,
	.cons		= MESON_SERIAL_CONSOLE,
};

static int meson_uart_probe(struct platform_device *pdev)
{
	struct resource *res_mem, *res_irq;
	struct meson_uart_port *mup;
	struct uart_port *up;
	struct clk *clk;
	int ret = 0;

	if (pdev->dev.of_node)
		pdev->id = of_alias_get_id(pdev->dev.of_node, "serial");

	if (pdev->id < 0 || pdev->id >= AML_UART_PORT_NUM)
		return -EINVAL;

	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res_mem)
		return -ENODEV;

	res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res_irq)
		return -ENODEV;

	if (meson_ports[pdev->id]) {
		dev_err(&pdev->dev, "port %d already allocated\n", pdev->id);
		return -EBUSY;
	}

	mup = devm_kzalloc(&pdev->dev, sizeof(struct meson_uart_port),
			   GFP_KERNEL);
	if (!mup)
		return -ENOMEM;

	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	mup->mem_size = resource_size(res_mem);

	up = &mup->port;
	up->uartclk = clk_get_rate(clk);
	up->iotype = UPIO_MEM;
	up->mapbase = res_mem->start;
	up->irq = res_irq->start;
	up->flags = UPF_BOOT_AUTOCONF | UPF_IOREMAP | UPF_LOW_LATENCY;
	up->dev = &pdev->dev;
	up->line = pdev->id;
	up->type = PORT_MESON;
	up->x_char = 0;
	up->ops = &meson_uart_ops;
	up->fifosize = 64;

	meson_ports[pdev->id] = mup;
	platform_set_drvdata(pdev, mup);

	ret = uart_add_one_port(&meson_uart_driver, up);
	if (ret)
		meson_ports[pdev->id] = NULL;

	return ret;
}

static int meson_uart_remove(struct platform_device *pdev)
{
	struct meson_uart_port *mup;

	mup = platform_get_drvdata(pdev);
	uart_remove_one_port(&meson_uart_driver, &mup->port);
	meson_ports[pdev->id] = NULL;

	return 0;
}


static const struct of_device_id meson_uart_dt_match[] = {
	{ .compatible = "amlogic,meson-uart" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, meson_uart_dt_match);

static  struct platform_driver meson_uart_platform_driver = {
	.probe		= meson_uart_probe,
	.remove		= meson_uart_remove,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "meson_uart",
		.of_match_table	= meson_uart_dt_match,
	},
};

static int __init meson_uart_init(void)
{
	int ret;

	ret = uart_register_driver(&meson_uart_driver);
	if (ret)
		return ret;

	ret = platform_driver_register(&meson_uart_platform_driver);
	if (ret)
		uart_unregister_driver(&meson_uart_driver);

	return ret;
}

static void __exit meson_uart_exit(void)
{
	platform_driver_unregister(&meson_uart_platform_driver);
	uart_unregister_driver(&meson_uart_driver);
}

module_init(meson_uart_init);
module_exit(meson_uart_exit);

MODULE_AUTHOR("Carlo Caione <carlo@caione.org>");
MODULE_DESCRIPTION("Amlogic Meson serial port driver");
MODULE_LICENSE("GPL v2");
