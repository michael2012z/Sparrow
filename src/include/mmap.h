#ifndef _MMAP_H_
#define _MMAP_H_

#include <type.h>
#include <memory.h>
#include <list.h>


struct page;
struct mm_struct;
struct vm_area_struct;


/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
struct vm_area_struct {
  struct mm_struct * vm_mm;	/* The address space we belong to. */
  unsigned long vm_start;		/* Our start address within vm_mm. */
  unsigned long vm_end;		/* The first byte after our end address
							   within vm_mm. */

  unsigned long vm_pgoff;		/* Offset (within vm_file) in PAGE_SIZE
								   units, *not* PAGE_CACHE_SIZE */
  struct file * vm_file;		/* File we map to (can be NULL). */

  /* linked list of VM areas per task, sorted by address */
  struct list_head list;
};

struct mm_struct {
  unsigned long pgd;			/* base address of page table */
  struct vm_area_struct mmap;		/* list of VMAs */
  unsigned long mmap_base;		/* base of mmap area */
  unsigned long task_size;		/* size of task vm space */

  int map_count;				/* number of VMAs */

  unsigned long start_code, end_code, start_data, end_data;
  unsigned long start_brk, brk, start_stack;
  unsigned long arg_start, arg_end, env_start, env_end;

};



/* VMA management */

struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr);

bool add_vma(struct mm_struct *mm, struct vm_area_struct *vma);

bool remove_vma(struct mm_struct *mm, struct vm_area_struct *vma);


unsigned long mmap_region(struct mm_struct *mm, struct file *file, unsigned long addr, unsigned long len, unsigned long pgoff);
unsigned long do_mmap(struct mm_struct *mm, struct file *file, unsigned long addr, unsigned long len, unsigned long offset);
unsigned long do_brk(struct mm_struct *mm, unsigned long addr, unsigned long len);
void add_page_anon_vma(struct page *page, struct vm_area_struct *vma, unsigned long address);

#endif
