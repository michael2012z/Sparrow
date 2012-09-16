
src_dir := ../../../fs/ListFS

objects	:= test.o parser.o parser-test.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
