#include <type.h>
#include <mm.h>
#include "page_alloc.h"
#include "slab_alloc.h"
#include "page.h"
#include <printk.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

/* For caches of: 
 * 8, 16, 32, 64, 96, 128, 192, 256, 512, 1024, 2048
 */
static struct kcache kcache_array[SLAB_CACHE_COUNT];

static int object_size_array[SLAB_CACHE_COUNT]={8, 16, 32, 64, 96, 128, 192, 256, 512, 1024, 2048};
static int slab_size_array[SLAB_CACHE_COUNT]={1, 1, 1, 2, 2, 4, 4, 8, 8, 8, 8};

struct kcache* get_kcache(int index) {
  if (index < SLAB_CACHE_COUNT)
	return &kcache_array[index];
  else
	return NULL;
}

static void slab_init(struct page* slab, int objects) {
  int i;
  unsigned char* map_item = (unsigned char *)slab->virtual;

  printk(PR_SS_MM, PR_LVL_DBG4, "slab->virtual = %x, objects = %x\n", map_item, objects);

  for(i = 0; i < objects; i++) {
	map_item[i] = (unsigned char)0x00;
  }
}

/* allocate a buddy, add it to end of slabs list */
static bool slabset_grow(struct slabset* set) {
  struct page* pg = NULL;

  printk(PR_SS_MM, PR_LVL_DBG4, "slabset_grow()\n");
  printk(PR_SS_MM, PR_LVL_DBG4, "need to alloc %x pages\n", set->parent->slab_size);
  pg = pages_alloc(set->parent->slab_size);
  
  printk(PR_SS_MM, PR_LVL_DBG4, "slab allocated : %x\n", pg);

  if (NULL == pg)
	return false;

  set->slab_count ++;
  set->slabs.prev->next = &pg->manage;
  set->slabs.prev = &pg->manage;
  pg->manage.next = &set->slabs;
  pg->manage.prev = &set->slabs;

  slab_init(pg, set->parent->objects_per_slab);
  return true;
}

/* remove the first slab of slabset */
static void slabset_shrink(struct slabset* set) {
  struct page* slab = container_of((set->slabs.next), struct page, manage);

  set->slabs.next = slab->manage.next;

  if (set->slabs.prev == &slab->manage) /* also the last slab */
	set->slabs.prev = &set->slabs;

  slab->manage.prev = NULL;
  slab->manage.next = NULL;
  pages_free(slab);

  set->slab_count --;
}

void slab_alloc_init() {
  int i;
  int wasted_bytes;
  
  //  printk(PR_SS_MM, PR_LVL_DBG4, "slab_alloc_init()\n");
  
  for(i = 0; i < SLAB_CACHE_COUNT; i++) {
	kcache_array[i].object_size = object_size_array[i];
	kcache_array[i].slab_size = slab_size_array[i];
	kcache_array[i].objects_per_slab = (kcache_array[i].slab_size * PAGE_SIZE)/(kcache_array[i].object_size + 1);
	wasted_bytes = (kcache_array[i].slab_size * PAGE_SIZE) % kcache_array[i].object_size;
	kcache_array[i].align = kcache_array[i].slab_size * PAGE_SIZE - kcache_array[i].objects_per_slab * kcache_array[i].object_size - wasted_bytes;
	/* full set */
	kcache_array[i].full_set.parent = &kcache_array[i];
	kcache_array[i].full_set.slab_count = 0;
	kcache_array[i].full_set.objects_free = 0;
	INIT_LIST_HEAD(&(kcache_array[i].full_set.slabs));
	/* partial set */
	kcache_array[i].partial_set.parent = &kcache_array[i];
	kcache_array[i].partial_set.slab_count = 0;
	kcache_array[i].partial_set.objects_free = 0;
	INIT_LIST_HEAD(&(kcache_array[i].partial_set.slabs));
	/* empty set */
	kcache_array[i].empty_set.parent = &kcache_array[i];
	kcache_array[i].empty_set.slab_count = 0;
	kcache_array[i].empty_set.objects_free = 0;
	INIT_LIST_HEAD(&(kcache_array[i].empty_set.slabs));

#ifdef SLAB_OPTIMIZE
	/* for optimization, remain an empty slab in empty set */
	/* grow the empty_set */
	slabset_grow(&(kcache_array[i].empty_set));
#endif
  }
}

/* get the first slab of slabs */
static struct page* slabset_get_first_slab(struct slabset* set) {
  if(set->slabs.next != &set->slabs) {
	/* not empty */
	return container_of(set->slabs.next, struct page, manage);
  }else {
	return NULL;
  }
}

/* add a slab(buddy) to the end of slabs */
static void slabset_add_slab(struct page* slab, struct slabset* set) {
  set->slabs.prev->next = &slab->manage;
  set->slabs.prev = &slab->manage;
  slab->manage.next = &set->slabs;
  slab->manage.prev = &set->slabs;
  set->slab_count ++;
}

/* remove the slab from slabs */
static void slabset_remove_slab(struct page* slab, struct slabset* set) {
  struct list_head* current = &set->slabs;

  while(&slab->manage != current->next) {
	printk(PR_SS_MM, PR_LVL_DBG5, "slab = %x, &slab->manage = %x, current->next = %x\n", slab, &slab->manage, current->next);
	current = current->next;
  }

  current->next = current->next->next;
  if (slab->manage.next == &set->slabs) {
	set->slabs.prev = current;
  }
  set->slab_count --;
}

/* whether a slab is contained in the slabset */
static bool slabset_contain_slab(struct slabset* set, struct page* slab) {
  struct list_head* current = set->slabs.next;
  while(&set->slabs != current) {
	if (&slab->manage == current) {
	  return true;
	}
	current = current->next;
  }
  return false;
}

/* allocate an object from a slab */
static void* slab_alloc_object(struct page* slab, int object_size, int objects_per_slab, int align) {
  unsigned char* map = (unsigned char*)slab->virtual;
  int i;
  void * result = NULL;

  printk(PR_SS_MM, PR_LVL_DBG4, "slab_alloc_object(): slab = %x, order = %x\n", slab, slab->order);

  for (i = 0; i < objects_per_slab; i++) {
	if ((unsigned char)0 == map[i]) {
	  /* found */
	  map[i] = (unsigned char)0xff;
	  result = (void *)((map + align) + object_size * i);
	  break;
	}
  }

  return result;
}

/* free the allocated object */
static bool slab_free_object(struct page* slab, int object_size, int objects_per_slab, int align, void* object) {
  unsigned char* map = (unsigned char*)slab->virtual;
  int i;

  printk(PR_SS_MM, PR_LVL_DBG5, "slab_free_object(): slab = %x\n", slab);
  for (i = 0; i < objects_per_slab; i++) {
	if (object == (map + align + object_size * i)) {
	  /* found */
	  printk(PR_SS_MM, PR_LVL_DBG5, "objects index found: %x\n", i);
	  printk(PR_SS_MM, PR_LVL_DBG5, "map = %x\n", map);
	  if ((unsigned char)0 == map[i])
		printk(PR_SS_MM, PR_LVL_ERR, "returned memory %x was not allocated\n", (unsigned int)object);
	  map[i] = (unsigned char)0;
	  return true;
	}
  }

  printk(PR_SS_MM, PR_LVL_ERR, "returned memory %x is not a valid address\n", (unsigned int)object);
  return false;
}

/* check whether the slab is full */
static bool is_slab_full(struct page* slab, int objects_per_slab) {
  unsigned char* map = (unsigned char *)slab->virtual;
  while((objects_per_slab--) > 0) {
	if (map[objects_per_slab] == (unsigned char)0)
	  return false;
  }
  return true;
}

/* check whether the slab is empty */
static bool is_slab_empty(struct page* slab, int objects_per_slab) {
  unsigned char* map = (unsigned char *)slab->virtual;
  while((objects_per_slab--) > 0) {
	if (map[objects_per_slab] == (unsigned char)0xff)
	  return false;
  }
  return true;
}

/* allocate object from a cache */
static void* kcache_alloc(struct kcache* cache) {
  struct page* slab;
  void* result = NULL;

  if (0 == cache->partial_set.slab_count) {
	/* need to move a slab from empty set to here */
	if (0 == cache->empty_set.slab_count) {
	  /* empty set is also empty, need to grow */
	  bool grow = false;
	  grow = slabset_grow(&(cache->empty_set));
	  if (false == grow) {
		printk(PR_SS_MM, PR_LVL_ERR, "why can't empty slab grow?\n");
		while(1);
		return NULL;
	  }
	}
	/* by now, empty set must have some slab */
	slab = slabset_get_first_slab(&(cache->empty_set));
	if (NULL == slab) {
	  printk(PR_SS_MM, PR_LVL_ERR, "empty slab shouldn't be empty\n");
	  while(1);
	  return NULL;
	}
	slabset_remove_slab(slab, &(cache->empty_set));
	slabset_add_slab(slab, &(cache->partial_set));
  }
  
  /* by now, partial set must have some slab */
  /* get first slab from partial set */
  slab = slabset_get_first_slab(&(cache->partial_set));
  if (NULL == slab) {
	printk(PR_SS_MM, PR_LVL_ERR, "partial set can't be empty\n");
	while(1);
	return NULL;
  }

  /* alloc from this slab */
  result = slab_alloc_object(slab, cache->object_size, cache->objects_per_slab, cache->align);

  /* if the slab become full, move it to full set */
  if (is_slab_full(slab, cache->objects_per_slab)) {
	slabset_remove_slab(slab, &(cache->partial_set));
	slabset_add_slab(slab, &(cache->full_set));
  }

  return result;
}

void* slab_alloc(int size) {
  int cache_index;

  if((size > 2048) || (size <= 0))
	return NULL;

  for(cache_index = 0; cache_index < SLAB_CACHE_COUNT; cache_index++) 
	if (object_size_array[cache_index] >= size)
	  break;
  printk(PR_SS_MM, PR_LVL_DBG4, "slab_alloc(): cache_index = %x\n", cache_index);

  return kcache_alloc(&kcache_array[cache_index]);
}


void slab_free(void* addr) {
  struct page* pg;
  struct page* slab;
  struct slabset* set;
  struct kcache* cache;
  bool freed;

  printk(PR_SS_MM, PR_LVL_DBG5, "slab_free(): trying to free address: %x\n", addr);

  if (NULL == addr) {
	return;
  }

  pg = virtual_to_page(addr);
  printk(PR_SS_MM, PR_LVL_DBG5, "pg = %x, order = %x\n", pg, pg->order);
  slab = page_to_buddy(pg);
  printk(PR_SS_MM, PR_LVL_DBG5, "slab = %x\n", slab);
  printk(PR_SS_MM, PR_LVL_DBG5, "slab->manage.prev = %x\n", slab->manage.prev);
  
  if ((NULL == slab->manage.prev) || (NULL == slab->manage.next)) {
	printk(PR_SS_MM, PR_LVL_ERR, "invalid slab %x\n", slab);	
	return;
  }

  set = container_of((slab->manage.prev), struct slabset, slabs);
  printk(PR_SS_MM, PR_LVL_DBG5, "set = %x\n", set);

  if (false == slabset_contain_slab(set, slab)) {
	printk(PR_SS_MM, PR_LVL_ERR, "slab is not contained in a proper slabset\n");
	while(1);
  }

  cache = set->parent;
  printk(PR_SS_MM, PR_LVL_DBG5, "cache = %x\n", cache);

  freed = slab_free_object(slab, cache->object_size, cache->objects_per_slab, cache->align, addr);

  if (false == freed) {
	printk(PR_SS_MM, PR_LVL_ERR, "returned memory %x can't be freed\n", (unsigned int)addr);
	return;
  }

  if (set == &cache->full_set) {
	slabset_remove_slab(slab, &(cache->full_set));
	slabset_add_slab(slab, &(cache->partial_set));
	set = &cache->partial_set;
  } 

  if ((set == &cache->partial_set) && (is_slab_empty(slab, cache->objects_per_slab))) {
	slabset_remove_slab(slab, &(cache->partial_set));
	slabset_add_slab(slab, &(cache->empty_set));
  } 

#ifdef SLAB_OPTIMIZE
  /* for optimization, remain an empty slab in empty set */
  while (cache->empty_set.slab_count > 1) {
#else
  /* no optimization, remain all slabs in empty set */
  while (cache->empty_set.slab_count > 0) {
#endif
	/* free a slab */
	slabset_shrink(&cache->empty_set);
  }
}


/* Debug methods.
static void print_slab(struct page* slab, int objects_per_slab) {
  int i = 0, free_count = 0;
  unsigned char* map = (unsigned char*)slab->virtual;

  printk(PR_SS_MM, PR_LVL_INFO, "print slab: %x, virtual = %x\n", slab, map);
  for(i = 0; i < objects_per_slab; i++) 
	if (map[i] == (unsigned char)0)
	  free_count ++;
  
  printk(PR_SS_MM, PR_LVL_INFO, "%x objects free, %x objects allocated\n", free_count, (objects_per_slab - free_count));
}

static void print_slabset(struct slabset* set) {
  struct page* slab;
  struct list_head* next = set->slabs.next;
  int i=0;
  printk(PR_SS_MM, PR_LVL_INFO, "print slabset: %x\n", set);
  printk(PR_SS_MM, PR_LVL_INFO, "slab_count = %x\n", set->slab_count);
  while(next != &set->slabs) {
	slab = container_of(next, struct page, manage);
	printk(PR_SS_MM, PR_LVL_INFO, "%d th slab:\n", i);
	i++;
	print_slab(slab, set->parent->objects_per_slab);
	next = next->next;
  }
}

static void print_kcache(struct kcache* cache) {
  printk(PR_SS_MM, PR_LVL_INFO, "print kcache: %x\n", cache);
  printk(PR_SS_MM, PR_LVL_INFO, "object_size = %x, slab_size = %x, objects_per_slab = %x, align = %x\n", cache->object_size, cache->slab_size, cache->objects_per_slab, cache->align);
  printk(PR_SS_MM, PR_LVL_INFO, "full_set:\n");
  print_slabset(&cache->full_set);
  printk(PR_SS_MM, PR_LVL_INFO, "partial_set:\n");
  print_slabset(&cache->partial_set);
  printk(PR_SS_MM, PR_LVL_INFO, "empty_set:\n");
  print_slabset(&cache->empty_set);
}

static void print_all_kcache() {
  int i;
  printk(PR_SS_MM, PR_LVL_INFO, "print all kcache\n");
  for(i = 0; i < SLAB_CACHE_COUNT; i++) {
	print_kcache(&kcache_array[i]);
  }
}

*/
