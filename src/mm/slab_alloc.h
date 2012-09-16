#ifndef _SLAB_ALLOC_H_
#define _SLAB_ALLOC_H_

#include <list.h>

struct kcache;

struct slabset {
  struct kcache* parent;
  int slab_count;
  int objects_free;
  struct list_head slabs;
};

struct kcache {
  int object_size;		/* in bytes */
  int slab_size;		/* in pages */
  int objects_per_slab;
  unsigned long align;	/* start of objects */
  struct slabset full_set;
  struct slabset partial_set;
  struct slabset empty_set;
};

#define SLAB_CACHE_COUNT 11


void slab_alloc_init();
void* slab_alloc(int size);
void slab_free(void* addr);

struct kcache* get_kcache(int index);

#endif
