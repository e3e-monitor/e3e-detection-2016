
CC=c++
DEBUG=-g -Wall
CPPFLAGS=-std=c++14 -lfftw3f $(DEBUG)

HDR=src/stft.h src/mfcc.h src/e3e_detection.h
SRC=stft.cpp
OBJS=src/stft.o src/mfcc.o
TESTS=test_complex test_fftw test_stft test_stft_speed test_mfcc

%.o: %.c $(HDR)
	$(CC) -c -o $@ $< $(CPPFLAGS)

test_mfcc: $(OBJS) tests/test_mfcc.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_stft_speed: $(OBJS) tests/test_stft_speed.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_stft: $(OBJS) tests/test_stft.o src/stft.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_fftw: tests/test_fftw.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_complex: tests/test_complex.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

tests: $(TESTS)

clean:
	rm -f tests/*.o ./test_* src/*.o
