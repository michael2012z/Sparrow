#include <type.h>
#include <memory.h>
#include <math.h>
#include <mm.h>
#include <list.h>
#include <printk.h>
#include "page_alloc.h"
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

struct page* pages_map;

struct page* free_pages_head;

#define free_page_index(page) (page - free_pages_head)


/* order 0 ~ BUDDY_MAX_ORDER*/
struct page* free_list[BUDDY_MAX_ORDER+1]; /* store continuous buddy */
struct page* frag_list[BUDDY_MAX_ORDER+1]; /* store un-continuous buddy */


/* return:
 * 0 : not aligned to this order
 * others : aligned to this order
 */
static int is_aligned_to_order(struct page* page, int order) {
  unsigned long mask = _order_of(order) * sizeof(struct page);

  return (((unsigned long)page - (unsigned long)free_pages_head)%mask == 0);

}

static bool check_buddy_sanity(struct page* page) {
  struct page* current = page;
  struct page* previous = NULL;
  int i, order = page->order;

  for (i=0; i<_order_of(order); i++) {
	if (current->order != order) {
	  printk(PR_SS_MM, PR_LVL_ERR, "error in buddy (order diff): order = %x, current->order = %x\n", order, current->order);
	  return false;
	}
	previous = current;
	current = container_of((current->list.next), struct page, list);
	if ((i != (_order_of(order) - 1)) && (current - previous) != 1) {
	  printk(PR_SS_MM, PR_LVL_ERR, "error in buddy (not continuous): previous = %x, current = %x, order = %x\n", free_page_index(previous), free_page_index(current), order);
	  return false;
	}
  }

  if (page != current) {
	printk(PR_SS_MM, PR_LVL_ERR, "error in buddy (not loop): page = %x, current = %x, order = %x\n", free_page_index(page), free_page_index(current), order);
	return false;
  }

  return true;
}


void page_alloc_init() {
  int i = 0;
  int nr_free = MEMORY_PAGES - EXCEPTION_PAGES;
  struct page* current_page = pages_map; 
  struct page* last_page = &(pages_map[MEMORY_PAGES - EXCEPTION_PAGES - 1]);
  free_pages_head = NULL;
  
  printk(PR_SS_MM, PR_LVL_DBG2, "total memory pages : %x\n", nr_free);
  /* find the first free page, the pages after it are all free */
  while(current_page->allocated) {
	nr_free --;
	current_page = container_of((current_page->list.next), struct page, list);
  }
  printk(PR_SS_MM, PR_LVL_DBG2, "free memory pages : %x\n", nr_free);

  /* cut the free page list from all-page list */
  list_remove_chain(&current_page->list, &last_page->list);  

  free_pages_head = current_page;

  for (i = BUDDY_MAX_ORDER; i >= 0; i--) {
	printk(PR_SS_MM, PR_LVL_DBG2, "check order %d, _order_of(%d) = %x, nr_free = %x\n", i, i, _order_of(i), nr_free);
	free_list[i] = NULL;
	frag_list[i] = NULL;
	if (nr_free >= _order_of(i)) {
	  struct page* new_head;
	  nr_free -= _order_of(i);
	  free_list[i] = current_page;
	  new_head = container_of(((current_page + _order_of(i) - 1)->list.next), struct page, list);
	  printk(PR_SS_MM, PR_LVL_DBG2, "current_page = %x, last_page = %x, new_head = %x\n", current_page, last_page, new_head);
	  printk(PR_SS_MM, PR_LVL_DBG2, "current_page = %x, %x, %x\n", &current_page->list, current_page->list.prev, current_page->list.next);
	  printk(PR_SS_MM, PR_LVL_DBG2, "new_head     = %x, %x, %x\n", &new_head->list, new_head->list.prev, new_head->list.next);
	  printk(PR_SS_MM, PR_LVL_DBG2, "last_page    = %x, %x, %x\n", &last_page->list, last_page->list.prev, last_page->list.next);
	  list_remove_chain(&new_head->list, &last_page->list);
	  printk(PR_SS_MM, PR_LVL_DBG2, "nr_free = %x, current_page = %x, new_head = %x\n", nr_free, current_page, new_head);
	  do {
		current_page->order = i;
		current_page = container_of((current_page->list.next), struct page, list);
	  } while (current_page != free_list[i]);
	  current_page = new_head; 
	}
  }

  /* following code is only writen for debug */
  nr_free = 0;
  for (i = BUDDY_MAX_ORDER; i >= 0; i--) {
	int pages = 0;
	if (0 != free_list[i]) 
	  pages = container_of((free_list[i]->list.prev), struct page, list) - container_of((&free_list[i]->list), struct page, list) + 1;

	printk(PR_SS_MM, PR_LVL_DBG2, "buddy level %d hold %x pages\n", i, pages);
	nr_free += pages;
  }
  printk(PR_SS_MM, PR_LVL_DBG2, "total pages in free list: %x\n", nr_free);
}


static struct page* remove_first_nth_order_buddy_from_frag_list(int order) {
  struct page* ret = 0;
  struct page* current_page = 0;
  int i;

  /* first, check frag_list, if there is buddy in it, return it and update the chain. */
  ret = frag_list[order];
  current_page = ret;
  /* get the beginning of next buddy */
  for(i = 0; i < _order_of(order); i++)
	current_page = container_of((current_page->list.next), struct page, list);

  if (current_page == ret) {
	/* there is only one buddy in this level */
	frag_list[order] = NULL;
	printk(PR_SS_MM, PR_LVL_DBG3, "%x is the only buddy in frag list level %d, removed\n", free_page_index(ret), order);
  } else {
	/* there is other buddy in this level */
	frag_list[order] = current_page;
	list_remove_chain(&ret->list, current_page->list.prev);
	printk(PR_SS_MM, PR_LVL_DBG3, "%x is not the only buddy in frag list level %d, removed\n", free_page_index(ret), order);
	printk(PR_SS_MM, PR_LVL_DBG3, "now %x is the head buddy in frag list level %d, removed\n", free_page_index(current_page), order);
  }
  return ret;
}

static struct page* pages_split_recur(int order) {
  struct page* ret = NULL;
  struct page* pages_from_upper = NULL;
  struct page* current_page = NULL;
  int i;

  if (NULL != free_list[order]) {
	/* If there is buddy in this level, return it directly.
	 * Then check if frag list on this level is empty,
	 * If there is buddy in frag list, move it to free list,
	 * Else, set this level as null */
	ret = free_list[order];
	printk(PR_SS_MM, PR_LVL_DBG2, "%x is on level %d of free list, returned\n", free_page_index(ret), order);
	if (NULL == frag_list[order])
	  free_list[order] = NULL;
	else
	  free_list[order] = remove_first_nth_order_buddy_from_frag_list(order);
  } else {
	/* else, get pages from upper level*/
	if (order < BUDDY_MAX_ORDER)
	  pages_from_upper = pages_split_recur(order + 1);
	else
	  pages_from_upper = NULL;

	if (NULL != pages_from_upper) {
	  printk(PR_SS_MM, PR_LVL_DBG2, "%x is obtained from higher level %d of free list\n", free_page_index(pages_from_upper), order+1);
	  /* get pages from upper level successfully */
	  free_list[order] = pages_from_upper;
	  /* set the order of these pages */
	  current_page = pages_from_upper;
	  for(i = 0; i < _order_of(order+1); i++) {
		current_page->order = order;
		current_page = container_of((current_page->list.next), struct page, list);
	  }
	  /* split all pages into 2 parts */
	  current_page = pages_from_upper;
	  for(i = 0; i < _order_of(order); i++)
		current_page = container_of((current_page->list.next), struct page, list);

	  ret = current_page;
	  list_remove_chain(&current_page->list, pages_from_upper->list.prev);
	  printk(PR_SS_MM, PR_LVL_DBG2, "%x splited and returned for level %d from free list\n", free_page_index(ret), order);
	} else {
	  /* there isn't any pages in upper level */
	  ret = NULL;
	  printk(PR_SS_MM, PR_LVL_DBG2, "no buddy can be allocated on level %d from free list\n", order);
	}
  }

  return ret;
}


static struct page* pages_alloc_by_order(int order) {
  struct page* ret = 0;
  struct page* current_page = 0;
  int i;

  if (order >= BUDDY_MAX_ORDER)
	return NULL;

  if (NULL != frag_list[order]) {
	/* first, check frag_list, if there is buddy in it, return it and update the chain. */
	ret = remove_first_nth_order_buddy_from_frag_list(order);
  } else { /* turn to free list */
	ret = pages_split_recur(order);
  }

  return ret;
}


struct page* pages_alloc(int pages) {
  int order = 0;
  struct page* ret = 0;

  if (0 >= pages)
	return 0;

  while (_order_of(order) < pages) {
	order++;
  }

  printk(PR_SS_MM, PR_LVL_DBG2, "need to alloc %d pages, %d order\n", pages, order);

  ret = pages_alloc_by_order(order);

  /* Add a flag (1010 1010) to buddy, indicate that it's been allocated. */
  if (NULL != ret)
	ret->allocated = 'a';

  return ret;
}


static void pages_combine_recur(struct page* page) {
  int order = 0;
  int i = 0;
  struct page* upper_buddy = 0;
  struct page* current_page = 0;

  if (NULL != page)
	order = page->order;
  else
	return;

  if (false == check_buddy_sanity(page))
	while(1);

  /* first, check the free_list[order], if it's null, set the page to it. */
  if (NULL == free_list[order]) {
	free_list[order] = page;
	printk(PR_SS_MM, PR_LVL_DBG3, "level %d of free list is empty, put buddy %x here\n", order, free_page_index(page));
	return;
  } else {
	/* else, check whether the page is continuous with free_list[order] or any buddy in frag_list */
	if ((math_abs(page - free_list[order])) == (_order_of(order))) {
	  printk(PR_SS_MM, PR_LVL_DBG3, "try to combine buddies: %x -- %x, order = %x, %d, %d, %d\n", free_page_index(page), free_page_index(free_list[order]), order, (page - free_list[order]), (math_abs(page - free_list[order])), _order_of(order));
	  /* first, check free_list[order] */
	  if ((page + _order_of(order)) == free_list[order]) { /* "page" is in front */
		upper_buddy = page;
	  }	else { /* buddy in free list is in front */
		upper_buddy = free_list[order];
	  }
	  if (is_aligned_to_order(upper_buddy, order+1)) {
		/* buddy found */
		if (upper_buddy == page) {
		  /* page is the head */
		  list_add_chain_tail(&free_list[order]->list, free_list[order]->list.prev, &upper_buddy->list);
		  printk(PR_SS_MM, PR_LVL_DBG3, "buddies combined A: %x -- %x, order = %x\n", free_page_index(container_of(&(upper_buddy->list), struct page, list)), free_page_index(container_of(&(free_list[order]->list), struct page, list)), order);
		} else {
		  /* free_list[order] is the head */
		  list_add_chain_tail(&page->list, page->list.prev, &upper_buddy->list);
		  printk(PR_SS_MM, PR_LVL_DBG3, "buddies combined B: %x -- %x, order = %x\n", free_page_index(container_of(&(upper_buddy->list), struct page, list)), free_page_index(container_of(&(page->list), struct page, list)), order);
		}
		/* Move first buddy of frag list to free list, if there is any. */
		if (NULL == frag_list[order])
		  free_list[order] = NULL;
		else
		  free_list[order] = remove_first_nth_order_buddy_from_frag_list(order);
		/* set the order of these pages */
		current_page = upper_buddy;
		for(i = 0; i < _order_of(order + 1); i++) {
		  current_page->order = order + 1;
		  current_page = container_of((current_page->list.next), struct page, list);
		}
		/* combine it to upper level */
		pages_combine_recur(upper_buddy);
		return;
	  }
	}
	
	/* fail to combine with free_list[order], try frag_list[order] */
	if (NULL == frag_list[order]) {
	  frag_list[order] = page;
	  return;
	} else {
	  struct page* current_buddy = frag_list[order];
	  struct page* next_buddy = 0;

	  do { 
		/* get the next buddy */
		next_buddy = current_buddy;
		for (i = 0; i < _order_of(order); i ++) {
		  next_buddy = container_of((next_buddy->list.next), struct page, list);
		}
		printk(PR_SS_MM, PR_LVL_DBG3, "get the first buddy %x of level %d in frag list\n", free_page_index(next_buddy), order);
		/* loop for each buddy in frag_list[order] */
		if (math_abs(page - current_buddy) == _order_of(order)) {
		  if ((page + _order_of(order)) == current_buddy) {
			upper_buddy = page;
		  }	else {
			upper_buddy = current_buddy;
		  }
		  if (is_aligned_to_order(upper_buddy, order+1)) {
			printk(PR_SS_MM, PR_LVL_DBG3, "%x is indeed the buddy of %x\n", free_page_index(next_buddy), free_page_index(page));
			/* buddy found, first remove current_buddy from frag_list */
			/* now, next_buddy is the next buddy behind current buddy */
			if (next_buddy == current_buddy) {
			  /* current_buddy is the only one in frag_list[order] */
			  frag_list[order] = 0;
			} else if(current_buddy == frag_list[order]) {
			  /* not the only, but the head */
			  frag_list[order] = next_buddy;
			  list_remove_chain(&current_buddy->list, next_buddy->list.prev);
			} else {
			  /* current_boddy is the middle or tail */
			  list_remove_chain(&current_buddy->list, next_buddy->list.prev);
			}

			/*now current_buddy and page should be combined */
			if (upper_buddy == page) {
			  /* page is the head */
			  list_add_chain_tail(&current_buddy->list, current_buddy->list.prev, &upper_buddy->list);
			  printk(PR_SS_MM, PR_LVL_DBG3, "buddies combined C: %x -- %x\n", free_page_index(container_of(&(upper_buddy->list), struct page, list)), free_page_index(container_of(&(current_buddy->list), struct page, list)));
			} else {
			  /* free_list[order] is the head */
			  list_add_chain_tail(&page->list, page->list.prev, &upper_buddy->list);
			  printk(PR_SS_MM, PR_LVL_DBG3, "buddies combined D: %x -- %x\n", free_page_index(container_of(&(upper_buddy->list), struct page, list)), free_page_index(container_of(&(page->list), struct page, list)));
			}

			/* set the order of these pages */
			current_page = upper_buddy;
			for(i = 0; i < _order_of(order + 1); i++) {
			  current_page->order = order + 1;
			  current_page = container_of((current_page->list.next), struct page, list);
			}
			/* combine it to upper level */
			pages_combine_recur(upper_buddy);
			return;
		  }
		}
		/* go to next buddy */
		current_buddy = next_buddy;
	  } while(current_buddy != frag_list[order]);

	  /* buddy not found, add the new-arrived buddy into frag_list */
	  list_add_chain_tail(&page->list, page->list.prev, &frag_list[order]->list);
	}
  }
}


void pages_free(struct page* page) {
  if (0 == page)
	return;
  
  if ('a' != page->allocated)
	printk(PR_SS_MM, PR_LVL_ERR, "buddy %x is not valid to free\n", page);
  else {
	pages_combine_recur(page);
	page->allocated = 0;
  }
}


void print_free_list() {
  int nr_free = 0, i = 0;
  
  printk(PR_SS_MM, PR_LVL_INF, "print_free_list()\n");
  
  for (i = BUDDY_MAX_ORDER; i >= 0; i--) {
	int pages = 0;
	if (0 != free_list[i]) 
	  pages = container_of((free_list[i]->list.prev), struct page, list) - container_of((&free_list[i]->list), struct page, list) + 1;

	printk(PR_SS_MM, PR_LVL_INF, "free list level %d hold %x pages: ", i, pages);
	if (pages > 0)
	  printk(PR_SS_MM, PR_LVL_INF, " %x", free_page_index(free_list[i]));
	printk(PR_SS_MM, PR_LVL_INF, "\n");
	nr_free += pages;
  }
  printk(PR_SS_MM, PR_LVL_INF, "total pages in free list: %x\n", nr_free);

}

void print_frag_list() {
  int nr_free = 0, i = 0;
  struct page* current_page = 0;
  
  printk(PR_SS_MM, PR_LVL_INFO, "print_frag_list()\n");
  
  for (i = BUDDY_MAX_ORDER; i >= 0; i--) {
	int pages = 0;
	if (0 != frag_list[i]) {
	  printk(PR_SS_MM, PR_LVL_INFO, "frag list level %d is not empty, buddies address: \n", i);
	  current_page = frag_list[i];
	  do {
		if((pages%_order_of(i)) == 0)
		  printk(PR_SS_MM, PR_LVL_INFO, " %x \n", free_page_index(current_page));
		current_page = container_of((current_page->list.next), struct page, list);
		pages ++;
	  } while(current_page != frag_list[i]);
	}
	nr_free += pages;
  }
  printk(PR_SS_MM, PR_LVL_INFO, "total pages in frag list: %x\n", nr_free);

}

