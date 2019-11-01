#include <immintrin.h>

int foo_external(int formalname);

int findbyte(char* haystack, char needle, int bogus) {
   // volatile int bogus2 = bogus;
    foo_external(++bogus);
    __m128i haystackv = *(__m128i*)haystack;
    __m128i cmpres = _mm_cmpeq_epi8(haystackv, _mm_set1_epi8(needle));
    return _mm_movemask_epi8(cmpres); 
}