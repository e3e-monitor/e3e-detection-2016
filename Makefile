
CC := g++
DEBUG := -g #-Wall
SPEEDFLAGS := -O3 -mcpu=cortex-a9 -ftree-vectorize -funroll-loops -ftree-loop-ivcanon -mfloat-abi=hard #-mfpu=neon-vfpv4 #
#CPPFLAGS := -std=c++14 -lfftw3f $(SPEEDFLAGS)
CPPFLAGS := -std=c++14 $(DEBUG)
LDFLAGS := -L "./lib"
LIB := -lpyramicio -lfftw3f
INC := -I include

SRCDIR := src
BUILDDIR := build

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f | grep \.$(SRCEXT)) #stft.cpp srpphat.cpp windows.cpp
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TESTS := $(shell find tests -type f | grep \.$(SRCEXT) | cut -f 1 -d '.' | xargs basename -a)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	$(CC) $(LDFLAGS) $(INC) $(CPPFLAGS) -c -o $@ $< $(LIB) 

$(TESTS): $(OBJECTS)
	mkdir -p tests/bin
	$(CC) $(LDFLAGS) $(INC) $(CPPFLAGS) tests/$@.cpp -o tests/bin/$@ $^ $(LIB)

tests: $(TESTS)

clean:
	rm -f bin/* build/* tests/bin/*
