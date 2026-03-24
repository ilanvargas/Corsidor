GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 --coverage -I$(GSL_PATH)/include 
LDFLAGS = -lm -lgsl -lgslcblas -ldl --coverage \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64

CLIENTS = libclient1.so libclient2.so

all: build

build: server client

server: src/graph.o src/graph_show src/move.o src/player.o src/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: $(CLIENTS)

libclient1.so: src/player.o src/graph.o src/move.o
	gcc -shared -o libclient1.so src/player.o src/graph.o src/move.o $(LDFLAGS)

libclient2.so: src/player.o src/graph.o src/move.o
	gcc -shared -o libclient2.so src/player.o src/graph.o src/move.o $(LDFLAGS)

build_tests: test/test_graph

test/test_graph: test/test_graph.c src/graph_show.o
	gcc $(CFLAGS) -I./src $^ -o $@ $(LDFLAGS)
	./$@

alltests: build_tests
	./test/test_graph

test: alltests

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

install: build build_tests
	cp server ./install/
	cp libclient1.so ./install/
	cp libclient2.so ./install/
	cp test/test_graph ./install/


clean:
	rm -f src/*.o test/*.o
	rm -f server libclient1.so libclient2.so test/test_graph
	rm -f ./install/*

PHONY: all build client build_tests alltests test install clean

