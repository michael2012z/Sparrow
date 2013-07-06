
CFLAGS	+=  $(IFLAGS)

objects	:= mm.o map.o bootmem.o page_alloc.o slab_alloc.o page.o mmap.o alloc.o fault.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
