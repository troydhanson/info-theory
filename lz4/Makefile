PROGS := do-lz4 undo-lz4 frame-lz4 unframe-lz4
SRCS := $(wildcard *.c)
OBJS := ${SRCS:.c=.o}

INCLUDE_DIRS := /usr/include
LIBRARY_DIRS := /usr/lib
LIBS := lz4

CFLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach libdir,$(LIBRARY_DIRS),-L$(libdir))
LDFLAGS += $(foreach lib,$(LIBS),-l$(lib))

CFLAGS += -Wall -Wextra -g -O0

all: $(PROGS)

$(PROGS): %: %.o
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) 

.PHONY: clean

clean:
	@- $(RM) $(OBJS) $(PROGS)
