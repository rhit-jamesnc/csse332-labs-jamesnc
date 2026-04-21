volatile int sink = 3;

static int
compute1(int a, int b, int c)
{
  int result = a;
  result     = result * 3 + b;
  result     = result ^ c;
  result     = result - (a << 2);
  result     = result + (b >> 1);
  result     = result | (c & 0xFF);
  result     = result * 7 - a;
  result     = result ^ (b + c);
  return result;
}

static int
compute2(int x, int y)
{
  int r = x + y;
  r     = r * 5;
  r     = r - x;
  r     = r ^ y;
  r     = r + (x << 3);
  r     = r - (y >> 2);
  r     = r | 0x1234;
  r     = r & 0xFFFF;
  r     = r * 11;
  r     = r + x - y;
  return r;
}

static int
compute3(int a, int b, int c, int d)
{
  int t1  = a + b;
  int t2  = c - d;
  int t3  = t1 * t2;
  int t4  = t3 ^ a;
  int t5  = t4 + b;
  int t6  = t5 - c;
  int t7  = t6 | d;
  int t8  = t7 & 0xFFFFFF;
  int t9  = t8 * 13;
  int t10 = t9 + t1;
  return t10 - t2;
}

static int
compute4(int n)
{
  int acc = 1;
  acc     = acc + n;
  acc     = acc * 2;
  acc     = acc + n + 1;
  acc     = acc * 3;
  acc     = acc + n + 2;
  acc     = acc * 4;
  acc     = acc + n + 3;
  acc     = acc * 5;
  acc     = acc + n + 4;
  acc     = acc ^ n;
  acc     = acc - n;
  acc     = acc | (n << 4);
  acc     = acc & 0xFFFFF;
  return acc;
}

static int
compute5(int a, int b)
{
  int x = a;
  int y = b;
  x     = x + y;
  y     = y - a;
  x     = x * 2;
  y     = y * 3;
  x     = x + y;
  y     = y - x;
  x     = x ^ y;
  y     = y | x;
  x     = x & 0xFFFF;
  y     = y & 0xFFFF;
  x     = x + 100;
  y     = y + 200;
  x     = x - 50;
  y     = y - 100;
  x     = x * y;
  return x + y;
}

static int
compute6(int p, int q, int r)
{
  int a = p + q + r;
  int b = p - q + r;
  int c = p + q - r;
  int d = p * q;
  int e = q * r;
  int f = p * r;
  int g = a + b + c;
  int h = d + e + f;
  int i = g ^ h;
  int j = i + a - b;
  int k = j * 7;
  int l = k - c + d;
  int m = l | e;
  int n = m & 0xFFFFFF;
  int o = n + f - g;
  return o + h;
}

static int
compute7(int val)
{
  int r = val;
  r     = (r << 1) + val;
  r     = (r << 2) - val;
  r     = (r >> 1) + val;
  r     = (r << 3) ^ val;
  r     = (r >> 2) | val;
  r     = (r << 4) & 0xFFFFF;
  r     = r + val * 2;
  r     = r - val * 3;
  r     = r + val * 5;
  r     = r ^ val * 7;
  r     = r | val;
  r     = r & 0xFFFFFF;
  return r;
}

static int
compute8(int a, int b, int c, int d, int e)
{
  int sum   = a + b + c + d + e;
  int prod  = 1;
  prod      = prod * (a + 1);
  prod      = prod * (b + 1);
  prod      = prod & 0xFFFF;
  prod      = prod * (c + 1);
  prod      = prod & 0xFFFF;
  prod      = prod * (d + 1);
  prod      = prod & 0xFFFF;
  int diff  = a - b + c - d + e;
  int xored = a ^ b ^ c ^ d ^ e;
  int ored  = a | b | c | d | e;
  int anded = (a & b) | (c & d) | (d & e);
  return sum + prod + diff + xored + ored + anded;
}

static int
compute9(int x)
{
  int a = x + 1;
  int b = x + 2;
  int c = x + 3;
  int d = x + 4;
  int e = x + 5;
  int f = a * b;
  int g = c * d;
  int h = e * a;
  int i = f + g + h;
  int j = i - a - b - c;
  int k = j ^ d ^ e;
  int l = k | f;
  int m = l & 0xFFFFFF;
  int n = m + g - h;
  int o = n * 3;
  int p = o + i - j;
  return p + k;
}

static int
compute10(int m, int n)
{
  int r1  = m + n;
  int r2  = m - n;
  int r3  = m * n;
  int r4  = r1 + r2;
  int r5  = r2 + r3;
  int r6  = r3 + r1;
  int r7  = r4 ^ r5;
  int r8  = r5 | r6;
  int r9  = r6 & r7;
  int r10 = r7 + r8 + r9;
  int r11 = r10 - r1;
  int r12 = r11 + r2;
  int r13 = r12 - r3;
  int r14 = r13 ^ r4;
  int r15 = r14 | r5;
  return r15 + r6;
}

static int
big_computation1(int seed)
{
  int v1  = compute1(seed, seed + 1, seed + 2);
  int v2  = compute2(v1, seed + 3);
  int v3  = compute3(v2, v1, seed, seed + 4);
  int v4  = compute4(v3);
  int v5  = compute5(v4, v3);
  int v6  = compute6(v5, v4, v3);
  int v7  = compute7(v6);
  int v8  = compute8(v7, v6, v5, v4, v3);
  int v9  = compute9(v8);
  int v10 = compute10(v9, v8);
  return v10;
}

static int
big_computation2(int seed)
{
  int a = seed * 3 + 7;
  int b = a ^ seed;
  int c = b + a - seed;
  int d = c * 5;
  int e = d - c + b;
  int f = e | a;
  int g = f & 0xFFFF;
  int h = g + seed;
  int i = h - a;
  int j = i ^ b;
  int k = j + c;
  int l = k - d;
  int m = l | e;
  int n = m & f;
  int o = n + g;
  int p = o - h;
  int q = p ^ i;
  int r = q + j;
  int s = r - k;
  int t = s | l;
  return t + m - n;
}

static int
big_computation3(int x, int y)
{
  int r = x + y;
  r     = r * 2 + x;
  r     = r * 3 + y;
  r     = r * 4 - x;
  r     = r * 5 - y;
  r     = r ^ x;
  r     = r | y;
  r     = r & 0xFFFFF;
  r     = r + x * 7;
  r     = r - y * 11;
  r     = r + x * 13;
  r     = r - y * 17;
  r     = r ^ (x + y);
  r     = r | (x - y);
  r     = r & 0xFFFFFF;
  r     = r + x + y;
  r     = r - x + y;
  r     = r + x - y;
  r     = r - x - y;
  return r;
}

static int
loop_computation1(int n)
{
  int sum = 0;
  int i   = 0;
  while(i < n && i < 100) {
    sum = sum + i;
    sum = sum ^ i;
    sum = sum + (i << 1);
    sum = sum - (i >> 1);
    sum = sum | i;
    sum = sum & 0xFFFFFF;
    i   = i + 1;
  }
  return sum;
}

static int
loop_computation2(int n)
{
  int prod = 1;
  int i    = 1;
  while(i <= n && i <= 20) {
    prod = prod * i;
    prod = prod & 0xFFFF;
    prod = prod + i;
    prod = prod ^ i;
    i    = i + 1;
  }
  return prod;
}

static int
array_simulation(int base)
{
  int a0 = base;
  int a1 = base + 1;
  int a2 = base + 2;
  int a3 = base + 3;
  int a4 = base + 4;
  int a5 = base + 5;
  int a6 = base + 6;
  int a7 = base + 7;
  int a8 = base + 8;
  int a9 = base + 9;

  int sum   = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
  int xored = a0 ^ a1 ^ a2 ^ a3 ^ a4 ^ a5 ^ a6 ^ a7 ^ a8 ^ a9;

  a0 = a0 * 2;
  a1 = a1 * 3;
  a2 = a2 * 4;
  a3 = a3 * 5;
  a4 = a4 * 6;
  a5 = a5 * 7;
  a6 = a6 * 8;
  a7 = a7 * 9;
  a8 = a8 * 10;
  a9 = a9 * 11;

  int sum2 = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;

  return sum + sum2 + xored;
}

static int
nested_computation(int x)
{
  int level1 = x + 10;
  int level2 = level1 * 2 + x;
  int level3 = level2 * 3 + level1;
  int level4 = level3 * 4 + level2;
  int level5 = level4 * 5 + level3;

  level1 = level1 ^ level5;
  level2 = level2 | level4;
  level3 = level3 & 0xFFFF;
  level4 = level4 + level1;
  level5 = level5 - level2;

  int r1 = level1 + level2;
  int r2 = level3 + level4;
  int r3 = level5 + r1;
  int r4 = r2 + r3;

  return r4 + level1 - level2 + level3 - level4 + level5;
}

static int
bitwise_heavy(int a, int b)
{
  int r = a;
  r     = r << 1;
  r     = r | b;
  r     = r >> 1;
  r     = r ^ a;
  r     = r << 2;
  r     = r & b;
  r     = r >> 2;
  r     = r | a;
  r     = r << 3;
  r     = r ^ b;
  r     = r >> 3;
  r     = r & a;
  r     = r << 4;
  r     = r | b;
  r     = r >> 4;
  r     = r ^ a;
  r     = r & 0xFFFFFF;
  r     = r + a + b;
  r     = r - a + b;
  r     = r + a - b;
  r     = r - a - b;
  r     = r ^ (a | b);
  r     = r | (a ^ b);
  r     = r & (a + b);
  return r;
}

static int
arithmetic_chain(int start_val)
{
  int v = start_val;
  v     = v + 1;
  v     = v * 2;
  v     = v + 3;
  v     = v * 4;
  v     = v + 5;
  v     = v * 6;
  v     = v + 7;
  v     = v * 8;
  v     = v & 0xFFFF;
  v     = v + 9;
  v     = v * 10;
  v     = v + 11;
  v     = v * 12;
  v     = v + 13;
  v     = v * 14;
  v     = v + 15;
  v     = v * 16;
  v     = v & 0xFFFF;
  v     = v - 1;
  v     = v + 2;
  v     = v - 3;
  v     = v + 4;
  v     = v - 5;
  v     = v + 6;
  v     = v - 7;
  v     = v + 8;
  v     = v ^ start_val;
  return v;
}

static int
mixed_ops(int a, int b, int c)
{
  int x = a + b * c;
  int y = a * b + c;
  int z = a * b * c;
  z     = z & 0xFFFF;
  int w = x + y + z;
  int p = x - y + z;
  int q = x + y - z;
  int r = x - y - z;
  int s = w ^ p;
  int t = q | r;
  int u = s & t;
  int v = u + w - p;
  int m = v * 3 + q;
  int n = m - r * 2;
  return n + s - t + u;
}

static int
conditional_like(int x, int y)
{
  int result = 0;
  int cond   = x - y;
  int mask1  = cond >> 31;
  int mask2  = ~mask1;

  int branch1 = x * 2 + y;
  int branch2 = x + y * 2;

  result = (branch1 & mask1) | (branch2 & mask2);

  cond  = x + y - 100;
  mask1 = cond >> 31;
  mask2 = ~mask1;

  int branch3 = result * 3;
  int branch4 = result * 5;

  result = (branch3 & mask1) | (branch4 & mask2);

  return result;
}

static int
extra_computation1(int n)
{
  int a = n * 17 + 23;
  int b = a ^ n;
  int c = b + a * 2;
  int d = c - b + n;
  int e = d | c;
  int f = e & 0xFFFFF;
  int g = f + d - c;
  int h = g ^ e;
  int i = h * 7;
  int j = i - g + f;
  return j + a - b + c;
}

static int
extra_computation2(int x, int y)
{
  int p1  = x + y;
  int p2  = x - y;
  int p3  = x * y;
  p3      = p3 & 0xFFFF;
  int p4  = p1 + p2 + p3;
  int p5  = p1 * p2;
  p5      = p5 & 0xFFFF;
  int p6  = p4 + p5;
  int p7  = p6 ^ p3;
  int p8  = p7 | p2;
  int p9  = p8 & p1;
  int p10 = p9 + p6 - p5;
  return p10;
}

static int
extra_computation3(int a, int b, int c)
{
  int r1  = a + b + c;
  int r2  = a * b;
  r2      = r2 & 0xFFFF;
  int r3  = b * c;
  r3      = r3 & 0xFFFF;
  int r4  = c * a;
  r4      = r4 & 0xFFFF;
  int r5  = r1 + r2 + r3 + r4;
  int r6  = r5 ^ r1;
  int r7  = r6 | r2;
  int r8  = r7 & r3;
  int r9  = r8 + r4;
  int r10 = r9 - r5 + r6;
  int r11 = r10 ^ r7;
  int r12 = r11 | r8;
  return r12 + r9 - r10;
}

int
start(void)
{
  int result = 0;

  /* Call all the computation functions to ensure they're included */
  result = result + compute1(1, 2, 3);
  result = result + compute2(result, 5);
  result = result + compute3(1, 2, 3, 4);
  result = result + compute4(result);
  result = result + compute5(result, 10);
  result = result + compute6(1, 2, 3);
  result = result + compute7(result);
  result = result + compute8(1, 2, 3, 4, 5);
  result = result + compute9(result);
  result = result + compute10(result, 20);

  result = result + big_computation1(result);
  result = result + big_computation2(result);
  result = result + big_computation3(result, 50);

  result = result + loop_computation1(50);
  result = result + loop_computation2(15);

  result = result + array_simulation(result);
  result = result + nested_computation(result);
  result = result + bitwise_heavy(result, 0x5555);
  result = result + arithmetic_chain(result);
  result = result + mixed_ops(result, 10, 20);
  result = result + conditional_like(result, 100);

  result = result + extra_computation1(result);
  result = result + extra_computation2(result, 30);
  result = result + extra_computation3(result, 40, 50);

  /* Additional inline operations to ensure we exceed page size */
  result = result * 2 + 1;
  result = result * 3 + 2;
  result = result * 4 + 3;
  result = result * 5 + 4;
  result = result & 0xFFFFFF;

  result = result ^ 0x12345;
  result = result | 0x67890;
  result = result & 0xFFFFF;
  result = result + 100;
  result = result - 50;

  /* Store to volatile to prevent optimization */
  sink = result;

  return result;
}
