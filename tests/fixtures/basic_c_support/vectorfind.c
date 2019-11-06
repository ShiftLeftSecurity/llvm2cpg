#include <immintrin.h>

int foo_external(int formalname);

int findbyte(char* haystack, char needle, int bogus) {
   // volatile int bogus2 = bogus;
    foo_external(++bogus);
    __m128i haystackv = *(__m128i*)haystack;
    __m128i cmpres = _mm_cmpeq_epi8(haystackv, _mm_set1_epi8(needle));
    return _mm_movemask_epi8(cmpres); 
}

typedef struct {int x; int y; int z;} pointT;

int extractX(pointT point){
    return point.x;
}

int callExtractX(int a, int b){
    pointT myPoint = {a, b, -1};
    return extractX(myPoint);
}

typedef struct {} SingletonT;

void fooSingleton(SingletonT arg){
    return;
}