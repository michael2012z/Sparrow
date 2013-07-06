#include <type.h>
#include <errno.h>
#include <memory.h>
#include <printk.h>
#include <mm.h>
#include "mmap.h"

/*
 * mm_struct hold a list of vm_area_struct as vma_list
 */


struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
  struct vm_area_struct *vma = NULL;
  struct list_head *pos = NULL, *head = &mm->mmap.list;

  list_for_each(pos, head) {
	vma = list_entry(pos, struct vm_area_struct, list);
	if ((addr >= vma->vm_start) && (addr < vma->vm_end))
	  break;
	else
	  vma = NULL;
  }

  return vma;
}


bool add_vma(struct mm_struct *mm, struct vm_area_struct *vma) {
  struct vm_area_struct *current = NULL;
  struct list_head *pos = NULL, *head = &mm->mmap.list;

  printk(PR_SS_MM, PR_LVL_DBG6, "add_vma():\n");
  printk(PR_SS_MM, PR_LVL_DBG6, " mm = %x, vma = %x, vma->vm_start = %x, vma->vm_end = %x\n", mm, vma, vma->vm_start, vma->vm_end);

  /* Go through all vma. */
  list_for_each(pos, head) {
	current = list_entry(pos, struct vm_area_struct, list);
	printk(PR_SS_MM, PR_LVL_DBG6, " current = %x, current->vm_start = %x, current_vm_end = %x\n", current, current->vm_start, current->vm_end);
	if (vma->vm_start < current->vm_start) {
	  /* Found the first vma with start address higher. */
	  if (vma->vm_end > current->vm_start) {
		/* Check for conflict. */
		printk(PR_SS_MM, PR_LVL_DBG6, " return false 1\n");
		return false;
	  } else {
		list_add_tail(&vma->list, pos);
		printk(PR_SS_MM, PR_LVL_DBG6, " return true 2\n");
		return true;
	  }
	}
  }

  /* The address is highest, add to tail. */
  list_add_tail(&vma->list, head);
  printk(PR_SS_MM, PR_LVL_DBG6, " return true 3\n");
  return true;
}


bool remove_vma(struct mm_struct *mm, struct vm_area_struct *vma) {
  if (vma == &mm->mmap) {
	/* The first vma is address 0, reserved. */
	return false;
  } else {
	list_del(&vma->list);
	return true;
  }
}


unsigned long do_mmap(struct mm_struct *mm, struct file *filep, unsigned long addr, unsigned long len, unsigned long offset)
{
  unsigned long pgoff = offset >> PAGE_SHIFT;
  struct vm_area_struct *vma;
  int error;

  // test
  static struct vm_area_struct vma1, vma2, vmab1, vmab2;
  // test ~

  // test
  //  vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));

  printk(PR_SS_MM, PR_LVL_DBG6, "do_mmap():\n");
  printk(PR_SS_MM, PR_LVL_DBG6, "mm = %x, addr = %x, len = %x, offset = %x, pgoff = %x\n", mm, addr, len, offset, pgoff);

  if(NULL != filep) {
	if (filep->buf == (void *)0xc4040000) // demo 1
	  vma = &vma1;
	if (filep->buf == (void *)0xc4080000) // demo 2
	  vma = &vma2;
  } else {
	if (NULL == vmab1.vm_mm)
	  vma = &vmab1;
	else
	  vma = &vmab2;
  }
  // test ~

  if (!vma) {
	error = -ENOMEM;
	return error;
  }

  vma->vm_mm = mm;
  vma->vm_start = addr;
  vma->vm_end = addr + len;
  vma->vm_file = filep;
  vma->vm_pgoff = pgoff;

  add_vma(mm, vma);

  return addr;
}


unsigned long do_brk(struct mm_struct *mm, unsigned long addr, unsigned long len)
{
  return do_mmap(mm, NULL, addr, len, 0);
}

void add_page_anon_vma(struct page *page, struct vm_area_struct *vma, unsigned long address) {
  printk(PR_SS_MM, PR_LVL_DBG6, "	add_page_anon_vma(): fatal error, function not implemented.\n");
  while(1);
}
