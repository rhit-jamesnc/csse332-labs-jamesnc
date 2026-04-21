TARGETS=1.rf 2.rf 3.rf 4.rf 5.rf 6.rf 7.rf

.PHONY: all clean

# Use this to avoid make deleting all intermediate files.
.SECONDARY:

all: ${TARGETS}

%.rf: %.elf
	./build_rf.py -i $^ -o $@

%.elf: %.o
	ld -T memlayout.ld -o $@ $<

%.o: %.c
	$(CC) -c -O0 -o $@ $<

clean:
	rm -rf *.o *.elf *.rf
