#include <type.h>
#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <printk.h>
#include <uart.h>
#include <ring_buffer.h>

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

extern int ring_buffer_enabled;
extern struct ring_buffer *user_ring_buffer;
/*
 * IRQ handler for the uart
 */
static void uart_irq_handle(unsigned int irq)
{
  unsigned int uart_index = (irq - IRQ_S3CUART_BASE0)/4;
  unsigned int regs;

  if (uart_index > 3)
    printk(PR_SS_IRQ, PR_LVL_DBG2, "%s, invalid uart irq: irq = %x, uart_index = %x\n", __func__, irq, uart_index);	

  regs = uart_irqs[uart_index].regs;
  
  if (UART_IRQ_RXD == (irq & 0x0f)) { /* data received */
	int n, error;
	char ch;

	while(1) {
	  n = __raw_readl(regs + S3C64XX_UFSTAT);
	  printk(PR_SS_IRQ, PR_LVL_DBG2, "%s, %x char(s) in FIFO\n", __func__, n);
	  if (0 == n)
		break;
	  
	  error = __raw_readl(regs + S3C64XX_UERSTAT);
	  printk(PR_SS_IRQ, PR_LVL_DBG2, "%s, there %s error in FIFO\n", __func__, error?"is":"isn't");
	  
	  ch = __raw_readl(regs + S3C64XX_URXH);
	  printk(PR_SS_IRQ, PR_LVL_DBG2, "%s, '%c' read from FIFO\n", __func__, ch);

	  uart_input_char(ch);
	  
	}
  } else if (UART_IRQ_TXD == (irq & 0x0f)) { /* data to transmit */
	if (ring_buffer_enabled)
	  while(!ring_buffer_empty(user_ring_buffer)) {
		if (__raw_readl(regs + S3C64XX_UFSTAT)) /* if there is error */
		  break;
		else
		  __raw_writel(regs + S3C64XX_UTXH, ring_buffer_get_char(user_ring_buffer));
	  }
  }

}

static inline unsigned int uart_irq_base(unsigned int irq)
{
	struct s3c_uart_irq *uirq = &uart_irqs[(irq - IRQ_S3CUART_BASE0)/4];
	return uirq->regs;
}

static inline unsigned int uart_irq_bitmap(unsigned int irq)
{
	return irq & 3;
}

static void uart_irq_mask(unsigned int irq)
{
	unsigned int regs = uart_irq_base(irq);
	unsigned int bit = uart_irq_bitmap(irq);
	u32 reg;

	reg = __raw_readl(regs + S3C64XX_UINTM);
	reg |= (1 << bit);
	__raw_writel(regs + S3C64XX_UINTM, reg);
}

static void uart_irq_unmask(unsigned int irq)
{
	unsigned int regs = uart_irq_base(irq);
	unsigned int bit = uart_irq_bitmap(irq);
	u32 reg;

	/* unmask of TXD irqs are triggered by print */
	if (UART_IRQ_TXD == (irq & 0x0f))
	  return;

	reg = __raw_readl(regs + S3C64XX_UINTM);
	reg &= ~(1 << bit);
	__raw_writel(regs + S3C64XX_UINTM, reg);
}

void arm_uart0_tx_start() {
  unsigned int irq = IRQ_S3CUART_BASE0 + UART_IRQ_TXD;
  unsigned int regs = uart_irq_base(irq);
  unsigned int bit = uart_irq_bitmap(irq);
  u32 reg;

  reg = __raw_readl(regs + S3C64XX_UINTM);
  reg &= ~(1 << bit);
  __raw_writel(regs + S3C64XX_UINTM, reg);
}

static void uart_irq_ack(unsigned int irq)
{
	unsigned int regs = uart_irq_base(irq);
	unsigned int bit = uart_irq_bitmap(irq);

	__raw_writel(regs + S3C64XX_UINTP, 1 << bit);
}

static void vic_uart_irq_handle(unsigned int irq)
{
  struct s3c_uart_irq *uirq = &uart_irqs[irq - IRQ_UART0];
  u32 pend;
  int base;
  
  //  printk(PR_SS_IRQ, PR_LVL_DBG2, "%s: irq = %x\n", __func__, irq);
  
  pend = __raw_readl(uirq->regs + S3C64XX_UINTP);
  base = uirq->base_irq;
  
  //  printk(PR_SS_IRQ, PR_LVL_DBG2, "%s: irq = %d, pend = %x, base = %x\n", __func__, irq, pend, base);
  
  if (pend & (1 << 0))
	generic_handle_irq(base);
  if (pend & (1 << 1))
	generic_handle_irq(base + 1);
  if (pend & (1 << 2))
	generic_handle_irq(base + 2);
  if (pend & (1 << 3))
	generic_handle_irq(base + 3);
}

static void __init uart_irq_init(struct s3c_uart_irq *uirq)
{
  struct irq_handler *handler;
  unsigned int irq;
  int offs;
  
  printk(PR_SS_IRQ, PR_LVL_DBG0, "%s: parent_irq = %d, base_irq = %d\n", __func__, uirq->parent_irq, uirq->base_irq);
  
  /* replace main irq handle function */
  handler = irq_to_handler(uirq->parent_irq);;
  handler->handle = vic_uart_irq_handle;
  
  /* mask all interrupts at the start. */
  __raw_writel(uirq->regs + S3C64XX_UINTM, 0xf);
  
  /* set each sub irq handler under main */
  for (offs = 0; offs < 3; offs++) {
	irq = uirq->base_irq + offs;
	handler = irq_to_handler(irq);
	handler->irq = irq;
	handler->mask = uart_irq_mask;
	handler->ack = uart_irq_ack;
	handler->handle = uart_irq_handle;
	handler->unmask = uart_irq_unmask;
  }
}

void __init uart_irq_inits() {
  struct s3c_uart_irq *irq = uart_irqs;
  unsigned int nr_irqs = ARRAY_SIZE(uart_irqs);

  printk(PR_SS_IRQ, PR_LVL_DBG0, "%s: irq = %x, nr_irqs = %x\n", __func__, irq, nr_irqs);

  for (; nr_irqs > 0; nr_irqs--, irq++)
	uart_irq_init(irq);

  //  printk(PR_SS_IRQ, PR_LVL_DBG0, "%s: finish\n", __func__);
}

static void __init uart_setup() {
  /* setup UART0 only*/
  unsigned int regs = uart_irqs[0].regs;
  __raw_writel(regs + S3C64XX_UINTM, 0xf);
  __raw_writel(regs + S3C64XX_UCON, 0x785);
  __raw_writel(regs + S3C64XX_ULCON, 0x7);
  __raw_writel(regs + S3C64XX_UFCON, 0x17);
  __raw_writel(regs + S3C64XX_UFCON, 0x11);
  __raw_writel(regs + S3C64XX_UCON, 0x785);
  __raw_writel(regs + S3C64XX_UCON, 0x385);
  __raw_writel(regs + S3C64XX_ULCON, 0x3);
  __raw_writel(regs + S3C64XX_UBRDIV, 0x23);
  __raw_writel(regs + S3C64XX_UMCON, 0x0);
  __raw_writel(regs + S3C64XX_DIVSLOT, 0x80);

  return;
}

void __init arm_init_uart() {
  uart_setup();
  uart_irq_unmask(IRQ_S3CUART_BASE0);
  /*
  uart_irq_unmask(IRQ_S3CUART_BASE1);
  uart_irq_unmask(IRQ_S3CUART_BASE2);
  uart_irq_unmask(IRQ_S3CUART_BASE3);
  */
  /* unmask all interrupts at the start. */
  __raw_writel(S3C_VA_UART0 + S3C64XX_UINTM, 0xe);
  /*
  __raw_writel(S3C_VA_UART1 + S3C64XX_UINTM, 0xf);
  __raw_writel(S3C_VA_UART2 + S3C64XX_UINTM, 0xf);
  __raw_writel(S3C_VA_UART3 + S3C64XX_UINTM, 0xf);
  */
}
