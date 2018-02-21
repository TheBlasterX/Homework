.PHONY: clean
build:
	mpicc tema3.c -o filtru -std=c99 -lm

run:
	mpirun -np 11 ./filtru topologie.in imagini.in statistica.out

clean:
	rm -f filtru
