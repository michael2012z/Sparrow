#include <ptrace.h>
#include <string.h>
#include "head.h"
#include <printk.h>
#include <list.h>

void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp) {
	unsigned long *stack = (unsigned long *)sp;
	memset(regs->uregs, 0, sizeof(regs->uregs));
	regs->ARM_cpsr = USR_MODE;
	regs->ARM_cpsr |= PSR_ENDSTATE;
	regs->ARM_pc = pc & ~1;		/* pc */
	regs->ARM_sp = sp;		/* sp */
	regs->ARM_r2 = stack[2];	/* r2 (envp) */
	regs->ARM_r1 = stack[1];	/* r1 (argv) */
	regs->ARM_r0 = stack[0];	/* r0 (argc) */
	return;
}

/*
 * Shuffle the argument into the correct register before calling the
 * thread function.  r4 is the thread argument, r5 is the pointer to
 * the thread function, and r6 points to the exit function.
 */
extern void kernel_thread_helper(void);
asm(	".pushsection .text\n"
"	.align\n"
"	.type	kernel_thread_helper, #function\n"
"kernel_thread_helper:\n"
"	msr	cpsr_c, r7\n"
"	mov	r0, r4\n"
"	mov	lr, r6\n"
"	mov	pc, r5\n"
"	.size	kernel_thread_helper, . - kernel_thread_helper\n"
"	.popsection");

void kernel_thread_exit(void) {
}

void arm_create_kernel_thread(int (*fn)(void *), void *arg, struct pt_regs *regs)
{
	memset(regs, 0, sizeof(regs));
	regs->ARM_r4 = (unsigned long)arg;
	regs->ARM_r5 = (unsigned long)fn;
	regs->ARM_r6 = (unsigned long)kernel_thread_exit;
	regs->ARM_r7 = SVC_MODE | PSR_ENDSTATE | PSR_ISETSTATE;
	regs->ARM_pc = (unsigned long)kernel_thread_helper;
   	regs->ARM_cpsr = regs->ARM_r7 | PSR_I_BIT;
}


void cpu_idle() {
  printk(PR_SS_INI, PR_LVL_ERROR, "cpu_idle() \n");
  while(1) {asm("nop\n");}
  /* Should call cpu founction:
ENTRY(cpu_v6_do_idle)
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4		@ DWB - WFI may enter a low-power mode
	mcr	p15, 0, r1, c7, c0, 4		@ wait for interrupt
	mov	pc, lr

   */
}

void arm_health_check(void) {
  CRASHIF(S_R0 != offsetof(struct pt_regs, ARM_r0));
  CRASHIF(S_R1 != offsetof(struct pt_regs, ARM_r1));
  CRASHIF(S_R2 != offsetof(struct pt_regs, ARM_r2));
  CRASHIF(S_R3 != offsetof(struct pt_regs, ARM_r3));
  CRASHIF(S_R4 != offsetof(struct pt_regs, ARM_r4));
  CRASHIF(S_R5 != offsetof(struct pt_regs, ARM_r5));
  CRASHIF(S_R6 != offsetof(struct pt_regs, ARM_r6));
  CRASHIF(S_R7 != offsetof(struct pt_regs, ARM_r7));
  CRASHIF(S_R8 != offsetof(struct pt_regs, ARM_r8));
  CRASHIF(S_R9 != offsetof(struct pt_regs, ARM_r9));
  CRASHIF(S_R10 != offsetof(struct pt_regs, ARM_r10));
  CRASHIF(S_FP != offsetof(struct pt_regs, ARM_fp));
  CRASHIF(S_IP != offsetof(struct pt_regs, ARM_ip));
  CRASHIF(S_SP != offsetof(struct pt_regs, ARM_sp));
  CRASHIF(S_LR != offsetof(struct pt_regs, ARM_lr));
  CRASHIF(S_PC != offsetof(struct pt_regs, ARM_pc));
  CRASHIF(S_PSR != offsetof(struct pt_regs, ARM_cpsr));
  CRASHIF(S_OLD_R0 != offsetof(struct pt_regs, ARM_ORIG_r0));
  CRASHIF(S_FRAME_SIZE != sizeof(struct pt_regs));
}

unsigned int arm_calc_kernel_domain() {
  return domain_val(DOMAIN_USER, DOMAIN_MANAGER) | domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | domain_val(DOMAIN_TABLE, DOMAIN_MANAGER) | domain_val(DOMAIN_IO, DOMAIN_CLIENT);
}
