#ifndef _MMU_H_
#define _MMU_H_

/* CPU do it */
#define set_pte_ext(ptep,pte,ext) 

void flush_pmd_entry(pmd_t *pmd);

void __sync_icache(pte_t pteval);

void set_pte_at(struct mm_struct *mm, unsigned long addr, pte_t *ptep, pte_t pteval);

#endif
