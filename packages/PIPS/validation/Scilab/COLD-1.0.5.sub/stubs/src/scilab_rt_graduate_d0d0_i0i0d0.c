
#include <stdio.h>

#include <stdlib.h>

void scilab_rt_graduate_d0d0_i0i0d0(double scalarin0, 
    double scalarin1,
     int* scalarout0, 
    int* scalarout1, 
    double* scalarout2)
{
  printf("%f", scalarin0);

  printf("%f", scalarin1);

  *scalarout0 = rand();

  *scalarout1 = rand();

  *scalarout2 = rand();

}
