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
void map_fs_to_ram();

void* kmalloc(int size);
void kfree(void *p);

struct mm_struct;
int do_translation_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr);


#endif
