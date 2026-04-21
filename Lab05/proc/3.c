int global = 100;

static int
add(int x, int y)
{
  return x + y + global;
}

int
start()
{
  int x = 2, y = 3;

  return add(x, y);
}
