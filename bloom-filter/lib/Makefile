LIB := libbf.a

all: $(LIB)

CFLAGS += -Wall -Wextra -g -O0

bf.o: bf.c bf.h
	$(CC) -c $(CFLAGS) $<

$(LIB): bf.o
	ar cr $@ $<

.PHONY: clean

clean:
	@- $(RM) *.o $(LIB)
