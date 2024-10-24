
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern double __ieee754_sqrt(double x);

int sqrt_1(int num)
{
    float   temp, sqrt_val;
    int     number;

    number = num;
    sqrt_val = number / 2;
    temp = 0;

    // It will stop only when temp is the square of our number
    while( sqrt_val != temp )
    {
        // setting sqrt_val as temp to save the value for modifications
        temp = sqrt_val;

        // main logic for square root of any number (Non Negative)
        sqrt_val = (number/temp + temp) / 2;
    }

//    printf("Square root of '%d' is '%f'", number, sqrt_val);

    return sqrt_val;
}

float sqrt_2(float num)
{
    float root_val, precision = 0.00001;

    for(root_val = 1; (root_val*root_val) <= num; ++root_val);           //Integer part

    for(--root_val; (root_val*root_val) < num; root_val += precision);  //Fractional part

    return root_val;
}

float sqrt_3(int val)
{
    int left = 0, right = val, mid;
    float sqrt_val = 0.0f;
    float inc_val = 0.1;

    while( left <= right )  // finding the integral part
    {
        mid = (left + right) / 2;
        if( (mid * mid) == val )
        {
            sqrt_val = mid;
            break;
        }

        if( (mid * mid) < val )
        {
            sqrt_val = left;
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    for(int i = 0; i < 3; i++) // finding the fractional part
    {
        while( (sqrt_val * sqrt_val) <= val )
        {
            sqrt_val += inc_val;
        }

        sqrt_val = sqrt_val - inc_val;
        inc_val = inc_val / 10;
    }
    return sqrt_val;
}


uint32_t sqrt_4(uint32_t x)
{
    uint32_t i = 0;

    if (x >= 65536)
    {
        if ((i + 32768) * (i + 32768) <= x) i += 32768;
        if ((i + 16384) * (i + 16384) <= x) i += 16384;
        if ((i + 8192) * (i + 8192) <= x) i += 8192;
        if ((i + 4096) * (i + 4096) <= x) i += 4096;
        if ((i + 2048) * (i + 2048) <= x) i += 2048;
        if ((i + 1024) * (i + 1024) <= x) i += 1024;
        if ((i + 512) * (i + 512) <= x) i += 512;
        if ((i + 256) * (i + 256) <= x) i += 256;
    }
    if ((i + 128) * (i + 128) <= x) i += 128;
    if ((i + 64) * (i + 64) <= x) i += 64;
    if ((i + 32) * (i + 32) <= x) i += 32;
    if ((i + 16) * (i + 16) <= x) i += 16;
    if ((i + 8) * (i + 8) <= x) i += 8;
    if ((i + 4) * (i + 4) <= x) i += 4;
    if ((i + 2) * (i + 2) <= x) i += 2;
    if ((i + 1) * (i + 1) <= x) i += 1;
    return i;
}

int main()
{
    srand(clock());

    for(int i = 0; i < 10; i++)
    {
        uint32_t        value = rand();

        printf("====== round %d, value= %d\n", i, value);
        printf("c sqrt() = %f\n", (float)sqrt((double)value));
        printf("sqrt_1() = %f\n", (float)sqrt_1(value));
        printf("sqrt_2() = %f\n", (float)sqrt_2((float)value));
        printf("sqrt_3() = %f\n", (float)sqrt_3(value));
        printf("sqrt_4() = %f\n", (float)sqrt_4(value));
        printf("i3e_sqrt() = %f\n", (float)__ieee754_sqrt((double)value));

    }
    return 0;
}
