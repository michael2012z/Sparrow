
GLIBCA	:= glibc.a
export GLIBCA

IFLAGS	:=
CFLAGS	:= -c -Wall -nostdinc -mlittle-endian

subdirs := src/

.PHONY: $(subdirs)

all: lib/$(GLIBCA)

src/$(GLIBCA): ${subdirs}

lib/$(GLIBCA): src/$(GLIBCA)
	cp -f src/$(GLIBCA) lib/$(GLIBCA)

$(subdirs):
	$(MAKE) --directory=$@

clean:
	rm -f lib/$(GLIBCA)
	for d in $(subdirs);	\
		do	\
			$(MAKE) --directory=$$d clean || exit 1;	\
		done
