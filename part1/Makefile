all: pi.out

pi.out: pi.c
	gcc -lm -lpthread -mavx2 pi.c -o pi.out -std=c99
clean:
	rm -f *.o *.s pi.out *~