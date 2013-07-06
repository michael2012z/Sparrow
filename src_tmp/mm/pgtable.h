#ifndef _PGTABLE_H_
#define _PGTABLE_H_

pte_t mk_pte(struct page *p);

pte_t *pte_offset_map(pmd_t *pmd, unsigned long addr);

void pmd_populate(pmd_t *pmdp, pgtable_t pgtable);

#endif
