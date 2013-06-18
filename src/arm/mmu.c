#include <type.h>
#include <memory.h>
#include <mmap.h>
#include <mmu.h>
#include <printk.h>


void flush_pgd_entry(pgd_t *pgd)
{
  asm("mcr	p15, 0, %0, c7, c10, 1	@ flush_pmd"
	  : : "r" (pgd) : "cc");
  asm("mcr p15, 0, %0, c7, c10, 4"
	  : : "r" (0) : "memory");

}


/* CPU do it */
#define set_pte_ext(ptep,pte,ext) 

void __sync_icache(pte_t pteval)
{
  asm("mcr	p15, 0, %0, c7, c5, 0"
	  : : "r" (0));
}



void set_pte_at(struct mm_struct *mm, unsigned long addr, pte_t *ptep, pte_t pteval)
{
  __sync_icache(pteval);
  set_pte_ext(ptep, pteval, PTE_EXT_NG);
  printk(PR_SS_MM, PR_LVL_INF, "set_pte_ext(): fatal error, function not implemented.\n");
  while(1);
}



