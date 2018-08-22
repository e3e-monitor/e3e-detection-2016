#
# Makefile to compile the pyramic demo and tests
#
# Author: Robin Scheibler

CC := g++
DEBUG := -g -Wall
SPEEDFLAGS=-O3 -ffast-math -ftree-vectorize -funroll-loops # -mcpu=cortex-a9 -ftree-loop-ivcanon -mfloat-abi=hard
CPPFLAGS := -std=c++14 -lfftw3f $(SPEEDFLAGS)
#CPPFLAGS := -std=c++14 $(DEBUG) 
LDFLAGS := -L "./lib"
LIB := -lfftw3f -lpyramicio -lpthread
INC := -I "./include"

SRCDIR := src
BUILDDIR := build

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f | grep \.$(SRCEXT)$$) #stft.cpp srpphat.cpp windows.cpp
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TESTS := $(shell find tests -type f | grep \.$(SRCEXT)$$ | cut -f 1 -d '.' | xargs basename -a)

hello:
	@echo $(TESTS)
	@echo $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p build
	$(CC) -c -o $@ $< $(LDFLAGS) $(INC) $(LIB) $(CPPFLAGS)

$(TESTS): $(OBJECTS)
	mkdir -p tests/bin
	$(CC) tests/$@.cpp -o tests/bin/$@ $^ $(LDFLAGS) $(INC) $(LIB) $(CPPFLAGS)

tests: $(TESTS)

clean:
	rm -f bin/* build/* tests/bin/*
