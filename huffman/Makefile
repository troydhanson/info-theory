PROGS = huff
OBJS=huff.o code.o
all: $(PROGS)

#CFLAGS += -g
CFLAGS += -O2 -DNDEBUG
CFLAGS += -Wall 

$(OBJS): %.o: %.c code.h
	$(CC) -c $(CFLAGS) $< 

huff: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

.PHONY: clean

clean:
	rm -f *.o $(PROGS)
	make -C tests clean
