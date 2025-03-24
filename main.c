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

#define CORDIC_MAXITER  16
#define CORDIC_PI       0x10000000

int CORDIC_ZTBL[] = {
    0x04000000, 0x025C80A4, 0x013F670B, 0x00A2223B,
    0x005161A8, 0x0028BAFC, 0x00145EC4, 0x000A2F8B, 0x000517CA, 0x00028BE6,
    0x000145F3, 0x0000A2FA, 0x0000517D, 0x000028BE, 0x0000145F, 0x00000A30
};

int fxpt_atan2(int y, int x)
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
    for(float degree = 0.0f; degree < 360.0f; degree += 0.1f)
    {
        float   ideal_sin = 0.0f, ideal_cos = 0.0f;
        float   sim_sin = 0.0f, sim_cos = 0.0f;
        float   err_rate = 0.0f;

    #if 1
        /* '0 ~ 0xFFFFFFFF' map to '0 ~ 2PI' */
        uint64_t    radian = 0;
        radian = (uint64_t)((degree/360) * 0x80000000);

        sim_cos = (float)arm_cos_q31((q31_t)radian);
        sim_sin = (float)arm_sin_q31((q31_t)radian);
        ideal_sin = sin(degree2rad(degree)) * 0x80000000;
        ideal_cos = cos(degree2rad(degree)) * 0x80000000;
    #elif 1
        /* '0 ~ 0xFFFF' map to '0 ~ 2PI' */
        int32_t     radian = 0;
        radian = (int32_t)((degree/360.0f) * (1<<15));

        sim_cos = arm_cos_q15((q15_t)radian);
        sim_sin = arm_sin_q15((q15_t)radian);
        ideal_sin = Q15(sin(degree2rad(degree)));
        ideal_cos = Q15(cos(degree2rad(degree)));
    #else
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
    }
    return;
}

static void
_test_arm_atan2(void)
{
    for(float degree = -180.0f; degree < 180.0f; degree += 0.1f)
    {
        float32_t   ideal_sin = 0.0f, ideal_cos = 0.0f;
        float       ideal_radian = 0.0f;
        float       err_rate = 0.0f;

        if( degree == -180.0f )
            continue;

        ideal_sin    = sin(degree2rad(degree));
        ideal_cos    = cos(degree2rad(degree));
        ideal_radian = degree2rad(degree);

    #if 0
        /* Q31 atan2 */
        q31_t   sim_radian = 0;
        ideal_radian = Q29(ideal_radian);
        arm_atan2_q31((q31_t)Q24(ideal_sin), (q31_t)Q24(ideal_cos), &sim_radian);
    #elif 0
        /* Q15 atan2 */
        q15_t   sim_radian = 0;
        ideal_radian = Q13(ideal_radian);
        arm_atan2_q15((q15_t)Q15(ideal_sin), (q15_t)Q15(ideal_cos), &sim_radian);
    #elif 1
        int16_t     sim_radian = 0;

        ideal_radian = Q26(ideal_radian);
        sim_radian = fxpt_atan2(Q26(ideal_sin), Q26(ideal_cos));// >> 15;
    #else
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
    }
    return;
}

static void
_test_arm_cordic_rotation(void)
{
    for(float degree = 0.0f; degree < 360.0f; degree += 0.1f)
    {
        float       ideal_sin = 0.0f, ideal_cos = 0.0f;
        float       err_rate = 0.0f;

    #if 1

        #if 1
        q31_t   sim_sin = 0, sim_cos = 0;
        float   target_degree = 0.0f;

        /* degree: -180 ~ 179 */
        target_degree = (degree < 180) ? degree : degree - 360;
        target_degree = (int64_t)(0xFFFFFFFF * target_degree/360);
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

    #else
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

        err_rate = fabs(sim_cos - ideal_cos) * 100 / fabs(ideal_cos);
        printf("%3.6f degree cos: %5.6f : %5.6f    (rate= %2.6f %%)\n",
               (float)degree, (float)sim_cos, ideal_cos, fabs(err_rate) > CONFIG_TARGET_ERR_RATE ? fabs(err_rate) : 0);

        if( fabs(err_rate) > CONFIG_TARGET_ERR_RATE )
            printf("\n");
    }

    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
#if 1
	for (int i = 0; i < 360; i += 1)
    {
		int16_t     cos_val = 32767.0f * cos((float)i * PI / 180.0f);
		int16_t     sin_val = 32767.0f * sin((float)i * PI / 180.0f);
		uint32_t    atan2_clocks = 0;
		uint32_t    atan2f_clocks = 0;
		uint32_t    fxpt_at2_clocks = 0;

		int16_t at2f = 32767.0f * (float)atan2f((float)sin_val, (float)cos_val) / PI;


		int16_t fxpt_at2 = fxpt_atan2(sin_val << 15, cos_val << 15) >> 13;


		//float dif = 100.0f*abs(at2-fxpt_at2)/at2;

		int16_t iang = 2 * 32767.0f * i / 360.0f;

		printf(
				" angle = %i(%i), fxpt_atan2 = %i, atan2f = %i, sn= %i cs=%i \n",
				i, iang, fxpt_at2, at2f, sin_val, cos_val);

	}
#else

//    _test_arm_sin_cos();

    _test_arm_atan2();

//    _test_arm_cordic_rotation();
#endif // 0
    return 0;
}
