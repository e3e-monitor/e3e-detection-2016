
CC=c++
DEBUG=-g -Wall
CPPFLAGS=-std=c++14 -lfftw3f $(DEBUG)

MCDIR=../../matrix-creator-hal/cpp/driver/
MCOBJS=everloop_image everloop microphone_array wishbone_bus

HDR=src/stft.h src/mfcc.h src/e3e_detection.h src/windows.h
SRC=stft.cpp srpphat.cpp
OBJS=src/stft.o src/mfcc.o src/srpphat.o src/windows.o
TESTS=test_complex test_fftw test_stft test_stft_speed test_mfcc \
	test_sphere_sampling test_srpphat test_windows

%.o: %.c $(HDR)
	$(CC) -c -o $@ $< $(CPPFLAGS)

test_trigger_stft: $(OBJS) tests/test_trigger_stft.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS) -lmatrix_creator_hal -lwiringPi

test_srpphat: $(OBJS) tests/test_srpphat.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_sphere_sampling: $(OBJS) tests/test_sphere_sampling.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

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

test_windows: $(OBJS) tests/test_windows.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

tests: $(TESTS)

clean:
	rm -f tests/*.o ./test_* src/*.o
