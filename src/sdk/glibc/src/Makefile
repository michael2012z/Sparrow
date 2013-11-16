
CFLAGS	+=  $(IFLAGS)

objects	:= string.o printf.o malloc.o inputc.o stdlib.o

all: $(GLIBCA)

$(GLIBCA): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o $(GLIBCA)
