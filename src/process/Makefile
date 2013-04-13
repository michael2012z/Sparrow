
CFLAGS	+=  $(IFLAGS)

objects	:= process.o elf.o pid.o schedule.o scheduler_cfs.o scheduler_cfs_queue.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
