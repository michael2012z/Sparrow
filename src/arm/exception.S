#include <linkage.h>
#include <interrupt.h>
#include "vic.h"
#include "head.h"
#include "abort-macro.S"

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

	.macro	enable_irq_notrace
	cpsie	i
	.endm

	.macro enable_irq
	enable_irq_notrace
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
	@b b_dbg_nail
	mov r5, r0
	ldr	r0, .LCcralign
	mov r6, r0
	ldr	r0, [r0]
	mov r7, r0
	@b asm_dbg_nail
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

b_dbg_nail:
	ldr r0, =0xef005020
	mov r1, #0x42
	str r1,[r0]
	b b_dbg_nail

/*
 * Function: v6_early_abort
 *
 * Params  : r2 = address of aborted instruction
 *         : r3 = saved SPSR
 *
 * Returns : r0 = address of abort
 *	   : r1 = FSR, bit 11 = write
 *	   : r2-r8 = corrupted
 *	   : r9 = preserved
 *	   : sp = pointer to registers
 *
 * Purpose : obtain information about current aborted instruction.
 * Note: we read user space.  This means we might cause a data
 * abort here if the I-TLB and D-TLB aren't seeing the same
 * picture.  Unfortunately, this does happen.  We live with it.
 */
	.align	5
ENTRY(v6_early_abort)
	clrex
	mrc	p15, 0, r1, c5, c0, 0		@ get FSR
	mrc	p15, 0, r0, c6, c0, 0		@ get FAR
/*
 * Faulty SWP instruction on 1136 doesn't set bit 11 in DFSR (erratum 326103).
 * The test below covers all the write situations, including Java bytecodes
 */
	bic	r1, r1, #1 << 11		@ clear bit 11 of FSR
	tst	r3, #PSR_J_BIT			@ Java?
	movne	pc, lr
	do_thumb_abort
	ldreq	r3, [r2]			@ read aborted ARM instruction
	do_ldrd_abort
	tst	r3, #1 << 20			@ L = 0 -> write
	orreq	r1, r1, #1 << 11		@ yes.
	mov	pc, lr
ENDPROC(v6_early_abort)
		

/*
 * Function: v6_pabort
 *
 * Params  : r0 = address of aborted instruction
 *
 * Returns : r0 = address of abort
 *	   : r1 = IFSR
 *
 * Purpose : obtain information about current prefetch abort.
 */

	.align	5
ENTRY(v6_pabort)
	mrc	p15, 0, r1, c5, c0, 1		@ get IFSR
	mov	pc, lr
ENDPROC(v6_pabort)

		

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

__dabt_invalid:
	inv_entry BAD_DATA
	b	common_invalid
ENDPROC(__dabt_invalid)

__pabt_invalid:
	inv_entry BAD_PREFETCH
	b	common_invalid
ENDPROC(__pabt_invalid)

		
	.align	5
__irq_svc:
	svc_entry
	bl irq_svc_debug
	irq_handler
	bl check_and_schedule
	ldr	r4, [sp, #S_PSR]		@ irqs are already disabled
	svc_exit r4				@ return from exception
	.fnend
ENDPROC(__irq_svc)


	.align	5
__irq_usr:
	usr_entry
	get_thread_info tsk
	bl irq_usr_debug
	irq_handler
	bl check_and_schedule	
	mov	why, #0
	b	ret_to_user
	.fnend
ENDPROC(__irq_usr)


	.align	5
__dabt_svc:
	svc_entry

	@
	@ get ready to re-enable interrupts if appropriate
	@
	mrs	r9, cpsr
	tst	r3, #PSR_I_BIT
	biceq	r9, r9, #PSR_I_BIT

	@
	@ Call the processor-specific abort handler:
	@
	@  r2 - aborted context pc
	@  r3 - aborted context cpsr
	@
	@ The abort handler must return the aborted address in r0, and
	@ the fault status register in r1.  r9 must be preserved.
	@
	bl	v6_early_abort

	@
	@ set desired IRQ state, then call main handler
	@
	msr	cpsr_c, r9
	mov	r2, sp
	bl	do_DataAbort

	@
	@ IRQs off again before pulling preserved data off the stack
	@
	disable_irq_notrace

	@
	@ restore SPSR and restart the instruction
	@
	ldr	r2, [sp, #S_PSR]
	svc_exit r2				@ return from exception
	.fnend
ENDPROC(__dabt_svc)


	.align	5
__pabt_svc:
	svc_entry
	@
	@ re-enable interrupts if appropriate
	@
	mrs	r9, cpsr
	tst	r3, #PSR_I_BIT
	biceq	r9, r9, #PSR_I_BIT

	mov	r0, r2			@ pass address of aborted instruction.
	bl	v6_pabort
	msr	cpsr_c, r9			@ Maybe enable interrupts
	mov	r2, sp				@ regs
	bl	do_PrefetchAbort		@ call abort handler

	@
	@ IRQs off again before pulling preserved data off the stack
	@
	disable_irq_notrace

	@
	@ restore SPSR and restart the instruction
	@
	ldr	r2, [sp, #S_PSR]
	svc_exit r2				@ return from exception
	.fnend
ENDPROC(__pabt_svc)


	.align	5
__dabt_usr:
	usr_entry

	@
	@ Call the processor-specific abort handler:
	@
	@  r2 - aborted context pc
	@  r3 - aborted context cpsr
	@
	@ The abort handler must return the aborted address in r0, and
	@ the fault status register in r1.
	@
	bl	v6_early_abort

	@
	@ IRQs on, then call the main handler
	@
	enable_irq
	mov	r2, sp
	adr	lr, ret_from_exception
	b	do_DataAbort
	.fnend
ENDPROC(__dabt_usr)



	.align	5
__pabt_usr:
	usr_entry
	mov	r0, r2			@ pass address of aborted instruction.
	bl	v6_pabort
	enable_irq				@ Enable interrupts
	mov	r2, sp				@ regs
	bl	do_PrefetchAbort		@ call abort handler
	.fnend

	/* fall through */
/*
 * This is the return code to user mode for abort handlers
 */
@b asm_dbg_nail

ENTRY(ret_from_exception)
	.fnstart
	.cantunwind
	get_thread_info tsk
	mov	why, #0
	b	ret_to_user
	.fnend
ENDPROC(__pabt_usr)
ENDPROC(ret_from_exception)


/*
 * "slow" syscall return path.  "why" tells us if this was a real syscall.
 */
	.align	5
ENTRY(ret_to_user)
	disable_irq				@ disable interrupts
	restore_user_regs fast = 0, offset = 0
ENDPROC(ret_to_user)


		
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
@	ldr r0, =0xef005020
@	mov r1, #0x42
@	str r1,[r0]
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

/*
 * Data abort dispatcher
 * Enter in ABT mode, spsr = USR CPSR, lr = USR PC
 */
	vector_stub	dabt, ABT_MODE, 8

	.long	__dabt_usr			@  0  (USR_26 / USR_32)
	.long	__dabt_invalid			@  1  (FIQ_26 / FIQ_32)
	.long	__dabt_invalid			@  2  (IRQ_26 / IRQ_32)
	.long	__dabt_svc			@  3  (SVC_26 / SVC_32)
	.long	__dabt_invalid			@  4
	.long	__dabt_invalid			@  5
	.long	__dabt_invalid			@  6
	.long	__dabt_invalid			@  7
	.long	__dabt_invalid			@  8
	.long	__dabt_invalid			@  9
	.long	__dabt_invalid			@  a
	.long	__dabt_invalid			@  b
	.long	__dabt_invalid			@  c
	.long	__dabt_invalid			@  d
	.long	__dabt_invalid			@  e
	.long	__dabt_invalid			@  f

/*
 * Prefetch abort dispatcher
 * Enter in ABT mode, spsr = USR CPSR, lr = USR PC
 */
	vector_stub	pabt, ABT_MODE, 4

	.long	__pabt_usr			@  0 (USR_26 / USR_32)
	.long	__pabt_invalid			@  1 (FIQ_26 / FIQ_32)
	.long	__pabt_invalid			@  2 (IRQ_26 / IRQ_32)
	.long	__pabt_svc			@  3 (SVC_26 / SVC_32)
	.long	__pabt_invalid			@  4
	.long	__pabt_invalid			@  5
	.long	__pabt_invalid			@  6
	.long	__pabt_invalid			@  7
	.long	__pabt_invalid			@  8
	.long	__pabt_invalid			@  9
	.long	__pabt_invalid			@  a
	.long	__pabt_invalid			@  b
	.long	__pabt_invalid			@  c
	.long	__pabt_invalid			@  d
	.long	__pabt_invalid			@  e
	.long	__pabt_invalid			@  f


	.align	5

/* Unsupported exception. */
vector_rst:
	b do_unsupported_exception

vector_und:
	b do_unsupported_exception


	.align	5
ENTRY(vector_swi)
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0 - r12
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^
	mrs	r8, spsr			@ called from non-FIQ mode, so ok.
	str	lr, [sp, #S_PC]			@ Save calling PC
	str	r8, [sp, #S_PSR]		@ Save CPSR
	str	r0, [sp, #S_OLD_R0]		@ Save OLD_R0

	enable_irq

	get_thread_info tsk
	adr	tbl, sys_call_table		@ load syscall table pointer

	cmp	scno, #NR_syscalls		@ check upper syscall limit
	adr	lr, ret_to_user			@ return address
	ldrcc	pc, [tbl, scno, lsl #2]		@ call sys_* routine

	mov r0,scno		@ error handler
	b do_invalid_swi
ENDPROC(vector_swi)

	.equ NR_syscalls,5
		
	.type	sys_call_table, #object
ENTRY(sys_call_table)
	.long sys_reset
	.long sys_print
	.long sys_brk
	.long sys_sleep
	.long sys_inputc		


		
vector_fiq:
	b do_unsupported_exception

vector_addrexcptn:
	b do_unsupported_exception


/*
 * We group all the following data together to optimise
 * for CPUs with separate I & D caches.
 */
	.align	5

.LCvswi:
	.word	vector_swi

	.globl	__stubs_end
__stubs_end:

	.equ	stubs_offset, __vectors_start + 0x200 - __stubs_start



	.globl	__vectors_start
__vectors_start:
	b	vector_rst + stubs_offset
	b	vector_und + stubs_offset
	ldr	pc, .LCvswi + stubs_offset
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

	
