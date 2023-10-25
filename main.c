#include <stdio.h>
#include <stdlib.h>


static float    g_sqrt[] =
{
    0,
    1,
    1.41421,
    1.73205,
    2,
    2.23607,
    2.44949,
    2.64575,
    2.82843,
    3,
    3.16228,
    3.31662,
    3.4641,
    3.60555,
    3.74166,
    3.87298,
    4,
};


/**
 *  @brief the reciprocal of square root
 *              Q_rsqrt(fvalue) = 1 / sqrt(fvalue)
 *
 *  @param [in] fvalue      the float value
 *  @return
 *      the reciprocal of square root of fvalue
 */
float fast_rsqrt(float fvalue)
{
    long i;
    float x2, y;

    x2 = fvalue * 0.5f;
    y = fvalue;

    i = *(long*)&y;                 // evil floating
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (x2 * y * y)); // 1-st Newton-Raphson Iterative Method

    #if 1
    y = y * (1.5f - (x2 * y * y)); // 2-nd Newton-Raphson Iterative Method for Accuracy
    #endif

    return y;
}

int main()
{
    for(float value = 1.0f; value <= 16.0; value++)
    {
        float   v = g_sqrt[(int)value];

        printf("%d => sqrt= %f, rsqrt= %f, fast_rsqrt= %f\n",
               (int)value,
               v, 1/v,
               fast_rsqrt(value));
    }

    system("pause");
    return 0;
}
