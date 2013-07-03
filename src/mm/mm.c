#include <type.h>
#include <memory.h>
#include "mem_map.h"
#include "alloc.h"
#include <mm.h>
#include <printk.h>

unsigned long mm_pgd;

extern bool boot_alloc_ready;
extern bool page_alloc_ready;
extern bool slab_alloc_ready;


#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
				    : : "r" (0) : "memory")

static inline void flush_pgd_entry(pgd_t *pgd)
{
  asm("mcr	p15, 0, %0, c7, c10, 1	@ flush_pmd"
    : : "r" (pgd) : "cc");

  /* not enabled on s3c6410 by default
  asm("mcr	p15, 1, %0, c15, c9, 1  @ L2 flush_pmd"
      : : "r" (pgd) : "cc");
  */

  dsb();
}

static void create_mapping_section (unsigned long physical, unsigned long virtual) {
  pgd_t *pgd = pgd_offset(((pgd_t *)mm_pgd), virtual);
  *pgd = (pgd_t)(physical | 0x031);
  flush_pgd_entry(pgd);
}


static pte_t *pte_offset(pgd_t *pgd, unsigned long virtual) {
  pte_t *pt_base = (pte_t *)(*pgd & KILOBYTES_MASK);
  int index = (virtual & SECTION_MASK) >> PAGE_SHIFT;
  return (pt_base + index);
}

/* Each PT contains 256 items, and take 1K memory. But the size of each page is 4K, so each page hold 4 page table.
 * Here we're different from Linux, in which a page contains 2 linux tables and 2 hardware tables. As we don't support page swap in/out, so we don't need any extra information besides the hardware pagetable.
 */
static void create_mapping_page (unsigned long physical, unsigned long virtual) {
  pgd_t *pgd = pgd_offset(((pgd_t *)mm_pgd), virtual);
  pte_t *pte;
  if (NULL == *pgd) {
    /* populate pgd */
    pte = kmalloc(PAGE_SIZE);
    /* 4 continuous page table fall in the same page. */
    pgd_t *aligned_pgd = (pgd_t *)((unsigned long)pgd & PAGE_MASK);

	int i = 0;
	for (; i < 4; i++) {
	  aligned_pgd[i] = ((unsigned long)(__virt_to_phys(pte) + i * 256 * sizeof(pte_t))) | 0x01;
	  flush_pgd_entry(&aligned_pgd[i]);
	}
  }
  
  /* populate pte */
  pte = pte_offset(pgd, virtual);
  *pte = (physical & (~PAGE_MASK)) | 0x032;

}

static void print_map_desc(struct map_desc *map) {
  printk(PR_SS_MM, PR_LVL_DBG7, "print_map_desc(): md = %x\n", map);  
  printk(PR_SS_MM, PR_LVL_DBG7, "physical = %x, virtual = %x, length = %x, type = %x\n", map->physical, map->virtual, map->length, map->type);  
}

static void create_mapping(struct map_desc *md) {
  printk(PR_SS_MM, PR_LVL_DBG7, "create_mapping():\n");
  print_map_desc(md);
  switch(md->type) {
  case MAP_DESC_TYPE_SECTION:
    /* Physical/virtual address and length have to be aligned to 1M. */
    if ((md->physical & (~SECTION_MASK)) || (md->virtual & (~SECTION_MASK)) || (md->length & (~SECTION_MASK))) {
      /* error */
	  printk(PR_SS_MM, PR_LVL_ERROR, "create_mapping(): section map_desc data not aligned \n");
      return;
    } else {
      unsigned long physical = md->physical; 
      unsigned long virtual = md->virtual;
      while (virtual < (md->virtual + md->length)) {
		create_mapping_section(physical, virtual);
		physical += SECTION_SIZE;
		virtual += SECTION_SIZE;
      }
    }
    break;
  case MAP_DESC_TYPE_PAGE:
    /* Physical/virtual address and length have to be aligned to 4K. */
    if ((md->physical & (~PAGE_MASK)) || (md->virtual & (~PAGE_MASK)) || (md->length & (~PAGE_MASK))) {
      /* error */
	  printk(PR_SS_MM, PR_LVL_ERROR, "create_mapping(): page map_desc data not aligned \n");
      return;
    } else {
      unsigned long physical = md->physical; 
      unsigned long virtual = md->virtual;
      while (virtual < (md->virtual + md->length)) {
		create_mapping_page(physical, virtual);
		physical += PAGE_SIZE;
		virtual += PAGE_SIZE;
      }
    }
    break;
  default:
    /* error */
    break;
  }
}

static void map_low_memory() {
  struct map_desc map;
  map.physical = PHYS_OFFSET;
  map.virtual = PAGE_OFFSET;
  map.length = MEMORY_SIZE;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(&map);
}

static void map_vector_memory() {
  struct map_desc map;
  /* Alloc a page from bootmem, and get the physical address. */
  map.physical = __virt_to_phys((unsigned long)kmalloc(PAGE_SIZE));
  map.virtual = EXCEPTION_BASE;
  map.length = PAGE_SIZE;
  map.type = MAP_DESC_TYPE_PAGE;
  create_mapping(&map);
}

extern void * _debug_output_io;

static void map_debug_memory() {
  struct map_desc map;
  map.physical = 0x7f005020 & (~PAGE_MASK);
  map.virtual = (unsigned long)kmalloc(PAGE_SIZE);
  map.length = PAGE_SIZE;
  map.type = MAP_DESC_TYPE_PAGE;
  create_mapping(&map);
  _debug_output_io = (void *)((map.virtual & (~PAGE_MASK)) | (0x7f005020 & PAGE_MASK));
}

static void map_vic_memory() {
  struct map_desc map;
  /* VIC0 */
  map.physical = 0x71200000;
  map.virtual = 0xe1200000;
  map.length = SECTION_SIZE;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(&map);
  /* VIC1 */
  map.physical = 0x71300000;
  map.virtual = 0xe1300000;
  map.length = SECTION_SIZE;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(&map);

}


void mm_init() {
  boot_alloc_ready = false;
  page_alloc_ready = false;
  slab_alloc_ready = false;

  mm_pgd = PAGE_OFFSET + PAGE_TABLE_OFFSET;
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): mm_pgd = %x\n", mm_pgd);
  /* clear the page table at first*/
  prepare_page_table();
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): page table prepared\n");

  /* map main memory, lowmem in linux */
  map_low_memory();
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): low memory mapped\n");

  bootmem_initialize();
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): boot memory allocator initialized\n");

  boot_alloc_ready = true;
  
  /* map vector page */
  map_vector_memory();
  /* map debug page */
  map_debug_memory();
  /* map VIC page */
  map_vic_memory();

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
