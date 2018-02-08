SRCS=$(wildcard test*.c)
PROGS=$(patsubst %.c,%,  $(SRCS))
OBJS =$(patsubst %.c,%.o,$(SRCS))

CFLAGS = -I../src
CFLAGS += -Wall -Wextra
CFLAGS += -g -O0
CFLAGS += -DXZ_USE_CRC64

#CFLAGS += -O2

STATIC_OBJS=     \
	xz_crc32.o     \
	xz_crc64.o     \
	xz_dec_lzma2.o \
	xz_dec_stream.o

all: $(STATIC_OBJS) $(PROGS) tests

# this Makefile builds the ../src/ objects
# itself and links them right in statically
$(STATIC_OBJS):
	$(CC) -c $(CFLAGS) ../src/xz_crc32.c
	$(CC) -c $(CFLAGS) ../src/xz_crc64.c
	$(CC) -c $(CFLAGS) ../src/xz_dec_lzma2.c
	$(CC) -c $(CFLAGS) ../src/xz_dec_stream.c

# static pattern rule: multiple targets 
$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< 

$(PROGS): %: %.o $(STATIC_OBJS)
	$(CC) -o $@ $(CFLAGS) $< $(STATIC_OBJS) $(LDFLAGS)

.PHONY: clean tests

tests:
	perl ./do_tests

clean:	
	rm -f $(OBJS) $(STATIC_OBJS) $(PROGS) *.out
