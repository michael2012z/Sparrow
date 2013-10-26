#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include "vic.h"

/**
 * vic_init2 - common initialisation code
 * @base: Base of the VIC.
 *
 * Common initialisation code for registeration
 * and resume.
*/
static void vic_init2(void *base)
{
	int i;

	for (i = 0; i < 16; i++) {
		void *reg = base + VIC_VECT_CNTL0 + (i * 4);
		__raw_writel(reg, VIC_VECT_CNTL_ENABLE | i);
	}
}


static void vic_ack_irq(unsigned int irq)
{
	void *base = get_irq_chip_data(irq);
	irq &= 31;
	__raw_writel(base + VIC_INT_ENABLE_CLEAR, 1 << irq);
	/* moreover, clear the soft-triggered, in case it was the reason */
	__raw_writel(base + VIC_INT_SOFT_CLEAR, 1 << irq);
}

static void vic_mask_irq(unsigned int irq)
{
	void *base = get_irq_chip_data(irq);
	irq &= 31;
	__raw_writel(base + VIC_INT_ENABLE_CLEAR, 1 << irq);
}

static void vic_unmask_irq(unsigned int irq)
{
	void *base = get_irq_chip_data(irq);
	irq &= 31;
	__raw_writel(base + VIC_INT_ENABLE, 1 << irq);
}

#define vic_set_wake 0

static struct irq_chip vic_chip = {
	.name		= "VIC",
	.ack		= vic_ack_irq,
	.mask		= vic_mask_irq,
	.unmask		= vic_unmask_irq,
};

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
	//	return;
	vic_unmask_irq(IRQ_UART0);
	vic_unmask_irq(IRQ_UART1);
	vic_unmask_irq(IRQ_UART2);
	vic_unmask_irq(IRQ_UART3);
  }
}

static void __init vic_set_irq_sources(void *base,
				unsigned int irq_start, unsigned int vic_sources)
{
	unsigned int i;

	for (i = 0; i < 32; i++) {
		if (vic_sources & (1 << i)) {
			unsigned int irq = irq_start + i;

			set_irq_chip(irq, &vic_chip);
			set_irq_chip_data(irq, base);
			set_irq_handler(irq, handle_level_irq);
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
void __init vic_init(void *base, unsigned int irq_start,
		     unsigned int vic_sources)
{
	/* Disable all interrupts initially. */
	vic_disable(base);

	vic_init2(base);

	vic_set_irq_sources(base, irq_start, vic_sources);

	vic_enable(base);
}
