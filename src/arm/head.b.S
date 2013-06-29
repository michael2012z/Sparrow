#include <memory.h>
#include "head.h"
#include <linkage.h>

#define KERNEL_RAM_PADDR 0x50008000
#define KERNEL_RAM_VADDR	(PAGE_OFFSET + TEXT_OFFSET)
#define KERNEL_START	KERNEL_RAM_VADDR
#define KERNEL_END	_end

	.macro	pgtbl, rd
	ldr	\rd, =(KERNEL_RAM_PADDR - 0x4000)
	.endm

		
	__HEAD
ENTRY(stext)
	msr cpsr_c, PSR_F_BIT | PSR_I_BIT | SVC_MODE
	@ ensure svc mode and irqs disabled
	mrc	p15, 0, r9, c0, c0		@ get processor id
	@ but not used anymore
	bl	__create_page_tables

	ldr	r13, __switch_data		@ address to jump to after
						@ mmu has been enabled

	bl __v6_setup

	b __enable_mmu		

ENDPROC(stext)

		
/*
 * Setup common bits before finally enabling the MMU.  Essentially
 * this is just loading the page table pointer and domain access
 * registers.
 */
__enable_mmu:
	orr	r0, r0, #CR_A
	mov	r5, #(domain_val(DOMAIN_USER, DOMAIN_MANAGER) | \
		      domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | \
		      domain_val(DOMAIN_TABLE, DOMAIN_MANAGER) | \
		      domain_val(DOMAIN_IO, DOMAIN_CLIENT))
	mcr	p15, 0, r5, c3, c0, 0		@ load domain access register
	mcr	p15, 0, r4, c2, c0, 0		@ load page table pointer
	b	__turn_mmu_on
ENDPROC(__enable_mmu)


/*
 * Enable the MMU.  This completely changes the structure of the visible
 * memory space.  You will not be able to trace execution through this.
 * If you have an enquiry about this, *please* check the linux-arm-kernel
 * mailing list archives BEFORE sending another post to the list.
 *
 *  r0  = cp#15 control register
 *  r13 = *virtual* address to jump to upon completion
 *
 * other registers depend on the function called upon completion
 */
	.align	5
__turn_mmu_on:
	mov	r0, r0
	mcr	p15, 0, r0, c1, c0, 0		@ write control reg
	mrc	p15, 0, r3, c0, c0, 0		@ read id reg
	mov	r3, r3
	mov	r3, r13
	mov	pc, r3
ENDPROC(__turn_mmu_on)


/*
 * Setup the initial page tables.  We only setup the barest
 * amount which are required to get the kernel running, which
 * generally means mapping in the kernel code.
 *
 * r8  = machinfo
 * r9  = cpuid
 * r10 = procinfo
 *
 * Returns:
 *  r0, r3, r6, r7 corrupted
 *  r4 = physical page table address
 */
__create_page_tables:
	/* load page table address to r4, table size is 16K=0x4000 */
	pgtbl	r4				@ page table address

	/*
	 * Clear the 16K level 1 swapper page table
	 * In fact, clearing the 1st item is enough,
	 * because all the table will be clear again in C code.
	 */
	mov	r0, r4
	mov	r3, #0
	/* r6 is the end of page table */
	add	r6, r0, #0x4000
1:	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
	teq	r0, r6
	bne	1b
/*
	ldr	r7, [r10, #PROCINFO_MM_MMUFLAGS] @ mm_mmuflags
*/
	ldr r7, =(PMD_TYPE_SECT | PMD_SECT_AP_WRITE | PMD_SECT_AP_READ | PMD_FLAGS)
	/*
	 * Create identity mapping for first MB of kernel to
	 * cater for the MMU enable.  This identity mapping
	 * will be removed by paging_init().  We use our current program
	 * counter to determine corresponding section base address.
	 */
	mov	r6, pc
	mov	r6, r6, lsr #20			@ start of kernel section
	orr	r3, r7, r6, lsl #20		@ flags + kernel base
	/* now, r6 hold the kernal_base(physical)/1M */
	/* now, r3 hold the content of first item of page table */
	/* interesting mapping! keep the address accross the mapping unchanged
	 * r3 = 0x5xxxxxxx
	 */
	str	r3, [r4, r6, lsl #2]		@ identity mapping

	/*
	 * Now setup the pagetables for our kernel direct
	 * mapped region.
	 */
	/*
	 * How to understand following operation?
	 * Index of a virtual address section in mapping table is VA/1M (>>20)
	 * But don't forget, each mapping item is 4 bytes long.
	 * So offset = index * 4 (<<2). Totally, >>18 is OK.
	 */
	add	r0, r4,  #(KERNEL_START & 0xff000000) >> 18
	str	r3, [r0, #(KERNEL_START & 0x00f00000) >> 18]!
	/* r0 hold the address of mapping item for kernel section */
	ldr	r6, =(KERNEL_END - 1)
	add	r0, r0, #4
	add	r6, r4, r6, lsr #18
1:	cmp	r0, r6
	add	r3, r3, #1 << 20
	strls	r3, [r0], #4
	bls	1b

	/*
	 * Then map first 1MB of ram in case it contains our boot params.
	 */
	add	r0, r4, #PAGE_OFFSET >> 18
	/* r0 hold the address of mapping item for 0xc0000000 */
	orr	r6, r7, #(PHYS_OFFSET & 0xff000000)
	.if	(PHYS_OFFSET & 0x00f00000)
	orr	r6, r6, #(PHYS_OFFSET & 0x00f00000)
	.endif
	str	r6, [r0]




	/*
	 * Map 64M memory:
	 * Physical memory between 0x5000 0000 ~ 0x5400 0000
	 * Virtual memory between  0xC000 0000 ~ 0xC400 0000
	 */
	mov r3, #0 /* r3 is index*/
1:	mov r0, r3, lsl #20
	add r0, r0, #PAGE_OFFSET
	mov r0, r0, lsr #18
	add	r0, r0, r4
	/* r0 hold the address of mapping table item for target section */
	mov r6, r3, lsl #20
	add r6, r6, #PHYS_OFFSET
	/* r6 hold the section address */
	orr	r6, r7, r6
	str	r6, [r0]
	add r3, #1
	teq r3, #64
	bne 1b


	/* Last megabytes was mapped twice, it's also mapped to 0xfff00000 */
	/* Map 64th M memory to 0xfff00000, it's for interrupt vector table.
	 */
	/* now r3 equals 63, it's the index of last section  */
	mov r3, #63 /* r3 is index*/
	mov r0, #0xff000000
	orr r0, r0, #0xf00000 /* don't know why can't mov 0xfff00000 to r6 directly */
	mov r0, r0, lsr #18
	add	r0, r0, r4
	/* r0 hold the address of mapping table item for target section */
	mov r6, r3, lsl #20
	add r6, r6, #PHYS_OFFSET
	/* r6 hold the section address */
	orr	r6, r7, r6
	str	r6, [r0]


	/*
	 * Map another 64M memory for ListFS archived file:
	 * Physical memory between 0x5800 0000 ~ 0x5C00 0000
	 * Virtual memory between  0xC800 0000 ~ 0xCC00 0000
	 * The mapping section index is between 128 ~ 191
	 */
	mov r3, #128 /* r3 is index*/
1:	mov r0, r3, lsl #20
	add r0, r0, #PAGE_OFFSET
	mov r0, r0, lsr #18
	add	r0, r0, r4
	/* r0 hold the address of mapping table item for target section */
	mov r6, r3, lsl #20
	add r6, r6, #PHYS_OFFSET
	/* r6 hold the section address */
	orr	r6, r7, r6
	str	r6, [r0]
	add r3, #1
	teq r3, #191
	bne 1b



	/*
	 * Map the 1M from 0x71200000 to 0xe1200000, that is for VIC0.
	 */
	ldr	r0, =((0xe1200000) >> 18)
	add	r0, r4, r0
	/* r0 hold the address of mapping item for 0xe1200000 */
	ldr r6, =(0x71200000)
	orr	r6, r7, r6
	str	r6, [r0]

	/*
	 * Map the 1M from 0x71300000 to 0xe1300000, that is for VIC1.
	 */
	ldr	r0, =((0xe1300000) >> 18)
	add	r0, r4, r0
	/* r0 hold the address of mapping item for 0xe1300000 */
	ldr r6, =(0x71300000)
	orr	r6, r7, r6
	str	r6, [r0]


	/* Note: Map 1M more to store user program.
	 * This is the 64th Megabytes.
	 * Map from 0x54000000 to 0xc4000000
	 */
	ldr	r0, =((0xc4000000) >> 18)
	add	r0, r4, r0
	/* r0 hold the address of mapping item for 0xc4000000 */
	ldr r6, =(0x54000000)
	orr	r6, r7, r6
	str	r6, [r0]


	/*
	 * Map the 1M contains 0x7f005020 to debug.
	 */
	add	r0, r4, #((0xef005020) >> 18)
	/* r0 hold the address of mapping item for 0xef000000 */
	orr	r6, r7, #0x7f000000
	str	r6, [r0]


	/* Mapping result:
	 * 0xC000 0000  ->  0x5000 0000 (size = kernel size)
	 * 0x5000 0000  ->  0x5000 0000 (size = 1M)
	 */
		
	mov	pc, lr
ENDPROC(__create_page_tables)
	.ltorg


/* Following code is called after MMU enabled. */

	.align	2
	.type	__switch_data, %object
__switch_data:
	.long	__mmap_switched
	.long	__data_loc			@ r4
	.long	_data				@ r5
	.long	__bss_start			@ r6
	.long	_end				@ r7
	.long	init_thread_union + THREAD_START_SP @ sp

/*
 * The following fragment of code is executed with the MMU on in MMU mode,
 * and uses absolute addresses; this is not position independent.
 *
 *  r0  = cp#15 control register
 *  r1  = machine ID
 *  r2  = atags pointer
 *  r9  = processor ID
 */
__mmap_switched:
	adr	r3, __switch_data + 4

	ldmia	r3, {r4, r5, r6, r7, sp}
	cmp	r4, r5				@ Copy data segment if needed
1:	cmpne	r5, r6
	ldrne	fp, [r4], #4
	strne	fp, [r5], #4
	bne	1b

	mov	fp, #0				@ Clear BSS (and zero fp)
1:	cmp	r6, r7
	strcc	fp, [r6],#4
	bcc	1b

	b	start_kernel
ENDPROC(__mmap_switched)

#include "v6.S"
