
CFLAGS	+=  $(IFLAGS)

objects	:= main.o shell.o shell_handler.o #shell_test.o

all: $(BUILTIN)

$(BUILTIN): $(objects)
	$(LD) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
