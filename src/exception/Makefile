

CFLAGS	+=  $(IFLAGS)

objects	:= exception.o irq.o chip.o timer.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
