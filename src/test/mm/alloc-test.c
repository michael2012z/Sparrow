#include <stdio.h>
#include "/usr/include/string.h"
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <type.h>
#include <mm.h>
#include <slab_alloc.h>
#include "alloc-test-common.h"

static void test_case_common_init() {
  alloc_test_case_init_common();
  slab_alloc_init();
}

static int is_allocator_restored () {
  int i, ret = 1;

#ifndef SLAB_OPTIMIZE
  /* for optimization, empty slab keep a slab, page allocator can't be restored */
  ret = is_page_allocator_restored();  
  if (0 == ret)
	return ret;
#endif

  /* For each cache:
   * Empty set should has only 1 slab, and it should be empty.
   * Partial and full set shouldn't have any slab.
   */
  for (i = 0; i < SLAB_CACHE_COUNT; i++) {
	struct kcache* cache = get_kcache(i);
	struct slabset* full_set = &cache->full_set;
	struct slabset* partial_set = &cache->partial_set;
	struct slabset* empty_set = &cache->empty_set;
	CU_ASSERT(0 == full_set->slab_count);
	CU_ASSERT(list_empty(&full_set->slabs));
	CU_ASSERT(0 == partial_set->slab_count);
	CU_ASSERT(list_empty(&partial_set->slabs));
#ifdef SLAB_OPTIMIZE
	CU_ASSERT(1 == empty_set->slab_count);
	CU_ASSERT(!list_empty(&empty_set->slabs));
#else 
	CU_ASSERT(0 == empty_set->slab_count);
	CU_ASSERT(list_empty(&empty_set->slabs));
#endif
  }

  return ret;
}

static int random_object_size(int min, int max) {
  return (min + random()%(max - min + 1));
}

static void test_case_common_randomly(int min, int max, int allocation_max) {
  void *addr[allocation_max*2];
  int i = 0;
  int loc = 0;

  for(i = 0; i < allocation_max*2; i++)
	addr[i] = NULL;

  for(i = 0; i < allocation_max; i++) {
	void* ad = kmalloc(random_object_size(min, max));
	loc = random()%(allocation_max*2);

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, j;
	  for (j = loc; j < allocation_max*2; j++)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (j = loc; j >= 0; j--)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, j;
	  for (j = loc; j >= 0; j--)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (j = loc; j < allocation_max*2; j++)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  break;
		}
	}

  }

  for(i = 0; i < allocation_max*2; i++) {
	slab_free(addr[i]);
  }

}



static void test_case_slab_allocator_selection(void) {
  test_case_common_init();

  void *addr[1024];
  int i = 0;

  for(i = 0; i < 1024; i++) {
	addr[i] = kmalloc(random_object_size(1, 2048));
	struct page* p = (struct page*)virtual_to_page(addr[i]);
	p = (struct page*)page_to_buddy(p);
	/* should be allocated by slab allocator, so page->manage->* shouldn't be NULL */
	CU_ASSERT(NULL != p->manage.prev);
	CU_ASSERT(NULL != p->manage.next);
  }

  for(i = 0; i < 1024; i++) {
	kfree(addr[i]);
  }

  CU_ASSERT(is_allocator_restored());
}

static void test_case_page_allocator_selection(void) {
  test_case_common_init();
  void *addr[1024];
  int i = 0;

  for(i = 0; i < 1024; i++) {
	addr[i] = kmalloc(random_object_size(2049, 10*(PAGE_SIZE)));
	struct page* p = (struct page*)virtual_to_page(addr[i]);
	p = (struct page*)page_to_buddy(p);
	/* should be allocated by slab allocator, so page->manage->* should be NULL */
	CU_ASSERT(NULL == p->manage.prev);
	CU_ASSERT(NULL == p->manage.next);
  }

  for(i = 0; i < 1024; i++) {
	kfree(addr[i]);
  }

  CU_ASSERT(is_allocator_restored());
}

static void test_case_random_allocator(void) {
  test_case_common_init();
  test_case_common_randomly(1, 2048, 128);
  CU_ASSERT(is_allocator_restored());
}


static CU_TestInfo test_cases_array[] = {
  {"MM Allocator: test_case_slab_allocator_selection", test_case_slab_allocator_selection},
  {"MM Allocator: test_case_page_allocator_selection", test_case_page_allocator_selection},
  {"MM Allocator: test_case_random_allocator", test_case_random_allocator},
};

CU_TestInfo* get_mm_allocator_test_cases_array() {
  return test_cases_array;
}

int get_mm_allocator_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
