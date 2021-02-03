extern void *malloc(unsigned long);
extern void free(void *);

int main() {
  void *buf1 = malloc(42);
  void *buf2 = malloc(42);
  free(buf1);
  free(buf2);
  return 0;
}
