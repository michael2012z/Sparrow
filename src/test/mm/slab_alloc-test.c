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

static void test_case_common_forward(int min, int max, int allocation_max) {
  void *addr[allocation_max];
  int i = 0;

  for(i = 0; i < allocation_max; i++) {
	addr[i] = slab_alloc(random_object_size(min, max));
  }

  for(i = 0; i < allocation_max; i++) {
	slab_free(addr[i]);
  }
}

static void test_case_common_backward(int min, int max, int allocation_max) {
  void *addr[allocation_max];
  int i = 0;

  for(i = 0; i < allocation_max; i++) {
	addr[i] = slab_alloc(random_object_size(min, max));
  }

  for(i = allocation_max; i > 0; i--) {
	slab_free(addr[i-1]);
  }
}

static void test_case_common_randomly(int min, int max, int allocation_max) {
  void *addr[allocation_max*2];
  int i = 0;
  int loc = 0;

  for(i = 0; i < allocation_max*2; i++)
	addr[i] = NULL;

  for(i = 0; i < allocation_max; i++) {
	void* ad = slab_alloc(random_object_size(min, max));
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

static void test_case_common_randomly_writeread(int min, int max, int allocation_max) {
  void *addr[allocation_max*2];
  int size[allocation_max*2];
  int i = 0, m = 0;;
  int loc = 0;
  int success = 1;

  for(i = 0; i < allocation_max*2; i++) {
	addr[i] = NULL;
	size[i] = 0;
  }

  for(i = 0; i < allocation_max; i++) {
	int s = random_object_size(min, max);
	void* ad = slab_alloc(s);
	
	if (NULL == ad)
	  break;

	for(m = 0; m < s; m++)
	  *((char *)(ad + m)) = (char)0xff;


	loc = random()%(allocation_max*2);

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, j;
	  for (j = loc; j < allocation_max*2; j++)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  size[j] = s;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (j = loc; j >= 0; j--)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  size[j] = s;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, j;
	  for (j = loc; j >= 0; j--)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  size[j] = s;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (j = loc; j < allocation_max*2; j++)
		if (NULL == addr[j]) {
		  addr[j] = ad;
		  size[j] = s;
		  break;
		}
	}

  }

  for(i = 0; i < allocation_max*2; i++) {
	for(m = 0; m < size[i]; m++)
	  if (1 == success)
	  success = ((char)0xff == *((char *)(addr[i] + m)));
	slab_free(addr[i]);
  }

  CU_ASSERT(success);

}

static void test_case_common_randomly_abnormal(int min, int max, int allocation_max) {
  void *addr[allocation_max*2];
  int i = 0;
  int loc = 0;

  for(i = 0; i < allocation_max*2; i++)
	addr[i] = NULL;

  for(i = 0; i < allocation_max; i++) {
	void* ad = slab_alloc(random_object_size(min, max));
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
	/* Abnormal case. Lead a abnormal case every 16 normal cases. */
	int r = random()%16;

	slab_free(addr[i]);
	if (NULL == addr[i])
	  continue;
	
	if (0 == r) {
	  /* re-deallocate */
	  slab_free(addr[i]);	  
	} else if (1 == r) {
	  /* invalid address */
	  void* a = addr[i];
	  a += 2;
	  slab_free(a);
	}
  }

}


/* cache 1, 8 bytes */
static void test_case_cache01_1(void) {
  test_case_common_init();
  test_case_common_forward(1, 8, 4096);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache01_2(void) {
  test_case_common_init();
  test_case_common_backward(1, 8, 4096);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache01_3(void) {
  test_case_common_init();
  test_case_common_randomly(1, 8, 4096);
  CU_ASSERT(is_allocator_restored());
}

/* cache 2, 16 bytes */
static void test_case_cache02_1(void) {
  test_case_common_init();
  test_case_common_forward(9, 16, 4096);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache02_2(void) {
  test_case_common_init();
  test_case_common_backward(9, 16, 4096);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache02_3(void) {
  test_case_common_init();
  test_case_common_randomly(9, 16, 4096);
  CU_ASSERT(is_allocator_restored());
}

/* cache 3, 32 bytes */
static void test_case_cache03_1(void) {
  test_case_common_init();
  test_case_common_forward(17, 32, 2048);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache03_2(void) {
  test_case_common_init();
  test_case_common_backward(17, 32, 2048);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache03_3(void) {
  test_case_common_init();
  test_case_common_randomly(17, 32, 2048);
  CU_ASSERT(is_allocator_restored());
}

/* cache 4, 64 bytes */
static void test_case_cache04_1(void) {
  test_case_common_init();
  test_case_common_forward(33, 64, 1024);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache04_2(void) {
  test_case_common_init();
  test_case_common_backward(33, 64, 1024);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache04_3(void) {
  test_case_common_init();
  test_case_common_randomly(33, 64, 1024);
  CU_ASSERT(is_allocator_restored());
}

/* cache 5, 96 bytes */
static void test_case_cache05_1(void) {
  test_case_common_init();
  test_case_common_forward(65, 96, 1024);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache05_2(void) {
  test_case_common_init();
  test_case_common_backward(65, 96, 1024);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache05_3(void) {
  test_case_common_init();
  test_case_common_randomly(65, 96, 1024);
  CU_ASSERT(is_allocator_restored());
}

/* cache 6, 128 bytes */
static void test_case_cache06_1(void) {
  test_case_common_init();
  test_case_common_forward(97, 128, 512);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache06_2(void) {
  test_case_common_init();
  test_case_common_backward(97, 128, 512);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache06_3(void) {
  test_case_common_init();
  test_case_common_randomly(97, 128, 512);
  CU_ASSERT(is_allocator_restored());
}

/* cache 7, 192 bytes */
static void test_case_cache07_1(void) {
  test_case_common_init();
  test_case_common_forward(129, 192, 256);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache07_2(void) {
  test_case_common_init();
  test_case_common_backward(129, 192, 256);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache07_3(void) {
  test_case_common_init();
  test_case_common_randomly(129, 192, 256);
  CU_ASSERT(is_allocator_restored());
}

/* cache 8, 256 bytes */
static void test_case_cache08_1(void) {
  test_case_common_init();
  test_case_common_forward(193, 256, 128);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache08_2(void) {
  test_case_common_init();
  test_case_common_backward(193, 256, 128);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache08_3(void) {
  test_case_common_init();
  test_case_common_randomly(193, 256, 128);
  CU_ASSERT(is_allocator_restored());
}

/* cache 9, 512 bytes */
static void test_case_cache09_1(void) {
  test_case_common_init();
  test_case_common_forward(257, 512, 64);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache09_2(void) {
  test_case_common_init();
  test_case_common_backward(257, 512, 64);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache09_3(void) {
  test_case_common_init();
  test_case_common_randomly(257, 512, 64);
  CU_ASSERT(is_allocator_restored());
}

/* cache 10, 1024 bytes */
static void test_case_cache10_1(void) {
  test_case_common_init();
  test_case_common_forward(513, 1024, 32);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache10_2(void) {
  test_case_common_init();
  test_case_common_backward(513, 1024, 32);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache10_3(void) {
  test_case_common_init();
  test_case_common_randomly(513, 1024, 32);
  CU_ASSERT(is_allocator_restored());
}

/* cache 11, 2048 bytes */
static void test_case_cache11_1(void) {
  test_case_common_init();
  test_case_common_forward(1025, 2048, 16);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache11_2(void) {
  test_case_common_init();
  test_case_common_backward(1025, 2048, 16);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_cache11_3(void) {
  test_case_common_init();
  test_case_common_randomly(1025, 2048, 16);
  CU_ASSERT(is_allocator_restored());
}

/* randomly, normal cases */
static void test_case_all_normal_1(void) {
  test_case_common_init();
  test_case_common_randomly(1, 2048, 256);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_all_normal_2(void) {
  test_case_common_init();
  test_case_common_randomly(1, 2048, 512);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_all_normal_3(void) {
  test_case_common_init();
  test_case_common_randomly(1, 2048, 1024);
  CU_ASSERT(is_allocator_restored());
}

/* randomly, normal cases */
static void test_case_all_write_read_1(void) {
  test_case_common_init();
  test_case_common_randomly_writeread(1, 128, 4096);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_all_write_read_2(void) {
  test_case_common_init();
  test_case_common_randomly_writeread(129, 512, 2048);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_all_write_read_3(void) {
  test_case_common_init();
  test_case_common_randomly_writeread(513, 2048, 1024);
  CU_ASSERT(is_allocator_restored());
}

/* randomly, exception cases */
static void test_case_all_abnormal_1(void) {
  test_case_common_init();
  test_case_common_randomly_abnormal(9, 2048, 256);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_all_abnormal_2(void) {
  test_case_common_init();
  test_case_common_randomly_abnormal(9, 2048, 512);
  CU_ASSERT(is_allocator_restored());
}
static void test_case_all_abnormal_3(void) {
  test_case_common_init();
  test_case_common_randomly_abnormal(9, 2048, 1024);
  CU_ASSERT(is_allocator_restored());
}


static CU_TestInfo test_cases_array[] = {
  {"MM Slab Allocator: test_case_cache01_1", test_case_cache01_1},
  {"MM Slab Allocator: test_case_cache01_2", test_case_cache01_2},
  {"MM Slab Allocator: test_case_cache01_3", test_case_cache01_3},
  {"MM Slab Allocator: test_case_cache02_1", test_case_cache02_1},
  {"MM Slab Allocator: test_case_cache02_2", test_case_cache02_2},
  {"MM Slab Allocator: test_case_cache02_3", test_case_cache02_3},
  {"MM Slab Allocator: test_case_cache03_1", test_case_cache03_1},
  {"MM Slab Allocator: test_case_cache03_2", test_case_cache03_2},
  {"MM Slab Allocator: test_case_cache03_3", test_case_cache03_3},
  {"MM Slab Allocator: test_case_cache04_1", test_case_cache04_1},
  {"MM Slab Allocator: test_case_cache04_2", test_case_cache04_2},
  {"MM Slab Allocator: test_case_cache04_3", test_case_cache04_3},
  {"MM Slab Allocator: test_case_cache05_1", test_case_cache05_1},
  {"MM Slab Allocator: test_case_cache05_2", test_case_cache05_2},
  {"MM Slab Allocator: test_case_cache05_3", test_case_cache05_3},
  {"MM Slab Allocator: test_case_cache06_1", test_case_cache06_1},
  {"MM Slab Allocator: test_case_cache06_2", test_case_cache06_2},
  {"MM Slab Allocator: test_case_cache06_3", test_case_cache06_3},
  {"MM Slab Allocator: test_case_cache07_1", test_case_cache07_1},
  {"MM Slab Allocator: test_case_cache07_2", test_case_cache07_2},
  {"MM Slab Allocator: test_case_cache07_3", test_case_cache07_3},
  {"MM Slab Allocator: test_case_cache08_1", test_case_cache08_1},
  {"MM Slab Allocator: test_case_cache08_2", test_case_cache08_2},
  {"MM Slab Allocator: test_case_cache08_3", test_case_cache08_3},
  {"MM Slab Allocator: test_case_cache09_1", test_case_cache09_1},
  {"MM Slab Allocator: test_case_cache09_2", test_case_cache09_2},
  {"MM Slab Allocator: test_case_cache09_3", test_case_cache09_3},
  {"MM Slab Allocator: test_case_cache10_1", test_case_cache10_1},
  {"MM Slab Allocator: test_case_cache10_2", test_case_cache10_2},
  {"MM Slab Allocator: test_case_cache10_3", test_case_cache10_3},
  {"MM Slab Allocator: test_case_cache11_1", test_case_cache11_1},
  {"MM Slab Allocator: test_case_cache11_2", test_case_cache11_2},
  {"MM Slab Allocator: test_case_cache11_3", test_case_cache11_3},
  {"MM Slab Allocator: test_case_all_normal_1", test_case_all_normal_1},
  {"MM Slab Allocator: test_case_all_normal_2", test_case_all_normal_2},
  {"MM Slab Allocator: test_case_all_normal_3", test_case_all_normal_3},
  {"MM Slab Allocator: test_case_all_write_read_1", test_case_all_write_read_1},
  {"MM Slab Allocator: test_case_all_write_read_2", test_case_all_write_read_2},
  {"MM Slab Allocator: test_case_all_write_read_3", test_case_all_write_read_3},
  /*
  {"MM Slab Allocator: test_case_all_abnormal_1", test_case_all_abnormal_1},
  {"MM Slab Allocator: test_case_all_abnormal_2", test_case_all_abnormal_2},
  {"MM Slab Allocator: test_case_all_abnormal_3", test_case_all_abnormal_3},
  */
};

CU_TestInfo* get_mm_slab_allocator_test_cases_array() {
  return test_cases_array;
}

int get_mm_slab_allocator_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
