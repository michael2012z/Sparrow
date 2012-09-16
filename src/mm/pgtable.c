#include <type.h>
#include <memory.h>
#include <mm.h>
#include "pgtable.h"
#include <printk.h>

pte_t mk_pte(struct page * p) {
  printk(PR_SS_MM, PR_LVL_ERR, "mk_pte(): fatal error, function not implemented.\n");
  while(1);
  return NULL;
}

pte_t *pte_offset_map(pmd_t *pmd, unsigned long addr) {
  return NULL;
}

void pmd_populate(pmd_t *pmdp, pgtable_t pgtable)
{
  phys_addr_t pte = NULL; // need to convert pgtable to pte
  unsigned long pmdval = pte + PTE_HWTABLE_OFF;
  pmdp[0] = __pmd(pmdval);
  pmdp[1] = __pmd(pmdval + 256 * sizeof(pte_t));
  flush_pmd_entry(pmdp);
}
