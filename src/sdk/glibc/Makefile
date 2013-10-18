
GLIBCSO	:= glibc.so
export GLIBCSO

IFLAGS	:=
CFLAGS	:= -c -Wall -nostdinc -mlittle-endian

subdirs := src/

.PHONY: $(subdirs)

all: lib/$(GLIBCSO)

src/$(GLIBCSO): ${subdirs}

lib/$(GLIBCSO): src/$(GLIBCSO)
	cp -f src/$(GLIBCSO) lib/$(GLIBCSO)

$(subdirs):
	$(MAKE) --directory=$@

clean:
	rm -f lib/$(GLIBCSO)
	for d in $(subdirs);	\
		do	\
			$(MAKE) --directory=$$d clean || exit 1;	\
		done
