#include <linkage.h>
#include <ptrace.h>
#include <string.h>
#include "head.h"
#include <printk.h>


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
	printk(PR_SS_IRQ, PR_LVL_ERR, "A data abort happened, addr = %x\n", addr);
	while(1);
}

void __exception do_PrefetchAbort(unsigned long addr, unsigned int ifsr, struct pt_regs *regs)
{
	printk(PR_SS_IRQ, PR_LVL_ERR, "A prefetch abort happened, addr = %x\n", addr);
	while(1);
}

void __exception irq_usr_debug() {
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s\n", __func__);
}

void __exception irq_svc_debug() {
  printk(PR_SS_IRQ, PR_LVL_ERR, "%s\n", __func__);
}
