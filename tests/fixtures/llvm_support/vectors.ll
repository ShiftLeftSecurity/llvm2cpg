define <5 x float> @shuffle(<2 x float> %x, <2 x float> %y) {
  %res = shufflevector <2 x float> %x, <2 x float> %y, <5 x i32> <i32 undef, i32 2, i32 3, i32 0, i32 0>
  ret <5 x float> %res
}

define <2 x float> @insert(float %x, float %y) {
  %v = insertelement <2 x float> undef, float %x, i8 1 ; insert
  %res = insertelement <2 x float> %v, float %y, i16 0
  ret <2 x float> %res
}

define float @extract(<2 x float> %x, i32 %idx) {
  %res = extractelement <2 x float> %x, i32 %idx    ; extract
                                                    ;this is _bad_ code and only borderline valid, cf emitted native.
  ret float %res
}
;	movaps	%xmm0, -24(%rsp)
;	andl	$3, %edi
;	movss	-24(%rsp,%rdi,4), %xmm0 # xmm0 = mem[0],zero,zero,zero
;	retq

define void @vectorGEP(<4 x i8*> %ptrs, <4 x i32> %offsets){
    %A = getelementptr i8, <4 x i8*> %ptrs, <4 x i32> %offsets ; gep1
    %B = getelementptr i8, <4 x i8*> %ptrs, i32 5 ; gep2
    %P1 = extractelement <4 x i8*> %ptrs, i32 0
    %C = getelementptr i8, i8* %P1, <4 x i32> %offsets ; gep3
    ret void
}

; Somewhat realistic code to find all occurences of a needle in a haystack.
; in C with <immintrin.h>:  int findbyte(char* haystack, char needle) {return _mm_movemask_epi8(_mm_cmpeq_epi8(*(__m128i*)haystack, _mm_set1_epi8(needle)));}
; depends massively on compilation flags + context: clang enjoys using the alternative
; declare i32 @llvm.x86.sse2.pmovmskb.128(<16 x i8>)
define i32 @findbyte(i8* %haystack, i8 %needle){
    %ptr = bitcast i8* %haystack to <16 x i8>*
    %haystack_v = load <16 x i8>, <16 x i8>* %ptr, align 16
    %tmp = insertelement <1 x i8> undef, i8 %needle, i32 0
    %needle_vec = shufflevector <1 x i8> %tmp, <1 x i8> undef, <16 x i32> zeroinitializer ; shuffle
    %pos = icmp eq <16 x i8> %haystack_v, %needle_vec
    %pmovmskb = bitcast <16 x i1> %pos to i16
    %res = zext i16 %pmovmskb to i32
    ret i32 %res
}
