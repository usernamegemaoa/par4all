// Bug: over simplification

#include <stdio.h>

void Run();
int main() { Run(); return 0; }
void Run()
{
  int _u_lx = 400;
  int _u_ly = 100;
  double _tmpxx0 = ((double)_u_lx / 5);
  double _u_obst_x = (_tmpxx0+1);
  double _u_x[400][100];

  {
    int lv1, lv2;
    for(lv1 = 0; lv1 <= 399; lv1 += 1)
      for(lv2 = 0; lv2 <= 99; lv2 += 1)
	_u_x[lv1][lv2] = 0.0;
  }
  double _tmpxx8[400][100];
  {
    int lv1, lv2;
    400==400&&100==100?(void) 0:__assert_fail("400 == 400 && 100 == 100", ".cold.tmp/cylinder.cold-pre-p4a.c", 118, (const char *) 0);
    for(lv1 = 0; lv1 <= 399; lv1 += 1)
      for(lv2 = 0; lv2 <= 99; lv2 += 1)
	_tmpxx8[lv1][lv2] = _u_x[lv1][lv2]-_u_obst_x;
  }

   {
    int lv1, lv2;
    for(lv1 = 0; lv1 <= 399; lv1 += 1)
      for(lv2 = 0; lv2 <= 99; lv2 += 1)
	printf("%f", _tmpxx8[lv1][lv2]);
  }

}
