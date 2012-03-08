/*
 * (c) HPC Project - 2010-2011 - All rights reserved
 *
 */

#include "scilab_rt.h"


int __lv0;
int __lv1;
int __lv2;
int __lv3;

/*----------------------------------------------------*/


/*----------------------------------------------------*/

int main(int argc, char* argv[])
{
  scilab_rt_init(argc, argv, COLD_MODE_STANDALONE);

  /* testing datenum */
  /* a = datenum() it depends on the current time, so it  shouldn't be saved as ref */
  int _u_b[3][3];
  _u_b[0][0]=2004;
  _u_b[0][1]=2;
  _u_b[0][2]=29;
  _u_b[1][0]=1873;
  _u_b[1][1]=5;
  _u_b[1][2]=5;
  _u_b[2][0]=1944;
  _u_b[2][1]=26;
  _u_b[2][2]=6;
  int _u_c[3][6];
  _u_c[0][0]=2004;
  _u_c[0][1]=4;
  _u_c[0][2]=15;
  _u_c[0][3]=14;
  _u_c[0][4]=30;
  _u_c[0][5]=14;
  _u_c[1][0]=1873;
  _u_c[1][1]=5;
  _u_c[1][2]=5;
  _u_c[1][3]=23;
  _u_c[1][4]=59;
  _u_c[1][5]=59;
  _u_c[2][0]=1944;
  _u_c[2][1]=26;
  _u_c[2][2]=6;
  _u_c[2][3]=18;
  _u_c[2][4]=54;
  _u_c[2][5]=32;
  int _u_y[2][3];
  _u_y[0][0]=1945;
  _u_y[0][1]=2004;
  _u_y[0][2]=1990;
  _u_y[1][0]=1945;
  _u_y[1][1]=2004;
  _u_y[1][2]=1990;
  int _u_m[2][3];
  _u_m[0][0]=3;
  _u_m[0][1]=2;
  _u_m[0][2]=7;
  _u_m[1][0]=3;
  _u_m[1][1]=2;
  _u_m[1][2]=7;
  int _u_d[2][3];
  _u_d[0][0]=30;
  _u_d[0][1]=29;
  _u_d[0][2]=15;
  _u_d[1][0]=30;
  _u_d[1][1]=29;
  _u_d[1][2]=15;
  int _u_h[2][3];
  _u_h[0][0]=3;
  _u_h[0][1]=12;
  _u_h[0][2]=17;
  _u_h[1][0]=3;
  _u_h[1][1]=12;
  _u_h[1][2]=17;
  int _u_s[2][3];
  _u_s[0][0]=30;
  _u_s[0][1]=25;
  _u_s[0][2]=10;
  _u_s[1][0]=30;
  _u_s[1][1]=25;
  _u_s[1][2]=10;
  double _u_f[1][3];
  scilab_rt_datenum_i2_d2(3,3,_u_b,1,3,_u_f);
  scilab_rt_display_s0d2_("f",1,3,_u_f);
  double _u_g[1][3];
  scilab_rt_datenum_i2_d2(3,6,_u_c,1,3,_u_g);
  scilab_rt_display_s0d2_("g",1,3,_u_g);
  double _u_i[2][3];
  scilab_rt_datenum_i2i2i2i2i2i2_d2(2,3,_u_y,2,3,_u_m,2,3,_u_d,2,3,_u_h,2,3,_u_m,2,3,_u_s,2,3,_u_i);
  scilab_rt_display_s0d2_("i",2,3,_u_i);
  int _u_j = scilab_rt_datenum_i0i0i0_(2004,4,4);
  scilab_rt_display_s0i0_("j",_u_j);
  double _u_k = scilab_rt_datenum_i0i0i0i0i0i0_(2004,4,4,20,43,50);
  scilab_rt_display_s0d0_("k",_u_k);
  double _u_z = scilab_rt_datenum_d0d0d0_(2004.0,4.0,4);
  scilab_rt_display_s0d0_("z",_u_z);
  double _u_a = scilab_rt_datenum_d0d0d0d0d0d0_(2004.0,4.0,4.0,20.0,43.0,50.0);
  scilab_rt_display_s0d0_("a",_u_a);
  int _u_y2[3][1];
  _u_y2[0][0]=1945;
  _u_y2[1][0]=2004;
  _u_y2[2][0]=1990;
  int _u_m2[3][1];
  _u_m2[0][0]=3;
  _u_m2[1][0]=2;
  _u_m2[2][0]=7;
  int _u_d2[3][1];
  _u_d2[0][0]=30;
  _u_d2[1][0]=29;
  _u_d2[2][0]=15;
  int _u_h2[3][1];
  _u_h2[0][0]=3;
  _u_h2[1][0]=12;
  _u_h2[2][0]=17;
  int _u_s2[3][1];
  _u_s2[0][0]=30;
  _u_s2[1][0]=25;
  _u_s2[2][0]=10;
  double _u_l2[3][1];
  scilab_rt_datenum_i2i2i2i2i2i2_d2(3,1,_u_y2,3,1,_u_m2,3,1,_u_d2,3,1,_u_h2,3,1,_u_m2,3,1,_u_s2,3,1,_u_l2);
  scilab_rt_display_s0d2_("l2",3,1,_u_l2);

  scilab_rt_terminate();
}
