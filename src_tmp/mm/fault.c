#include <type.h>
#include <memory.h>
#include <ptrace.h>
#include <mmap.h>
#include <mm.h>
#include "pgtable.h"

static void do_memory_access_fault() {
  return;
}

static void do_out_of_memory() {
  return;
}

int do_page_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
  struct vm_area_struct *vma;

  // Find vma. If fail, means it's an access fault
  vma = find_vma(mm, addr);
  if (NULL == vma)
	goto access_fault;

  // Find PMD
  // Theoretically speaking, there is 3 level page mapping: PGD, PMD and PTE
  // But for ARM, we only use 2 level, so PGD is always same with PMD.
  // If not found: allocate page table, and update PMD
  pgd_t *pgd;
  pmd_t *pmd;
  pgd = pgd_offset(mm, addr);
  pmd = pmd_alloc(mm, pgd, addr);

  if (NULL == *pmd) {
	pgtable_t new = (pgtable_t)pages_alloc(1);
	if (NULL == new)
	  goto out_of_memory;
	pmd_populate(pmd, new);
  }
  // By here, the PMD should be well prepared.


  // Find PTE
  // Not found: allocate page, and update PTE
  pte_t *pte;
  pte = pte_offset_map(pmd, addr);

  if (0 == *pte) {
	// allocate the page
	struct page *page;
	pte_t entry;

	page = (struct page *)pages_alloc(1);
	if (NULL == page)
	  goto out_of_memory;
	entry = mk_pte(page);

	// do anonymous mapping
	add_page_anon_vma(page, vma, addr);

	set_pte_at(mm, addr, page, entry);

  }

  return;

access_fault:
  // prompt that there is memory access fault
  do_memory_access_fault();
  return;

out_of_memory:
  // prompt that there is not enough memory
  do_out_of_memory();
  return;
}

