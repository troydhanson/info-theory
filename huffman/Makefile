PROGS = huff
OBJS=huff.o code.o tpl.o
all: $(PROGS)

#CFLAGS += -g
CFLAGS += -O2
CFLAGS += -Wall 

$(OBJS): %.o: %.c code.h
	$(CC) -c $(CFLAGS) $< 

huff: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)
	make -C tests clean
