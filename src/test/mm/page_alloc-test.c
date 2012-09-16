#include <stdio.h>
#include "/usr/include/string.h"
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <type.h>
#include <mm.h>
#include <page_alloc.h>
#include "alloc-test-common.h"

extern struct page* free_list[];
extern struct page* frag_list[];

static void test_case_common_init() {
  alloc_test_case_init_common();
}

static int is_allocator_restored () {
  int ret = 1;

  ret = is_page_allocator_restored();

  if(0 == ret) {
	print_free_list();
	print_frag_list();
  }

  return ret;
}

static void test_case_01(void) {
  /* allocate and free 0 page */
  test_case_common_init();

  struct page* pages = pages_alloc(0);
  pages_free(pages);

  CU_ASSERT(is_allocator_restored());
}

static void test_case_02(void) {
  /* allocate and free 1 page */
  test_case_common_init();
  struct page* pages = pages_alloc(1);
  pages_free(pages);
  CU_ASSERT(is_allocator_restored());
}

static void test_case_03(void) {
  /* allocate pages in arithmetic series 1~20, and free them from beginning */
  test_case_common_init();

  struct page* pages[20];
  int i;

  for (i = 0; i < 20; i++) {
	pages[i] = pages_alloc(i+1);
	CU_ASSERT((NULL != pages[i]));
  }
  for (i = 0; i < 20; i++)
	pages_free(pages[i]);

  CU_ASSERT(is_allocator_restored());
}

static void test_case_04(void) {
  /* allocate pages in arithmetic series 1~20, and free them from end */
  test_case_common_init();

  struct page* pages[20];
  int i;

  for (i = 0; i < 20; i++) {
	pages[i] = pages_alloc(i+1);
	CU_ASSERT((NULL != pages[i]));
  }
  for (i = 19; i >= 0; i--)
	pages_free(pages[i]);

  CU_ASSERT(is_allocator_restored());
}

static void test_case_05(void) {
  /* allocate pages in arithmetic series till memory exhausted, and free them from beginning */
  test_case_common_init();

  struct page* pages[192];
  int i, last_size;

  for (i = 0; i < 192; i++) {
	pages[i] = pages_alloc(i+1);
	if(NULL == pages[i]) 
	  break;
  }
  last_size= i-1;
  for (i = 0; i <= last_size; i++)
	pages_free(pages[i]);

  CU_ASSERT(is_allocator_restored());
}

static void test_case_06(void) {
  /* allocate pages in arithmetic series till memory exhausted, and free them from end */
  test_case_common_init();

  struct page* pages[192];
  int i, last_size;

  for (i = 0; i < 192; i++) {
	pages[i] = pages_alloc(i+1);
	if(NULL == pages[i]) 
	  break;
  }
  last_size= i-1;
  for (i = last_size; i >= 0; i--)
	pages_free(pages[i]);

  CU_ASSERT(is_allocator_restored());
}

static int getFabonacci(int i) {
  static int fab[200] = {-1};
  if((i<0) || (i>=200))
	return -1;
  /* construct the array at first call */
  if (-1 == fab[0]) {
	fab[0] = 1;
	fab[1] = 1;
	int i = 2;
	for (i = 2; i < 200; i++)
	  fab[i] = fab[i-1] + fab[i-2];
  }
  return fab[i];
}

static void test_case_07(void) {
  /* allocate pages in Fabonacci series till memory exhausted, and free them from beginning */
  test_case_common_init();

  struct page* pages[192];
  int i, last_size;

  for (i = 0; i < 192; i++) {
	pages[i] = pages_alloc(getFabonacci(i));
	if(NULL == pages[i]) 
	  break;
  }
  last_size= i-1;
  for (i = 0; i <= last_size; i++)
	pages_free(pages[i]);

  CU_ASSERT(is_allocator_restored());
}

static void test_case_08(void) {
  /* allocate pages in Fabonacci series till memory exhausted, and free them from end */
  test_case_common_init();

  struct page* pages[192];
  int i, last_size;

  for (i = 0; i < 192; i++) {
	pages[i] = pages_alloc(getFabonacci(i));
	if(NULL == pages[i]) 
	  break;
  }
  last_size= i-1;
  for (i = last_size; i >= 0; i--)
	pages_free(pages[i]);

  CU_ASSERT(is_allocator_restored());
}

static void test_case_09(void) {
  /* allocate pages randomly, and free them randomly */
  test_case_common_init();

	print_free_list();
	print_frag_list();

  struct page* pages[4096] = {NULL}; /* prepare 4096 locations */
  struct page* p = NULL;
  int size, loc, j;

  /* only allocate 2048 times */
  for (j = 0; j < 1024; j++) {
	/* size 1~12 */
	size = random()%12 + 1;
	p = pages_alloc(size);

	//	printf("0x%x is allocated for %d pages\n", (unsigned int)p, size);
	if (NULL == p)
	  break;

	loc = random()%4096;

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, i;
	  for (i = loc; i < 4096; i++)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i >= 0; i--)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, i;
	  for (i = loc; i >= 0; i--)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i < 4096; i++)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  break;
		}
	}
  }

  for (j = 0; j < 4096; j++) {
	if (pages[j] != NULL) {
	  pages_free(pages[j]);
	  pages[j] = NULL;
	}
  }

  CU_ASSERT(is_allocator_restored());
}


static void test_case_10(void) {
  /* allocate pages randomly, and free them randomly */
  /* After allocating a page, write 0xff to every page, and check the writen value before freeing the page. */
  test_case_common_init();

  struct page* pages[4096] = {NULL}; /* prepare 4096 locations */
  int pages_cnt[4096] = {0};
  struct page* p = NULL;
  int size, loc, j, m;
  int success = 1;

  /* only allocate 2048 times */
  for (j = 0; j < 1024; j++) {
	/* size 1~12 */
	size = random()%12 + 1;
	p = pages_alloc(size);

	//	printf("0x%x is allocated for %d pages\n", (unsigned int)p, size);
	if (NULL == p)
	  break;

	/* Write the memory. */
	for (m = 0; m < size*PAGE_SIZE; m++)
	  *((char *)(p->virtual + m)) = (char)0xff;
	
	loc = random()%4096;

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, i;
	  for (i = loc; i < 4096; i++)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  pages_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i >= 0; i--)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  pages_cnt[i] = size;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, i;
	  for (i = loc; i >= 0; i--)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  pages_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i < 4096; i++)
		if (NULL == pages[i]) {
		  pages[i] = p;
		  pages_cnt[i] = size;
		  break;
		}
	}
  }

  for (j = 0; j < 4096; j++) {
	if (pages[j] != NULL) {
	  for (m = 0; m < pages_cnt[j]*PAGE_SIZE; m++)
		if (1 == success)
		  success = ((char)0xff == *((char *)(pages[j]->virtual + m)));
	  pages_free(pages[j]);
	  pages[j] = NULL;
	}
  }

  CU_ASSERT(success);
  CU_ASSERT(is_allocator_restored());
}



static CU_TestInfo test_cases_array[] = {
  {"MM Page Allocator: test_case_01", test_case_01},
  {"MM Page Allocator: test_case_02", test_case_02},
  {"MM Page Allocator: test_case_03", test_case_03},
  {"MM Page Allocator: test_case_04", test_case_04},
  {"MM Page Allocator: test_case_05", test_case_05},
  {"MM Page Allocator: test_case_06", test_case_06},
  {"MM Page Allocator: test_case_07", test_case_07},
  {"MM Page Allocator: test_case_08", test_case_08},
  {"MM Page Allocator: test_case_09", test_case_09},
  {"MM Page Allocator: test_case_10", test_case_10},
};

CU_TestInfo* get_mm_page_allocator_test_cases_array() {
  return test_cases_array;
}

int get_mm_page_allocator_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
