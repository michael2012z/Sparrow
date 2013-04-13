#ifndef _TYPE_H_
#define _TYPE_H_

typedef int bool;

enum {
	false	= 0,
	true	= 1
};

struct file {
  void *buf;
  int size;
};

#ifndef __ARCH_X86__
#define NULL 0
#endif

typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;
typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;

typedef unsigned int __u32;
typedef unsigned int u32;
typedef unsigned long long u64;

#endif
