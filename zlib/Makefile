all: gz ungz
LDFLAGS=-lz

gz: gz.o
	$(CC) -o $@ $< $(LDFLAGS)

ungz: ungz.o
	$(CC) -o $@ $< $(LDFLAGS)


.PHONY: clean

clean: 
	rm -f gz ungz *.o
