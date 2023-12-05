#---------------------------------------------------------------------------------------------------
# SnuPL/2 Runtime (AMD64) Makefile
#
# Author: Bernhard Egger <bernhard@csap.snu.ac.kr>
#
CC=gcc
CCFLAGS=-m64 -march=x86-64 -std=c99 -O2
AR=ar
ARFLAGS=rcs

RTE_OBJ=ARRAY.o IO.o
STD_LIB_NAME=snupl
STD_LIB=lib$(STD_LIB_NAME).a

.PHONY: clean

%.o: %.s
	$(CC) $(CCFLAGS) -c -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $^

$(STD_LIB): $(RTE_OBJ)
	$(AR) $(ARFLAGS) $@ $^

rte: $(STD_LIB)

clean:
	rm -rf *.o $(STD_LIB)
