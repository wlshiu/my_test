
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include "cordic-32bit.h"
//#include "cordic-16bit.h"

#define Q15(fval)                   ((int32_t)((fval) * 0x00008000L))
#define rad2degree(__rad__)         (int)(((__rad__) * 180)/3.141592f)
#define degree2rad(__degree__)      (((__degree__) * 3.141592f) / 180)

//#define M_PI 3.1415926535897932384626
#define K1 0.6072529350088812561694

uint8_t _clz(uint32_t x)
{
    uint8_t     n = 0;

    if( x == 0 )    return 32;

    if( x <= 0x0000FFFFul ) { n += 16; x <<= 16; }
    if( x <= 0x00FFFFFFul ) { n += 8; x <<= 8; }
    if( x <= 0x0FFFFFFFul ) { n += 4; x <<= 4; }
    if( x <= 0x3FFFFFFFul ) { n += 2; x <<= 2; }
    if( x <= 0x7FFFFFFFul ) { n += 1; x <<= 1; }
    return n;
}

/* ref. https://www.dcs.gla.ac.uk/~jhw/cordic/ */

int gen_cordic_table(void)
{
#define CONFIG_BIT_WIDTH        16 //32 // 16
    int i;
    int bits = CONFIG_BIT_WIDTH; // number of bits
//    int mul = (1 << (bits - 2));
    int mul = (1 << (bits - 1));

    int n = bits; // number of elements.
    int c;

    printf("/**\n");
    printf(" *  CORDIC in %d bit signed fixed point math\n", bits);
    printf(" *  Function is valid for arguments in range '-pi/2 < theta < pi/2'\n");
    printf(" *  for values 'pi/2 ~ pi': value = half_pi - (theta-half_pi) and similarly for values '-pi ~ pi/2'\n");
    printf(" *\n");
    printf(" *    1.0 = %d\n", mul);
    printf(" *    1/k = 0.607253\n");
    printf(" *     pi = 3.141592\n");
    printf(" */\n\n");

    printf("#include <stdint.h>\n");
    printf("#include <stdbool.h>\n\n");
    printf("#define CORDIC_1K           0x%08X\n", (int)(mul * K1));
    printf("#define CORDIC_Q%d_HALF_PI  0x%08X\n", 31 - _clz(mul), (int)(mul * (M_PI / 2)));
    printf("#define CORDIC_Q%d_MUL      %d\n", 31 - _clz(mul), (int)mul);
    printf("#define CORDIC_NTAB         %d\n\n\n", n);

    printf("int16_t     g_cordic_ctab [] = {\n    ");
    for(i = 0; i < n; i++)
    {
        c = (atan(pow(2, -i)) * mul);
        printf("0x%04X, ", c);
        if( i && (i & 0x7) == 0 )
            printf("\n    ");
    }
    printf("\n};\n\n");

    //Print the cordic function
    printf("void cordic_sincos(int32_t q%d_theta, int32_t *pSin_q%d_val, int32_t *pCos_q%d_val, int8_t n)\n{\n", 31 - _clz(mul), 31 - _clz(mul), 31 - _clz(mul));
    printf("    int32_t d, tx, ty, tz;\n");
    printf("    int32_t x = CORDIC_1K, y = 0;\n\n");
    printf("    n = (n > CORDIC_NTAB) ? CORDIC_NTAB : n;\n");
    printf("    for(int k = 0; k < n; ++k)\n");
    printf("    {\n");
    printf("        d = q%d_theta >> %d;\n", 31 - _clz(mul), (bits - 1));
    printf("        //get sign. for other architectures, you might want to use the more portable version\n");
    printf("        d = (q%d_theta >= 0) ? 0 : -1;\n", 31 - _clz(mul));
    printf("        tx = x - (((y >> k) ^ d) - d);\n");
    printf("        ty = y + (((x >> k) ^ d) - d);\n");
    printf("        tz = q%d_theta - ((g_cordic_ctab[k] ^ d) - d);\n", 31 - _clz(mul));
    printf("        x = tx;\n");
    printf("        y = ty;\n");
    printf("        q%d_theta = tz;\n ", 31 - _clz(mul));
    printf("    }\n\n");
    printf("    *pCos_q%d_val = x;\n", 31 - _clz(mul));
    printf("    *pSin_q%d_val = y;\n", 31 - _clz(mul));
    printf("    return;\n");
    printf("}\n");
    return 0;
}

//========================================================
/**
 *  CORDIC in 16 bit signed fixed point math
 *  Function is valid for arguments in range '-pi/2 < theta < pi/2'
 *  for values 'pi/2 ~ pi': value = half_pi - (theta-half_pi) and similarly for values '-pi ~ pi/2'
 *
 *    1.0 = 32768
 *    1/k = 0.607253
 *     pi = 3.141592
 */

#include <stdint.h>
#include <stdbool.h>

#define CORDIC_1K           0x00004DBA
#define CORDIC_Q15_HALF_PI  0x0000C90F
#define CORDIC_Q15_MUL      32768
#define CORDIC_NTAB         16


int16_t     g_cordic_ctab [] = {
    0x6487, 0x3B58, 0x1F5B, 0x0FEA, 0x07FD, 0x03FF, 0x01FF, 0x00FF, 0x007F,
    0x003F, 0x001F, 0x000F, 0x0007, 0x0003, 0x0001, 0x0000,
};

void cordic_sincos(int32_t q15_theta, int32_t *pSin_q15_val, int32_t *pCos_q15_val, int8_t n)
{
    int32_t d, tx, ty, tz;
    int32_t x = CORDIC_1K, y = 0;

    n = (n > CORDIC_NTAB) ? CORDIC_NTAB : n;
    for(int k = 0; k < n; ++k)
    {
        d = q15_theta >> 15;
        //get sign. for other architectures, you might want to use the more portable version
        d = (q15_theta >= 0) ? 0 : -1;
        tx = x - (((y >> k) ^ d) - d);
        ty = y + (((x >> k) ^ d) - d);
        tz = q15_theta - ((g_cordic_ctab[k] ^ d) - d);
        x = tx;
        y = ty;
        q15_theta = tz;
     }

    *pCos_q15_val = x;
    *pSin_q15_val = y;
    return;
}
//========================================================

#define QVALUE_MUL      CORDIC_Q15_MUL

//Print out sin(x) vs fp CORDIC sin(x)
int main(int argc, char **argv)
{
    FILE    *fout = 0;
    float   target_err_rate = 0.1f;
    float   phase;
    int sin_val, cos_val;

//    gen_cordic_table();

    fout = fopen("sin_cos.csv", "w");
    fprintf(fout, "ideal_sin, sim_sin, ideal_cos, sim_cos\n");

    for(int degree = 0; degree <= 360; degree++)
    {
        float   sim_sin = 0.0f;
        float   ideal_sin = 0.0f;
        float   ideal_cos = 0.0f;
        float   sim_cos = 0.0f;
        float   err_rate = 0.0f;

        int     sign_sin = 1;
        int     sign_cos = 1;

        if( degree >= 0 && degree <= 90 )
        {
            phase = degree * M_PI / 180;
        }
        else if( degree > 90 && degree <= 180 )
        {
            #if 1
            sign_cos = -1;
            phase = (180 - degree) * M_PI / 180;
            #else
            phase = degree * M_PI / 180;
            phase = M_PI - phase;
            #endif // 0
        }
        else if( degree > 180 && degree <= 270 )
        {
            sign_sin = -1;
            sign_cos = -1;
            phase = (degree - 180) * M_PI / 180;
        }
        else if( degree > 270 && degree <= 360 )
        {
            phase = (degree - 360) * M_PI / 180;
        }

        //use 32 iterations
        cordic_sincos((phase * QVALUE_MUL), &sin_val, &cos_val, 14);

        if( degree == 0 || degree == 180 || degree == 360 )
            sin_val = 0;

        if( degree == 90 || degree == 270 )
            cos_val = 0;

        //these values should be nearly equal
        #if 0
        sim_sin = (float)sin_val / QVALUE_MUL;
        sim_cos = (float)cos_val / QVALUE_MUL;
        ideal_sin = sin(phase);
        ideal_cos = cos(phase);
        #else
        sim_sin = sign_sin * (float)sin_val;
        sim_cos = sign_cos * (float)cos_val;
        ideal_sin = sin(degree2rad(degree)) * QVALUE_MUL;
        ideal_cos = cos(degree2rad(degree)) * QVALUE_MUL;
        #endif


#if 1
        err_rate = fabs(sim_sin - ideal_sin) * 100 / fabs(ideal_sin);
        printf("%5d degree sin: %5.6f : %5.6f\t(rate= %2.6f %%)\n",
               degree, sim_sin, ideal_sin, fabs(err_rate) > target_err_rate ? fabs(err_rate) : 0);

        err_rate = fabs(sim_cos - ideal_cos) * 100 / fabs(ideal_cos);
        printf("%5d degree cos: %5.6f : %5.6f\t(rate= %2.6f %%)\n",
               degree, sim_cos, ideal_cos, fabs(err_rate) > target_err_rate ? fabs(err_rate) : 0);
#else
        fprintf(fout, "%f, %f, %f, %f\n",
                ideal_sin, sim_sin, ideal_cos, sim_cos);
#endif
    }

    if( fout )  fclose(fout);

    return 0;
}
