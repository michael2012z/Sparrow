
subdirs := ListFS/

CFLAGS	+=  $(IFLAGS)

objects	:= vfs.o node.o mount.o

all: $(BUILTIN)

$(BUILTIN): $(objects) ${subdirs}
	$(LD) $(LFLAGS) -o $@ $(objects) ListFS/$(BUILTIN)

.PHONY: $(subdirs)

$(subdirs):
	$(MAKE) --directory=$@

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
	for d in $(subdirs);	\
		do	\
			$(MAKE) --directory=$$d clean || exit 1;	\
		done
