#include <ptrace.h>
#include <string.h>

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

