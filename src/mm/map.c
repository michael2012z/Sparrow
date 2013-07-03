#include <type.h>
#include <memory.h>
#include <mm.h>
#include "mem_map.h"
#include <list.h>
#include "bootmem.h"
#include <printk.h>

extern unsigned long mm_pgd;

static inline void clean_pgd_entry(pgd_t *pgd)
{
  asm("mcr	p15, 0, %0, c7, c10, 1	@ flush_pmd"
      : : "r" (pgd) : "cc");

  /* not enabled on s3c6410 by default
  asm("mcr	p15, 1, %0, c15, c9, 1  @ L2 flush_pmd"
      : : "r" (pgd) : "cc");
  */
}

void pgd_clear(pgd_t *pgdp) {
  *pgdp = 0;
  clean_pgd_entry(pgdp);
}

void prepare_page_table() {
  unsigned long addr;
  /* Map the space before kernel: 0 ~ 3G */
  for (addr = 0; addr < PAGE_OFFSET; addr += PGDIR_SIZE) {
    pgd_clear(pgd_offset(mm_pgd, addr));
  }
  /* The 1st megabytes is not cleared. This is the space that kernel binary is running, it has been mapped by assembly code in the beginning. */
  for (addr = PAGE_OFFSET + 0x100000; addr <= 0xffe00000; addr += PGDIR_SIZE) {
    pgd_clear(pgd_offset(mm_pgd, addr));
  }

}


/* Page mapping. */
extern struct page* pages_map;
void init_pages_map() {
  int i;
  unsigned char *map_item = (unsigned char *)BOOTMEM_MAP_ADDRESS;  
  /* calculate needed pages */
  int need_pages_count = ((MEMORY_PAGES * sizeof(struct page)) + PAGE_SIZE - 1)/PAGE_SIZE;
  struct page * page;

  printk(PR_SS_MM, PR_LVL_DBG6, "init_pages_map(): sizeof page = %x\n", sizeof(struct page));
  printk(PR_SS_MM, PR_LVL_DBG6, "init_pages_map(): need_pages_count = %x\n", need_pages_count);

  /* allocate memory */
  pages_map = (struct page *)bootmem_alloc(need_pages_count);
  printk(PR_SS_MM, PR_LVL_DBG6, "init_pages_map(): pages_map = %x\n", pages_map);

  if (!pages_map)
	return;

  printk(PR_SS_MM, PR_LVL_DBG6, "init_pages_map(): pages bitmap\n");
  /* initialize all pages */
  for (i = 0; i < MEMORY_PAGES; i++) {
	page = &pages_map[i];
	page->virtual = PAGE_OFFSET + (addr)(i * PAGE_SIZE);
	page->allocated = map_item[i];
	INIT_LIST_HEAD(&(page->list));
	list_add_tail(&(page->list), &(pages_map->list));
	
	//printk(PR_SS_MM, PR_LVL_DBG6, "virtual = %x, allocated = %c, next = %x, prev = %x\n", page->virtual, (page->allocated == 0)?'0':'f', page->list.next, page->list.prev);
	//printk(PR_SS_MM, PR_LVL_DBG6, "%c", (page->allocated == 0)?'0':'f');
	
  }
}

