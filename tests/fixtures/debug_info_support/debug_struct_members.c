struct Point {
  int x;
  int y;
};

struct PointPointer {
  int x;
  int y;
};

typedef struct Point PointTypedef;

int usePoint(struct Point p) {
  return p.x + p.y;
}

int usePointPointer(struct PointPointer *p) {
  return p->x + p->y;
}

int usePointTypedef(PointTypedef *p) {
  return p->x + p->y;
}
