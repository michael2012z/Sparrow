
CFLAGS	+=  $(IFLAGS)
AFLAGS	+= $(IFLAGS)
AFLAGS	+= -DTEXT_OFFSET=0x00008000

objects1 := head.o libfunc.o
objects2 := exception.o irq.o timer.o uart.o vic.o process.o entry.o unwind.o fault.o setup.o

objects := $(objects1) $(objects2)

all: $(objects1) $(BUILTIN)


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
