#include <stdio.h>
#include <CUnit/Basic.h>
#include <stdlib.h>
#include <type.h>
#include <memory.h>

#define printf 

#define BUFFER_LEADING_BYTES 8
#define MIN_ALLOCATION_SIZE 4
#define BUFFER_TRAILING_BYTES 4
#define MIN_BUFFER_SIZE (BUFFER_LEADING_BYTES + MIN_ALLOCATION_SIZE + BUFFER_TRAILING_BYTES)

extern void *my_malloc(int size);
extern void my_free(void *ptr);

unsigned long sim_heap_start = 0;
unsigned long sim_heap_end = 0;

/*  
 * If brk == 0, return current heap border
 */
unsigned long _brk(unsigned long brk) {
  printf("%s: brk = 0x%x, sim_heap_start = 0x%x, sim_heap_end = 0x%x\n", __func__, (unsigned int)brk, (unsigned int)sim_heap_start, (unsigned int)sim_heap_end);
  if (0 == brk)
	return sim_heap_end;
  else {
	sim_heap_end = (brk + 0x1000 - 1) & 0xfffff000;
	printf("%s: new sim_heap_end = 0x%x\n", __func__, (unsigned int)sim_heap_end);
	return sim_heap_end;
  }
}

extern unsigned long heap_start;
extern unsigned long heap_end;

static void test_case_01(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start + ((1 + 3) & 0xfffffffc) + BUFFER_LEADING_BYTES + BUFFER_TRAILING_BYTES);
  void *p = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p = my_malloc(1);
  CU_ASSERT(NULL != p);
  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);
  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_02(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start + ((18 + 3) & 0xfffffffc) + BUFFER_LEADING_BYTES + BUFFER_TRAILING_BYTES);
  void *p = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p = my_malloc(18);
  CU_ASSERT(NULL != p);
  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_03(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start + ((52 + 3) & 0xfffffffc) + BUFFER_LEADING_BYTES + BUFFER_TRAILING_BYTES);
  void *p = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p = my_malloc(52);
  CU_ASSERT(NULL != p);
  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_04(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *p = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p = my_malloc(52);
  CU_ASSERT(NULL != p);

  my_free(p);

  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_05(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *p1 = NULL, *p2 = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p1 = my_malloc(52);
  CU_ASSERT(NULL != p1);

  my_free(p1);

  p2 = my_malloc(96);
  CU_ASSERT(NULL != p2);

  my_free(p2);


  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_06(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *p1 = NULL, *p2 = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p1 = my_malloc(52);
  CU_ASSERT(NULL != p1);

  p2 = my_malloc(96);
  CU_ASSERT(NULL != p2);

  my_free(p1);

  my_free(p2);


  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_07(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *p1 = NULL, *p2 = NULL;
  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  p1 = my_malloc(52);
  CU_ASSERT(NULL != p1);

  p2 = my_malloc(96);
  CU_ASSERT(NULL != p2);

  my_free(p2);

  my_free(p1);

  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_08(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *buff[4096] = {NULL}; /* prepare 4096 locations */
  int buff_cnt[4096] = {0};
  void *p = NULL;
  int size, loc, j, m;

  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  printf("\n");

  for (j = 0; j < 8; j++) {
	/* size 1~12 */
	size = random()%128 + 1;
	printf("%s: j = %d, size = 0x%x\n", __func__, j, size);

	p = my_malloc(size);
	printf("%s: j = %d, p = 0x%x\n", __func__, j, (unsigned int)p);

	if (NULL == p)
	  break;

	loc = random()%4096;

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, i;
	  for (i = loc; i < 4096; i++)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i >= 0; i--)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, i;
	  for (i = loc; i >= 0; i--)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i < 4096; i++)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  break;
		}
	}
  }

  /* free buffers */
  for (j = 0; j < 4096; j++) {
	if (buff[j] != NULL) {
	  my_free(buff[j]);
	  buff[j] = NULL;
	}
  }

  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_09(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *buff[4096] = {NULL}; /* prepare 4096 locations */
  int buff_cnt[4096] = {0};
  void *p = NULL;
  int size, loc, j, m;

  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  for (j = 0; j < 32; j++) {
	/* size 1~12 */
	size = random()%128 + 1;
	p = my_malloc(size);

	if (NULL == p)
	  break;

	loc = random()%4096;

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, i;
	  for (i = loc; i < 4096; i++)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i >= 0; i--)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, i;
	  for (i = loc; i >= 0; i--)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i < 4096; i++)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  break;
		}
	}
  }

  /* free buffers */
  for (j = 0; j < 4096; j++) {
	if (buff[j] != NULL) {
	  my_free(buff[j]);
	  buff[j] = NULL;
	}
  }

  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static void test_case_10(void) {
  unsigned long expected_heap_end = page_align(sim_heap_start);
  void *buff[4096] = {NULL}; /* prepare 4096 locations */
  int buff_cnt[4096] = {0};
  void *p = NULL;
  int size, loc, j, m;

  heap_end = heap_start = 0;
  sim_heap_end = sim_heap_start;

  for (j = 0; j < 128; j++) {
	/* size 1~12 */
	size = random()%128 + 1;
	p = my_malloc(size);

	if (NULL == p)
	  break;

	loc = random()%4096;

	if (loc%2 == 0) { /* try upper location at first, then lower location */
	  int loc_found = 0, i;
	  for (i = loc; i < 4096; i++)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i >= 0; i--)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  break;
		}
	} else { /* try lower location at first, then upper location */
	  int loc_found = 0, i;
	  for (i = loc; i >= 0; i--)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  loc_found = 1;
		  break;
		}
	  if (loc_found)
		continue;
	  for (i = loc; i < 4096; i++)
		if (NULL == buff[i]) {
		  buff[i] = p;
		  buff_cnt[i] = size;
		  break;
		}
	}
  }

  /* free buffers */
  for (j = 0; j < 4096; j++) {
	if (buff[j] != NULL) {
	  my_free(buff[j]);
	  buff[j] = NULL;
	}
  }

  printf("%s: expected_heap_end = 0x%x\n", __func__, (unsigned int)expected_heap_end);

  CU_ASSERT(sim_heap_end == expected_heap_end);
}

static CU_TestInfo test_cases_array[] = {
  {"Glibc malloc: test_case_01", test_case_01},
  {"Glibc malloc: test_case_02", test_case_02},
  {"Glibc malloc: test_case_03", test_case_03},
  {"Glibc malloc: test_case_04", test_case_04},
  {"Glibc malloc: test_case_05", test_case_05},
  {"Glibc malloc: test_case_06", test_case_06},
  {"Glibc malloc: test_case_07", test_case_07},
  {"Glibc malloc: test_case_08", test_case_08},
  {"Glibc malloc: test_case_09", test_case_09},
  {"Glibc malloc: test_case_10", test_case_10},
};

CU_TestInfo* get_glibc_malloc_test_cases_array() {
  return test_cases_array;
}

int get_glibc_malloc_test_cases_count() {
  return sizeof(test_cases_array)/sizeof(CU_TestInfo);
}
