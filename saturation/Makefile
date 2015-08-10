PROGS = saturation
OBJS=saturation.o
all: $(PROGS)

CFLAGS += -g
CFLAGS += -Wall 
LDFLAGS = 

$(OBJS): %.o: %.c 
	$(CC) -c $(CFLAGS) $< 

ezip: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)

