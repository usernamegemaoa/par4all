



int main() {
  int a[10][10];
  int *p1,
      *p2;

  ((int (*)[10])p1)[10][10] = 0;
  //p2[10][10] = 0;

  ((int *)a)[12] = 0;
}
