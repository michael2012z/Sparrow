#include <type.h>
#include "page.h"
#include <math.h>
#include <printk.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

extern struct page* pages_map;
extern struct page* free_pages_head;
#ifdef __ARCH_X86__
extern void* test_page_offset;
#endif

struct page* virtual_to_page(void * virtual) {
#ifdef __ARCH_X86__
  int page_index = (int)((unsigned long)(virtual - test_page_offset)/(unsigned long)PAGE_SIZE);
#else
  int page_index = (int)((unsigned long)(virtual - PAGE_OFFSET)/(unsigned long)PAGE_SIZE);
#endif
  struct page* pg = pages_map + page_index;

  if(page_index < 0) {
	printk(PR_SS_MM, PR_LVL_DBG6, "invalid virtual address : %x\n", virtual);
	while(1);
	return 0;
  }

  return pg;
}

struct page* page_to_buddy(struct page *pg) {
  int offset = (pg - free_pages_head)%_order_of(pg->order);
  struct page* buddy = pg - offset;
  //  printk(PR_SS_MM, PR_LVL_DBG6, "page_to_buddy(): free_pages_head = %x, pg = %x, order = %x, buddy = %x", free_pages_head, pg, pg->order, buddy);

  return buddy;
}
