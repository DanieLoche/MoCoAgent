
XENO_CONFIG = /usr/xenomai/bin/xeno-config

XENO_CFLAGS = $(shell $(XENO_CONFIG) --alchemy --posix --cflags)
CXXFLAGS = $(XENO_CFLAGS) -Wall -g -O3 -std=c++11

XENO_LDFLAGS = $(shell $(XENO_CONFIG) --no-auto-init --alchemy --posix --ldflags)
LDFLAGS = $(XENO_LDFLAGS)

all: ProgTest
	g++ $(CXXFLAGS) *.o *.a -o _ProgTest $(LDFLAGS)

ProgTest:
	g++ $(CXXFLAGS) $(LDFLAGS) -c _progtest.c -lm
