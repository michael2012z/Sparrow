#include <linkage.h>
#include <interrupt.h>
#include "vic.h"
#include "head.h"


/*
 * These are the registers used in the syscall handler, and allow us to
 * have in theory up to 7 arguments to a function - r0 to r6.
 *
 * r7 is reserved for the system call number for thumb mode.
 *
 * Note that tbl == why is intentional.
 *
 * We must set at least "tsk" and "why" when calling ret_with_reschedule.
 */
scno	.req	r7		@ syscall number
tbl	.req	r8		@ syscall table pointer
why	.req	r8		@ Linux syscall (!= 0)
tsk	.req	r9		@ current thread_info


	.macro	disable_irq_notrace
	cpsid	i
	.endm

	.macro disable_irq
	disable_irq_notrace
	.endm

	.macro	get_thread_info, rd
	mov	\rd, sp, lsr #13
	mov	\rd, \rd, lsl #13
	.endm

	.macro	restore_user_regs, fast = 0, offset = 0
	ldr	r1, [sp, #\offset + S_PSR]	@ get calling cpsr
	ldr	lr, [sp, #\offset + S_PC]!	@ get pc
	msr	spsr_cxsf, r1			@ save in spsr_svc
	clrex					@ clear the exclusive monitor
	.if	\fast
	ldmdb	sp, {r1 - lr}^			@ get calling r1 - lr
	.else
	ldmdb	sp, {r0 - lr}^			@ get calling r0 - lr
	.endif
	mov	r0, r0				@ ARMv5T and earlier require a nop after ldm {}^
	add	sp, sp, #S_FRAME_SIZE - S_PC
	movs	pc, lr				@ return & move spsr_svc into cpsr
	.endm

	.macro	alignment_trap, rtemp
	ldr	\rtemp, .LCcralign
	ldr	\rtemp, [\rtemp]
	mcr	p15, 0, \rtemp, c1, c0
	.endm

	.macro	get_irqnr_preamble, base, tmp
	ldr	\base, =VA_VIC0
	.endm

	.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
	@ check the vic0
	mov	\irqnr, #IRQ_VIC0_BASE + 31
	ldr	\irqstat, [ \base, # VIC_IRQ_STATUS ]
	teq	\irqstat, #0
	@ otherwise try vic1
	addeq	\tmp, \base, #(VA_VIC1 - VA_VIC0)
	addeq	\irqnr, \irqnr, #(IRQ_VIC1_BASE - IRQ_VIC0_BASE)
	ldreq	\irqstat, [ \tmp, # VIC_IRQ_STATUS ]
	teqeq	\irqstat, #0
	@ 
	clzne	\irqstat, \irqstat
	subne	\irqnr, \irqnr, \irqstat
	.endm

/*
 * Interrupt handling.  Preserves r7, r8, r9
 */
	.macro	irq_handler
	get_irqnr_preamble r5, lr
1:	get_irqnr_and_base r0, r6, r5, lr
	movne	r1, sp
	@
	@ routine called with r0 = irq number, r1 = struct pt_regs *
	@
	adrne	lr, 1b
	bne	asm_do_IRQ
	.endm


	.macro	svc_entry, stack_hole=0
	.fnstart
	.save {r0 - pc}
	sub	sp, sp, #(S_FRAME_SIZE + \stack_hole - 4)
	tst	sp, #4
 	subeq	sp, sp, #4
	stmia	sp, {r1 - r12}

	ldmia	r0, {r1 - r3}
	add	r5, sp, #S_SP - 4	@ here for interlock avoidance
	mov	r4, #-1			@  ""  ""      ""       ""
	add	r0, sp, #(S_FRAME_SIZE + \stack_hole - 4)
 	addeq	r0, r0, #4
	str	r1, [sp, #-4]!		@ save the "real" r0 copied
					@ from the exception stack

	mov	r1, lr

	@
	@ We are now ready to fill in the remaining blanks on the stack:
	@
	@  r0 - sp_svc
	@  r1 - lr_svc
	@  r2 - lr_<exception>, already fixed up for correct return/restart
	@  r3 - spsr_<exception>
	@  r4 - orig_r0 (see pt_regs definition in ptrace.h)
	@
	stmia	r5, {r0 - r4}
	.endm


	.macro	svc_exit, rpsr
	msr	spsr_cxsf, \rpsr
	clrex					@ clear the exclusive monitor
	ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr
	.endm


	.align	5
.LCcralign:
	.word	cr_alignment

/*
 * User mode handlers
 *
 * EABI note: sp_svc is always 64-bit aligned here, so should S_FRAME_SIZE
 */

#if  (S_FRAME_SIZE & 7)
#error "sizeof(struct pt_regs) must be a multiple of 8"
#endif

	.macro	usr_entry
	.fnstart
	.cantunwind
	sub	sp, sp, #S_FRAME_SIZE
	stmib	sp, {r1 - r12}

	ldmia	r0, {r1 - r3}
	add	r0, sp, #S_PC		@ here for interlock avoidance
	mov	r4, #-1			@  ""  ""     ""        ""

	str	r1, [sp]		@ save the "real" r0 copied
					@ from the exception stack

	@
	@ We are now ready to fill in the remaining blanks on the stack:
	@
	@  r2 - lr_<exception>, already fixed up for correct return/restart
	@  r3 - spsr_<exception>
	@  r4 - orig_r0 (see pt_regs definition in ptrace.h)
	@
	@ Also, separately save sp_usr and lr_usr
	@
	stmia	r0, {r2 - r4}
 	stmdb	r0, {sp, lr}^

	@
	@ Enable the alignment trap while in kernel mode
	@
	alignment_trap r0

	.endm


/*
 * Invalid mode handlers
 */
	.macro	inv_entry, reason
	sub	sp, sp, #S_FRAME_SIZE
	stmib	sp, {r1 - lr}
	mov	r1, #\reason
	.endm


/*
 * "slow" syscall return path.  "why" tells us if this was a real syscall.
 */
ENTRY(ret_to_user)
ret_slow_syscall:
	disable_irq				@ disable interrupts
	restore_user_regs fast = 0, offset = 0
ENDPROC(ret_to_user)


@
@ common_invalid - generic code for failed exception (re-entrant version of handlers)
@
common_invalid:
	ldmia	r0, {r4 - r6}
	add	r0, sp, #S_PC		@ here for interlock avoidance
	mov	r7, #-1			@  ""   ""    ""        ""
	str	r4, [sp]		@ save preserved r0
	stmia	r0, {r5 - r7}		@ lr_<exception>,
					@ cpsr_<exception>, "old_r0"

	mov	r0, sp
	b	bad_mode
ENDPROC(common_invalid)

		
__irq_invalid:
	inv_entry BAD_IRQ
	b	common_invalid
ENDPROC(__irq_invalid)

		
	.align	5
__irq_svc:
	svc_entry
	irq_handler
	ldr	r4, [sp, #S_PSR]		@ irqs are already disabled
	svc_exit r4				@ return from exception
	.fnend
ENDPROC(__irq_svc)


	.align	5
__irq_usr:
	usr_entry
	get_thread_info tsk
	irq_handler
	mov	why, #0
	b	ret_to_user
	.fnend
ENDPROC(__irq_usr)

		
/*
 * Vector stubs.
 *
 * This code is copied to 0xffff0200 so we can use branches in the
 * vectors, rather than ldr's.  Note that this code must not
 * exceed 0x300 bytes.
 *
 * Common stub entry macro:
 *   Enter in IRQ mode, spsr = SVC/USR CPSR, lr = SVC/USR PC
 *
 * SP points to a minimal amount of processor-private memory, the address
 * of which is copied into r0 for the mode specific abort handler.
 */
	.macro	vector_stub, name, mode, correction=0
	.align	5

vector_\name:
	.if \correction
	sub	lr, lr, #\correction
	.endif

	@
	@ Save r0, lr_<exception> (parent PC) and spsr_<exception>
	@ (parent CPSR)
	@
	stmia	sp, {r0, lr}		@ save r0, lr
	mrs	lr, spsr
	str	lr, [sp, #8]		@ save spsr

	@
	@ Prepare for SVC32 mode.  IRQs remain disabled.
	@
	mrs	r0, cpsr
	eor	r0, r0, #(\mode ^ SVC_MODE | PSR_ISETSTATE)
	msr	spsr_cxsf, r0

	@
	@ the branch table must immediately follow this code
	@
	and	lr, lr, #0x0f
	mov	r0, sp
	ldr	lr, [pc, lr, lsl #2]
	movs	pc, lr			@ branch to handler in SVC mode
ENDPROC(vector_\name)

	.align	2
	@ handler addresses follow this label
1:
	.endm


	.globl	__stubs_start
__stubs_start:
/*
 * Interrupt dispatcher
 */
	vector_stub	irq, IRQ_MODE, 4

	.long	__irq_usr			@  0  (USR_26 / USR_32)
	.long	__irq_invalid			@  1  (FIQ_26 / FIQ_32)
	.long	__irq_invalid			@  2  (IRQ_26 / IRQ_32)
	.long	__irq_svc			@  3  (SVC_26 / SVC_32)
	.long	__irq_invalid			@  4
	.long	__irq_invalid			@  5
	.long	__irq_invalid			@  6
	.long	__irq_invalid			@  7
	.long	__irq_invalid			@  8
	.long	__irq_invalid			@  9
	.long	__irq_invalid			@  a
	.long	__irq_invalid			@  b
	.long	__irq_invalid			@  c
	.long	__irq_invalid			@  d
	.long	__irq_invalid			@  e
	.long	__irq_invalid			@  f

	.align	5

/* Unsupported exception. */
vector_rst:
	b asm_unsupported_exception

vector_und:
	b asm_unsupported_exception

vector_swi:
	b asm_unsupported_exception

vector_fiq:
	b asm_unsupported_exception

vector_addrexcptn:
	b asm_unsupported_exception

vector_pabt:
	b asm_unsupported_exception

vector_dabt:
	b asm_unsupported_exception


/*
 * We group all the following data together to optimise
 * for CPUs with separate I & D caches.
 */
	.align	5

	.globl	__stubs_end
__stubs_end:

	.equ	stubs_offset, __vectors_start + 0x200 - __stubs_start



	.globl	__vectors_start
__vectors_start:
	b	vector_rst + stubs_offset
	b	vector_und + stubs_offset
	b	vector_swi + stubs_offset
	b	vector_pabt + stubs_offset
	b	vector_dabt + stubs_offset
	b	vector_addrexcptn + stubs_offset
	b	vector_irq + stubs_offset
	b	vector_fiq + stubs_offset

	.globl	__vectors_end
__vectors_end:

	.data

	.globl	cr_alignment
	.globl	cr_no_alignment
cr_alignment:
	.space	4
cr_no_alignment:
	.space	4

	
