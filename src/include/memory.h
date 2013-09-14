#ifndef _MEMORY_H_
#define _MEMORY_H_

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL<<PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE-1))

#define SECTION_SHIFT		20
#define SECTION_SIZE		(1UL<<SECTION_SHIFT)
#define SECTION_MASK		(~(SECTION_SIZE-1))

#define THREAD_SIZE		8192
#define THREAD_START_SP		(THREAD_SIZE - 8)


/* Physical */
#define PHYS_OFFSET     0x50000000

/* Virtual */
#define PAGE_OFFSET     0xc0000000
#define TEXT_OFFSET     0x00008000
#define MEMORY_SIZE		0x4000000 /*64M*/
#define MEMORY_PAGES	(MEMORY_SIZE >> PAGE_SHIFT) /* MemorySize / 4K */
#define PAGE_TABLE_OFFSET		0x4000
#define PAGE_TABLE_ADDRESS	(PAGE_OFFSET + PAGE_TABLE_OFFSET)
#define BOOTMEM_MAP_ADDRESS		PAGE_OFFSET
#define BOOTMEM_MAP_SIZE		MEMORY_PAGES

#define TASK_SIZE		(PAGE_OFFSET - 0x01000000)

#define STACK_TOP		TASK_SIZE

/* Exception vectors table */
#define EXCEPTION_SIZE		0x100000
#define EXCEPTION_BASE		0xffff0000
#define EXCEPTION_PAGES		(EXCEPTION_SIZE >> PAGE_SHIFT)


#define	__phys_to_pfn(paddr)	((paddr) >> PAGE_SHIFT)
#define	__pfn_to_phys(pfn)	((pfn) << PAGE_SHIFT)

#define __virt_to_phys(x)	((x) - PAGE_OFFSET + PHYS_OFFSET)
#define __phys_to_virt(x)	((x) - PHYS_OFFSET + PAGE_OFFSET)


/*
 * Type.
*/
#ifndef __ASSEMBLY__
typedef unsigned long 		addr;
#endif

/* 2 level of memory mapping is defined:
 * Level 1 table is named as PGD - page global directory;
 * Level 2 table is named as PT  - page table. 
 */

/* Page directory */
#define PGDIR_SHIFT		20
#define PGDIR_SIZE		(1UL << PGDIR_SHIFT)
#define PGDIR_MASK		(~(PGDIR_SIZE-1))

/*
 * MM flags
 */
#define PMD_TYPE_SECT		(2 << 0)

#define PMD_SECT_BUFFERABLE	(1 << 2)
#define PMD_SECT_CACHEABLE	(1 << 3)
#define PMD_SECT_AP_WRITE	(1 << 10)
#define PMD_SECT_AP_READ	(1 << 11)


#define PMD_SECT_WB		(PMD_SECT_CACHEABLE | PMD_SECT_BUFFERABLE)
#define PMD_FLAGS	PMD_SECT_WB



#ifndef __ASSEMBLY__
typedef unsigned long pte_t;
typedef unsigned long pgd_t;

//typedef struct page *pgtable_t;
typedef u32 phys_addr_t;
#endif


#define __pte(x)        (x)
#define __pmd(x)        (x)


/* to find an entry in a page-table-directory */
#define pgd_index(addr)		((addr) >> PGDIR_SHIFT)
#define pgd_offset(pgd, addr)	((pgd_t *)(((pgd_t *)pgd) + pgd_index(addr)))

#define pte_index(addr)		(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

/*
 * Tools.
 */
#define KILOBYTES_SIZE		(1UL << 10)
#define KILOBYTES_MASK		(~(KILOBYTES_SIZE-1))
#define MEGABYTES_SIZE		(1UL << 20)
#define MEGABYTES_MASK		(~(MEGABYTES_SIZE-1))

#endif
