PROGS = mlzw
OBJS=mlzw.o code.o
all: $(PROGS)

CFLAGS += -g
CFLAGS += -O2 
CFLAGS += -DNDEBUG
#CFLAGS += -DHASH_EMIT_KEYS=3
CFLAGS += -Wall 

$(OBJS): %.o: %.c code.h
	$(CC) -c $(CFLAGS) $< 

mlzw: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

.PHONY: clean

clean:
	rm -f *.o $(PROGS)
