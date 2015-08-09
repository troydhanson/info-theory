PROGS = sf
OBJS=sf.o sfcode.o
all: $(PROGS)

CFLAGS += -g 
CFLAGS += -Wall 
LDFLAGS = -lm

$(OBJS): %.o: %.c sfcode.h
	$(CC) -c $(CFLAGS) $< 

sf: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)

