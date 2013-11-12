#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <printk.h>

struct irq_handler all_irq_handler[NR_IRQS];

void __init init_IRQ(void) {
  arm_init_irq();
}

struct irq_handler *irq_to_handler(unsigned int irq)
{
  return (irq < NR_IRQS) ? all_irq_handler + irq : 0;
}

extern unsigned long noise; /* random seed */
void generic_handle_irq(unsigned int irq) {
  struct irq_handler *handler = irq_to_handler(irq);

  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s: irq = %d\n", __func__, irq);
  
  if (irq < 64)
	noise++;

  handler->mask(irq);
  handler->ack(irq);
  handler->handle(irq);
  handler->unmask(irq);

}
