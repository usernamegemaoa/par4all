//init a and first column of b
//init b and all cluster use the first column for that
//transform 2D array (b) into 1D array (c)

int main(int toto)
{
   // Generated by Pass VARIABLE_REPLICATION
   int __dpvar__i_0, __dpvar__i_1, __dpvar__i_2, __dpvar__i_3;
   // Generated by Pass VARIABLE_REPLICATION
   int __dpvar__a_0[4], __dpvar__a_1[4], __dpvar__a_2[4], __dpvar__a_3[4];
   // Generated by Pass VARIABLE_REPLICATION
   int __dpvar__b_0[4][5], __dpvar__b_1[4][5], __dpvar__b_2[4][5], __dpvar__b_3[4][5];
#pragma distributed on_cluster=0
   {
      for(__dpvar__i_0 = 0; __dpvar__i_0 <= 3; __dpvar__i_0 += 1) {
         __dpvar__a_0[__dpvar__i_0] = __dpvar__i_0;
         __dpvar__a_1[__dpvar__i_0] = __dpvar__a_0[__dpvar__i_0];
         __dpvar__a_2[__dpvar__i_0] = __dpvar__a_0[__dpvar__i_0];
         __dpvar__a_3[__dpvar__i_0] = __dpvar__a_0[__dpvar__i_0];
         __dpvar__b_0[__dpvar__i_0][0] = __dpvar__a_0[__dpvar__i_0];
         __dpvar__b_1[__dpvar__i_0][0] = __dpvar__b_0[__dpvar__i_0][0];
         __dpvar__b_2[__dpvar__i_0][0] = __dpvar__b_0[__dpvar__i_0][0];
         __dpvar__b_3[__dpvar__i_0][0] = __dpvar__b_0[__dpvar__i_0][0];
      }
      __dpvar__i_1 = __dpvar__i_0;
      __dpvar__i_2 = __dpvar__i_0;
      __dpvar__i_3 = __dpvar__i_0;
   }
#pragma distributed on_cluster=0
   {
      int j;
      for(j = 1; j <= 4; j += 1) {
         __dpvar__b_0[0][j] = __dpvar__b_0[0][0]+j*10;
	 __dpvar__b_1[0][j] = __dpvar__b_0[0][j];
         __dpvar__b_2[0][j] = __dpvar__b_0[0][j];
         __dpvar__b_3[0][j] = __dpvar__b_0[0][j];
      }
   }
#pragma distributed on_cluster=1
   {
      int j;
      for(j = 1; j <= 4; j += 1) {
         __dpvar__b_1[1][j] = __dpvar__b_1[1][0]+j*10;
         __dpvar__b_0[1][j] = __dpvar__b_1[1][j];
         __dpvar__b_2[1][j] = __dpvar__b_1[1][j];
         __dpvar__b_3[1][j] = __dpvar__b_1[1][j];
      }
   }
#pragma distributed on_cluster=2
   {
      int j;
      for(j = 1; j <= 4; j += 1) {
         __dpvar__b_2[2][j] = __dpvar__b_2[2][0]+j*10;
         __dpvar__b_0[2][j] = __dpvar__b_2[2][j];
         __dpvar__b_1[2][j] = __dpvar__b_2[2][j];
         __dpvar__b_3[2][j] = __dpvar__b_2[2][j];
      }
   }
#pragma distributed on_cluster=3
   {
      int j;
      for(j = 1; j <= 4; j += 1) {
         __dpvar__b_3[3][j] = __dpvar__b_3[3][0]+j*10;
         __dpvar__b_0[3][j] = __dpvar__b_3[3][j];
         __dpvar__b_1[3][j] = __dpvar__b_3[3][j];
         __dpvar__b_2[3][j] = __dpvar__b_3[3][j];
      }
   }
   // Generated by Pass VARIABLE_REPLICATION
   int __dpvar__c_0[20], __dpvar__c_1[20], __dpvar__c_2[20], __dpvar__c_3[20];
#pragma distributed on_cluster=0
   {
      int j, k;
      for(__dpvar__i_0 = 0; __dpvar__i_0 <= 3; __dpvar__i_0 += 1)
         for(j = 1; j <= 4; j += 1) {
            k = __dpvar__i_0*4+j;
            __dpvar__c_0[k] = __dpvar__b_0[__dpvar__i_0][j];
            __dpvar__c_1[k] = __dpvar__c_0[k];
            __dpvar__c_2[k] = __dpvar__c_0[k];
            __dpvar__c_3[k] = __dpvar__c_0[k];
         }
      __dpvar__i_1 = __dpvar__i_0;
      __dpvar__i_2 = __dpvar__i_0;
      __dpvar__i_3 = __dpvar__i_0;
   }

   return 0;
}
