#include <type.h>
#include "mem_bank.h"
#include "mem_map.h"
#include "bootmem.h"
#include "page_alloc.h"

unsigned long mm_pgd;


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
  pgd_t *pgd = pgd_offset(mm, virtual);
  *pgd = (pgd_t)(physical | 0x40e);
  flush_pgd_entry();
}

static void create_mapping_page (unsigned long physical, unsigned long virtual) {
  pgd_t *pgd = pgd_offset(mm, virtual);
  pte_t *pte;
  if (NULL == *pgd) {
    /* populate pgd */
    pte == alloc one page memory;
    /* As 2 continuous pgd points to a same page table, so need to differ is current pgd the 1st one or 2nd.  */
    pgd_t *aligned_pgd = pgd;
    if (virtual & SECTION_SIZE)
      aligned_pgd -= 1;

    aligned_pgd[0] = __pa(pte) | property;
    aligned_pgd[1] = (__pa(pte) + 256 * sizeof(pte_t)) | property;
    flush_pgd_entry(aligned_pgd);
  }
  
  /* populate pte */

}

static void create_mapping(struct map_desc *md) {
  switch(md->type) {
  case MAP_DESC_TYPE_SECTION:
    /* Physical/virtual address and length have to be aligned to 1M. */
    if ((md->physical & SECTION_MASK) || (md->virtual & SECTION_MASK) || (md->length & SECTION_MASK)) {
      /* error */
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
    if ((md->physical & PAGE_MASK) || (md->virtual & PAGE_MASK) || (md->length & PAGE_MASK)) {
      /* error */
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
