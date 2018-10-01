CC := gcc
LD := gcc

ROOT ?= ../..

MAKE := make
NPROC=$(shell nproc)

CFLAGS += -Wall -Winline $(LDLIB)
CFLAGS += -O3 -DT$(NPROC)

LDFLAGS += -lpthread -lnuma -DT$(NPROC)

.PHONY: all clean

BINS = benchmark_tree_single_thread


all: $(BINS)

benchmark_list.o: benchmark_list.c benchmark_list.h
	$(CC) $(CFLAGS) -c -o $@ $<

tree_single_thread.o: tree_single_thread.c
	$(CC) $(CFLAGS) -c -o $@ $<

benchmark_tree_single_thread: tree_single_thread.o benchmark_list.o
	$(LD) $(LDLIB) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BINS) *.o *.so
