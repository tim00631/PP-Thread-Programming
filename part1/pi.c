#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <immintrin.h>
#include "simdxorshift128plus.h"

struct thread_info {
    int tid;
    long long number_of_toss;
    long long result;
};

__uint32_t xor128(void) {
    static __uint32_t x = 123456789;
    static __uint32_t y = 362436069;
    static __uint32_t z = 521288629;
    static __uint32_t w = 88675123;
    __uint32_t t;

    t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

double fRand() {
    // long long MAX = ((long long)RAND_MAX << 31) + RAND_MAX;
    // long long rand_num = ((long long)rand() << 31) + rand();
    // printf("%lld, %lld, %lf\n", rand_num, MAX, (double)rand_num/MAX);

    return xor128() / 4294967296.0;
    // return ((double)rand_num / (double)MAX);
}

void* child_thread(void* args) {
    // get data
    struct thread_info* info = (struct thread_info*)args;
    int tid = info->tid;
    long long number_of_toss = info->number_of_toss;
    // do calculation
    // printf("in thread %d: number_of_toss:%lld\n", info->tid, info->number_of_toss);
    // int i;

    // for (i = 0; i < number_of_toss; i++) {
    //     double x = fRand();
    //     double y = fRand();
    
    //     double distance_squared = x * x + y * y;
        
    //     if (distance_squared <= 1)
    //         info->result += 1;
    // }
    // printf("in thread %d: result:%lld\n", info->tid, info->result);
    // create a new key
    avx_xorshift128plus_key_t key;
    avx_xorshift128plus_init(324, 4444, &key); // values 324, 4444 are arbitrary, must be non-zero
    size_t i;
    size_t nBlockWidth = 4; 
    size_t cntBlock = number_of_toss / nBlockWidth;    // episode
    size_t cntRem = number_of_toss % nBlockWidth;    // remainder.

    // episode computing block
    for(i=0; i<cntBlock; ++i)
    {
        const __m256d unit = _mm256_set_pd(1, 1, 1, 1);
        // generate 32 random bytes
        __m256i random_x =  avx_xorshift128plus(&key);
        __m256i random_y = avx_xorshift128plus(&key);
        __m256d max = _mm256_set_pd(LONG_MAX,LONG_MAX,LONG_MAX,LONG_MAX);
        __m256d x = _mm256_div_pd(random_x, max);
        __m256d y = _mm256_div_pd(random_x, max);
        __m256d square_x = _mm256_mul_pd(x, x);
        __m256d square_y = _mm256_mul_pd(y, y);
        __m256d square_sum = _mm256_add_pd(square_x, square_y);
        __m256d v_cmp = _mm256_cmp_pd(square_sum, unit, _CMP_LE_OS);
        int result = _mm256_movemask_pd(v_cmp);

        if (result != 0) {
            info->result = info->result + _mm_popcnt_u32(result);
        }
    }

    // 处理剩下的.
    for(i=0; i<cntRem; ++i)
    {
        double x = fRand();
        double y = fRand();
        double distance_squared = x * x + y * y;
    
        if (distance_squared <= 1)
            info->result += 1;
    }

    pthread_exit(NULL);
}

int number_of_cores = 8;
long long total_of_tosses = 1e8;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
    if (argc == 3) {
        number_of_cores = atoi(argv[1]);
        total_of_tosses = atoll(argv[2]);
    }
    pthread_t threads[number_of_cores];
    struct thread_info info[number_of_cores];
    // pthread_t* threads = (pthread_t*)malloc(number_of_cores);
    long long total_in_circle = 0;
    int ret_code;
    srand(time(NULL));
    long long number_of_pthread_tosses = total_of_tosses / number_of_cores;
    long long remainder = total_of_tosses % number_of_cores;
    // printf("every:%lld\n", number_of_pthread_tosses);

    int i;
    for (i = 0; i < number_of_cores; i++) {
        info[i].tid = i;
        info[i].result = 0;
        if (remainder != 0 && i == 0) {
            info[i].number_of_toss = number_of_pthread_tosses + remainder;
        } else {
            info[i].number_of_toss = number_of_pthread_tosses;
        }
        ret_code = pthread_create(&threads[i], NULL, child_thread, (void*)&info[i]);
        if (ret_code) {
            printf("pthread_create() for thread[%d] has a ERROR, return code is %d\n", i, ret_code);
            exit(-1);
        }
    }
    for (i = 0; i < number_of_cores; i++) {
        pthread_join(threads[i], NULL);
        pthread_mutex_lock(&mutex);
        // printf("after thread %d: %lld\n", info[i].tid, info[i].result);
        total_in_circle += info[i].result;
        pthread_mutex_unlock(&mutex);
    }

    printf("%lf\n", (4 * (double)total_in_circle / (double)total_of_tosses));
    pthread_exit(NULL);
}