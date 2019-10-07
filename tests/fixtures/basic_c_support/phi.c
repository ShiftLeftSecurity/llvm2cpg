int foo(void);
int bar(void);

int foobar(char c) {
  return c ? foo() : bar();
}