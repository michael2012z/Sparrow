#include <linkage.h>
#include <interrupt.h>
#include "vic.h"
#include "head.h"


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

	
