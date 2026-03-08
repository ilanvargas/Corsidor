GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 -I$(GSL_PATH)/include
LDFLAGS = -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib

all: build

build: server client build_tests

build_tests:

server:

client:

alltests:

test: alltests

install: server client test

clean:
	@rm -f *~ src/*~ test/*~

.PHONY: client install test clean
