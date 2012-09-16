
CFLAGS	+=  $(IFLAGS)

objects	:= process.o elf.o pid.o schedule.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
