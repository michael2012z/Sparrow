#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifdef __ARCH_X86__
void *my_malloc(int size);
void my_free(void *ptr);
#else
void *malloc(int size);
void free(void *ptr);
void exit(int code);
void params(char* param1, char* param2, char* param3, char* param4);
int random();
#endif

#endif /* _STDLIB_H_ */
