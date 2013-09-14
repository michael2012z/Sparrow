#ifndef _ARM_HEAD_H_
#define _ARM_HEAD_H_


/*
 * PSR bits
 */
#define USR26_MODE	0x00000000
#define FIQ26_MODE	0x00000001
#define IRQ26_MODE	0x00000002
#define SVC26_MODE	0x00000003
#define USR_MODE	0x00000010
#define FIQ_MODE	0x00000011
#define IRQ_MODE	0x00000012
#define SVC_MODE	0x00000013
#define ABT_MODE	0x00000017
#define UND_MODE	0x0000001b
#define SYSTEM_MODE	0x0000001f
#define MODE32_BIT	0x00000010
#define MODE_MASK	0x0000001f
#define PSR_T_BIT	0x00000020
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_A_BIT	0x00000100
#define PSR_E_BIT	0x00000200
#define PSR_J_BIT	0x01000000
#define PSR_Q_BIT	0x08000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000

#define PSR_ENDSTATE	0

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_A	(1 << 1)	/* Alignment abort enable		*/

/*
 * Domain
 */
#define domain_val(dom,type)	((type) << (2*(dom)))

#define DOMAIN_KERNEL	0
#define DOMAIN_TABLE	0
#define DOMAIN_USER	1
#define DOMAIN_IO	2

/*
 * Domain types
 */
#define DOMAIN_NOACCESS	0
#define DOMAIN_CLIENT	1
#define DOMAIN_MANAGER	3




#define PSR_ISETSTATE	0



#define S_R0 0 /* offsetof(struct pt_regs, ARM_r0)	@ */
#define S_R1 4 /* offsetof(struct pt_regs, ARM_r1)	@ */
#define S_R2 8 /* offsetof(struct pt_regs, ARM_r2)	@ */
#define S_R3 12 /* offsetof(struct pt_regs, ARM_r3)	@ */
#define S_R4 16 /* offsetof(struct pt_regs, ARM_r4)	@ */
#define S_R5 20 /* offsetof(struct pt_regs, ARM_r5)	@ */
#define S_R6 24 /* offsetof(struct pt_regs, ARM_r6)	@ */
#define S_R7 28 /* offsetof(struct pt_regs, ARM_r7)	@ */
#define S_R8 32 /* offsetof(struct pt_regs, ARM_r8)	@ */
#define S_R9 36 /* offsetof(struct pt_regs, ARM_r9)	@ */
#define S_R10 40 /* offsetof(struct pt_regs, ARM_r10)	@ */
#define S_FP 44 /* offsetof(struct pt_regs, ARM_fp)	@ */
#define S_IP 48 /* offsetof(struct pt_regs, ARM_ip)	@ */
#define S_SP 52 /* offsetof(struct pt_regs, ARM_sp)	@ */
#define S_LR 56 /* offsetof(struct pt_regs, ARM_lr)	@ */
#define S_PC 60 /* offsetof(struct pt_regs, ARM_pc)	@ */
#define S_PSR 64 /* offsetof(struct pt_regs, ARM_cpsr)	@ */
#define S_OLD_R0 68 /* offsetof(struct pt_regs, ARM_ORIG_r0)	@ */
#define S_FRAME_SIZE 72 /* sizeof(struct pt_regs)	@ */



#define TI_FLAGS 0 /* offsetof(struct thread_info, flags)	@ */
#define _TIF_WORK_MASK		0x000000ff


/*
@ Bad Abort numbers
@ -----------------
@
*/
#define BAD_PREFETCH	0
#define BAD_DATA	1
#define BAD_ADDREXCPTN	2
#define BAD_IRQ		3
#define BAD_UNDEFINSTR	4


#define TTB_S		(1 << 1)
#define TTB_RGN_WBWA	(1 << 3)
#define TTB_FLAGS	TTB_RGN_WBWA|TTB_S
#define TTB_FLAGS_UP	TTB_RGN_WBWA


/* offset of thread_info */
#define TI_TASK 0 /* offsetof(struct thread_info, task) @ */
#define TI_CPU_DOMAIN 4 /* offsetof(struct thread_info, cpu_domain)	@ */
#define TI_TP_VALUE 8 /* offsetof(struct thread_info, tp_value)	@ */
#define TI_CPU_CONTEXT 12 /* offsetof(struct thread_info, cpu_context)	@ */


#endif
