
struct test {
  int i;
  int j;
};

struct test foo (int r) {
  struct test result;
  result.i = 0;
  result.j = r;
  return result;
}


int main () {
  struct test i;
  i = foo (7);
  return i.i;
}
