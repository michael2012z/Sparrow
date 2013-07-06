
CFLAGS	+=  $(IFLAGS)

objects	:= printk.o string.o stringx.o date.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
