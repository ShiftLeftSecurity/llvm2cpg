struct Foo {
  int a;
  int b;
};

typedef void (*ecall_t)(struct Foo *);
extern void *ecall(void *);

void use() {
  struct Foo f;
  ((ecall_t)ecall)(&f);
}