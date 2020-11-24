// Point
typedef struct {
  int x;
  int y;
} PRIM_POINT;

// Line
typedef struct {
  PRIM_POINT p1;
  PRIM_POINT p2;
} PRIM_LINE;

// Boxes
typedef struct {
  int x;
  int y;
  int width;
  int height;

  PRIM_LINE sides[4];
  PRIM_POINT points[4];
} PRIM_BOX;
