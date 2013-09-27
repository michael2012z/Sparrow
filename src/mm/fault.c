#include <type.h>
#include <memory.h>
#include <ptrace.h>
#include <mmap.h>
#include <mm.h>

static void do_memory_access_fault() {
  return;
}

static void do_out_of_memory() {
  return;
}

int do_page_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
  if (0)
	do_memory_access_fault();
  if (0)
	do_out_of_memory();
  return 0;
}

