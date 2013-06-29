
#CROSS_COMPILE	:= /usr/local/arm/4.2.2-eabi/usr/bin/arm-linux-
CROSS_COMPILE	:= /usr/local/arm/4.4.3-elf/bin/arm-elf-

CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)ld
AR	:= $(CROSS_COMPILE)ar
NM	:= $(CROSS_COMPILE)nm

IFLAGS	:= -I$(CURDIR)/include -isystem $(shell $(CC) -print-file-name=include)
IFLAGS	+= -I$(CURDIR)/arm/include -I$(CURDIR)
CFLAGS	:= -c -Werror -Wall -g -nostdinc -D__KERNEL__ -mlittle-endian -D__LINUX_ARM_ARCH__=6 
#-DSLAB_OPTIMIZE
# -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -mabi=aapcs-linux -mno-thumb-interwork -msoft-float 
AFLAGS	:= $(CFLAGS) -D__ASSEMBLY__
#CFLAGS	+= -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -marm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -Uarm -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow 
# -O2
LFLAGS	:= -EL -r

BUILTIN	:= built-in.o

export CC LD AR CFLAGS IFLAGS AFLAGS LFLAGS BUILTIN

subdirs := arm/ init/ kernel/ mm/ exception/ process/ fs/ usr/ test/

lklist	:= arm/head.o arm/init_task.o arm/built-in.o init/built-in.o kernel/built-in.o mm/built-in.o exception/built-in.o process/built-in.o fs/$(BUILTIN)

.PHONY: $(subdirs)

all: vmlinux.o vmlinux

vmlinux.o: ${subdirs}
	$(LD) $(LFLAGS) -o $@ $(lklist)

vmlinux: ${subdirs}
	$(CC) -static -nostartfiles -nostdlib -T arm/vmlinux.lds $(lklist) -o vmlinux -lgcc

$(subdirs):
	$(MAKE) --directory=$@

cscope:
	rm -f cscope.* 
	find ./ -name "*.[chS]" -print > cscope.files
	cscope -bqk -i cscope.files

clean:
	rm -f vmlinux.o vmlinux 
	for d in $(subdirs);	\
		do	\
			$(MAKE) --directory=$$d clean || exit 1;	\
		done


