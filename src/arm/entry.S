#include <linkage.h>
#include <head.h>


/*
 *	cpu_arm926_switch_mm(pgd_phys, context_id)
 *
 *	Set the translation table base pointer to be pgd_phys
 *
 *	- pgd_phys - physical address of new TTB
 *
 *	It is assumed that:
 *	- we are not using split page tables
 */
/* r0 = pgd_phys, * r1 = context_id
 */
ENTRY(cpu_v6_switch_mm)
	mov	r2, #0
	orr	r0, r0, #TTB_FLAGS_UP
	mcr	p15, 0, r2, c7, c5, 6		@ flush BTAC/BTB
	mcr	p15, 0, r2, c7, c10, 4		@ drain write buffer
	mcr	p15, 0, r0, c2, c0, 0		@ write Translation Table Base Register 0
	mcr	p15, 0, r1, c13, c0, 1		@ set context ID
	mov	pc, lr
ENDPROC(cpu_v6_switch_mm)

/* r0 = previous task_struct, r1 = previous thread_info, r2 = next thread_info
 */
ENTRY(__switch_to)
/* thread_info + TI_CPU_CONTEXT hold saved cpu context, registers value is stored */
/* now ip hold the address of the context of previous process */
	add	ip, r1, #TI_CPU_CONTEXT
/* now r3 hold TP value of next process */
	ldr	r3, [r2, #TI_TP_VALUE]
/* store current regs to prev thread_info */
	stmia	ip!, {r4 - sl, fp, sp, lr} 	@ Store most regs on 
/* store CPU_DOMAIN of next to r6 */
	ldr	r6, [r2, #TI_CPU_DOMAIN]
/* set tp value and domain to cp15 */
	mcr	p15, 0, r3, c13, c0, 3		@ yes, set TLS register
	mcr	p15, 0, r6, c3, c0, 0		@ Set domain register
/* now r4 hold the address of the next context */
	add	r4, r2, #TI_CPU_CONTEXT
/* put next context to registers */
	ldmia	r4, {r4 - sl, fp, sp, pc}  	@ Load all regs saved previously
ENDPROC(__switch_to)


ENTRY(arm_start_kernel_thread)
		ldr	r1, [sp, #S_PSR]	@ get calling cpsr
		ldr	lr, [sp, #S_PC]	@ get pc
		add	sp, sp, #S_SP
		msr	spsr_cxsf, r1			@ save in spsr_svc
		ldmdb	sp, {r0 - r12}			@ get calling r0 - r12
		add	sp, sp, #S_FRAME_SIZE - S_SP
		mov	pc, lr				@ return & move spsr_svc into cpsr
ENDPROC(arm_start_kernel_thread)
