
CFLAGS	+=  $(IFLAGS)
AFLAGS	+= $(IFLAGS)
AFLAGS	+= -DTEXT_OFFSET=0x00008000

objects1 := head.o init_task.o
objects2 := exception.o irq.o timer.o vic.o process.o entry.o mmu.o

objects := $(objects1) $(objects2)

all: $(BUILTIN) $(objects1)


$(BUILTIN): $(objects2)
	$(LD) $(LFLAGS) -o $@ $^ 


%.o: %.c
	$(CC) $(CFLAGS) $<

%.o: %.S
	$(CC) $(AFLAGS) $<

$(subdirs):
	$(MAKE) --directory=$@

clean:
	rm -f *.o
