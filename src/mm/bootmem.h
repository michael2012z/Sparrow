#ifndef _BOOTMEM_H_
#define _BOOTMEM_H_

void bootmem_initialize();
void *bootmem_alloc(int pages);
void bootmem_free(void *addr, int pages);
void bootmem_finalize();
void bootmem_test();

#endif
