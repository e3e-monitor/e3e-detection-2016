
CC=c++
DEBUG=-g
CPPFLAGS=-std=c++14 -lfftw3f $(DEBUG)

TESTS=test_complex test_fftw

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

test_fftw: tests/test_fftw.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

test_complex: tests/test_complex.o
	$(CC) -o tests/$@ $^ $(CPPFLAGS)

tests: $(TESTS)

clean:
	rm -f tests/*.o ./test_*
