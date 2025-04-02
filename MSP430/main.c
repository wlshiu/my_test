
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cordic.h"


#define PI                          3.141592f

#define degree2rad(__degree__)      (((__degree__) * PI) / 180)

int main(int argc, char **argv)
{
    FILE    *fout = 0;

    char    *pFilename = "cordic_rot_msp430.csv";

    if( !(fout = fopen(pFilename, "w")) )
    {
        printf("open %s fail ! \n", pFilename);
        while(1);
    }

    if( fout )
    {
        fprintf(fout, "degree, ideal-sin, ideal-cos, sim-sin, sim-cos, , err-rate-sin (%%), err-rate-cos (%%)\n");
    }

    for(float degree = 0.0f; degree < 360; degree += 0.001f)
    {
        float       ideal_sin = 0.0f, ideal_cos = 0.0f;
        float       err_rate = 0.0f;

        int         sim_sin = 0, sim_cos = 0;

        ideal_sin    = sin(degree2rad(degree)) * (1 << 15);
        ideal_cos    = cos(degree2rad(degree)) * (1 << 15);

        cordic_sincos((int)degree, 13, &sim_sin, &sim_cos);

        if( fout )
        {
            fprintf(fout, "%f, %f, %f, %f, %f, , %f, %f\n",
                    (float)degree,
                    (float)ideal_sin, (float)ideal_cos,
                    (float)sim_sin, (float)sim_cos,
                    fabs((float)sim_sin - ideal_sin) * 100 / fabs(ideal_sin),
                    fabs((float)sim_cos - ideal_cos) * 100 / fabs(ideal_cos));
        }
    }

    if( fout )      fclose(fout);

    return 0;
}
