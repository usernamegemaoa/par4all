int irbit2(iseed)
unsigned long *iseed;
{
        if (*iseed & 131072) {
                *iseed=((*iseed ^ 1 +2 +16) << 1) | 1;
                return 1;
        } else {
                *iseed <<= 1;
                return 0;
        }
}
