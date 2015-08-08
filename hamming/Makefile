PROGS = ecc
OBJS=ecc.o eccode.o
all: $(PROGS)

CFLAGS += -g
CFLAGS += -Wall 

$(OBJS): %.o: %.c eccode.h
	$(CC) -c $(CFLAGS) $< 

ecc: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)

