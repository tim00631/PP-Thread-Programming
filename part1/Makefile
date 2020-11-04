all: pi.out

pi.out: pi.c
	clang++ -lm -lpthread -mavx2 -mavx512vl -mavx512dq  pi.c -o pi.out -std=c++11 -Wall
clean:
	rm -f *.o *.s pi.out *~