
CFLAGS	+=  $(IFLAGS)

objects	:= string.o printf.o malloc.o inputc.o stdlib.o fault.o libfunc.o start.o

all: $(GLIBCA)

$(GLIBCA): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

%.o: %.S
	$(CC) $(AFLAGS) $<

clean:
	rm -f *.o $(GLIBCA)
