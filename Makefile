
CC=g++
DEBUG=-g #-Wall
SPEEDFLAGS=-O3 -mcpu=cortex-a9 -ftree-vectorize -funroll-loops -ftree-loop-ivcanon -mfloat-abi=hard #-mfpu=neon-vfpv4 #
#CPPFLAGS=-std=c++14 -lfftw3f $(SPEEDFLAGS)
CPPFLAGS=-std=c++14 $(DEBUG)
LIB := -lfftw3f -L lib
INC := -I include

SRCEXT=cpp
SRC=$(shell find src -type f) #stft.cpp srpphat.cpp windows.cpp
OBJS=src/stft.o src/windows.o
#TESTS=test_complex \
      test_fftw \
      test_stft \
      test_windows \
      test_stft_speed \
      test_beamforming_speed
TESTS=$(shell find tests -type f | grep \.cpp | cut -f 1 -d '.' | xargs basename -a)

hello:
	@echo "helloworld sources: $(SRC)"

%.o: %.$(SRCEXT)
	#@echo "Object: $@"
	$(CC) $(CPPFLAGS) $(INC) $(LIB) -c -o $@ $< 

$(TESTS): $(OBJS)
	$(CC) tests/$@.cpp -o bin/$@ $^ $(CPPFLAGS) $(INC) $(LIB)

tests: $(TESTS)

clean:
	rm -f bin/* build/* src/*.o

