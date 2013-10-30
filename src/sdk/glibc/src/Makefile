
CFLAGS	+=  $(IFLAGS)

objects	:= string.o printf.o malloc.o inputc.o stdlib.o

all: $(GLIBCSO)

$(GLIBCSO): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o $(GLIBCSO)
