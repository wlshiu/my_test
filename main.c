#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include "cordic-32bit.h"
#include "cordic-16bit.h"

#define Q15(fval)                   ((int32_t)((fval) * 0x00008000L))
#define rad2degree(__rad__)         (int)(((__rad__) * 180)/3.141592f)

//#define M_PI 3.1415926535897932384626
#define K1 0.6072529350088812561694

/* ref. https://www.dcs.gla.ac.uk/~jhw/cordic/ */

int gen_cordic_table(void)
{
#define CONFIG_BIT_WIDTH        16 //32 // 16
    int i;
    int bits = CONFIG_BIT_WIDTH; // number of bits
    int mul = (1 << (bits - 2));

    int n = bits; // number of elements.
    int c;

    printf("/**\n");
    printf(" *  CORDIC in %d bit signed fixed point math\n", bits);
    printf(" *  Function is valid for arguments in range [-pi/2, pi/2]\n");
    printf(" *  for values [pi/2, pi]: value = half_pi - (theta-half_pi) and similarly for values [-pi, pi/2]\n");
    printf(" *\n");
    printf(" *    1.0 = %d\n", mul);
    printf(" *    1/k = 0.607253\n");
    printf(" *     pi = 3.141592\n");
    printf("*/\n\n");

    printf("#include <stdint.h>\n");
    printf("#include <stdbool.h>\n\n");
    printf("#define CORDIC_1K       0x%08X\n", (int)(mul * K1));
    printf("#define HALF_PI         0x%08X\n", (int)(mul * (M_PI / 2)));
    printf("#define QVALUE_MUL      %d\n", (int)mul);
    printf("#define CORDIC_NTAB     %d\n\n\n", n);

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
    printf("void cordic_sincos(int theta, int32_t *pSin_val, int32_t *pCos_val, int8_t n)\n{\n");
    printf("    int d, tx, ty, tz;\n");
    printf("    int32_t x = CORDIC_1K, y = 0, z = theta;\n\n");
    printf("    n = (n > CORDIC_NTAB) ? CORDIC_NTAB : n;\n");
    printf("    for(int k = 0; k < n; ++k)\n");
    printf("    {\n");
    printf("        d = z >> %d;\n", (bits - 1));
    printf("        //get sign. for other architectures, you might want to use the more portable version\n");
    printf("        d = (z >= 0) ? 0 : -1;\n");
    printf("        tx = x - (((y >> k) ^ d) - d);\n");
    printf("        ty = y + (((x >> k) ^ d) - d);\n");
    printf("        tz = z - ((g_cordic_ctab[k] ^ d) - d);\n");
    printf("        x = tx;\n");
    printf("        y = ty;\n");
    printf("        z = tz;\n ");
    printf("    }\n\n");
    printf("    *pCos_val = x;\n");
    printf("    *pSin_val = y;\n");
    printf("    return;\n");
    printf("}\n");
    return 0;
}

//========================================================
#include <stdint.h>
#include <stdbool.h>

#define CORDIC_1K       0x000026DD
#define HALF_PI         0x00006487
#define QVALUE_MUL      16384
#define CORDIC_NTAB     16


int16_t     g_cordic_ctab [] = {
    0x3243, 0x1DAC, 0x0FAD, 0x07F5, 0x03FE, 0x01FF, 0x00FF, 0x007F, 0x003F,
    0x001F, 0x000F, 0x0007, 0x0003, 0x0001, 0x0000, 0x0000,
};

void cordic_sincos(int theta, int32_t *pSin_val, int32_t *pCos_val, int8_t n)
{
    int d, tx, ty, tz;
    int32_t x = CORDIC_1K, y = 0, z = theta;

    n = (n > CORDIC_NTAB) ? CORDIC_NTAB : n;
    for(int k = 0; k < n; ++k)
    {
        d = z >> 15;
        //get sign. for other architectures, you might want to use the more portable version
        d = (z >= 0) ? 0 : -1;
        tx = x - (((y >> k) ^ d) - d);
        ty = y + (((x >> k) ^ d) - d);
        tz = z - ((g_cordic_ctab[k] ^ d) - d);
        x = tx;
        y = ty;
        z = tz;
     }

    *pCos_val = x;
    *pSin_val = y;
    return;
}
//========================================================


//Print out sin(x) vs fp CORDIC sin(x)
int main(int argc, char **argv)
{
    FILE    *fout = 0;
    double p;
    int sin_val, cos_val;
    int i;

//    gen_cordic_table();

    fout = fopen("sin_cos.csv", "w");
    fprintf(fout, "ideal_sin, sim_sin, ideal_cos, sim_cos\n");

    for(i = -90; i <= 90; i++)
    {
        float   sim_sin = 0.0f;
        float   ideal_sin = 0.0f;
        float   ideal_cos = 0.0f;
        float   sim_cos = 0.0f;
        float   err_rate = 0.0f;

        #if 0
        p = (i / 50.0) * M_PI / 2;
        #else
        p = i * M_PI / 180;
        #endif // 0

        //use 32 iterations
//        cordic((p * QVALUE_MUL), &s, &c, 14);
        cordic_sincos((p * QVALUE_MUL), &sin_val, &cos_val, 14);

        //these values should be nearly equal
        #if 0
        sim_sin = (float)sin_val / QVALUE_MUL;
        sim_cos = (float)cos_val / QVALUE_MUL;
        ideal_sin = sin(p);
        ideal_cos = cos(p);
        #else
        sim_sin = (float)sin_val;
        sim_cos = (float)cos_val;
        ideal_sin = sin(p) * QVALUE_MUL;
        ideal_cos = cos(p) * QVALUE_MUL;
        #endif


#if 0
        err_rate = (sim_sin > ideal_sin ) ? sim_sin - ideal_sin : ideal_sin - sim_sin;
        err_rate = err_rate * 100 / (ideal_sin < 0 ? -ideal_sin : ideal_sin);
        err_rate = (err_rate < 0) ? -1.0*err_rate : err_rate;
        printf("%5d degree sin: %3.6f : %3.6f (rate= %2.6f %%)\n", i, sim_sin, ideal_sin, err_rate);

        err_rate = (sim_cos > ideal_cos ) ? sim_cos - ideal_cos : ideal_cos - sim_cos;
        err_rate = err_rate * 100 / (ideal_cos < 0 ? -ideal_cos : ideal_cos);
        err_rate = (err_rate < 0) ? -1.0*err_rate : err_rate;
        printf("%5d degree cos: %3.6f : %3.6f (rate= %2.6f %%)\n", i, sim_cos, ideal_cos, err_rate);
#endif
        fprintf(fout, "%f, %f, %f, %f\n",
                ideal_sin, sim_sin, ideal_cos, sim_cos);
    }

    if( fout )  fclose(fout);

    return 0;
}
