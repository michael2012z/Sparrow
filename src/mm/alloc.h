#ifndef _ALLOC_H_
#define _ALLOC_H_

bool  boot_alloc_ready = false;
bool  page_alloc_ready = false;
bool  slab_alloc_ready = false;


#include "bootmem.h"
#include "page_alloc.h"
#include "slab_alloc.h"

#endif /* _ALLOC_H_ */
