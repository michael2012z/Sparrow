#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <printk.h>


static inline void mask_ack_irq(struct irq_desc *desc, int irq)
{
	if (desc->chip->mask_ack)
		desc->chip->mask_ack(irq);
	else {
		desc->chip->mask(irq);
		if (desc->chip->ack)
			desc->chip->ack(irq);
	}
	desc->status |= IRQ_MASKED;
}

static inline void mask_irq(struct irq_desc *desc, int irq)
{
	if (desc->chip->mask) {
		desc->chip->mask(irq);
		desc->status |= IRQ_MASKED;
	}
}

static inline void unmask_irq(struct irq_desc *desc, int irq)
{
	if (desc->chip->unmask) {
		desc->chip->unmask(irq);
		desc->status &= ~IRQ_MASKED;
	}
}


void
handle_level_irq(unsigned int irq, struct irq_desc *desc)
{
	struct irqaction *action;
	irqreturn_t action_ret;

	mask_ack_irq(desc, irq);
	/*
	if (desc->status & IRQ_INPROGRESS)
		goto out_unlock;
	*/
	desc->status &= ~(IRQ_REPLAY | IRQ_WAITING);

	/*
	 * If its disabled or no action available
	 * keep it masked and get out of here
	 */
	action = desc->action;
	/*
	if (!action || (desc->status & IRQ_DISABLED))
		goto out_unlock;
	*/
	if (!action)
		goto out_unlock;
	  
	desc->status |= IRQ_INPROGRESS;

	action_ret = action->handler(irq, 0);

	desc->status &= ~IRQ_INPROGRESS;

  /*
	if (!(desc->status & (IRQ_DISABLED | IRQ_ONESHOT)))
		unmask_irq(desc, irq);
  */
	unmask_irq(desc, irq);

out_unlock:
	return;
}


int set_irq_chip(unsigned int irq, struct irq_chip *chip)
{
	struct irq_desc *desc = irq_to_desc(irq);
	desc->chip = chip;
	return 0;
}


int set_irq_data(unsigned int irq, void *data)
{
	struct irq_desc *desc = irq_to_desc(irq);

	desc->handler_data = data;
	return 0;
}


int set_irq_chip_data(unsigned int irq, void *data)
{
	struct irq_desc *desc = irq_to_desc(irq);

	desc->chip_data = data;

	return 0;
}

int set_irq_handler(unsigned int irq, irq_flow_handler_t handle)
{
	struct irq_desc *desc = irq_to_desc(irq);

	printk(PR_SS_IRQ, PR_LVL_INF, "%s, irq = %d, desc = %x\n", __func__, irq, desc);

	desc->handle_irq = handle;

	return 0;

}

int set_irq_flags(unsigned int irq, unsigned int iflags)
{
  struct irq_desc *desc = irq_to_desc(irq);

  desc->status |= IRQ_NOREQUEST | IRQ_NOPROBE | IRQ_NOAUTOEN;
  if (iflags & IRQF_VALID)
	desc->status &= ~IRQ_NOREQUEST;
  if (iflags & IRQF_PROBE)
	desc->status &= ~IRQ_NOPROBE;
  if (!(iflags & IRQF_NOAUTOEN))
	desc->status &= ~IRQ_NOAUTOEN;
  return 0;
}
