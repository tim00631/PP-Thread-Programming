all: pi.out

pi.out: pi.c
	gcc -g -lm -pthread -mavx2 -mavx512vl -mavx512dq  pi.c simdxorshift128plus.c -o pi.out -std=c99 -Wall
clean:
	rm -f *.o *.s pi.out *~