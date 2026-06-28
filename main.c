
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef _PI
#define _PI     3.1416f
#endif

#define SCALER_FACTOR       (0x1 << 7)

static FILE     *g_fout = 0;

static uint8_t   g_sin[10] =
{
    0, 22, 43, 64, 82, 98, 110, 120, 126, 128
};

typedef struct weight
{
    int16_t     weight_sin;
    int16_t     weight_cos;
} weight_t;

/* degree 0 ~ 359 */
int get_weitht(uint32_t degree, weight_t *pWeight)
{
    while( degree >= 360 )
        degree -= 360;

    int     idx = (degree * 13) >> 7;

    if( idx >= 27 )
    {
        idx -= 27;
        pWeight->weight_sin = -1 * g_sin[9 - idx];
        pWeight->weight_cos = g_sin[idx];
    }
    else if( idx >= 18 )
    {
        idx -= 18;
        pWeight->weight_sin = -1 * g_sin[idx];
        pWeight->weight_cos = -1 * g_sin[9 - idx];
    }
    else if( idx >= 9 )
    {
        idx -= 9;
        pWeight->weight_sin = g_sin[9 - idx];
        pWeight->weight_cos = -1 * g_sin[idx];
    }
    else
    {
        pWeight->weight_sin = g_sin[idx];
        pWeight->weight_cos = g_sin[9 - idx];
    }
    return 0;
}

int main()
{
    if( !(g_fout = fopen("out.csv", "w")) )
    {
        printf("open fail \n");
        while(1);
    }

    for (int deg = 0; deg <= 360; deg += 10)
    {
        weight_t        weight = {0};

        // 將角度轉換為弧度
        float   rad = deg * M_PI / 180.0;

        // 計算 sin 和 cos
        float sin_val = sin(rad);
        float cos_val = cos(rad);

        get_weitht(deg, &weight);

        fprintf(g_fout, "%d, %d, %d\n", deg, weight.weight_sin, weight.weight_cos);
//        fprintf(g_fout, "%3d, %d, %d\n", deg, (int)(sin_val * SCALER_FACTOR), (int)(cos_val * SCALER_FACTOR));
    }

    if( g_fout )    fclose(g_fout);

    return 0;
}
