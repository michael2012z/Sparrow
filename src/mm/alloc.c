#include <type.h>
#include <mm.h>
#include <printk.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif


void* kmalloc(int size) {
  if (size <= 0)
	return NULL;
  else if ((boot_alloc_ready == true) && (page_alloc_ready == false) && (slab_alloc_ready ==false)) {
	printk(PR_SS_MM, PR_LVL_DBG4, "boot allocator is choosen to allocate %d bytes\n", size);
	int n = (size + PAGE_SIZE - 1)/PAGE_SIZE;
	return bootmem_alloc(n);
  } else if (size <= (PAGE_SIZE/2)) {
	printk(PR_SS_MM, PR_LVL_DBG4, "slab allocator is choosen to allocate %d bytes\n", size);
	return (void*)slab_alloc(size);
  } else {
	struct page* pages;
	int n = (size + PAGE_SIZE - 1)/PAGE_SIZE;
	printk(PR_SS_MM, PR_LVL_DBG2, "page allocator is choosen to allocate %d pages\n", n);
	pages = pages_alloc(n);
	return (void*)pages->virtual;
  }
}

#ifdef __ARCH_X86__
extern void* test_page_offset;
#endif

void kfree(void *p) {
#ifdef __ARCH_X86__
  if (((unsigned int)p < (unsigned int)test_page_offset) || ((unsigned int)p > (unsigned int)(PAGE_OFFSET + MEMORY_SIZE))) {
#else
  if (((unsigned int)p < (unsigned int)PAGE_OFFSET) || ((unsigned int)p > (unsigned int)(PAGE_OFFSET + MEMORY_SIZE))) {
#endif
	printk(PR_SS_MM, PR_LVL_ERR, "returned memory %x is out of area\n", (unsigned int)p);
	return;
  }
  else {
	if ((boot_alloc_ready == true) && (page_alloc_ready == false) && (slab_alloc_ready ==false)) {
	  bootmem_free(p);
	} else {
	  struct page *pg, *buddy;
	  pg = (struct page*)virtual_to_page(p);
	  buddy = (struct page*)page_to_buddy(pg);
	  if ((NULL == buddy->manage.prev) && (NULL == buddy->manage.next)) {
		printk(PR_SS_MM, PR_LVL_DBG3, "returned memory %x was allocated by page allocator\n", (unsigned int)p);
		pages_free(buddy);
	  } else {
		printk(PR_SS_MM, PR_LVL_DBG5, "returned memory %x was allocated by slab allocator\n", (unsigned int)p);
		slab_free(p);
	  }
	}
  }
}
