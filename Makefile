
CC=c++
DEBUG=-g -Wall
#SPEEDFLAGS=-O3 -ffast-math -mcpu=cortex-a9 -ftree-vectorize -funroll-loops -ftree-loop-ivcanon -mfloat-abi=hard #-mfpu=neon-vfpv4 #
#SPEEDFLAGS=-O3 -ffast-math -ftree-vectorize -funroll-loops
SPEEDFLAGS=-O3 -ftree-vectorize -funroll-loops
CPPFLAGS=-std=c++14 -lfftw3f $(SPEEDFLAGS)
#CPPFLAGS=-std=c++14 -lfftw3f $(DEBUG)

HDR=src/stft.h src/mfcc.h src/e3e_detection.h src/windows.h
SRC=stft.cpp srpphat.cpp windows.cpp
OBJS=src/stft.o src/windows.o
TESTS=test_complex test_fftw test_stft test_windows test_stft_speed \
    test_beamforming_speed
#test_sphere_sampling

%.o: %.c $(HDR)
	$(CC) -c -o $@ $< $(CPPFLAGS)

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

test_beamforming_speed: $(OBJS) tests/test_beamforming_speed.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

tests: $(TESTS)

clean:
	rm -f tests/*.o ./test_* src/*.o
