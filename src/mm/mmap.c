#include <type.h>
#include <errno.h>
#include <memory.h>
#include <printk.h>
#include <mm.h>
#include "mmap.h"

/*
 * mm_struct hold a list of vm_area_struct as vma_list
 */


struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr) {
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

/* Input parameters:
 *   mm       - mm struct pointer
 *   filep    - file pointer
 *   addr     - start address, not aligned
 *   len      - length of the area, not aligned
 *   offset   - content offset in file
 */
unsigned long do_mmap(struct mm_struct *mm, struct file *filep, unsigned long addr, unsigned long len, unsigned long offset)
{
  struct vm_area_struct *vma;
  int error;

  vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));

  printk(PR_SS_MM, PR_LVL_DBG6, "do_mmap():\n");
  printk(PR_SS_MM, PR_LVL_DBG6, "mm = %x, addr = %x, len = %x, offset = %x\n", mm, addr, len, offset);

  if (!vma) {
	error = -ENOMEM;
	return error;
  }

  vma->vm_mm = mm;
  /* align addr to get vma start */
  vma->vm_start = page_start(addr);  
  /* the offset from the beginning of the vma */
  vma->vm_offset = page_offset(addr);
  /* align addr + len to get vma end */
  vma->vm_end = page_align(addr+len);
  /* real content length */
  vma->vm_length = len;
  /* the offset from the beginning of the file */
  vma->vm_fileoffset = offset;
  /* file */
  vma->vm_file = (struct file *)kmalloc(sizeof(struct file));
  vma->vm_file->buf = filep->buf;
  vma->vm_file->size = filep->size;

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


void print_all_vma(struct mm_struct *mm) {
  struct vm_area_struct *vma = NULL;
  struct list_head *pos = NULL, *head = &mm->mmap.list;
  printk(PR_SS_MM, PR_LVL_DBG6, "%s:\n", __func__);

  list_for_each(pos, head) {
	vma = list_entry(pos, struct vm_area_struct, list);
	printk(PR_SS_MM, PR_LVL_DBG6, "%s: start = %x, end = %x, offset = %x, length = %x, file = %x\n", __func__, vma->vm_start, vma->vm_end, vma->vm_offset, vma->vm_length, vma->vm_file);
  }
}

int expand_stack(struct vm_area_struct *vma, unsigned long addr) {
  int ret = 0;
  if (addr >= vma->vm_end)
	ret = -1; /* exceed stack up-boundary */
  else if (addr >= vma->vm_start)
	ret = 1; /* in stack, not need to expand */
  else if (addr < vma->vm_end - STACK_SIZE_LIMIT)
	ret = -3; /* in unknown area, invalid memory access */
  else {
	unsigned long new_start = page_start(addr);
	printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: new_start = %x, vma->vm_start = %x\n", __func__, new_start, vma->vm_start);
	if (new_start < vma->vm_start) {
	  vma->vm_start = new_start;
	  vma->vm_length = vma->vm_end - vma->vm_start;
	}
	ret = 0;
  }
  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: ret = %x\n", __func__, ret);
  return ret;
}
