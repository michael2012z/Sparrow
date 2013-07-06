#ifndef _MEM_MAP_H_
#define _MEM_MAP_H_


#define MAP_ITEM_SHIFT		20 /* cover 20 bits */ 
#define map_length_to_count(x)		((x) >> MAP_ITEM_SHIFT)
#define map_count_to_length(x)		((x) << MAP_ITEM_SHIFT)

void map_memory_bank(struct membank *bank);

void init_pages_map();

#endif
