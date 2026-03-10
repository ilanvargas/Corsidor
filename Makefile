GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 -I$(GSL_PATH)/include
LDFLAGS = -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib

all: build build_tests

build: server client 

build_tests:

server: src/graph.o src/move.o src/player.o src/main.o
	$(CC) $(CFLAGS) -o install/$@ $^

client:

alltests:

test: alltests

install: server client test

%.o: %.c
	$(CC) $(CLFAGS) -c $< -o $@

clean:
	@rm -f *~ src/*~ test/*~

.PHONY: client install test clean
