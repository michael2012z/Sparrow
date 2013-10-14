#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifdef __ARCH_X86__
void *my_malloc(int size);
void my_free(void *ptr);
#else
void *malloc(int size);
void free(void *ptr);
#endif

#endif /* _STDLIB_H_ */
