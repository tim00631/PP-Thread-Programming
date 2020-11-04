#include "SIMDxorshift/include/simdxorshift128plus.h"
int main (){
    // create a new key
    avx_xorshift128plus_key_t mykey;
    avx_xorshift128plus_init(324,4444,&mykey); // values 324, 4444 are arbitrary, must be non-zero

    // generate 32 random bytes, do this as many times as you want
    __m256i randomstuff =  avx_xorshift128plus(&mykey);
}
