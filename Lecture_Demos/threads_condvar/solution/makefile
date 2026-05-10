.PHONY: all clean

SRCS:=$(wildcard *.c)
BINS:=$(SRCS:%.c=%.run)

CFLAGS+=-ggdb
LDLIBS+=-pthread

all: $(BINS)

%.run: %.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o *.run

