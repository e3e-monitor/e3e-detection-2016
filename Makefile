
CC=c++
DEBUG=-g -Wall
CPPFLAGS=-std=c++14 -lfftw3f $(DEBUG)

OBJS=stft.o
TESTS=test_complex test_fftw

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

test_stft: tests/test_stft.o src/stft.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_fftw: tests/test_fftw.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_complex: tests/test_complex.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

tests: $(TESTS)

clean:
	rm -f tests/*.o ./test_* src/*.o
