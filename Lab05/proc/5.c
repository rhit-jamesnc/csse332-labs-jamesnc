static char p1[4096] = {'a'};
static char p2[4096] = {'b'};
static char p3[1024] = {'c'};

int
start()
{
  return p1[0] + p2[0] + p3[0];
}
