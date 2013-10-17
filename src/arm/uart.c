#include <type.h>
#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <printk.h>
#include <uart.h>

static struct s3c_uart_irq uart_irqs[] = {
	[0] = {
		.regs		= S3C_VA_UART0,
		.base_irq	= IRQ_S3CUART_BASE0,
		.parent_irq	= IRQ_UART0,
	},
	[1] = {
		.regs		= S3C_VA_UART1,
		.base_irq	= IRQ_S3CUART_BASE1,
		.parent_irq	= IRQ_UART1,
	},
	[2] = {
		.regs		= S3C_VA_UART2,
		.base_irq	= IRQ_S3CUART_BASE2,
		.parent_irq	= IRQ_UART2,
	},
	[3] = {
		.regs		= S3C_VA_UART3,
		.base_irq	= IRQ_S3CUART_BASE3,
		.parent_irq	= IRQ_UART3,
	},
};

/*
 * IRQ handler for the uart
 */
static irqreturn_t
s3c6410_uart_interrupt(int irq, void *dev_id)
{
  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s, irq = %d\n", __func__, irq);
  return IRQ_HANDLED;
}

static struct irqaction s3c6410_uart_irq = {
	.name		= "S3C6410 UART IRQ",
	.handler	= s3c6410_uart_interrupt,
};

static inline unsigned int s3c_irq_uart_base(unsigned int irq)
{
	struct s3c_uart_irq *uirq = get_irq_chip_data(irq);
	return uirq->regs;
}

static inline unsigned int s3c_irq_uart_bit(unsigned int irq)
{
	return irq & 3;
}

static void s3c_irq_uart_mask(unsigned int irq)
{
	unsigned int regs = s3c_irq_uart_base(irq);
	unsigned int bit = s3c_irq_uart_bit(irq);
	u32 reg;

	reg = __raw_readl(regs + S3C64XX_UINTM);
	reg |= (1 << bit);
	__raw_writel(regs + S3C64XX_UINTM, reg);
}

static void s3c_irq_uart_maskack(unsigned int irq)
{
	unsigned int regs = s3c_irq_uart_base(irq);
	unsigned int bit = s3c_irq_uart_bit(irq);
	u32 reg;

	reg = __raw_readl(regs + S3C64XX_UINTM);
	reg |= (1 << bit);
	__raw_writel(regs + S3C64XX_UINTM, reg);
	__raw_writel(regs + S3C64XX_UINTP, 1 << bit);
}

static void s3c_irq_uart_unmask(unsigned int irq)
{
	unsigned int regs = s3c_irq_uart_base(irq);
	unsigned int bit = s3c_irq_uart_bit(irq);
	u32 reg;

	reg = __raw_readl(regs + S3C64XX_UINTM);
	reg &= ~(1 << bit);
	__raw_writel(regs + S3C64XX_UINTM, reg);
}

static void s3c_irq_uart_ack(unsigned int irq)
{
	unsigned int regs = s3c_irq_uart_base(irq);
	unsigned int bit = s3c_irq_uart_bit(irq);

	__raw_writel(regs + S3C64XX_UINTP, 1 << bit);
}

static void s3c_irq_demux_uart(unsigned int irq, struct irq_desc *desc)
{
	struct s3c_uart_irq *uirq = desc->handler_data;
	u32 pend = __raw_readl(uirq->regs + S3C64XX_UINTP);
	int base = uirq->base_irq;

	if (pend & (1 << 0))
		generic_handle_irq(base);
	if (pend & (1 << 1))
		generic_handle_irq(base + 1);
	if (pend & (1 << 2))
		generic_handle_irq(base + 2);
	if (pend & (1 << 3))
		generic_handle_irq(base + 3);
}

static struct irq_chip s3c_irq_uart = {
	.name		= "s3c-uart",
	.mask		= s3c_irq_uart_mask,
	.unmask		= s3c_irq_uart_unmask,
	.mask_ack	= s3c_irq_uart_maskack,
	.ack		= s3c_irq_uart_ack,
};

static void __init s3c_init_uart_irq(struct s3c_uart_irq *uirq)
{
	struct irq_desc *desc = irq_to_desc(uirq->parent_irq);
	unsigned int reg_base = uirq->regs;
	unsigned int irq;
	int offs;

	printk(PR_SS_IRQ, PR_LVL_DBG0, "%s: reg_base = %x\n", __func__, reg_base);

	/* mask all interrupts at the start. */
	__raw_writel(reg_base + S3C64XX_UINTM, 0xf);

	for (offs = 0; offs < 3; offs++) {
		irq = uirq->base_irq + offs;
		set_irq_chip(irq, &s3c_irq_uart);
		set_irq_chip_data(irq, uirq);
		set_irq_handler(irq, handle_level_irq);
		setup_irq(irq, &s3c6410_uart_irq);
	}

	desc->handler_data = uirq;
	desc->handle_irq = s3c_irq_demux_uart;
}

void __init s3c_init_uart_irqs() {
  struct s3c_uart_irq *irq = uart_irqs;
  unsigned int nr_irqs = ARRAY_SIZE(uart_irqs);

  printk(PR_SS_IRQ, PR_LVL_DBG0, "%s: irq = %x, nr_irqs = %x\n", __func__, irq, nr_irqs);

  for (; nr_irqs > 0; nr_irqs--, irq++)
	s3c_init_uart_irq(irq);

  printk(PR_SS_IRQ, PR_LVL_DBG0, "%s: finish\n", __func__);
}

static void __init s3c6410_uart_setup() {
  return;
}

void __init arm_init_uart() {
  s3c6410_uart_setup();
  s3c_irq_uart_unmask(IRQ_S3CUART_BASE0);
  /* unmask all interrupts at the start. */
  __raw_writel(S3C_VA_UART0 + S3C64XX_UINTM, 0x0);

}
