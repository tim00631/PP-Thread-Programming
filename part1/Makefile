all: pi.out

pi.out: pi.c
	gcc -lm -lpthread pi.c -o pi.out -std=c99
clean:
	rm -f *.o *.s pi.out *~