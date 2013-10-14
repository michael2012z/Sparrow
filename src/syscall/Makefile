
CFLAGS	+=  $(IFLAGS)

objects	:= syscall_memory.o  syscall_process.o  syscall_io.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
