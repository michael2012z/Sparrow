
CFLAGS	+=  $(IFLAGS)

DEMOS := demo_1 demo_2 
LISTFS:= ListFS.fs

all: $(LISTFS)
$(LISTFS): $(DEMOS)

demo_1: 
	$(CC) -nostdlib demo_1.c -o demo_1 
	cp -f demo_1 ./fs_root/fs/

demo_2: 
	$(CC) -nostdlib demo_2.c -o demo_2
	cp -f demo_2 ./fs_root/fs/

ListFS.fs:
	python ListFSArchiver.py

clean:
	rm -f $(DEMOS) $(LISTFS)
