#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <printk.h>
#include <timer.h>

/* We assume the IRQ_TIMER0..IRQ_TIMER4 range is continuous. */

static void vic_timer_irq_handle(unsigned int irq) {
  unsigned int timer_irq = irq - IRQ_TIMER0_VIC + IRQ_TIMER0;
  generic_handle_irq(timer_irq);  
}

static void timer_irq_mask(unsigned int irq)
{
	unsigned int reg = __raw_readl(S3C64XX_TINT_CSTAT);
	reg &= 0x1f;  /* mask out pending interrupts */
	reg &= ~(1 << (irq - IRQ_TIMER0));
	__raw_writel(S3C64XX_TINT_CSTAT, reg);
}

static void timer_irq_unmask(unsigned int irq)
{
	unsigned int reg = __raw_readl(S3C64XX_TINT_CSTAT);
	reg &= 0x1f;  /* mask out pending interrupts */
	reg |= 1 << (irq - IRQ_TIMER0);
	__raw_writel(S3C64XX_TINT_CSTAT, reg);
}

static void timer_irq_ack(unsigned int irq)
{
	unsigned int reg = __raw_readl(S3C64XX_TINT_CSTAT);
	reg &= 0x1f;
	reg |= (1 << 5) << (irq - IRQ_TIMER0);
	__raw_writel(S3C64XX_TINT_CSTAT, reg);
}

static void timer_irq_handle(unsigned int irq)
{
  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s, irq = %d\n", __func__, irq);
  on_timer();
}



static void __init timer_setup(int timer_irq) {
  unsigned long tcon, tcfg0, tcfg1, tcnt;

  printk(PR_SS_IRQ, PR_LVL_DBG1, "%s, timer_irq = %d\n", __func__, timer_irq);

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


static void __init timer_irq_init(unsigned int parent_irq, unsigned int timer_irq)
{
  struct irq_handler *handler;

  /* main irq handler */
  handler = irq_to_handler(parent_irq);
  handler->handle = vic_timer_irq_handle;

  /* sub irq handler */
  handler = irq_to_handler(timer_irq);
  handler->irq = timer_irq;
  handler->mask = timer_irq_mask;
  handler->ack = timer_irq_ack;
  handler->handle = timer_irq_handle;
  handler->unmask = timer_irq_unmask;
}

void __init timer_irq_inits() {
  timer_irq_init(IRQ_TIMER0_VIC, IRQ_TIMER0);
  timer_irq_init(IRQ_TIMER1_VIC, IRQ_TIMER1);
  timer_irq_init(IRQ_TIMER2_VIC, IRQ_TIMER2);
  timer_irq_init(IRQ_TIMER3_VIC, IRQ_TIMER3);
}

void __init arm_init_timer() {
  timer_setup(IRQ_TIMER4);
  timer_irq_unmask(IRQ_TIMER4);
}
