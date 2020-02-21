struct Point {
  int x;
  int y;
  struct {
    int a, b;
  } something;
};

struct PointPointer {
  int x;
  int y;
  struct {
    int a, b;
  } something;
};

int usePoint(struct Point p) {
  return p.something.a + p.something.b + p.x + p.y;
}

int usePointPointer(struct PointPointer *p) {
  return p->something.a + p->something.b + p->x + p->y;
}
