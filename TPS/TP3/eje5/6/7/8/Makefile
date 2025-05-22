CC = mpicc
CFLAGS = -std=c99 -O3 -Wall
LDFLAGS = -lm

all: mpitest3

mpitest3: mpitest3.o sortutils.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	-rm -f *.o
	-rm -f mpitest3
