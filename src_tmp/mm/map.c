#include <type.h>
#include <memory.h>
#include <mm.h>
#include "mem_bank.h"
#include "mem_map.h"
#include <list.h>
#include "bootmem.h"
#include <printk.h>

/* Section mapping. */

static void create_section_map_item(addr physical, addr virtual) {
  unsigned long table_item;
  addr *table_item_addr;

  /* site proterty */
  table_item = PMD_TYPE_SECT | PMD_SECT_AP_WRITE | PMD_SECT_AP_READ | PMD_FLAGS;

  /* ensure the physical address is aligned to 1M */
  physical = physical & 0xfff00000;

  /* construct the table item */
  table_item = table_item | physical;

  /* calculate the table item address */
  table_item_addr = (addr *)(PAGE_TABLE_ADDRESS + ((virtual - PAGE_OFFSET) >> MAP_ITEM_SHIFT) * 4);

  printk(PR_SS_MM, PR_LVL_DBG6, "create_section_map_item() : table_item = %x, table_item_addr = %x\n", table_item, table_item_addr);

  /* write mapping table */
  *table_item_addr = table_item;
}


static void flush_mmu() {
  addr page_table_addr = PAGE_TABLE_ADDRESS;
  asm("mcr	p15, 0, %0, c7, c10, 1	@ flush_pmd"
	  : : "r" (page_table_addr) : "cc");
  asm("mcr p15, 0, %0, c7, c10, 4"
	  : : "r" (0) : "memory");
}

void map_memory_bank(struct membank *bank)
{
	int items_count = map_length_to_count(bank->size);
	int i;
	addr physical, virtual;

	for (i = 0; i < items_count; i++) {
	  physical = bank->start + map_count_to_length(i);
	  virtual = __phys_to_virt(bank_phys_start(bank)) + i * MEGABYTES_SIZE;
	  printk(PR_SS_MM, PR_LVL_DBG6, "map_memory_bank() : physical = %x, virtual = %x\n", physical, virtual);
	  create_section_map_item(physical, virtual);
	}

	flush_mmu();
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
	page->allocated = (map_item[i]==0)?0:0xff;
	INIT_LIST_HEAD(&(page->list));
	list_add_tail(&(page->list), &(pages_map->list));
	
	//printk(PR_SS_MM, PR_LVL_DBG6, "virtual = %x, allocated = %c, next = %x, prev = %x\n", page->virtual, (page->allocated == 0)?'0':'f', page->list.next, page->list.prev);
	//printk(PR_SS_MM, PR_LVL_DBG6, "%c", (page->allocated == 0)?'0':'f');
	
  }
}

