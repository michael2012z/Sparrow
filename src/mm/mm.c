#include <type.h>
#include <memory.h>
#include <linkage.h>
#include <interrupt.h>
#include "mem_map.h"
#include "alloc.h"
#include <mm.h>
#include <printk.h>
#include <mmap.h>
#include <string.h>
#include <uart.h>

unsigned long kernel_pgd;

extern bool boot_alloc_ready;
extern bool page_alloc_ready;
extern bool slab_alloc_ready;


#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
				    : : "r" (0) : "memory")

static inline void flush_pgd_entry(pgd_t *pgd)
{
  asm("mcr	p15, 0, %0, c7, c10, 1	@ flush_pmd"
    : : "r" (pgd) : "cc");

  dsb();
}

static void create_mapping_section (unsigned long mm_pgd, unsigned long physical, unsigned long virtual) {
  pgd_t *pgd = pgd_offset(((pgd_t *)mm_pgd), virtual);
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: pgd = %x, *pgd = %x\n", __func__, pgd, (physical | 0x0c02));  
  //  *pgd = (pgd_t)(physical | 0x031);
  *pgd = (pgd_t)(physical | 0x0c02);
  flush_pgd_entry(pgd);
}


static pte_t *pte_offset(pte_t *pt, unsigned long virtual) {
  int index = (virtual & ~SECTION_MASK) >> PAGE_SHIFT;
  return (pt + index);
}

/* Each PT contains 256 items, and take 1K memory. But the size of each page is 4K, so each page hold 4 page table.
 * Here we're different from Linux, in which a page contains 2 linux tables and 2 hardware tables. As we don't support page swap in/out, so we don't need any extra information besides the hardware pagetable.
 */
static void create_mapping_page (unsigned long mm_pgd, unsigned long physical, unsigned long virtual) {
  pgd_t *pgd = pgd_offset(((pgd_t *)mm_pgd), virtual);
  pte_t *pte, pte_value, *pt = NULL;
  /* 4 continuous page table fall in the same page. */
  pgd_t *aligned_pgd = (pgd_t *)((unsigned long)pgd & 0xfffffff0);

  printk(PR_SS_MM, PR_LVL_DBG7, "%s: mm_pgd = %x, pgd = %x, virtual = %x\n", __func__, mm_pgd, pgd, virtual);  
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: aligned_pgd = %x\n", __func__, aligned_pgd);  

  if (NULL == *pgd) {
    /* populate pgd */
    pte = kmalloc(PAGE_SIZE);
	printk(PR_SS_MM, PR_LVL_DBG7, "%s: populating pgd: allocated page table: virt = %x, phys = %x\n", __func__, pte, __virt_to_phys((unsigned long)pte));  

	int i = 0;
	for (; i < 4; i++) {
	  pgd_t pgd_value = ((unsigned long)(__virt_to_phys((unsigned long)pte) + i * 256 * sizeof(pte_t))) | 0x01;
	  printk(PR_SS_MM, PR_LVL_DBG7, "%s: populating pgd: pgd = %x, *pgd = %x\n", __func__, &aligned_pgd[i], pgd_value);  
	  aligned_pgd[i] = pgd_value;
	  flush_pgd_entry(&aligned_pgd[i]);
	}
  } 

  printk(PR_SS_MM, PR_LVL_DBG7, "%s: *pgd = %x\n", __func__, *pgd);
  pt = (pte_t *)(__phys_to_virt((unsigned long)*pgd) & KILOBYTES_MASK);
  /* populate pte */
  if (NULL == pt) {
	printk(PR_SS_MM, PR_LVL_ERR, "%s: page table not found: %x\n", __func__, pt);
	while(1);
  }

  pte = pte_offset(pt, virtual);
  pte_value = (physical & PAGE_MASK) | 0x032;
  //pte_value = (physical & PAGE_MASK) | 0x02a;
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: pt = %x, pte = %x, *pte = %x\n", __func__, pt, pte, pte_value);  
  *pte = pte_value;
}

static void print_map_desc(struct map_desc *map) {
  printk(PR_SS_MM, PR_LVL_DBG7, "print_map_desc(): md = %x\n", map);  
  printk(PR_SS_MM, PR_LVL_DBG7, "physical = %x, virtual = %x, length = %x, type = %x\n", map->physical, map->virtual, map->length, map->type);  
}

static void create_mapping(unsigned long mm_pgd, struct map_desc *md) {
  printk(PR_SS_MM, PR_LVL_DBG7, "create_mapping():\n");
  print_map_desc(md);
  switch(md->type) {
  case MAP_DESC_TYPE_SECTION:
    /* Physical/virtual address and length have to be aligned to 1M. */
    if ((md->physical & (~SECTION_MASK)) || (md->virtual & (~SECTION_MASK)) || (md->length & (~SECTION_MASK))) {
      /* error */
	  printk(PR_SS_MM, PR_LVL_ERROR, "create_mapping(): section map_desc data not aligned \n");
      return;
    } else {
      unsigned long physical = md->physical; 
      unsigned long virtual = md->virtual;
	  /*
	  if (virtual == (EXCEPTION_BASE & SECTION_MASK)) {
		create_mapping_section(physical, virtual);
		return;
	  }
	  */
      while (virtual < (md->virtual + md->length)) {
		create_mapping_section(mm_pgd, physical, virtual);
		physical += SECTION_SIZE;
		virtual += SECTION_SIZE;
      }
    }
    break;
  case MAP_DESC_TYPE_PAGE:
    /* Physical/virtual address and length have to be aligned to 4K. */
    if ((md->physical & (~PAGE_MASK)) || (md->virtual & (~PAGE_MASK)) || (md->length & (~PAGE_MASK))) {
      /* error */
	  printk(PR_SS_MM, PR_LVL_ERROR, "create_mapping(): page map_desc data not aligned \n");
      return;
    } else {
      unsigned long physical = md->physical; 
      unsigned long virtual = md->virtual;
      while (virtual < (md->virtual + md->length)) {
		create_mapping_page(mm_pgd, physical, virtual);
		physical += PAGE_SIZE;
		virtual += PAGE_SIZE;
      }
    }
    break;
  default:
    /* error */
    break;
  }
}

static void map_low_memory() {
  struct map_desc map;
  map.physical = PHYS_OFFSET;
  map.virtual = PAGE_OFFSET;
  map.length = MEMORY_SIZE;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(kernel_pgd, &map);
}

static void map_vector_memory() {
  struct map_desc map;
  map.physical = __virt_to_phys((unsigned long)kmalloc(PAGE_SIZE));
  map.virtual = EXCEPTION_BASE;
  map.length = PAGE_SIZE;
  map.type = MAP_DESC_TYPE_PAGE;
  create_mapping(kernel_pgd, &map);
}

extern void * _debug_output_io;

static void map_debug_memory() {
  struct map_desc map;
  map.physical = S3C_PA_UART & PAGE_MASK;
  map.virtual = S3C_VA_UART & PAGE_MASK;
  map.length = PAGE_SIZE;
  map.type = MAP_DESC_TYPE_PAGE;

  /* Have to disable printk, otherwise any printing before mapping finish will lead to mapping fault. */
  printk_disable();
  /* Remove existing section mapping: */
  pgd_t *pgd = pgd_offset(((pgd_t *)kernel_pgd), map.virtual);
  *pgd = 0;
  /* Create new mapping. */
  create_mapping(kernel_pgd, &map);
  _debug_output_io = (void *)((map.virtual & PAGE_MASK) | (0x7f005020 & ~PAGE_MASK));
  printk_enable();
  printk(PR_SS_MM, PR_LVL_DBG7, "%s: debug io is mapped to %x\n", __func__, _debug_output_io);
}

static void map_vic_memory() {
  struct map_desc map;
  /* VIC0 */
  map.physical = PA_VIC0;
  map.virtual = VA_VIC0;
  map.length = SECTION_SIZE;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(kernel_pgd, &map);
  /* VIC1 */
  map.physical = PA_VIC1;
  map.virtual = VA_VIC1;
  map.length = SECTION_SIZE;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(kernel_pgd, &map);

}

static void map_timer_memory() {
  struct map_desc map;
  map.physical = S3C6410_TIMER_PA;
  map.virtual = S3C6410_TIMER_BASE;
  map.length = PAGE_SIZE;
  map.type = MAP_DESC_TYPE_PAGE;
  create_mapping(kernel_pgd, &map);
}

static void clean_user_space() {
  unsigned long virtual = 0;
  for (virtual = 0; virtual < PAGE_OFFSET; virtual += SECTION_SIZE) {
	pgd_t *pgd = pgd_offset(((pgd_t *)kernel_pgd), virtual);
	*pgd = (unsigned long)0;
	//	printk(PR_SS_MM, PR_LVL_DBG7, "%s: virtual = %x, pgd = %x, *pgd = %x\n", __func__, virtual, pgd, *pgd);
  }
}


void mm_init() {
  boot_alloc_ready = false;
  page_alloc_ready = false;
  slab_alloc_ready = false;

  kernel_pgd = PAGE_OFFSET + PAGE_TABLE_OFFSET;
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): kernel_pgd = %x\n", kernel_pgd);
  /* clear the page table at first*/
  prepare_page_table();
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): page table prepared\n");

  /* map main memory, lowmem in linux */
  map_low_memory();
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): low memory mapped\n");

  bootmem_initialize();
  printk(PR_SS_MM, PR_LVL_DBG7, "mm_init(): boot memory allocator initialized\n");

  boot_alloc_ready = true;
  
  /* map vector page */
  map_vector_memory();
  /* map debug page */
  map_debug_memory();
  /* map VIC page */
  map_vic_memory();
  /* map timer IRQ page */
  map_timer_memory();
  /* clean 0~3G space */
  clean_user_space();

  //  bootmem_test();
  init_pages_map();

  page_alloc_init();
  //  pages_alloc_test();
  page_alloc_ready = true;

  bootmem_finalize();
  boot_alloc_ready = false;

  slab_alloc_init();
  slab_alloc_ready = true;
  //  slab_alloc_test();

}

/*
 * Map 64M memory for ListFS archived file:
 * Physical memory between 0x5800 0000 ~ 0x5C00 0000
 * Virtual memory between  0xC800 0000 ~ 0xCC00 0000
 * The mapping section index is between 128 ~ 191
 */
void map_fs_to_ram() {
  struct map_desc map;
  map.physical = 0x58000000;
  map.virtual = 0xC8000000;
  map.length = SECTION_SIZE*64;
  map.type = MAP_DESC_TYPE_SECTION;
  create_mapping(kernel_pgd, &map);
}










/* map the page around addr */
static int fix_page_fault(unsigned long mm_pgd, unsigned long addr, struct vm_area_struct *vma) {
  /* establish the mapping, 1 page each time */
  struct map_desc map;
  unsigned long aligned_addr = page_start(addr);

  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s:\n", __func__);

  map.physical = __virt_to_phys((unsigned long)kmalloc(PAGE_SIZE));
  map.virtual = aligned_addr;
  map.length = PAGE_SIZE;
  map.type = MAP_DESC_TYPE_PAGE;

  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: map.physical = %x, map.virtual = %x, map.length = %x, map.type = %x\n", __func__, map.physical, map.virtual, map.length, map.type);
  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: map.physical = %x, map.virtual = %x, map.length = %x, map.type = %x\n", __func__, map.physical, map.virtual, map.length, map.type);

  create_mapping(mm_pgd, &map);

  /* copy the content if needed */
  if (vma->vm_file) {
	unsigned long target_start, source_start, size;
	unsigned long vm_start = vma->vm_start;
	unsigned long vm_offset = vma->vm_offset;
	unsigned long vm_length = vma->vm_length;
	unsigned long vm_end = vma->vm_end;
	unsigned long file_buf = (unsigned long)vma->vm_file->buf;
	unsigned long file_offset = vma->vm_fileoffset;

	printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: vm_start = %x, vm_end = %x, vm_offset = %x, vm_length = %x\n", __func__, vm_start, vm_end, vm_offset, vm_length);
	printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: file_buf = %x, file_offset = %x\n", __func__, file_buf, file_offset);

	if (aligned_addr == vm_start) {
	  /* in first page */
	  target_start = addr;
	  source_start = file_buf + file_offset;
	  if (vm_length > (aligned_addr + PAGE_SIZE - addr))
		size = aligned_addr + PAGE_SIZE - addr;
	  else
		size = vm_length;
	} else {
	  /* midle or end page */
	  target_start = aligned_addr;
	  source_start = aligned_addr - vm_start - vm_offset + file_offset + file_buf;
	  if (((aligned_addr + PAGE_SIZE) - (vm_start + vm_offset)) < vm_length)
		size = PAGE_SIZE;
	  else
		size = vm_length - (aligned_addr - vm_start - vm_offset);
	}
	printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: target_start = %x, source_start = %x, size = %x\n", __func__, target_start, source_start, size);
	memcpy((void *)target_start, (void *)source_start, size);
  }

  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: finished\n", __func__);
  
  return 0;
}

int do_translation_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr) {
  struct vm_area_struct *vma;

  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s:\n", __func__);

  if (addr >= TASK_SIZE)
	return -1;

  print_all_vma(mm);

  vma = find_vma(mm, addr);
  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: vma = %x\n", __func__, vma);

  if (!vma)
	return -2;

  if (vma->vm_start > addr) {
	/* program is growing stack, vma is the stack, it must be the last vm_area in user space*/
	printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: target address is not in any vma, maybe stack-expanding.\n", __func__);
	if (0 > expand_stack(vma, addr)) {
	  /* stack boundary was exceeded, this is invalid memory access */
	  printk(PR_SS_IRQ, PR_LVL_DBG5, "%s: target address exceeds the stack boundary.\n", __func__);
	  return -3;
	}
  }

  return fix_page_fault(mm->pgd, addr, vma);
}

