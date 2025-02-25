#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include "cordic-32bit.h"
#include "cordic-16bit.h"


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

    printf("//Cordic in %d bit signed fixed point math\n", bits);
    printf("//Function is valid for arguments in range -pi/2 -- pi/2\n");
    printf("//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2\n");
    printf("//\n");
    printf("// 1.0 = %d\n", mul);
    printf("// 1/k = 0.6072529350088812561694\n");
    printf("// pi = 3.1415926535897932384626\n");

    printf("//Constants\n");
    printf("#define cordic_1K 0x%08X\n", (int)(mul * K1));
    printf("#define half_pi 0x%08X\n", (int)(mul * (M_PI / 2)));
    printf("#define MUL %f\n", (double)mul);
    printf("#define CORDIC_NTAB %d\n", n);

    printf("int cordic_ctab [] = {");
    for(i = 0; i < n; i++)
    {
        c = (atan(pow(2, -i)) * mul);
        printf("0x%08X, ", c);
    }
    printf("};\n\n");

    //Print the cordic function
    printf("void cordic(int theta, int *s, int *c, int n)\n{\n  int k, d, tx, ty, tz;\n");
    printf("  int x=cordic_1K,y=0,z=theta;\n  n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;\n");
    printf("  for (k=0; k<n; ++k)\n  {\n    d = z>>%d;\n", (bits - 1));
    printf("    //get sign. for other architectures, you might want to use the more portable version\n");
    printf("    //d = z>=0 ? 0 : -1;\n    tx = x - (((y>>k) ^ d) - d);\n    ty = y + (((x>>k) ^ d) - d);\n");
    printf("    tz = z - ((cordic_ctab[k] ^ d) - d);\n    x = tx; y = ty; z = tz;\n  }  \n *c = x; *s = y;\n}\n");
    return 0;
}


//Print out sin(x) vs fp CORDIC sin(x)
int main(int argc, char **argv)
{
    double p;
    int s, c;
    int i;

    gen_cordic_table();

    printf("codic : math\n");
    for(i = -50; i < 50; i++)
    {
        int     theta = 0;

        p = (i / 50.0) * M_PI / 2; // theta: -pi/2 ~ pi/2

        theta = p * MUL;
        //use 32 iterations
        cordic(theta, &s, &c, 32);

        //these values should be nearly equal
        printf("theta %d: %f vs. %f\n", theta, s / MUL, sin(p));
    }

    return 0;
}
