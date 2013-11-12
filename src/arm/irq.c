#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <ptrace.h>
#include <printk.h>
#include <uart.h>


/*
 * do_IRQ handles all hardware IRQ's.  Decoded IRQs should not
 * come via this function.  Instead, they should provide their
 * own 'handler'
 */
void __exception asm_do_IRQ(unsigned int irq, struct pt_regs *regs)
{
  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: %d\n", __func__, irq);

  if (irq >= NR_IRQS) {
	printk(PR_SS_IRQ, PR_LVL_DBG1, "Bad IRQ %d\n", irq);
  } else {
	generic_handle_irq(irq);
  }

  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: %d return\n", __func__, irq);
}


static void __init s3c6410_init_irq(void)
{
	/* VIC0 is missing IRQ7, VIC1 is fully populated. */
  unsigned int vic0_valid = (~0 & ~(1 << 7));
  unsigned int vic1_valid = ~0;

  printk(PR_SS_IRQ, PR_LVL_INF, "%s: initialising interrupts\n", __func__);

  /* initialize the pair of VICs */
  vic_init((void *)VA_VIC0, IRQ_VIC0_BASE, vic0_valid);
  vic_init((void *)VA_VIC1, IRQ_VIC1_BASE, vic1_valid);

  /* initialize timer irq */
  timer_irq_inits();

  /* initialize uart irq */
  uart_irq_inits();
}


void arm_init_irq() {
  s3c6410_init_irq();  
}

