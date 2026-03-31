GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install

CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 --coverage -I$(GSL_PATH)/include -I./src
LDFLAGS = -lm -lgsl -lgslcblas -ldl --coverage -L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64

CLIENTS = libclient.so
TESTS = test_graph_impl

all: build

build: server client

server: src/graph_impl.o src/main.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

client: $(CLIENTS)

libclient.so: src/player.o
	$(CC) $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

test: build_tests
	./test_graph_impl
	dot -Tpdf test1.dot -o test1.pdf

build_tests: $(TESTS)

test_graph_impl: src/graph_impl.o test/graph_impl_test.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

install: build build_tests
	cp server $(CLIENTS) $(TESTS) ./install/

clean:
	rm -f src/*.o src/*.gcno test/*.o test/*.gcno
	rm -f server $(CLIENTS) $(TESTS)
	rm -f ./install/*
	rm -f *.pdf *.dot

.PHONY: all build client build_tests test install clean

