#include <type.h>
#include <linkage.h>
#include <printk.h>
#include <mm.h>
#include <process.h>

extern struct task_struct *current_task;

#define BAD_ADDR(x) ((unsigned long)(x) >= TASK_SIZE)

long sys_brk(unsigned long brk) {
  unsigned long start_heap = current_task->mm.start_heap;
  unsigned long end_heap = current_task->mm.end_heap;
  printk(PR_SS_IRQ, PR_LVL_DBG6, "%s: brk = %x, start_heap = %x, end_heap = %x\n", __func__, brk, start_heap, end_heap);

  /* query current brk */
  if (0 == brk)
	return current_task->mm.end_heap;
  else if (brk < start_heap) {
	printk(PR_SS_IRQ, PR_LVL_ERR, "%s: invalid brk request\n", __func__);
	return -1;
  } else if ((brk >= start_heap) && (brk <= end_heap)) {
	printk(PR_SS_IRQ, PR_LVL_ERR, "%s: heap has already covered brk\n", __func__);
	return brk;
  } else { /* brk > end_heap, need to map new memory */ 
	unsigned long addr;
	addr = do_brk(&(current_task->mm), end_heap, (brk-end_heap));
	if (BAD_ADDR(addr)) /* fail, return current heap end */
	  return end_heap;
	else {/* succeed, update heap end */
	  end_heap = page_align(brk);
	  current_task->mm.end_heap = end_heap;
	  return end_heap;
	}
  }

}
