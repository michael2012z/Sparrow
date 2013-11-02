CFLAGS	+= -I../../util
src_dir := ../../util

objects	:= stringx.o date.o printk.o ring_buffer.o ring_buffer-test.o test.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
