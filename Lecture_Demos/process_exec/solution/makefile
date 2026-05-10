SOURCES:=$(wildcard *.c)
TARGETS:=$(SOURCES:%.c=%.run)

.PHONY: clean all echo

all: $(TARGETS)

echo:
	@echo $(SOURCES)
	@echo $(TARGETS)

%.run: %.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.run *.o
