#include <stdio.h>
#include "/usr/include/string.h"
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <type.h>
#include <mm.h>
#include <page_alloc.h>

extern struct page* pages_map;
extern struct page* free_list[];
extern struct page* frag_list[];
extern void * test_page_offset;

static struct page* free_list_status[BUDDY_MAX_ORDER+1];

void record_state_of_allocator() {
  int i;
  for(i = 0; i < (BUDDY_MAX_ORDER+1); i++)
	free_list_status[i] = free_list[i];
}

void alloc_test_case_init_common() {
  /* Initialize pages_map. */
  int i;
  struct page* page;
  int prefix_gap_pages; /* random size */
  int postfix_gap_pages; /* fixed size */

  prefix_gap_pages = 50 + random()%50; /* kernel size */
  postfix_gap_pages = 20; /* IRQ area size */

  //  printf("test_case_common_init(): %d pages reserved for kernel area\n", prefix_gap_pages);
  //  printf("test_case_common_init(): %d pages reserved for IRQ area\n", postfix_gap_pages);

  for (i = 0; i < MEMORY_PAGES; i++) {
	page = &pages_map[i];
	page->virtual = (addr)(test_page_offset + i*PAGE_SIZE);
	if ((i < prefix_gap_pages) || (i > (MEMORY_PAGES - postfix_gap_pages)))
	  page->allocated = (unsigned char)0xff;
	else
	  page->allocated = 0;
	INIT_LIST_HEAD(&(page->list));
	list_add_tail(&(page->list), &(pages_map->list));
  }

  /* Initialize the allocator. */
  page_alloc_init();

  record_state_of_allocator();
}

int is_page_allocator_restored () {
  int i, ret = 1;
  for(i = 0; i < (BUDDY_MAX_ORDER+1); i++)
	if (free_list_status[i] != free_list[i]) {
	  printf("Note, free_list_status[%d] = 0x%x, free_list[%d] = 0x%x.\n", i, (unsigned int)free_list_status[i], i, (unsigned int)free_list[i]);
	  ret = 0;
	}

  for(i = 0; i < (BUDDY_MAX_ORDER+1); i++)
	if (NULL != frag_list[i]) {
	  printf("Note, frag_list[%d] is not empty\n", i);
	  ret = 0;
	}

  return ret;
}
