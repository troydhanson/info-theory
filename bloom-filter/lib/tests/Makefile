SRCS=$(wildcard test*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
PROGS=$(patsubst %.o,%,$(OBJS))

CFLAGS  = -I..
CFLAGS += -g -O0
CFLAGS += -Wall -Wextra

LDFLAGS = -L.. -lbf

TEST_TARGET=run_tests
TESTS=./do_tests

all: $(OBJS) $(PROGS) $(TEST_TARGET) 

# static pattern rule: multiple targets 

$(OBJS): %.o: %.c ../libbf.a
	$(CC) -o $@ -c $(CFLAGS) $< 

$(PROGS): %: %.o
	$(CC) -o $@ $< $(LDFLAGS)

run_tests: $(PROGS)
	perl $(TESTS)

.PHONY: clean

clean:	
	rm -f $(PROGS) $(OBJS) *.o test*.out 
