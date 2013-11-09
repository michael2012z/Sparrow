#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include "vic.h"
#include <printk.h>

static void vic_init2(void *base)
{
	int i;
	for (i = 0; i < 16; i++) {
		void *reg = base + VIC_VECT_CNTL0 + (i * 4);
		__raw_writel(reg, VIC_VECT_CNTL_ENABLE | i);
	}
}

static void* get_vic_base(unsigned int irq) {
  if (irq < (IRQ_VIC0_BASE + 32))
	return (void *)VA_VIC0;
  else if (irq < (IRQ_VIC1_BASE + 32))
	return (void *)VA_VIC1;
  else 
	return (void *)0;
}

static void vic_ack_irq(unsigned int irq)
{
	void *base = get_vic_base(irq);
	irq &= 31;

	printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: base = %x, irq = %d\n", __func__, (unsigned int)base, irq);

	__raw_writel(base + VIC_INT_ENABLE_CLEAR, 1 << irq);
	/* moreover, clear the soft-triggered, in case it was the reason */
	__raw_writel(base + VIC_INT_SOFT_CLEAR, 1 << irq);
}

static void vic_mask_irq(unsigned int irq)
{
	void *base = get_vic_base(irq);
	irq &= 31;
	printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: base = %x, irq = %d\n", __func__, (unsigned int)base, irq);
	__raw_writel(base + VIC_INT_ENABLE_CLEAR, 1 << irq);
}

static void vic_unmask_irq(unsigned int irq)
{
	void *base = get_vic_base(irq);
	irq &= 31;
	printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: base = %x, irq = %d\n", __func__, (unsigned int)base, irq);
	__raw_writel(base + VIC_INT_ENABLE, 1 << irq);
}

/* default irq handler, to be replaced */
static void vic_handle_irq(unsigned int irq)
{
  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: irq = %d\n", __func__, irq);
  return;
}

static void __init vic_disable(void *base)
{
  __raw_writel(base + VIC_INT_SELECT, 0);
  __raw_writel(base + VIC_INT_ENABLE, 0);
  __raw_writel(base + VIC_INT_ENABLE_CLEAR, ~0);
  __raw_writel(base + VIC_IRQ_STATUS, 0);
  __raw_writel(base + VIC_ITCR, 0);
  __raw_writel(base + VIC_INT_SOFT_CLEAR, ~0);
}

static void __init vic_enable(void *base)
{
  if ((unsigned long)base == VA_VIC0)
	vic_unmask_irq(IRQ_TIMER4_VIC);
  else if ((unsigned long)base == VA_VIC1) {
	vic_unmask_irq(IRQ_UART0);
	/*
	vic_unmask_irq(IRQ_UART1);
	vic_unmask_irq(IRQ_UART2);
	vic_unmask_irq(IRQ_UART3);
	*/
  }
}

static void __init vic_set_irq_sources(void *base, unsigned int irq_start, unsigned int vic_sources)
{
	unsigned int i;

	for (i = 0; i < 32; i++) {
		if (vic_sources & (1 << i)) {
			unsigned int irq = irq_start + i;
			struct irq_handler *handler = irq_to_handler(irq);
			handler->irq = i;
			handler->mask = vic_mask_irq;
			handler->ack = vic_ack_irq;
			handler->handle = vic_handle_irq;
			handler->unmask = vic_unmask_irq;
		}
	}
}

/**
 * vic_init - initialise a vectored interrupt controller
 * @base: iomem base address
 * @irq_start: starting interrupt number, must be muliple of 32
 * @vic_sources: bitmask of interrupt sources to allow
 * @resume_sources: bitmask of interrupt sources to allow for resume
 */
void __init vic_init(void *base, unsigned int irq_start, unsigned int vic_sources)
{
	vic_disable(base);

	vic_init2(base);

	vic_set_irq_sources(base, irq_start, vic_sources);

	vic_enable(base);
}
