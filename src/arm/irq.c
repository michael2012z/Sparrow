#include <linkage.h>
#include <interrupt.h>
#include <irq.h>
#include <ptrace.h>
#include <printk.h>

/*
 * bad_mode handles the impossible case in the vectors.  If you see one of
 * these, then it's extremely serious, and could mean you have buggy hardware.
 * It never returns, and never tries to sync.  We hope that we can at least
 * dump out some state information...
 */
void bad_mode(struct pt_regs *regs, int reason)
{

}


struct pt_regs *  __irq_regs;

#define __verify_pcpu_ptr(ptr)	do {					\
	const void *__vpp_verify = (typeof(ptr))0;		\
	(void)__vpp_verify;						\
} while (0)


#define VERIFY_PERCPU_PTR(__p) ({			\
	__verify_pcpu_ptr((__p));			\
	(typeof(*(__p)) *)(__p);	\
})

#define __get_cpu_var(var)	(*VERIFY_PERCPU_PTR(&(var)))

static inline struct pt_regs *set_irq_regs(struct pt_regs *new_regs)
{
	struct pt_regs *old_regs, **pp_regs = &__get_cpu_var(__irq_regs);

	old_regs = *pp_regs;
	*pp_regs = new_regs;
	return old_regs;
}

/*
 * do_IRQ handles all hardware IRQ's.  Decoded IRQs should not
 * come via this function.  Instead, they should provide their
 * own 'handler'
 */
void __exception asm_do_IRQ(unsigned int irq, struct pt_regs *regs)
{
  struct pt_regs *old_regs = set_irq_regs(regs);

  if (irq >= NR_IRQS) {
	printk(PR_SS_IRQ, PR_LVL_INF, "Bad IRQ %d\n", irq);
  } else {
	generic_handle_irq(irq);
  }

  set_irq_regs(old_regs);
}


static void __init s3c6410_init_irq(void)
{
	/* VIC0 is missing IRQ7, VIC1 is fully populated. */
  unsigned int vic0_valid = (~0 & ~(1 << 7));
  unsigned int vic1_valid = ~0;

  printk(PR_SS_IRQ, PR_LVL_INF, "%s: initialising interrupts\n", __func__);

  /* initialise the pair of VICs */
  vic_init((void *)VA_VIC0, IRQ_VIC0_BASE, vic0_valid);
  vic_init((void *)VA_VIC1, IRQ_VIC1_BASE, vic1_valid);

  /* add the timer sub-irqs */
  /* comment out timer 0 ~ 3 */
  /*
  s3c_init_timer_irq(IRQ_TIMER0);
  s3c_init_timer_irq(IRQ_TIMER1);
  s3c_init_timer_irq(IRQ_TIMER2);
  s3c_init_timer_irq(IRQ_TIMER3);
  */
  printk(PR_SS_IRQ, PR_LVL_INF, "%s: initialising interrupts\n", __func__);
  s3c_init_timer_irq(IRQ_TIMER4_VIC, IRQ_TIMER4);

}


void arm_init_irq() {
  s3c6410_init_irq();  
}



void __exception test_IRQ_1() {
  printk(PR_SS_IRQ, PR_LVL_INF, "++++++++++++++++++ %s\n", __func__);
}

void __exception test_IRQ_2() {
  printk(PR_SS_IRQ, PR_LVL_INF, "++++++++++++++++++ %s\n", __func__);
}

/*
 * General handling for unsupported exception.
 */
void __exception asm_unsupported_exception(unsigned int code)
{
	printk(PR_SS_IRQ, PR_LVL_ERR, "An unsupported exception happened, code = %d\n", code);
	while(1);
}

