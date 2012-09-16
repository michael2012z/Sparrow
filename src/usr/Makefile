
CFLAGS	+=  $(IFLAGS)

DEMOS := demo_1 demo_2

all: $(DEMOS)

demo_1: 
	$(CC) -nostdlib demo_1.c -o demo_1 

demo_2: 
	$(CC) -nostdlib demo_2.c -o demo_2

clean:
	rm -f $(DEMOS)
