#ifndef _MM_H_
#define _MM_H_

#include <memory.h>
#include <list.h>


struct page {
  addr virtual;
  char allocated;
  char order;
  struct list_head manage;
  struct list_head list;
};


void mm_init();
void prepare_page_table();

void* kmalloc(int size);

void kfree(void *p);


#endif
