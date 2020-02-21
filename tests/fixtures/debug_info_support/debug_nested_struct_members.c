struct Name {
  int length;
  char *buffer;
};

struct Hobby {
  int length;
  char *hobbyName;
};

struct Person {
  int age;
  struct Name name;
  struct Hobby *hobbies;
};

int foo(struct Person p) {
  return p.age;
}