
CC=arm-linux-gcc
LD=arm-linux-ld
CFLAGS= -c -g 
LDFLAGS= -Tstart.lds

all: demo

demo: start.o
	$(LD) $(LDFLAGS) start.o -o demo
	arm-linux-objdump -xS demo > demo.s
	arm-linux-readelf -a demo > demo.r
	arm-linux-nm demo > demo.n

start.o: start.S
	$(CC) $(CFLAGS) start.S

clean:
	rm -f demo.s demo.r demo.n start.o demo
