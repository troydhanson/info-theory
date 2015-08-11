CFLAGS = -g
OBJS = ent eo2 tbl rel mkpb 
MATH = -lm

all: $(OBJS) $(DOC)

ent: ent.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

eo2: eo2.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

tbl: tbl.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

rel: rel.c
	$(CC) $(CFLAGS) -o $@ $< $(MATH)

.PHONY: clean

clean:
	rm -rf *.o $(OBJS) *.dSYM
