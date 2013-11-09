#ifndef _IRQ_H_
#define _IRQ_H_


#define NR_IRQS		96


/* Internal flags */
#define IRQ_INPROGRESS		0x00000100	/* IRQ handler active - do not enter! */
#define IRQ_DISABLED		0x00000200	/* IRQ disabled - do not enter! */
#define IRQ_PENDING		0x00000400	/* IRQ pending - replay on enable */
#define IRQ_REPLAY		0x00000800	/* IRQ has been replayed but not acked yet */
#define IRQ_AUTODETECT		0x00001000	/* IRQ is being autodetected */
#define IRQ_WAITING		0x00002000	/* IRQ not yet seen - for autodetection */
#define IRQ_LEVEL		0x00004000	/* IRQ level triggered */
#define IRQ_MASKED		0x00008000	/* IRQ masked - shouldn't be seen again */
#define IRQ_PER_CPU		0x00010000	/* IRQ is per CPU */
#define IRQ_NOPROBE		0x00020000	/* IRQ is not valid for probing */
#define IRQ_NOREQUEST		0x00040000	/* IRQ cannot be requested */
#define IRQ_NOAUTOEN		0x00080000	/* IRQ will not be enabled on request irq */
#define IRQ_WAKEUP		0x00100000	/* IRQ triggers system wakeup */
#define IRQ_MOVE_PENDING	0x00200000	/* need to re-target IRQ destination */
#define IRQ_NO_BALANCING	0x00400000	/* IRQ is excluded from balancing */
#define IRQ_SPURIOUS_DISABLED	0x00800000	/* IRQ was disabled by the spurious trap */
#define IRQ_MOVE_PCNTXT		0x01000000	/* IRQ migration from process context */
#define IRQ_AFFINITY_SET	0x02000000	/* IRQ affinity was set from userspace*/
#define IRQ_SUSPENDED		0x04000000	/* IRQ has gone through suspend sequence */
#define IRQ_ONESHOT		0x08000000	/* IRQ is not unmasked after hardirq */
#define IRQ_NESTED_THREAD	0x10000000	/* IRQ is nested into another, no own handler thread */



/* IRQ flag */

/*
 * These flags used only by the kernel as part of the
 * irq handling routines.
 *
 * IRQF_DISABLED - keep irqs disabled when calling the action handler.
 *                 DEPRECATED. This flag is a NOOP and scheduled to be removed
 * IRQF_SAMPLE_RANDOM - irq is used to feed the random generator
 * IRQF_SHARED - allow sharing the irq among several devices
 * IRQF_PROBE_SHARED - set by callers when they expect sharing mismatches to occur
 * IRQF_TIMER - Flag to mark this interrupt as timer interrupt
 * IRQF_PERCPU - Interrupt is per cpu
 * IRQF_NOBALANCING - Flag to exclude this interrupt from irq balancing
 * IRQF_IRQPOLL - Interrupt is used for polling (only the interrupt that is
 *                registered first in an shared interrupt is considered for
 *                performance reasons)
 * IRQF_ONESHOT - Interrupt is not reenabled after the hardirq handler finished.
 *                Used by threaded interrupts which need to keep the
 *                irq line disabled until the threaded handler has been run.
 * IRQF_NO_SUSPEND - Do not disable this IRQ during suspend
 *
 */
#define IRQF_VALID	(1 << 0)
#define IRQF_PROBE	(1 << 1)
#define IRQF_NOAUTOEN	(1 << 2)
#define IRQF_DISABLED		0x00000020
#define IRQF_SAMPLE_RANDOM	0x00000040
#define IRQF_SHARED		0x00000080
#define IRQF_PROBE_SHARED	0x00000100
#define __IRQF_TIMER		0x00000200
#define IRQF_PERCPU		0x00000400
#define IRQF_NOBALANCING	0x00000800
#define IRQF_IRQPOLL		0x00001000
#define IRQF_ONESHOT		0x00002000
#define IRQF_NO_SUSPEND		0x00004000

#define IRQF_TIMER		(__IRQF_TIMER | IRQF_NO_SUSPEND)


/**
 * enum irqreturn
 * @IRQ_NONE		interrupt was not from this device
 * @IRQ_HANDLED		interrupt was handled by this device
 * @IRQ_WAKE_THREAD	handler requests to wake the handler thread
 */
enum irqreturn {
	IRQ_NONE,
	IRQ_HANDLED,
	IRQ_WAKE_THREAD,
};

typedef enum irqreturn irqreturn_t;

typedef irqreturn_t (*irq_handler_t)(int, void *);


struct irq_handler {
  /* IRQ information */
  unsigned int		irq;
  const char		*name;
  unsigned int		status;		/* IRQ status */
  unsigned int		depth;		/* nested irq disables */
  /* IRQ operation */
  void		(*enable)(unsigned int irq);
  void		(*disable)(unsigned int irq);
  void		(*ack)(unsigned int irq);
  void		(*mask)(unsigned int irq);
  void		(*handle)(unsigned int irq);
  void		(*unmask)(unsigned int irq);
};


struct irq_handler *irq_to_handler(unsigned int irq);
void generic_handle_irq(unsigned int irq);
void __init init_IRQ(void);


#endif
