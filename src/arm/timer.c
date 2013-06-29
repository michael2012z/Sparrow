#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <printk.h>
#include <timer.h>

/* We assume the IRQ_TIMER0..IRQ_TIMER4 range is continuous. */

static void s3c_irq_timer_mask(unsigned int irq)
{
	unsigned int reg = __raw_readl(S3C64XX_TINT_CSTAT);
	reg &= 0x1f;  /* mask out pending interrupts */
	reg &= ~(1 << (irq - IRQ_TIMER0));
	__raw_writel(S3C64XX_TINT_CSTAT, reg);
}

static void s3c_irq_timer_unmask(unsigned int irq)
{
	unsigned int reg = __raw_readl(S3C64XX_TINT_CSTAT);
	reg &= 0x1f;  /* mask out pending interrupts */
	reg |= 1 << (irq - IRQ_TIMER0);
	__raw_writel(S3C64XX_TINT_CSTAT, reg);
}

static void s3c_irq_timer_ack(unsigned int irq)
{
	unsigned int reg = __raw_readl(S3C64XX_TINT_CSTAT);
	reg &= 0x1f;
	reg |= (1 << 5) << (irq - IRQ_TIMER0);
	__raw_writel(S3C64XX_TINT_CSTAT, reg);
}

static struct irq_chip s3c_irq_timer = {
	.name		= "s3c-timer",
	.mask		= s3c_irq_timer_mask,
	.unmask		= s3c_irq_timer_unmask,
	.ack		= s3c_irq_timer_ack,
};


static void __init s3c6410_timer_setup(int timer_irq) {
  unsigned long tcon, tcfg0, tcfg1, tcnt;

  /* Original config and control is read, but not used. */
  tcon = __raw_readl(S3C6410_TIMER_TCON);
  tcfg0 = __raw_readl(S3C6410_TIMER_TCFG0);
  tcfg1 = __raw_readl(S3C6410_TIMER_TCFG1);

  /* Configure Timer 4: */
  /* prescaler = 8 */
  tcfg0 |= 0x800;
  /* write back cfg0, cfg1 not changed */
  __raw_writel(S3C6410_TIMER_TCFG0, tcfg0);
  __raw_writel(S3C6410_TIMER_TCFG1, tcfg1);

  /* stop timer 4 */
  tcon &= S3C6410_TIMER_TCON_T4DISABLE;
  tcon |= S3C6410_TIMER_TCON_T4RELOAD;
  tcon |= S3C6410_TIMER_TCON_T4UPDATE;
  __raw_writel(S3C6410_TIMER_TCON, tcon);

  /* write count value */
  tcnt = 0x3000;
  __raw_writel(S3C6410_TIMER_TCNTB4, tcnt);

  /* start the timer */
  tcon |= S3C6410_TIMER_TCON_T4START;
  tcon &= ~S3C6410_TIMER_TCON_T4UPDATE;
  __raw_writel(S3C6410_TIMER_TCON, tcon);

}


/*
 * IRQ handler for the timer
 */
static irqreturn_t
s3c6410_timer_interrupt(int irq, void *dev_id)
{
  /*
	timer_tick();
  */
  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s, irq = %d\n", __func__, irq);
  on_timer();
  return IRQ_HANDLED;
}

static struct irqaction s3c6410_timer_irq = {
	.name		= "S3C6410 Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= s3c6410_timer_interrupt,
};


/**
 * s3c_init_vic_timer_irq() - initialise timer irq chanined off VIC.\
 * @parent_irq: The parent IRQ on the VIC for the timer.
 * @timer_irq: The IRQ to be used for the timer.
 *
 * Register the necessary IRQ chaining and support for the timer IRQs
 * chained of the VIC.
 */
void __init s3c_init_timer_irq(unsigned int parent_irq, unsigned int timer_irq)
{
  struct irq_desc *parent_desc = irq_to_desc(parent_irq);

  parent_desc->handler_data = (void *)timer_irq;

  set_irq_chip(timer_irq, &s3c_irq_timer);
  set_irq_handler(timer_irq, handle_level_irq);
  set_irq_flags(timer_irq, IRQF_VALID);
  setup_irq(timer_irq, &s3c6410_timer_irq);
}


void __init arm_init_timer() {
  s3c6410_timer_setup(IRQ_TIMER4);
  s3c_irq_timer_unmask(IRQ_TIMER4);
}
