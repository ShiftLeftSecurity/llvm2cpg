int foo(int param) {
  int local;
  if (param % 2 == 0) {
    local = param * 42;
  } else {
    local = 0;
  }
  return local;
}