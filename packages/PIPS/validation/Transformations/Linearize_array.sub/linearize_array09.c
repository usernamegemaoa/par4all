void p4a_kernel_main(int I_1,  int I_2,  int (*array0)[I_1-I_2+1],  int i)
{
   // Loop nest P4A end
   if (i<=I_1-I_2)
      (*array0)[I_2+i-I_2] = I_2+i;
}

