
CFLAGS	+= -I$(CURDIR)/../../mm
src_dir := ../../mm

objects	:= test.o page.o alloc-test-common.o page_alloc.o page_alloc-test.o slab_alloc.o slab_alloc-test.o alloc.o alloc-test.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
