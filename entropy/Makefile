CFLAGS = -g
OBJS = ent tbl rel mkpb 
MATH = -lm

ASCIIDOC=/usr/bin/asciidoc
#DOC = Entropy.html

all: $(OBJS) $(DOC)

ent: ent.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

tbl: tbl.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

rel: rel.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

Entropy.html: Entropy.txt
	if [ -x $(ASCIIDOC) ]; then $(ASCIIDOC) $<; fi

.PHONY: clean

clean:
	rm -rf *.o $(OBJS) *.dSYM
