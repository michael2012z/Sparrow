#include <type.h>
#include "mem_bank.h"
#include "mem_map.h"
#include "bootmem.h"
#include "page_alloc.h"

unsigned long mm_pgd;

static void map_low_memory() {
  struct membank membank;
  membank.start = PHYS_OFFSET;
  membank.size = MEMORY_SIZE - EXCEPTION_SIZE;
  map_memory_bank(&membank);
}

void mm_init() {
  mm_pgd = PAGE_OFFSET + PAGE_TABLE_OFFSET;
  /* clear the page table at first*/
  prepare_page_table();
  /* map main memory, lowmem in linux */
  map_low_memory();
  bootmem_initialize();
  
  /* map vector page */
  map_vector_page();
  /* map debug page */
  map_debug_page();

  //  bootmem_test();
  init_pages_map();

  page_alloc_init();
  //  pages_alloc_test();

  bootmem_finalize();

  slab_alloc_init();
  //  slab_alloc_test();


}
