#include <type.h>
#include <memory.h>
#include <printk.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

/* 
 * Use physical address 0x50000000 ~ 0x50004000 as a map.
 * Each byte stands for a 4K page.
 * Totally (0x50004000 - 0x50000000) * 4K= 64M can be mapped.
 * If the byte is 0x00, the page is free.
 * If the byte is non zero, the page is reserved:
 *   If the value is between (0 ~ 0xff), this is the beginning byte of a continuous allocation, 
 *      the value is the number of allocated pages.
 *   If the value is 0xff, this is not the beginning byte of the continuous allocation.
 */

unsigned int _kernel_end = 0;

void bootmem_initialize() {
  /* Calculate how many pages the kernel occupies. */
  int kernel_end_pages = (_kernel_end - PAGE_OFFSET + PAGE_SIZE - 1)/PAGE_SIZE;
  int i;
  unsigned char *map_item = (unsigned char *)BOOTMEM_MAP_ADDRESS;  

  printk(PR_SS_MM, PR_LVL_DBG0, "_kernel_end = %x, kernel_end_pages = %d, BOOTMEM_MAP_SIZE = %d\n", _kernel_end, kernel_end_pages, BOOTMEM_MAP_SIZE);

  /* kernel image, occupied */
  i = 0;
  map_item[i++] = (unsigned char)kernel_end_pages;
  for (; i < kernel_end_pages; i++) {
	map_item[i] = (unsigned char)0xff;
  }

  /* free area */
  for (; i < (BOOTMEM_MAP_SIZE); i++) {
	map_item[i] = (unsigned char)0x00;
  }

}

/* size = count of pages */
void *bootmem_alloc(int pages) {
  int i, cont = 0;
  unsigned char *map_item = (unsigned char *)BOOTMEM_MAP_ADDRESS;  

  if (0 == pages)
	return NULL;
  
  for (i=0; i<BOOTMEM_MAP_SIZE; i++) {
    if (0x00 == (unsigned char)map_item[i]) {
      cont ++;
    } else {
      cont = 0;
    }
    if (cont == pages) {
      printk(PR_SS_MM, PR_LVL_DBG0, "i = %d, cont = %d\n", i ,cont);
      for (; cont>0; cont--) {
	map_item[i - pages + cont] = (unsigned char)0xff;
      }
      map_item[i - pages + 1] = (unsigned char)(0xff & pages);
      return (void*)(PAGE_OFFSET + (i - (pages - 1)) * PAGE_SIZE);
    }
  }
  return NULL;
}

void bootmem_free(void *addr) {
  int page_index = ((unsigned long)addr - PAGE_OFFSET)/PAGE_SIZE;
  unsigned char *map_item = (unsigned char *)PAGE_OFFSET; 
  int pages = map_item[page_index];
  for (; pages>0; pages--) {
	map_item[page_index + pages - 1] = 0x00;
  }
}

void bootmem_finalize() {
  int i;
  unsigned char *map_item = (unsigned char *)PAGE_OFFSET;
  for (i = 0; i < BOOTMEM_MAP_SIZE; i++) {
	map_item[i] = (unsigned char)0x00;
  }
}
