#include <errno.h>
#include <ptrace.h>
#include <mmap.h>
#include "elf.h"
#include <printk.h>

static void print_elf_file(struct file *filep) {
  int row=0, i, j, tmp;
  int digits;
  unsigned char c;
  static unsigned char hex_sym[] = "0123456789abcdef";

  printk(PR_SS_PROC, PR_LVL_DBG1, "\nprint_elf_file():\n");
  printk(PR_SS_PROC, PR_LVL_DBG1, "          00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff ");
  for (i=0; i<filep->size; i++) {
	// now only print several lines
	if (i >= 16 * 8)
	  break;

	if (0 == i%16) {
	  // print line number
	  printk(PR_SS_PROC, PR_LVL_DBG1, "\n");
	  tmp = row;
	  digits = 0;
	  do {
		tmp = tmp / 16;
		digits ++;
	  }	while(0 != tmp);
	  for (j = 0; j < (7-digits); j++)
		printk(PR_SS_PROC, PR_LVL_DBG1, "0");
	  for (j = digits; j >0; j--) {
		tmp = 0x0f << (4 * (j-1));
		tmp = row & tmp;
		tmp = tmp >> (4 * (j-1));
		printk(PR_SS_PROC, PR_LVL_DBG1, "%c", hex_sym[tmp]);
	  }
	  printk(PR_SS_PROC, PR_LVL_DBG1, "0: ");
	  row++;
	}
	c = ((unsigned char *)filep->buf)[i];
	// print first digit
	printk(PR_SS_PROC, PR_LVL_DBG1, "%c", hex_sym[(c&0x0f0) >> 4]);
	printk(PR_SS_PROC, PR_LVL_DBG1, "%c", hex_sym[(c&0x0f)]);
	printk(PR_SS_PROC, PR_LVL_DBG1, " ");
  }
}

static void print_elf_phdr(struct elf_phdr *x) {
  printk(PR_SS_PROC, PR_LVL_DBG1, "\nprint_elf_phdr():\n");
  printk(PR_SS_PROC, PR_LVL_DBG1, " p_type = %x \n p_offset = %x \n p_vaddr = %x \n p_paddr = %x \n p_filesz = %x \n p_memsz = %x \n p_flags = %x \n p_align = %x \n", x->p_type, x->p_offset, x->p_vaddr, x->p_paddr, x->p_filesz, x->p_memsz, x->p_flags, x->p_align);
}

static void print_elf_hdr(struct elf_hdr *x) {
  int i;
  printk(PR_SS_PROC, PR_LVL_DBG1, "\nprint_elf_hdr():\n");
  for (i=0; i<4; i++)
	printk(PR_SS_PROC, PR_LVL_DBG1, " e_ident[%d] = %c(%x)\n", i, x->e_ident[i], x->e_ident[i]);
  printk(PR_SS_PROC, PR_LVL_DBG1, " e_type = %x \n e_machine = %x \n e_version = %x \n e_entry = %x \n e_phoff = %x \n e_shoff = %x \n e_flags = %x \n e_ehsize = %x \n e_phentsize = %x \n e_phnum = %x \n e_shentsize = %x \n e_shnum = %x \n e_shstrndx = %x \n", x->e_type, x->e_machine, x->e_version, x->e_entry, x->e_phoff, x->e_shoff, x->e_flags, x->e_ehsize, x->e_phentsize, x->e_phnum, x->e_shentsize, x->e_shnum, x->e_shstrndx);

}

static int elf_check_structure(struct elf_hdr *x)
{
  int retval = ENOERROR;

  printk(PR_SS_PROC, PR_LVL_DBG1, "\nelf_check_structure():\n");

  /* Check magic number. */
  if (memcmp(x->e_ident, ELFMAG, SELFMAG) != 0) {
	retval = EGENERAL;
	goto out;
  }

  /* Check whether it's executable. */
  if (x->e_type != ET_EXEC) {
	retval = EGENERAL;
	goto out;
  }

  /* Check the architecture. */
  if (x->e_machine != EM_ARM) {
	retval = EGENERAL;
	goto out;
  }

out:
  printk(PR_SS_PROC, PR_LVL_DBG1, " retval = %x\n", retval);
  return retval;
}

static unsigned long elf_map(struct mm_struct *mm, struct file *filep, unsigned long addr, struct elf_phdr *eppnt)
{
	unsigned long map_addr;
	unsigned long size = eppnt->p_filesz + ELF_PAGEOFFSET(eppnt->p_vaddr);
	/* This is to calculate the offset in from the beginning of page, because the content of file will be mapped in page by page. */
	unsigned long off = eppnt->p_offset - ELF_PAGEOFFSET(eppnt->p_vaddr);

	printk(PR_SS_PROC, PR_LVL_DBG1, "elf_map():\n");
	printk(PR_SS_PROC, PR_LVL_DBG1, " addr = %x, size = %x\n", addr, size);

	addr = ELF_PAGESTART(addr);
	size = ELF_PAGEALIGN(size);

	printk(PR_SS_PROC, PR_LVL_DBG1, " addr = %x, size = %x, off = %x\n", addr, size, off);

	/* mmap() will return -EINVAL if given a zero size, but a
	 * segment with zero filesize is perfectly valid */
	if (!size)
		return addr;

	map_addr = do_mmap(mm, filep, addr, size, off);

	return(map_addr);
}

static int set_brk(struct mm_struct *mm, unsigned long start, unsigned long end)
{
  printk(PR_SS_PROC, PR_LVL_DBG1, "\nset_brk():\n");
  printk(PR_SS_PROC, PR_LVL_DBG1, " start = %x, end = %x\n", start, end);

  start = ELF_PAGEALIGN(start);
  end = ELF_PAGEALIGN(end);

  printk(PR_SS_PROC, PR_LVL_DBG1, " start = %x, end = %x\n", start, end);

  if (end > start) {
	unsigned long addr;
	addr = do_brk(mm, start, end - start);
	if (BAD_ADDR(addr))
	  return addr;
  }
  return 0;
}


// bprm->file is a point to the binary code of elf file.
int load_elf_binary(struct file *filep, struct pt_regs *regs, struct mm_struct *mm)
{

  int retval, i;
  unsigned long elf_bss, elf_brk;
  unsigned long start_code, end_code, start_data, end_data;
  unsigned long error;
  unsigned long elf_entry;
  struct elf_hdr elf_header;
  struct elf_phdr *elf_ppnt, *elf_phdata;

  printk(PR_SS_PROC, PR_LVL_DBG1, "\nload_elf_binary():\n");

  print_elf_file(filep);

  elf_header = *((struct elf_hdr *)filep->buf);

  print_elf_hdr(&elf_header);

  /* Check the magic number. */
  retval = ENOERROR;

  if (elf_check_structure(&elf_header) < 0)
	goto out;

  /* Read the program header. */
  elf_phdata = (struct elf_phdr *)((char *)filep->buf + elf_header.e_phoff);

  elf_ppnt = elf_phdata;
  elf_bss = 0;
  elf_brk = 0;
  start_code = ~0UL;
  end_code = 0;
  start_data = 0;
  end_data = 0;

  printk(PR_SS_PROC, PR_LVL_DBG1, " e_phnum = %x\n", elf_header.e_phnum);

  /* Map the ELF file into virtual memory space. */
  elf_ppnt = elf_phdata;
  for(i = 0; i < elf_header.e_phnum; i++, elf_ppnt++) {
	int elf_prot = 0, elf_flags;
	unsigned long k, vaddr;

	print_elf_phdr(elf_ppnt);

	if (elf_ppnt->p_type != PT_LOAD)
	  continue;
	else
	  printk(PR_SS_PROC, PR_LVL_DBG1, " loadable program header found\n");

	vaddr = elf_ppnt->p_vaddr;

	error = elf_map(mm, filep, vaddr, elf_ppnt);

	printk(PR_SS_PROC, PR_LVL_DBG1, "  elf_map = %x\n", error);

	if (BAD_ADDR(error)) {
	  goto out;
	}

	k = elf_ppnt->p_vaddr;
	if (k < start_code)
	  start_code = k;
	if (start_data < k)
	  start_data = k;

	k = elf_ppnt->p_vaddr + elf_ppnt->p_filesz;
	if (k > elf_bss)
	  elf_bss = k;
	if (end_code < k)
	  end_code = k;
	if (end_data < k)
	  end_data = k;
	k = elf_ppnt->p_vaddr + elf_ppnt->p_memsz;
	if (k > elf_brk)
	  elf_brk = k;

  }

  printk(PR_SS_PROC, PR_LVL_DBG1, " elf_bss = %x, elf_brk = %x, start_code = %x, end_code = %x, start_data = %x, end_data = %x\n",elf_bss, elf_brk, start_code, end_code, start_data, end_data);

  /* Allocate space for BSS segment, and map it. */
  retval = set_brk(mm, elf_bss, elf_brk);
  printk(PR_SS_PROC, PR_LVL_DBG1, " set_brk = %x\n", retval);
  if (retval)
	goto out;

  elf_entry = elf_header.e_entry;
  if (BAD_ADDR(elf_entry)) {
	retval = -EINVAL;
	goto out;
  }
  printk(PR_SS_PROC, PR_LVL_DBG1, " elf_entry = %x\n", elf_entry);

  mm->end_code = end_code;
  mm->start_code = start_code;
  mm->start_data = start_data;
  mm->end_data = end_data;

  start_thread(regs, elf_entry, mm->start_stack);
  retval = 0;

 out:
  return retval;

}

