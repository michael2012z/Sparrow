#ifndef _PAGE_H_
#define _PAGE_H_

#include <mm.h>

struct page* virtual_to_page(void * virtual);
struct page* page_to_buddy(struct page *pg);

#endif
