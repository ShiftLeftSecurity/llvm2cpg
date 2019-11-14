#include <gtest/gtest.h>
#include <llvm2cpg/Demangler/Demangler.h>
#include <string>
#include <utility>
#include <vector>

struct Expectation {
  std::string input;
  std::string fullName;
  std::string name;
  Expectation(std::string input, std::string fullName, std::string name)
      : input(std::move(input)), fullName(std::move(fullName)), name(std::move(name)) {}
};

TEST(Demangler, tableTests) {
  llvm2cpg::Demangler demangler;

  std::vector<Expectation> expectations({
      /// Sanity checks
      Expectation("", "", ""),
      Expectation("hello", "hello", "hello"),
      Expectation("_hello", "_hello", "_hello"),

      /// C++

      // clang-format off

      Expectation("_ZN6Simple8sayHelloEi",
                  "Simple::sayHello(int)",
                  "sayHello"),

      Expectation("_ZNSt3__19nullptr_tC1EMNS0_5__natEi",
                  "std::__1::nullptr_t::nullptr_t(int std::__1::nullptr_t::__nat::*)",
                  "nullptr_t"),

      Expectation("_ZNSt3__16vectorIiNS_9allocatorIiEEED1Ev",
                  "std::__1::vector<int, std::__1::allocator<int> >::~vector()",
                  "~vector"),

      Expectation("_Z7doStuff5Hello",
                  "doStuff(Hello)",
                  "doStuff"),

      Expectation("_ZNSt3__1L7forwardIPiEERT_RNS_16remove_referenceIS2_E4typeE",
                  "int*& std::__1::forward<int*>(std::__1::remove_reference<int*>::type&)",
                  "forward<int*>"),

      Expectation("_ZNKSt3__16vectorIiNS_9allocatorIiEEE17__annotate_deleteEv",
                  "std::__1::vector<int, std::__1::allocator<int> >::__annotate_delete() const",
                  "__annotate_delete"),

      Expectation("_ZdlPv",
                  "operator delete(void*)",
                  "operator delete"),

      Expectation("_ZNSt3__1L4sortIPfZ7abssortS1_jE3$_0EEvT_S3_T0_",
                  "void std::__1::sort<float*, abssort(float*, unsigned int)::$_0>(float*, float*, abssort(float*, unsigned int)::$_0)",
                  "sort<float*, abssort(float*, unsigned int)::$_0>"),

      Expectation("_ZNSt3__16__sortIRZ7abssortPfjE3$_0S1_EEvT0_S4_T_",
                  "void std::__1::__sort<abssort(float*, unsigned int)::$_0&, float*>(float*, float*, abssort(float*, unsigned int)::$_0&)",
                  "__sort<abssort(float*, unsigned int)::$_0&, float*>"),

      Expectation("_ZNSt3__1L4swapIfEENS_9enable_ifIXaasr21is_move_constructibleIT_EE5valuesr18is_move_assignableIS2_EE5valueEvE4typeERS2_S5_",
                  "std::__1::enable_if<(is_move_constructible<float>::value) && (is_move_assignable<float>::value), void>::type std::__1::swap<float>(float&, float&)",
                  "swap<float>"),

      Expectation("_ZZ7abssortPfjENK3$_0clEff",
                  "abssort(float*, unsigned int)::$_0::operator()(float, float) const",
                  "operator()"),

      Expectation("_ZNSt3__1L4moveIRfEEONS_16remove_referenceIT_E4typeEOS3_",
                  "std::__1::remove_reference<float&>::type&& std::__1::move<float&>(float&&&)",
                  "move<float&>"),

      Expectation("_ZNSt3__1neIPiEEbRKNS_11__wrap_iterIT_EES6_",
                  "bool std::__1::operator!=<int*>(std::__1::__wrap_iter<int*> const&, std::__1::__wrap_iter<int*> const&)",
                  "operator!=<int*>"),

      Expectation("_Z7memoizeIXadL_Z3fibiEEEii",
                  "int memoize<&(fib(int))>(int)",
                  "memoize<&(fib(int))>"),

      Expectation("_ZNSt3__1L16forward_as_tupleIJEEENS_5tupleIJDpOT_EEES4_",
                  "std::__1::tuple<> std::__1::forward_as_tuple<>()",
                  "forward_as_tuple<>"),

      // clang-format on
  });

  for (Expectation &expectation : expectations) {
    std::string actualFullName = demangler.extractFullName(expectation.input);
    std::string actualName = demangler.extractName(expectation.input);

    ASSERT_EQ(actualFullName, expectation.fullName);
    ASSERT_EQ(actualName, expectation.name);
  }
}
