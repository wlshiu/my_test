/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/03/24
 * @license
 * @description
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fast_math_functions.h"
#include "controller_functions.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TARGET_ERR_RATE          0.1f

#define Q0(fval)                    ((int32_t)((fval) * 0x00000001L))
#define Q1(fval)                    ((int32_t)((fval) * 0x00000002L))
#define Q2(fval)                    ((int32_t)((fval) * 0x00000004L))
#define Q3(fval)                    ((int32_t)((fval) * 0x00000008L))
#define Q4(fval)                    ((int32_t)((fval) * 0x00000010L))
#define Q5(fval)                    ((int32_t)((fval) * 0x00000020L))
#define Q6(fval)                    ((int32_t)((fval) * 0x00000040L))
#define Q7(fval)                    ((int32_t)((fval) * 0x00000080L))
#define Q8(fval)                    ((int32_t)((fval) * 0x00000100L))
#define Q9(fval)                    ((int32_t)((fval) * 0x00000200L))
#define Q10(fval)                   ((int32_t)((fval) * 0x00000400L))
#define Q11(fval)                   ((int32_t)((fval) * 0x00000800L))
#define Q12(fval)                   ((int32_t)((fval) * 0x00001000L))
#define Q13(fval)                   ((int32_t)((fval) * 0x00002000L))
#define Q14(fval)                   ((int32_t)((fval) * 0x00004000L))
#define Q15(fval)                   ((int32_t)((fval) * 0x00008000L))
#define Q16(fval)                   ((int32_t)((fval) * 0x00010000L))
#define Q17(fval)                   ((int32_t)((fval) * 0x00020000L))
#define Q18(fval)                   ((int32_t)((fval) * 0x00040000L))
#define Q19(fval)                   ((int32_t)((fval) * 0x00080000L))
#define Q20(fval)                   ((int32_t)((fval) * 0x00100000L))
#define Q21(fval)                   ((int32_t)((fval) * 0x00200000L))
#define Q22(fval)                   ((int32_t)((fval) * 0x00400000L))
#define Q23(fval)                   ((int32_t)((fval) * 0x00800000L))
#define Q24(fval)                   ((int32_t)((fval) * 0x01000000L))
#define Q25(fval)                   ((int32_t)((fval) * 0x02000000L))
#define Q26(fval)                   ((int32_t)((fval) * 0x04000000L))
#define Q27(fval)                   ((int32_t)((fval) * 0x08000000L))
#define Q28(fval)                   ((int32_t)((fval) * 0x10000000L))
#define Q29(fval)                   ((int32_t)((fval) * 0x20000000L))
#define Q30(fval)                   ((int32_t)((fval) * 0x40000000L))

//=============================================================================
//                  Macro Definition
//=============================================================================
#define degree2rad(__degree__)      (((__degree__) * PI) / 180)
#define rad2degree(__rad__)         (((__rad__) * 180)/PI)

static inline float range_to_2pi(float radian)
{
    /* 0 ~ 2PI */
    if( radian > 0 )
    {
        while( radian > PI )
            radian -= 2*PI;
    }
    else
    {
        while( radian < -PI )
            radian += 2*PI;
    }
    return radian;
}

static inline float range_to_0_360(float degree)
{
    return degree;
}
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

#define CORDIC_MAXITER  16 // 16
#define CORDIC_PI       0x10000000

int CORDIC_ZTBL[] = {
    0x04000000, 0x025C80A4, 0x013F670B, 0x00A2223B,
    0x005161A8, 0x0028BAFC, 0x00145EC4, 0x000A2F8B, 0x000517CA, 0x00028BE6,
    0x000145F3, 0x0000A2FA, 0x0000517D, 0x000028BE, 0x0000145F, 0x00000A30
};

int fxpt_atan2(int32_t y, int32_t x)
{
	int k, tx, z = 0, fl = 0;

	if (x < 0)
    {
		fl = ((y > 0) ? 1 : -1);
		x = -x;
		y = -y;
	}

	for (k = 0; k < CORDIC_MAXITER; k++)
    {
		tx = x;
		if (y <= 0)
        {
			x -= (y >> k);
			y += (tx >> k);
			z -= CORDIC_ZTBL[k];
		}
		else
        {
			x += (y >> k);
			y -= (tx >> k);
			z += CORDIC_ZTBL[k];
		}
	}

	if (fl != 0)
    {
		z += fl * CORDIC_PI;
	}

	return z;   // radians multiplied by factor M
	//*m = x; // sqrt(x^2+y^2) multiplied by gain F
}

static void
_test_arm_sin_cos(void)
{
#define CONFIG_SIN_COS_Q31          1
#define CONFIG_SIN_COS_Q15          1
#define CONFIG_SIN_COS_FLOAT        1

    FILE    *fout = 0;

#if (CONFIG_SIN_COS_Q31)
    char    *pFilename = "sin_cos_arm_q31.csv";
#elif (CONFIG_SIN_COS_Q15)
    char    *pFilename = "sin_cos_arm_q15.csv";
#elif (CONFIG_SIN_COS_FLOAT)
    char    *pFilename = "sin_cos_arm_float.csv";
#endif


    if( !(fout = fopen(pFilename, "w")) )
    {
        printf("open %s fail ! \n", pFilename);
        while(1);
    }

    if( fout )
    {
        fprintf(fout, "degree, ideal-sin, ideal-cos, sim-sin, sim-cos, , err-rate-sin (%%), err-rate-cos (%%)\n");
    }

    for(float degree = 0.0f; degree < 360.0f; degree += 0.1f)
    {
        float   ideal_sin = 0.0f, ideal_cos = 0.0f;
        float   sim_sin = 0.0f, sim_cos = 0.0f;
        float   err_rate = 0.0f;

    #if (CONFIG_SIN_COS_Q31)
        /* '0 ~ 0xFFFFFFFF' map to '0 ~ 2PI' */
        uint64_t    radian = 0;
        radian = (uint64_t)((degree/360) * 0x80000000);

        sim_cos = (float)arm_cos_q31((q31_t)radian);
        sim_sin = (float)arm_sin_q31((q31_t)radian);
        ideal_sin = sin(degree2rad(degree)) * 0x80000000;
        ideal_cos = cos(degree2rad(degree)) * 0x80000000;
    #elif (CONFIG_SIN_COS_Q15)
        /* '0 ~ 0xFFFF' map to '0 ~ 2PI' */
        int32_t     radian = 0;
        radian = (int32_t)((degree/360.0f) * (1<<15));

        sim_cos = arm_cos_q15((q15_t)radian);
        sim_sin = arm_sin_q15((q15_t)radian);
        ideal_sin = Q15(sin(degree2rad(degree)));
        ideal_cos = Q15(cos(degree2rad(degree)));
    #elif (CONFIG_SIN_COS_FLOAT)
        /* float */
        sim_cos = arm_cos_f32(degree2rad(degree));
        sim_sin = arm_sin_f32(degree2rad(degree));
        ideal_sin = sin(degree2rad(degree));
        ideal_cos = cos(degree2rad(degree));
    #endif // 1

        err_rate = fabs(sim_sin - ideal_sin) * 100 / fabs(ideal_sin);
        printf("%3.6f degree sin: %5.6f : %5.6f    (rate= %2.6f %%)\n",
               (float)degree, sim_sin, ideal_sin, fabs(err_rate) > CONFIG_TARGET_ERR_RATE ? fabs(err_rate) : 0);

        if( fabs(err_rate) > CONFIG_TARGET_ERR_RATE )
            printf("\n");

        err_rate = fabs(sim_cos - ideal_cos) * 100 / fabs(ideal_cos);
        printf("%3.6f degree cos: %5.6f : %5.6f    (rate= %2.6f %%)\n",
               (float)degree, sim_cos, ideal_cos, fabs(err_rate) > CONFIG_TARGET_ERR_RATE ? fabs(err_rate) : 0);

        if( fabs(err_rate) > CONFIG_TARGET_ERR_RATE )
            printf("\n");

        if( fout )
        {
            fprintf(fout, "%f, %f, %f, %f, %f, , %f, %f\n",
                    (float)degree,
                    ideal_sin, ideal_cos,
                    sim_sin, sim_cos,
                    fabs(sim_sin - ideal_sin) * 100 / fabs(ideal_sin),
                    fabs(sim_cos - ideal_cos) * 100 / fabs(ideal_cos));
        }
    }

    if( fout )      fclose(fout);
    return;
}

static void
_test_arm_atan2(void)
{
#define CONFIG_ATAN2_Q31          1
#define CONFIG_ATAN2_Q15          1
#define CONFIG_ATAN2_FLOAT        1

    FILE    *fout = 0;

#if (CONFIG_ATAN2_Q31)
    char    *pFilename = "atan2_arm_q31.csv";
#elif (CONFIG_ATAN2_Q15)
    char    *pFilename = "atan2_arm_q15.csv";
#elif (CONFIG_ATAN2_FLOAT)
    char    *pFilename = "atan2_arm_float.csv";
#endif


    if( !(fout = fopen(pFilename, "w")) )
    {
        printf("open %s fail ! \n", pFilename);
        while(1);
    }

    if( fout )
    {
        fprintf(fout, "degree, ideal-radian, sim-radian, , err-rate (%%)\n");
    }

    for(float degree = -180.0f; degree < 180.0f; degree += 0.001f)
    {
        float32_t   ideal_sin = 0.0f, ideal_cos = 0.0f;
        float       ideal_radian = 0.0f;
        float       err_rate = 0.0f;

        if( degree == -180.0f )
            continue;

        ideal_sin    = sin(degree2rad(degree));
        ideal_cos    = cos(degree2rad(degree));
        ideal_radian = degree2rad(degree);

    #if (CONFIG_ATAN2_Q31)
        /* Q31 atan2 */
        q31_t   sim_radian = 0;
        ideal_radian = Q29(ideal_radian);
        arm_atan2_q31((q31_t)Q24(ideal_sin), (q31_t)Q24(ideal_cos), &sim_radian);
    #elif (CONFIG_ATAN2_Q15)
        /* Q15 atan2 */
        q15_t   sim_radian = 0;
        ideal_radian = Q13(ideal_radian);
        arm_atan2_q15((q15_t)Q15(ideal_sin), (q15_t)Q15(ideal_cos), &sim_radian);
    #elif 0
        int16_t     sim_radian = 0;

        ideal_radian = Q26(ideal_radian);
        sim_radian = fxpt_atan2(Q26(ideal_sin), Q26(ideal_cos));// >> 15;
    #elif (CONFIG_ATAN2_FLOAT)
        /* float */
        float32_t   sim_radian = 0.0f;
        #if 0
        /* c-lib math */
        sim_radian = atan2(ideal_sin, ideal_cos);
        #else
        arm_atan2_f32(ideal_sin, ideal_cos, &sim_radian);
        #endif

    #endif // 0

        err_rate = fabs((float)sim_radian - ideal_radian) * 100 / fabs(ideal_radian);
        printf("%3.6f degree: radian=(%5.6f, %5.6f)    (rate= %2.6f %%)\n",
               (float)degree, (float)sim_radian, ideal_radian, fabs(err_rate) > CONFIG_TARGET_ERR_RATE ? fabs(err_rate) : 0);

        if( fabs(err_rate) > CONFIG_TARGET_ERR_RATE )
            printf("\n");

        if( fout )
        {
            fprintf(fout, "%f, %f, %f, , %f\n",
                    (float)degree,
                    (float)ideal_radian, (float)sim_radian,
                    fabs((float)sim_radian - ideal_radian) * 100 / fabs(ideal_radian));
        }
    }

    if( fout )      fclose(fout);
    return;
}

static void
_test_arm_cordic_rotation(void)
{
#define CONFIG_CORDIC_ROT_Q31          1
#define CONFIG_CORDIC_ROT_Q15          1
#define CONFIG_CORDIC_ROT_FLOAT        1

    FILE    *fout = 0;

#if (CONFIG_CORDIC_ROT_Q31)
    char    *pFilename = "cordic_rot_arm_q31.csv";
#elif (CONFIG_CORDIC_ROT_Q15)
    char    *pFilename = "cordic_rot_arm_q15.csv";
#elif (CONFIG_CORDIC_ROT_FLOAT)
    char    *pFilename = "cordic_rot_arm_float.csv";
#endif

    if( !(fout = fopen(pFilename, "w")) )
    {
        printf("open %s fail ! \n", pFilename);
        while(1);
    }

    if( fout )
    {
        fprintf(fout, "degree, ideal-sin, ideal-cos, sim-sin, sim-cos, , err-rate-sin (%%), err-rate-cos (%%)\n");
    }

    for(float degree = 0.0f; degree < 360.0f; degree += 0.001f)
    {
        float       ideal_sin = 0.0f, ideal_cos = 0.0f;
        float       err_rate = 0.0f;

    #if (CONFIG_CORDIC_ROT_Q31)

        #if 1
        q31_t   sim_sin = 0, sim_cos = 0;
        float   target_degree = 0.0f;

        /* degree: -180 ~ 179 */
        target_degree = (degree < 180) ? degree : degree - 360;
        target_degree = (0xFFFFFFFF * (double)target_degree/360);
        arm_sin_cos_q31(target_degree, &sim_sin, &sim_cos);
        #else
        uint64_t    radian = 0;
        float       sim_sin = 0.0f, sim_cos = 0.0f;
        radian = (uint64_t)((degree/360) * 0x80000000);

        sim_cos = (float)arm_cos_q31((q31_t)radian);
        sim_sin = (float)arm_sin_q31((q31_t)radian);
        #endif

        ideal_sin = sin(degree2rad(degree)) * 0x80000000;
        ideal_cos = cos(degree2rad(degree)) * 0x80000000;

    #elif (CONFIG_CORDIC_ROT_Q15)
        int32_t     radian = 0;
        q15_t       sim_sin = 0, sim_cos = 0;

        radian = (int32_t)((degree/360.0f) * (1<<15));

        sim_cos = arm_cos_q15((q15_t)radian);
        sim_sin = arm_sin_q15((q15_t)radian);

        ideal_sin = sin(degree2rad(degree)) * (1 <<15);
        ideal_cos = cos(degree2rad(degree)) * (1 <<15);
    #elif (CONFIG_CORDIC_ROT_FLOAT)
        /* float */
        float32_t   sim_sin = 0.0f, sim_cos = 0.0f;

        arm_sin_cos_f32(degree, &sim_sin, &sim_cos);
        ideal_sin = sin(degree2rad(degree));
        ideal_cos = cos(degree2rad(degree));
    #endif // 1


        err_rate = fabs(sim_sin - ideal_sin) * 100 / fabs(ideal_sin);
        printf("%3.6f degree sin: %5.6f : %5.6f    (rate= %2.6f %%)\n",
               (float)degree, (float)sim_sin, ideal_sin, fabs(err_rate) > CONFIG_TARGET_ERR_RATE ? (float)fabs(err_rate) : 0);

        if( fabs(err_rate) > CONFIG_TARGET_ERR_RATE )
            printf("\n");

        err_rate = fabs((float)sim_cos - ideal_cos) * 100 / fabs(ideal_cos);
        printf("%3.6f degree cos: %5.6f : %5.6f    (rate= %2.6f %%)\n",
               (float)degree, (float)sim_cos, ideal_cos, fabs(err_rate) > CONFIG_TARGET_ERR_RATE ? fabs(err_rate) : 0);

        if( fabs(err_rate) > CONFIG_TARGET_ERR_RATE )
            printf("\n");


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
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
#if 0
	for(float degree = 0; degree < 360.0f; degree += 0.1f)
    {
		int32_t     cos_val = Q30(cos((float)degree * PI / 180.0f));
		int32_t     sin_val = Q30(sin((float)degree * PI / 180.0f));

		int16_t     at2f = Q15((float)atan2f((float)sin_val, (float)cos_val) / PI);

		int16_t     fxpt_at2 = fxpt_atan2(sin_val, cos_val) >> 13;

		float       diff = 100.0f*fabs(at2f - fxpt_at2) /fabs(at2f);

		int16_t     iang = 2 * 32767.0f * degree / 360.0f;

		printf(" degree = %3.6f(%i), fxpt_atan2 = %d, atan2f = %d, sin_val= %d cos_val= %d, err_rate= %5.6f\n",
				(float)degree, iang, fxpt_at2, at2f, sin_val, cos_val, diff);

        if( diff > 0.1f )
            printf("\n");

	}
#else

//    _test_arm_sin_cos();

//    _test_arm_atan2();

    _test_arm_cordic_rotation();
#endif // 0
    return 0;
}
