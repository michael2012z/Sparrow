
src_dir := ../../sdk/glibc/src

objects	:= malloc.o malloc-test.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
