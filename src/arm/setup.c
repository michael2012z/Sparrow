#include <type.h>
#include <linkage.h>
#include <ptrace.h>
#include <list.h>
#include "head.h"
#include <printk.h>
#include <setup.h>

#define PLC	"I"

void arm_cpu_reset() {
  asm("mov	r1, #0\n"
	  "mcr	p15, 0, r1, c7, c0, 0		@ flush cache\n"
	  "mcr	p15, 0, r1, c5, c0, 0		@ flush TLB\n"
	  "mov	r1, #0x30\n"
	  "mcr	p15, 0, r1, c1, c0, 0		@ turn off MMU etc\n"
	  "ldr r0, =(0x50000000)\n"
	  "mov	pc, r0\n");
}

struct stack {
	u32 irq[3];
	u32 abt[3];
	u32 und[3];
};

static struct stack stacks;

void arm_cpu_init() {
  struct stack *stk = &stacks;
	/*
	 * setup stacks for re-entrant exception handlers
	 */
	__asm__ (
	"msr	cpsr_c, %1\n\t"
	"add	r14, %0, %2\n\t"
	"mov	sp, r14\n\t"
	"msr	cpsr_c, %3\n\t"
	"add	r14, %0, %4\n\t"
	"mov	sp, r14\n\t"
	"msr	cpsr_c, %5\n\t"
	"add	r14, %0, %6\n\t"
	"mov	sp, r14\n\t"
	"msr	cpsr_c, %7"
	    :
	    : "r" (stk),
	      PLC (PSR_F_BIT | PSR_I_BIT | IRQ_MODE),
	      "I" (offsetof(struct stack, irq[0])),
	      PLC (PSR_F_BIT | PSR_I_BIT | ABT_MODE),
	      "I" (offsetof(struct stack, abt[0])),
	      PLC (PSR_F_BIT | PSR_I_BIT | UND_MODE),
	      "I" (offsetof(struct stack, und[0])),
	      PLC (PSR_F_BIT | PSR_I_BIT | SVC_MODE)
	    : "r14");

}
