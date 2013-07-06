
src_dir := ../../fs

objects	:= test.o node.o node-test.o vfs-test.o

subdirs := ListFS/

.PHONY: $(subdirs)

all: $(BUILTIN)

$(subdirs):
	$(MAKE) --directory=$@

$(BUILTIN): $(objects) ${subdirs}
	$(LD) $(LFLAGS) -o $@ $(objects) ListFS/$(BUILTIN)

%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
	for d in $(subdirs);	\
		do	\
			$(MAKE) --directory=$$d clean || exit 1;	\
		done
