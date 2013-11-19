
CFLAGS:= -I../sdk/glibc/inc

DEMOS := demo_1 demo_2 demo_3 demo_4 demo_5 demo_6 demo_7 demo_8 demo_9 sudoku
LISTFS:= ListFS.fs

all: $(LISTFS)
$(LISTFS): $(DEMOS)

demo_1: demo_1.c 
	$(CC) -nostdlib demo_1.c -o demo_1 
	cp -f demo_1 ./fs_root/fs/bin/

demo_2: demo_2.c
	$(CC) -nostdlib demo_2.c -o demo_2
	cp -f demo_2 ./fs_root/fs/bin/

demo_3: demo_3.c
	$(CC) -nostdlib demo_3.c -o demo_3
	cp -f demo_3 ./fs_root/fs/bin/

demo_4: demo_4.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c demo_4.c -o demo_4.o
	$(CC) -nostdlib demo_4.o ../sdk/glibc/lib/glibc.a -o demo_4
	cp -f demo_4 ./fs_root/fs/bin/

demo_5: demo_5.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c demo_5.c -o demo_5.o
	$(CC) -nostdlib demo_5.o ../sdk/glibc/lib/glibc.a -o demo_5
	cp -f demo_5 ./fs_root/fs/bin/

demo_6: demo_6.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c demo_6.c -o demo_6.o
	$(CC) -nostdlib demo_6.o ../sdk/glibc/lib/glibc.a -o demo_6
	cp -f demo_6 ./fs_root/fs/bin/

demo_7: demo_7.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c demo_7.c -o demo_7.o
	$(CC) -nostdlib demo_7.o ../sdk/glibc/lib/glibc.a -o demo_7
	cp -f demo_7 ./fs_root/fs/bin/

demo_8: demo_8.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c demo_8.c -o demo_8.o
	$(CC) -nostdlib demo_8.o ../sdk/glibc/lib/glibc.a -o demo_8
	cp -f demo_8 ./fs_root/fs/bin/

demo_9: demo_9.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c demo_9.c -o demo_9.o
	$(CC) -nostdlib demo_9.o ../sdk/glibc/lib/glibc.a -o demo_9
	cp -f demo_9 ./fs_root/fs/bin/

sudoku: sudoku.c ../sdk/glibc/lib/glibc.a
	$(CC) $(CFLAGS) -nostdlib -c sudoku.c -o sudoku.o
	$(CC) -nostdlib sudoku.o ../sdk/glibc/lib/glibc.a -o sudoku
	cp -f sudoku ./fs_root/fs/bin/

ListFS.fs:
	python ListFSArchiver.py

clean:
	rm -f *.o
	rm -f $(DEMOS) $(LISTFS)
