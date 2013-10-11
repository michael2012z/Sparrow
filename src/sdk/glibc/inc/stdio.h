#ifndef _STDIO_H_
#define _STDIO_H_

#ifdef __ARCH_X86__
void my_printf(const char *fmt, ...);
#else
void printf(const char *fmt, ...);
#endif

#endif /* _STDIO_H_ */
