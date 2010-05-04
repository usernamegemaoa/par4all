/* SIMD.c */
int PHI(int L, int X1, int X2);
void SIMD_PHIW(int R[4], int L[4], int X1[4], int X2[4]);
void SIMD_GTD(int R[4], int X1[4], int X2[4]);
void SIMD_LOAD_V4SI(int VEC[4], int BASE[4]);
void SIMD_LOAD_V4SF(float VEC[4], float BASE[4]);
void SIMD_LOAD_V2DF(double VEC[2], double BASE[2]);
void SIMD_LOAD_GENERIC_V2DF(double VEC[2], double X0, double X1);
void SIMD_LOAD_GENERIC_V4SI(int VEC[4], int X0, int X1, int X2, int X3);
void SIMD_LOAD_GENERIC_V4SF(float VEC[4], float X0, float X1, float X2, float X3);
void SIMD_LOAD_CONSTANT_V4SF(float VEC[4], float X0, float X1, float X2, float X3);
void SIMD_LOAD_CONSTANT_V4SI(int VEC[4], int X0, int X1, int X2, int X3);
void SIMD_SAVE_V4SI(int VEC[4], int BASE[4]);
void SIMD_SAVE_V4SF(float VEC[4], float BASE[4]);
void SIMD_SAVE_V2DF(double VEC[2], double BASE[2]);
void SIMD_MASKED_SAVE_V4SF(float VEC[4], float BASE[3]);
void SIMD_SAVE_GENERIC_V2DF(double VEC[2], double X1[1], double X2[1]);
void SIMD_SAVE_GENERIC_V4SI(int VEC[4], int X1[1], int X2[1], int X3[1], int X4[1]);
void SIMD_SAVE_GENERIC_V4SF(float VEC[4], float X1[1], float X2[1], float X3[1], float X4[1]);
void SIMD_GTPS(int DEST[4], float SRC1[4], float SRC2[4]);
void SIMD_GTPD(int DEST[2], double SRC1[2], double SRC2[2]);
void SIMD_PHIPS(float DEST[4], int COND[4], float SRC1[4], float SRC2[4]);
void SIMD_ADDPS(float DEST[4], float SRC1[4], float SRC2[4]);
void SIMD_SUBPS(float DEST[4], float SRC1[4], float SRC2[4]);
void SIMD_UMINPS(float DEST[4], float SRC1[4]);
void SIMD_MULPS(float DEST[4], float SRC1[4], float SRC2[4]);
void SIMD_DIVPD(double DEST[2], double SRC1[2], double SRC2[2]);
void SIMD_MULPD(double DEST[2], double SRC1[2], double SRC2[2]);
void SIMD_ADDPD(double DEST[2], double SRC1[2], double SRC2[2]);
void SIMD_SUBPD(double DEST[2], double SRC1[2], double SRC2[2]);
void SIMD_DIVPS(float DEST[4], float SRC1[4], float SRC2[4]);
void SIMD_MAXPS(float DEST[4], float SRC1[4], float SRC2[4]);
void SIMD_LOAD_V2SI_TO_V2SF(int VEC[2], float TO[2]);
void SIMD_SAVE_V2SI_TO_V2SF(int TO[2], float VEC[2]);
void SIMD_LOAD_CONSTANT_V2SF(float VEC[2], float HIGH, float LOW);
void SIMD_LOAD_CONSTANT_V2SI(int VEC[2], int HIGH, int LOW);
void SIMD_LOAD_V2SI(int VEC[2], int BASE[2]);
void SIMD_LOAD_GENERIC_V2SI(int VEC[2], int X1, int X2);
void SIMD_SAVE_V2SI(int VEC[2], int BASE[2]);
void SIMD_SAVE_GENERIC_V2SI(int VEC[2], int X1[1], int X2[1]);
void SIMD_SAVE_V2DI(int VEC[2], int BASE[2]);
void SIMD_ADDW(short DEST[8], short SRC1[8], short SRC2[8]);
void SIMD_SUBW(short DEST[8], short SRC1[8], short SRC2[8]);
void SIMD_MULW(short DEST[8], short SRC1[8], short SRC2[8]);
void SIMD_DIVW(short DEST[2], short SRC1[2], short SRC2[2]);
void SIMD_LOAD_GENERIC_V8HI(short VEC[8], short BASE0, short BASE1, short BASE2, short BASE3, short BASE4, short BASE5, short BASE6, short BASE7);
void SIMD_LOAD_V8HI(short VEC[8], short BASE[8]);
void SIMD_LOAD_V4QI_TO_V4HI(short VEC[4], char BASE[4]);
void SIMD_SAVE_V8HI(short VEC[8], short BASE[8]);
void SIMD_PHID(int DEST[4], int COND[4], int SRC1[4], int SRC2[4]);
void SIMD_ADDD(int DEST[4], int SRC1[4], int SRC2[4]);
void SIMD_SUBD(int DEST[4], int SRC1[4], int SRC2[4]);
void SIMD_MULD(int DEST[4], int SRC1[4], int SRC2[4]);
void SIMD_DIVD(int DEST[4], int SRC1[4], int SRC2[4]);
void SIMD_LOAD_CONSTANT_V8QI(char VEC[8], int HIGH, int LOW);
void SIMD_LOAD_V8QI(char VEC[8], char BASE[8]);
void SIMD_LOAD_GENERIC_V8QI(char VEC[8], char X1, char X2, char X3, char X4, char X5, char X6, char X7, char X8);
void SIMD_SAVE_V8QI(char VEC[8], char BASE[8]);
void SIMD_SAVE_GENERIC_V8QI(char VEC[8], char *X0, char X1[1], char X2[1], char X3[1], char X4[1], char X5[1], char X6[1], char X7[1]);
void SIMD_ADDB(char DEST[8], char SRC1[8], char SRC2[8]);
void SIMD_SUBB(char DEST[8], char SRC1[8], char SRC2[8]);
void SIMD_MULB(char DEST[8], char SRC1[8], char SRC2[8]);
void SIMD_MOVPS(float DEST[2], float SRC[2]);
void SIMD_MOVD(int DEST[2], int SRC[2]);
void SIMD_MOVW(short DEST[4], short SRC[4]);
void SIMD_MOVB(char DEST[8], char SRC[8]);
void SIMD_OPPPS(float DEST[2], float SRC[2]);
void SIMD_OPPD(int DEST[2], int SRC[2]);
void SIMD_OPPW(short DEST[4], short SRC[4]);
void SIMD_OPPB(char DEST[8], char SRC[8]);
void SIMD_SETPS(float DEST[2], float SRC[2]);
void SIMD_SETD(int DEST[2], int SRC[2]);
void SIMD_SETW(short DEST[4], short SRC[4]);
void SIMD_SETB(char DEST[8], char SRC[8]);
void SIMD_LOAD_CONSTANT_V2DF(double vec[2], double v0, double v1);
