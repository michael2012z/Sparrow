
src_dir := ../../init

objects	:= test.o shell.o shell-test.o shell_handler-test.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
