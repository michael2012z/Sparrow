#include <type.h>
#include "mem_bank.h"
#include "mem_map.h"
#include "bootmem.h"
#include "page_alloc.h"

struct membank membank;
unsigned long mm_pgd;

void mm_init() {
  mm_pgd = PAGE_OFFSET + PAGE_TABLE_OFFSET;
  membank.start = PHYS_OFFSET;
  membank.size = MEMORY_SIZE - EXCEPTION_SIZE;
  map_memory_bank(&membank);

  bootmem_initialize();
  //  bootmem_test();
  init_pages_map();

  page_alloc_init();
  //  pages_alloc_test();

  bootmem_finalize();

  slab_alloc_init();
  //  slab_alloc_test();


}
