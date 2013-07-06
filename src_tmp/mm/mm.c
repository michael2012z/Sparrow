#include <type.h>
#include <memory.h>
#include <linkage.h>
#include <interrupt.h>
#include "mem_bank.h"
#include "mem_map.h"
#include "alloc.h"
#include <mm.h>
#include <printk.h>

struct membank membank;
unsigned long mm_pgd;

extern bool boot_alloc_ready;
extern bool page_alloc_ready;
extern bool slab_alloc_ready;

void mm_init() {
  boot_alloc_ready = false;
  page_alloc_ready = false;
  slab_alloc_ready = false;

  mm_pgd = PAGE_OFFSET + PAGE_TABLE_OFFSET;
  membank.start = PHYS_OFFSET;
  membank.size = MEMORY_SIZE - EXCEPTION_SIZE;
  map_memory_bank(&membank);

  bootmem_initialize();
  boot_alloc_ready = true;
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): boot memory allocator initialized\n");

  //  bootmem_test();
  init_pages_map();

  page_alloc_init();
  //  pages_alloc_test();
  page_alloc_ready = true;

  bootmem_finalize();
  boot_alloc_ready = false;

  slab_alloc_init();
  slab_alloc_ready = true;
  //  slab_alloc_test();


}
