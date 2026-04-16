int A[10]  = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int global = 100;

static int
add(int x, int y)
{
  A[4] = x;
  A[5] = y;
  return x + y + global;
}

int
start()
{
  int x = 2, y = 3;

  return add(x, y);
}
