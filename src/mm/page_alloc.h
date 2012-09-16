#ifndef _PAGE_ALLOC_H_
#define _PAGE_ALLOC_H_

#define BUDDY_MAX_ORDER		20

struct page* pages_alloc(int pages);
void pages_free(struct page* page);

void page_alloc_init();

void print_free_list();
void print_frag_list();

#endif
