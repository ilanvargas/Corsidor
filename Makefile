GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
#GSL_PATH ?= $(HOME)/corsidor-S6/gsl
DOT ?= dot
#DOT ?= $(HOME)/corsidor-S6/graphviz/bin/dot

CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 --coverage -I$(GSL_PATH)/include -I./src
LDFLAGS = -lm -lgsl -lgslcblas -ldl --coverage \
        -L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
				-Wl,--rpath=${GSL_PATH}/lib

CLIENTS = tourist.so zscarecrow.so foenem.so
TESTS = test_move_valid test_waterloo test_player_foenem_funcs

all: build

build: server client

server: src/graph_impl.o src/move_valid.o src/waterloo.o src/main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

client: $(CLIENTS)

tourist.so: src/player_tourist.o src/tourist.o
	$(CC) $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

foenem.so: src/graph_impl.o src/player_foenem.o src/move_valid.o
	$(CC) $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

zscarecrow.so: src/player_scarecrow.o
	$(CC) $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

dot_test: test_graph_impl
	./test_graph_impl
	$(DOT) -Tpdf test1.dot -o test1.pdf
	$(DOT) -Tpdf parallelogram_m5_test.dot -o parallelogram_m5_test.pdf
	$(DOT) -Tpdf parallelogram_m9_test.dot -o parallelogram_m9_test.pdf
	$(DOT) -Tpdf triangular_test.dot -o triangular_test.pdf

test: build_tests
	./test_move_valid
	./test_waterloo
	./test_player_foenem_funcs

build_tests: $(TESTS)

test_waterloo: src/waterloo.o src/graph_impl.o test/test_waterloo.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_move_valid: src/move_valid.o src/graph_impl.o test/test_move_valid.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_player_foenem_funcs: src/player_foenem.o src/graph_impl.o src/move_valid.o test/test_player_foenem_funcs.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_graph_impl: src/graph_impl.o test/test_graph_impl.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

install: build build_tests
	cp server $(CLIENTS) $(TESTS) ./install/

clean:
	rm -f src/*.o src/*.gcno src/*gcda test/*.o test/*.gcno test/*.gcda
	rm -f server $(CLIENTS) $(TESTS) test_graph_impl
	rm -f ./install/*
	rm -f *.pdf *.dot
	rm -f *.gcda *.gcno

.PHONY: all build client build_tests test install clean

