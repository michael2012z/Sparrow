#ifndef _LINUX_LINKAGE_H_
#define _LINUX_LINKAGE_H_

#define __HEAD		.section	".head.text","ax"

#ifndef __ALIGN
#define __ALIGN		.align 4,0x90
#endif

#define ALIGN __ALIGN

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:
#endif


#ifndef END
#define END(name) \
  .size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
  .type name, %function; \
  END(name)
#endif


#define __init		__attribute__ ((__section__(".init.text")))
#define __initdata		__attribute__ ((__section__(".init.data")))
#define __exception	__attribute__((section(".exception.text")))


#define __raw_readl(a)		(*(volatile unsigned int *)(a))
#define __raw_writel(a,v)	(*(volatile unsigned int *)(a) = (v))


#endif
