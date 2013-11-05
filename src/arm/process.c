#include <ptrace.h>
#include <string.h>
#include "head.h"
#include <printk.h>
#include <list.h>
#include <type.h>
#include <process.h>
#include <vfs.h>

void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp) {
  //	unsigned long *stack = (unsigned long *)sp;
  memset(regs->uregs, 0, sizeof(regs->uregs));
  regs->ARM_cpsr = USR_MODE;
  regs->ARM_cpsr |= PSR_ENDSTATE;
  regs->ARM_pc = pc & ~1;		/* pc */
  regs->ARM_sp = sp;		/* sp */
  //	regs->ARM_r2 = stack[2];	/* r2 (envp) */
  //	regs->ARM_r1 = stack[1];	/* r1 (argv) */
  //	regs->ARM_r0 = stack[0];	/* r0 (argc) */
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

extern struct task_struct *current_task;

void kernel_thread_exit(void) {
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ---------------------------------------------\n", __func__); 
  current_task->sched_en.state = PROCESS_STATE_DEAD;
  while(1);
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

void arm_create_user_thread(int (*fn)(char *), void *arg, struct pt_regs *regs)
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
  printk(PR_SS_INI, PR_LVL_INF, "cpu_idle() \n");
  current_task->sched_en.state = PROCESS_STATE_WAITING;
  current_task->sched_en.waiting_type = PROCESS_WAITING_TYPE_TIME;
  current_task->sched_en.wake_up_jiffy = 0xfffff000; /* never wake up */
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

extern struct task_struct *current_task;
int arm_kernel_execve(char *filename, char *const argv[], char *const envp[])
{
  struct pt_regs* regs = &current_task->regs;
  int ret;
  struct file exe_file;
  vfs_node* file;

  file = vfs_find_node(filename);
  if (NULL == file)
	goto out;

  exe_file.buf = file->file.addr;
  exe_file.size = file->file.size;

  memset(regs, 0, sizeof(struct pt_regs));

  ret = execute_binary(current_task, &exe_file);
  if (ret < 0)
	goto out;
  
  /*
   * Save argc to the register structure for userspace.
   */
  regs->ARM_r0 = ret;
  
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: target SP value = %x\n", __func__, ((unsigned long)task_thread_info(current_task) + ((unsigned long)THREAD_START_SP - (unsigned long)sizeof(*regs))));
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: going to push process %d into user mode\n", __func__, current_task->pid); 
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: task_thread_info = %x\n", __func__, task_thread_info(current_task));   
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: THREAD_START_SP = %x, sizeof(*regs) = %x\n", __func__, THREAD_START_SP, sizeof(*regs));
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: target SP value = %x\n", __func__, ((unsigned long)task_thread_info(current_task) + ((unsigned long)THREAD_START_SP - (unsigned long)sizeof(*regs))));
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: target SP value = %x\n", __func__, ((unsigned long)task_thread_info(current_task) + ((unsigned long)THREAD_START_SP - (unsigned long)sizeof(*regs))));
  printk(PR_SS_PROC, PR_LVL_DBG1, "%s: target SP value = %x\n", __func__, ((unsigned long)task_thread_info(current_task) + ((unsigned long)THREAD_START_SP - (unsigned long)sizeof(*regs))));

  print_regs(regs);

  //  while(1);
  /*
   * We were successful.  We won't be returning to our caller, but
   * instead to user space by manipulating the kernel stack.
   */
  asm(	"add	r0, %0, %1\n\t"
	"mov	r1, %2\n\t"
	"mov	r2, %3\n\t"
	"bl	memmove\n\t"	/* copy regs to top of stack */
	"mov	r8, #0\n\t"	/* not a syscall */
	"mov	r9, %0\n\t"	/* thread structure */
	"mov	sp, r0\n\t"	/* reposition stack pointer */
	//	"bl asm_dbg_nail\n\t"
	"b	ret_to_user"
	:
	: "r" (task_thread_info(current_task)),
	  "Ir" (THREAD_START_SP - sizeof(*regs)),
	  "r" (regs),
	  "Ir" (sizeof(*regs))
	: "r0", "r1", "r2", "r3", "ip", "lr", "memory");
  
 out:
	return ret;
}

void print_regs (struct pt_regs* regs) {
  printk(PR_SS_INI, PR_LVL_ERR, "%s: print ARM registers: %x\n", __func__, regs); 
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r0       = %x\n", __func__, regs->ARM_r0);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r1       = %x\n", __func__, regs->ARM_r1);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r2       = %x\n", __func__, regs->ARM_r2);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r3       = %x\n", __func__, regs->ARM_r3);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r4       = %x\n", __func__, regs->ARM_r4);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r5       = %x\n", __func__, regs->ARM_r5);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r6       = %x\n", __func__, regs->ARM_r6);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r7       = %x\n", __func__, regs->ARM_r7);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r8       = %x\n", __func__, regs->ARM_r8);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r9       = %x\n", __func__, regs->ARM_r9);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_r10      = %x\n", __func__, regs->ARM_r10);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_fp       = %x\n", __func__, regs->ARM_fp);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_ip       = %x\n", __func__, regs->ARM_ip);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_sp       = %x\n", __func__, regs->ARM_sp);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_lr       = %x\n", __func__, regs->ARM_lr);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_pc       = %x\n", __func__, regs->ARM_pc);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_cpsr     = %x\n", __func__, regs->ARM_cpsr);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_cpsr     = %x\n", __func__, regs->ARM_cpsr);
  printk(PR_SS_INI, PR_LVL_ERR, "%s: ARM_ORIG_r0  = %x\n", __func__, regs->ARM_ORIG_r0);
}
