#include <type.h>
#include <linkage.h>
#include <ptrace.h>
#include <string.h>
#include "head.h"
#include <printk.h>
#include <mm.h>
#include <process.h>

extern struct task_struct *current_task;


static int do_unknown_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs) {
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s: an unknown fault happened, fsr = %d\n", __func__, fsr);
  while(1);
  return 0;
}

/*
 * bad_mode handles the impossible case in the vectors.  If you see one of
 * these, then it's extremely serious, and could mean you have buggy hardware.
 * It never returns, and never tries to sync.  We hope that we can at least
 * dump out some state information...
 */
void bad_mode(struct pt_regs *regs, int reason)
{

}


/*
 * General handling for unsupported exception.
 */
void __exception asm_unsupported_exception(unsigned int code)
{
	printk(PR_SS_IRQ, PR_LVL_ERR, "An unsupported exception happened, code = %d\n", code);
	while(1);
}


/*
 * Dispatch a data abort to the relevant handler.
 */
void __exception do_DataAbort(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s: A data abort happened, addr = %x\n", __func__, addr);
  while(1);
}

void __exception do_PrefetchAbort(unsigned long addr, unsigned int ifsr, struct pt_regs *regs)
{
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s: A prefetch abort happened, addr = %x, ifsr = %x\n", __func__, addr, ifsr);
  switch(ifsr) {
  case 5:
	do_translation_fault(&(current_task->mm), addr, ifsr);
	break;
  default:
	do_unknown_fault(addr, ifsr, regs);
	break;
  }
}

void __exception irq_usr_debug() {
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s\n", __func__);
}

void __exception irq_svc_debug() {
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s\n", __func__);
}

extern unsigned long mm_pgd;

void asm_dbg_nail() {
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s\n", __func__);
  register unsigned long _ARM_r0 asm ("r0");
  register unsigned long _ARM_r1 asm ("r1");
  register unsigned long _ARM_r2 asm ("r2");
  register unsigned long _ARM_r3 asm ("r3");
  register unsigned long _ARM_r4 asm ("r4");
  register unsigned long _ARM_r5 asm ("r5");
  register unsigned long _ARM_r6 asm ("r6");
  register unsigned long _ARM_r7 asm ("r7");
  register unsigned long _ARM_r8 asm ("r8");
  register unsigned long _ARM_r9 asm ("r9");
  register unsigned long _ARM_r10 asm ("r10");
  register unsigned long _ARM_fp asm ("fp");
  register unsigned long _ARM_ip asm ("ip");
  register unsigned long _ARM_sp asm ("sp");
  register unsigned long _ARM_lr asm ("lr");
  register unsigned long _ARM_pc asm ("pc");
  //  register unsigned long cpsr asm ("cpsr");
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r0       = %x\n", __func__, _ARM_r0);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r1       = %x\n", __func__, _ARM_r1);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r2       = %x\n", __func__, _ARM_r2);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r3       = %x\n", __func__, _ARM_r3);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r4       = %x\n", __func__, _ARM_r4);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r5       = %x\n", __func__, _ARM_r5);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r6       = %x\n", __func__, _ARM_r6);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r7       = %x\n", __func__, _ARM_r7);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r8       = %x\n", __func__, _ARM_r8);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r9       = %x\n", __func__, _ARM_r9);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_r10      = %x\n", __func__, _ARM_r10);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_fp       = %x\n", __func__, _ARM_fp);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_ip       = %x\n", __func__, _ARM_ip);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_sp       = %x\n", __func__, _ARM_sp);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_lr       = %x\n", __func__, _ARM_lr);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_pc       = %x\n", __func__, _ARM_pc);
  //  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_cpsr     = %x\n", __func__, cpsr);
  //  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_cpsr     = %x\n", __func__, cpsr);
  //  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: _ARM_ORIG_r0  = %x\n", __func__, _ARM_ORIG_r0);

  // check pmd
  pgd_t *pc_pgd = pgd_offset(((pgd_t *)current_task->mm.pgd), 0x8000);
  printk(PR_SS_PROC, PR_LVL_DBG3, "%s: pc pgd:     %x = %x\n", __func__, pc_pgd, *pc_pgd);  

  while(1);
}

