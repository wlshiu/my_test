#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARM_DSP_TABLE_ATTRIBUTE
#define __STATIC_FORCEINLINE
#define CONFIG_TARGET_ERR_RATE          0.1f




#define FAST_MATH_TABLE_SIZE  512
#define FAST_MATH_Q31_SHIFT   (32 - 10)
#define FAST_MATH_Q15_SHIFT   (16 - 10)

#define CONTROLLER_Q31_SHIFT  (32 - 9)
/* 1.31(q31) Fixed value of 2/360 */
/* -1 to +1 is divided into 360 values so total spacing is (2/360) */
#define INPUT_SPACING         0xB60B61


#define CONTROLLER_Q15_SHIFT  (16 - 9)


#ifndef PI
#define PI               3.14159265358979f
#endif

typedef int16_t q15_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

#define degree2rad(__degree__)      (((__degree__) * PI) / 180)
#define rad2degree(__rad__)         (((__rad__) * 180)/PI)
#define _IQ15(val)          ((q15_t)((val) * 0x00008000L))

__STATIC_FORCEINLINE q31_t clip_q63_to_q31(q63_t x)
{
    return ((q31_t) (x >> 32) != ((q31_t) x >> 31)) ?
           ((0x7FFFFFFF ^ ((q31_t) (x >> 63)))) : (q31_t) x;
}

__STATIC_FORCEINLINE q15_t clip_q31_to_q15(q31_t x)
{
    return ((q31_t) (x >> 16) != ((q31_t) x >> 15)) ?
           ((0x7FFF ^ ((q15_t) (x >> 31)))) : (q15_t) x;
}

/**
  @par
  Table values are in Q31 (1.31 fixed-point format) and generation is done in
  three steps.  First, generate sin values in floating point:
  <pre>
  tableSize = 512;
  for (n = 0; n < (tableSize + 1); n++)
  {
 	sinTable[n] = sin(2*PI*n/tableSize);
  } </pre>
  where PI value is 3.14159265358979
 @par
  Second, convert floating-point to Q31 (Fixed point):
 	(sinTable[i] * pow(2, 31))
 @par
  Finally, round to the nearest integer value:
  	sinTable[i] += (sinTable[i] > 0 ? 0.5 : -0.5);
 */
const q31_t sinTable_q31[FAST_MATH_TABLE_SIZE + 1] ARM_DSP_TABLE_ATTRIBUTE =
{
    0L, 26352928L, 52701887L, 79042909L, 105372028L, 131685278L, 157978697L,
    184248325L, 210490206L, 236700388L, 262874923L, 289009871L, 315101295L,
    341145265L, 367137861L, 393075166L, 418953276L, 444768294L, 470516330L,
    496193509L, 521795963L, 547319836L, 572761285L, 598116479L, 623381598L,
    648552838L, 673626408L, 698598533L, 723465451L, 748223418L, 772868706L,
    797397602L, 821806413L, 846091463L, 870249095L, 894275671L, 918167572L,
    941921200L, 965532978L, 988999351L, 1012316784L, 1035481766L, 1058490808L,
    1081340445L, 1104027237L, 1126547765L, 1148898640L, 1171076495L, 1193077991L,
    1214899813L, 1236538675L, 1257991320L, 1279254516L, 1300325060L, 1321199781L,
    1341875533L, 1362349204L, 1382617710L, 1402678000L, 1422527051L, 1442161874L,
    1461579514L, 1480777044L, 1499751576L, 1518500250L, 1537020244L, 1555308768L,
    1573363068L, 1591180426L, 1608758157L, 1626093616L, 1643184191L, 1660027308L,
    1676620432L, 1692961062L, 1709046739L, 1724875040L, 1740443581L, 1755750017L,
    1770792044L, 1785567396L, 1800073849L, 1814309216L, 1828271356L, 1841958164L,
    1855367581L, 1868497586L, 1881346202L, 1893911494L, 1906191570L, 1918184581L,
    1929888720L, 1941302225L, 1952423377L, 1963250501L, 1973781967L, 1984016189L,
    1993951625L, 2003586779L, 2012920201L, 2021950484L, 2030676269L, 2039096241L,
    2047209133L, 2055013723L, 2062508835L, 2069693342L, 2076566160L, 2083126254L,
    2089372638L, 2095304370L, 2100920556L, 2106220352L, 2111202959L, 2115867626L,
    2120213651L, 2124240380L, 2127947206L, 2131333572L, 2134398966L, 2137142927L,
    2139565043L, 2141664948L, 2143442326L, 2144896910L, 2146028480L, 2146836866L,
    2147321946L, 2147483647L, 2147321946L, 2146836866L, 2146028480L, 2144896910L,
    2143442326L, 2141664948L, 2139565043L, 2137142927L, 2134398966L, 2131333572L,
    2127947206L, 2124240380L, 2120213651L, 2115867626L, 2111202959L, 2106220352L,
    2100920556L, 2095304370L, 2089372638L, 2083126254L, 2076566160L, 2069693342L,
    2062508835L, 2055013723L, 2047209133L, 2039096241L, 2030676269L, 2021950484L,
    2012920201L, 2003586779L, 1993951625L, 1984016189L, 1973781967L, 1963250501L,
    1952423377L, 1941302225L, 1929888720L, 1918184581L, 1906191570L, 1893911494L,
    1881346202L, 1868497586L, 1855367581L, 1841958164L, 1828271356L, 1814309216L,
    1800073849L, 1785567396L, 1770792044L, 1755750017L, 1740443581L, 1724875040L,
    1709046739L, 1692961062L, 1676620432L, 1660027308L, 1643184191L, 1626093616L,
    1608758157L, 1591180426L, 1573363068L, 1555308768L, 1537020244L, 1518500250L,
    1499751576L, 1480777044L, 1461579514L, 1442161874L, 1422527051L, 1402678000L,
    1382617710L, 1362349204L, 1341875533L, 1321199781L, 1300325060L, 1279254516L,
    1257991320L, 1236538675L, 1214899813L, 1193077991L, 1171076495L, 1148898640L,
    1126547765L, 1104027237L, 1081340445L, 1058490808L, 1035481766L, 1012316784L,
    988999351L, 965532978L, 941921200L, 918167572L, 894275671L, 870249095L,
    846091463L, 821806413L, 797397602L, 772868706L, 748223418L, 723465451L,
    698598533L, 673626408L, 648552838L, 623381598L, 598116479L, 572761285L,
    547319836L, 521795963L, 496193509L, 470516330L, 444768294L, 418953276L,
    393075166L, 367137861L, 341145265L, 315101295L, 289009871L, 262874923L,
    236700388L, 210490206L, 184248325L, 157978697L, 131685278L, 105372028L,
    79042909L, 52701887L, 26352928L, 0L, -26352928L, -52701887L, -79042909L,
    -105372028L, -131685278L, -157978697L, -184248325L, -210490206L, -236700388L,
    -262874923L, -289009871L, -315101295L, -341145265L, -367137861L, -393075166L,
    -418953276L, -444768294L, -470516330L, -496193509L, -521795963L, -547319836L,
    -572761285L, -598116479L, -623381598L, -648552838L, -673626408L, -698598533L,
    -723465451L, -748223418L, -772868706L, -797397602L, -821806413L, -846091463L,
    -870249095L, -894275671L, -918167572L, -941921200L, -965532978L, -988999351L,
    -1012316784L, -1035481766L, -1058490808L, -1081340445L, -1104027237L,
    -1126547765L, -1148898640L, -1171076495L, -1193077991L, -1214899813L,
    -1236538675L, -1257991320L, -1279254516L, -1300325060L, -1321199781L,
    -1341875533L, -1362349204L, -1382617710L, -1402678000L, -1422527051L,
    -1442161874L, -1461579514L, -1480777044L, -1499751576L, -1518500250L,
    -1537020244L, -1555308768L, -1573363068L, -1591180426L, -1608758157L,
    -1626093616L, -1643184191L, -1660027308L, -1676620432L, -1692961062L,
    -1709046739L, -1724875040L, -1740443581L, -1755750017L, -1770792044L,
    -1785567396L, -1800073849L, -1814309216L, -1828271356L, -1841958164L,
    -1855367581L, -1868497586L, -1881346202L, -1893911494L, -1906191570L,
    -1918184581L, -1929888720L, -1941302225L, -1952423377L, -1963250501L,
    -1973781967L, -1984016189L, -1993951625L, -2003586779L, -2012920201L,
    -2021950484L, -2030676269L, -2039096241L, -2047209133L, -2055013723L,
    -2062508835L, -2069693342L, -2076566160L, -2083126254L, -2089372638L,
    -2095304370L, -2100920556L, -2106220352L, -2111202959L, -2115867626L,
    -2120213651L, -2124240380L, -2127947206L, -2131333572L, -2134398966L,
    -2137142927L, -2139565043L, -2141664948L, -2143442326L, -2144896910L,
    -2146028480L, -2146836866L, -2147321946L, (q31_t)0x80000000, -2147321946L,
    -2146836866L, -2146028480L, -2144896910L, -2143442326L, -2141664948L,
    -2139565043L, -2137142927L, -2134398966L, -2131333572L, -2127947206L,
    -2124240380L, -2120213651L, -2115867626L, -2111202959L, -2106220352L,
    -2100920556L, -2095304370L, -2089372638L, -2083126254L, -2076566160L,
    -2069693342L, -2062508835L, -2055013723L, -2047209133L, -2039096241L,
    -2030676269L, -2021950484L, -2012920201L, -2003586779L, -1993951625L,
    -1984016189L, -1973781967L, -1963250501L, -1952423377L, -1941302225L,
    -1929888720L, -1918184581L, -1906191570L, -1893911494L, -1881346202L,
    -1868497586L, -1855367581L, -1841958164L, -1828271356L, -1814309216L,
    -1800073849L, -1785567396L, -1770792044L, -1755750017L, -1740443581L,
    -1724875040L, -1709046739L, -1692961062L, -1676620432L, -1660027308L,
    -1643184191L, -1626093616L, -1608758157L, -1591180426L, -1573363068L,
    -1555308768L, -1537020244L, -1518500250L, -1499751576L, -1480777044L,
    -1461579514L, -1442161874L, -1422527051L, -1402678000L, -1382617710L,
    -1362349204L, -1341875533L, -1321199781L, -1300325060L, -1279254516L,
    -1257991320L, -1236538675L, -1214899813L, -1193077991L, -1171076495L,
    -1148898640L, -1126547765L, -1104027237L, -1081340445L, -1058490808L,
    -1035481766L, -1012316784L, -988999351L, -965532978L, -941921200L,
    -918167572L, -894275671L, -870249095L, -846091463L, -821806413L, -797397602L,
    -772868706L, -748223418L, -723465451L, -698598533L, -673626408L, -648552838L,
    -623381598L, -598116479L, -572761285L, -547319836L, -521795963L, -496193509L,
    -470516330L, -444768294L, -418953276L, -393075166L, -367137861L, -341145265L,
    -315101295L, -289009871L, -262874923L, -236700388L, -210490206L, -184248325L,
    -157978697L, -131685278L, -105372028L, -79042909L, -52701887L, -26352928L, 0
};


/**
  @par
  Table values are in Q15 (1.15 fixed-point format) and generation is done in
  three steps.  First,  generate sin values in floating point:
  <pre>
  tableSize = 512;
  for (n = 0; n < (tableSize + 1); n++)
  {
 	sinTable[n] = sin(2*PI*n/tableSize);
  } </pre>
  where PI value is  3.14159265358979
 @par
  Second, convert floating-point to Q15 (Fixed point):
 	(sinTable[i] * pow(2, 15))
 @par
  Finally, round to the nearest integer value:
  	sinTable[i] += (sinTable[i] > 0 ? 0.5 :-0.5);
 */
const q15_t sinTable_q15[FAST_MATH_TABLE_SIZE + 1] ARM_DSP_TABLE_ATTRIBUTE =
{
    0, 402, 804, 1206, 1608, 2009, 2411, 2811, 3212, 3612, 4011, 4410, 4808,
    5205, 5602, 5998, 6393, 6787, 7180, 7571, 7962, 8351, 8740, 9127, 9512,
    9896, 10279, 10660, 11039, 11417, 11793, 12167, 12540, 12910, 13279,
    13646, 14010, 14373, 14733, 15091, 15447, 15800, 16151, 16500, 16846,
    17190, 17531, 17869, 18205, 18538, 18868, 19195, 19520, 19841, 20160,
    20475, 20788, 21097, 21403, 21706, 22006, 22302, 22595, 22884, 23170,
    23453, 23732, 24008, 24279, 24548, 24812, 25073, 25330, 25583, 25833,
    26078, 26320, 26557, 26791, 27020, 27246, 27467, 27684, 27897, 28106,
    28311, 28511, 28707, 28899, 29086, 29269, 29448, 29622, 29792, 29957,
    30118, 30274, 30425, 30572, 30715, 30853, 30986, 31114, 31238, 31357,
    31471, 31581, 31686, 31786, 31881, 31972, 32058, 32138, 32214, 32286,
    32352, 32413, 32470, 32522, 32568, 32610, 32647, 32679, 32706, 32729,
    32746, 32758, 32766, 32767, 32766, 32758, 32746, 32729, 32706, 32679,
    32647, 32610, 32568, 32522, 32470, 32413, 32352, 32286, 32214, 32138,
    32058, 31972, 31881, 31786, 31686, 31581, 31471, 31357, 31238, 31114,
    30986, 30853, 30715, 30572, 30425, 30274, 30118, 29957, 29792, 29622,
    29448, 29269, 29086, 28899, 28707, 28511, 28311, 28106, 27897, 27684,
    27467, 27246, 27020, 26791, 26557, 26320, 26078, 25833, 25583, 25330,
    25073, 24812, 24548, 24279, 24008, 23732, 23453, 23170, 22884, 22595,
    22302, 22006, 21706, 21403, 21097, 20788, 20475, 20160, 19841, 19520,
    19195, 18868, 18538, 18205, 17869, 17531, 17190, 16846, 16500, 16151,
    15800, 15447, 15091, 14733, 14373, 14010, 13646, 13279, 12910, 12540,
    12167, 11793, 11417, 11039, 10660, 10279, 9896, 9512, 9127, 8740, 8351,
    7962, 7571, 7180, 6787, 6393, 5998, 5602, 5205, 4808, 4410, 4011, 3612,
    3212, 2811, 2411, 2009, 1608, 1206, 804, 402, 0, -402, -804, -1206,
    -1608, -2009, -2411, -2811, -3212, -3612, -4011, -4410, -4808, -5205,
    -5602, -5998, -6393, -6787, -7180, -7571, -7962, -8351, -8740, -9127,
    -9512, -9896, -10279, -10660, -11039, -11417, -11793, -12167, -12540,
    -12910, -13279, -13646, -14010, -14373, -14733, -15091, -15447, -15800,
    -16151, -16500, -16846, -17190, -17531, -17869, -18205, -18538, -18868,
    -19195, -19520, -19841, -20160, -20475, -20788, -21097, -21403, -21706,
    -22006, -22302, -22595, -22884, -23170, -23453, -23732, -24008, -24279,
    -24548, -24812, -25073, -25330, -25583, -25833, -26078, -26320, -26557,
    -26791, -27020, -27246, -27467, -27684, -27897, -28106, -28311, -28511,
    -28707, -28899, -29086, -29269, -29448, -29622, -29792, -29957, -30118,
    -30274, -30425, -30572, -30715, -30853, -30986, -31114, -31238, -31357,
    -31471, -31581, -31686, -31786, -31881, -31972, -32058, -32138, -32214,
    -32286, -32352, -32413, -32470, -32522, -32568, -32610, -32647, -32679,
    -32706, -32729, -32746, -32758, -32766, -32768, -32766, -32758, -32746,
    -32729, -32706, -32679, -32647, -32610, -32568, -32522, -32470, -32413,
    -32352, -32286, -32214, -32138, -32058, -31972, -31881, -31786, -31686,
    -31581, -31471, -31357, -31238, -31114, -30986, -30853, -30715, -30572,
    -30425, -30274, -30118, -29957, -29792, -29622, -29448, -29269, -29086,
    -28899, -28707, -28511, -28311, -28106, -27897, -27684, -27467, -27246,
    -27020, -26791, -26557, -26320, -26078, -25833, -25583, -25330, -25073,
    -24812, -24548, -24279, -24008, -23732, -23453, -23170, -22884, -22595,
    -22302, -22006, -21706, -21403, -21097, -20788, -20475, -20160, -19841,
    -19520, -19195, -18868, -18538, -18205, -17869, -17531, -17190, -16846,
    -16500, -16151, -15800, -15447, -15091, -14733, -14373, -14010, -13646,
    -13279, -12910, -12540, -12167, -11793, -11417, -11039, -10660, -10279,
    -9896, -9512, -9127, -8740, -8351, -7962, -7571, -7180, -6787, -6393,
    -5998, -5602, -5205, -4808, -4410, -4011, -3612, -3212, -2811, -2411,
    -2009, -1608, -1206, -804, -402, 0
};



/**
  @brief         Q31 sin_cos function.
  @param[in]     theta    scaled input value in degrees (0 ~ 2^32) maps to [-180, 179]
  @param[out]    pSinVal  points to processed sine output
  @param[out]    pCosVal  points to processed cosine output

  The Q31 input value is in the range [-1 0.999999] and is mapped to a degree value in the range [-180 179].
 */
void arm_sin_cos_q31(
    int32_t theta,
    q31_t   *pSinVal,
    q31_t   *pCosVal)
{
    q31_t fract;                                     /* Temporary input, output variables */
    uint16_t indexS, indexC;                         /* Index variable */
    q31_t f1, f2, d1, d2;                            /* Two nearest output values */
    q31_t Dn, Df;
    q63_t temp;

    /* Calculate the nearest index */
    indexS = (uint32_t)theta >> CONTROLLER_Q31_SHIFT;
    indexC = (indexS + 128) & 0x1ff;

    /* Calculation of fractional value */
    fract = (theta - (indexS << CONTROLLER_Q31_SHIFT)) << 8;

    /* Read two nearest values of input value from the cos & sin tables */
    f1 =  sinTable_q31[indexC  ];
    f2 =  sinTable_q31[indexC + 1];
    d1 = -sinTable_q31[indexS  ];
    d2 = -sinTable_q31[indexS + 1];

    Dn = 0x1921FB5; /* delta between the two points (fixed), in this case 2*pi/FAST_MATH_TABLE_SIZE */
    Df = f2 - f1;   /* delta between the values of the functions */

    temp = Dn * ((q63_t)d1 + d2);
    temp = temp - ((q63_t)Df << 32);
    temp = (q63_t)fract * (temp >> 31);
    temp = temp + ((3 * (q63_t)Df << 31) - (d2 + ((q63_t)d1 << 1)) * Dn);
    temp = (q63_t)fract * (temp >> 31);
    temp = temp + (q63_t)d1 * Dn;
    temp = (q63_t)fract * (temp >> 31);

    /* Calculation of cosine value */
    *pCosVal = clip_q63_to_q31((temp >> 31) + (q63_t)f1);

    /* Read two nearest values of input value from the cos & sin tables */
    f1 = sinTable_q31[indexS ];
    f2 = sinTable_q31[indexS + 1];
    d1 = sinTable_q31[indexC ];
    d2 = sinTable_q31[indexC + 1];

    Df = f2 - f1; // delta between the values of the functions
    temp = Dn * ((q63_t)d1 + d2);
    temp = temp - ((q63_t)Df << 32);
    temp = (q63_t)fract * (temp >> 31);
    temp = temp + ((3 * (q63_t)Df << 31) - (d2 + ((q63_t)d1 << 1)) * Dn);
    temp = (q63_t)fract * (temp >> 31);
    temp = temp + (q63_t)d1 * Dn;
    temp = (q63_t)fract * (temp >> 31);

    /* Calculation of sine value */
    *pSinVal = clip_q63_to_q31((temp >> 31) + (q63_t)f1);
}

/**
  @brief         Q15 sin_cos function.
  @param[in]     theta    scaled input value in degrees, (0 ~ 2^16) maps to [-180, 179]
  @param[out]    pSinVal  points to processed sine output
  @param[out]    pCosVal  points to processed cosine output

  The Q15 input value is in the range [-1 0.999999] and is mapped to a degree value in the range [-180 179].
 */
void arm_sin_cos_q15(
    int16_t theta,
    q15_t   *pSinVal,
    q15_t   *pCosVal)
{
    q15_t fract;                                     /* Temporary input, output variables */
    uint16_t indexS, indexC;                         /* Index variable */
    q15_t f1, f2, d1, d2;                            /* Two nearest output values */
    q15_t Dn, Df;
    q31_t temp;

    /* Calculate the nearest index */
    indexS = (uint16_t)theta >> CONTROLLER_Q15_SHIFT;
    indexC = (indexS + 128) & 0x1ff;

    /* Calculation of fractional value */
    fract = (theta - (indexS << CONTROLLER_Q15_SHIFT)) << 8;

    /* Read two nearest values of input value from the cos & sin tables */
    f1 =  sinTable_q15[indexC  ];
    f2 =  sinTable_q15[indexC + 1];
    d1 = -sinTable_q15[indexS  ];
    d2 = -sinTable_q15[indexS + 1];

    Dn = 0x192;     /* delta between the two points (fixed), in this case 2*pi/FAST_MATH_TABLE_SIZE */
    Df = f2 - f1;   /* delta between the values of the functions */

    temp = Dn * ((q31_t)d1 + d2);
    temp = temp - ((q31_t)Df << 16);
    temp = (q31_t)fract * (temp >> 15);
    temp = temp + ((3 * (q31_t)Df << 15) - (d2 + ((q31_t)d1 << 1)) * Dn);
    temp = (q31_t)fract * (temp >> 15);
    temp = temp + (q31_t)d1 * Dn;
    temp = (q31_t)fract * (temp >> 15);

    /* Calculation of cosine value */
    *pCosVal = clip_q31_to_q15((temp >> 15) + (q31_t)f1);

    /* Read two nearest values of input value from the cos & sin tables */
    f1 = sinTable_q15[indexS ];
    f2 = sinTable_q15[indexS + 1];
    d1 = sinTable_q15[indexC ];
    d2 = sinTable_q15[indexC + 1];

    Df = f2 - f1; // delta between the values of the functions
    temp = Dn * ((q31_t)d1 + d2);
    temp = temp - ((q31_t)Df << 16);
    temp = (q31_t)fract * (temp >> 15);
    temp = temp + ((3 * (q31_t)Df << 15) - (d2 + ((q31_t)d1 << 1)) * Dn);
    temp = (q31_t)fract * (temp >> 15);
    temp = temp + (q31_t)d1 * Dn;
    temp = (q31_t)fract * (temp >> 15);

    /* Calculation of sine value */
    *pSinVal = clip_q31_to_q15((temp >> 15) + (q31_t)f1);

    return;
}





int main()
{
#define CONFIG_CORDIC_ROT_Q31          0
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

#if 1
    if( fout )
    {
        fprintf(fout, "sim-sin, sim-cos\n");
    }

    for(int degree = 0; degree < 1 <<16; degree += 1)
    {
        q15_t   sim_sin = 0, sim_cos = 0;
        arm_sin_cos_q15((q15_t)degree, &sim_sin, &sim_cos);

        if( fout )
        {
            fprintf(fout, "%d, %d\n", sim_sin, sim_cos);
        }
    }
#else

    if( fout )
    {
        fprintf(fout, "degree, ideal-sin, ideal-cos, sim-sin, sim-cos, , err-rate-sin (%%), err-rate-cos (%%)\n");
    }

    for(float degree = 0.0f; degree < 360.0f; degree += 0.01f)
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
        radian = (uint64_t)((degree/360) * (1 << 31));

        sim_cos = (float)arm_cos_q31((q31_t)radian);
        sim_sin = (float)arm_sin_q31((q31_t)radian);
        #endif

        ideal_sin = sin(degree2rad(degree)) * (1 << 31);
        ideal_cos = cos(degree2rad(degree)) * (1 << 31);

    #elif (CONFIG_CORDIC_ROT_Q15)

        #if 1
        q15_t   sim_sin = 0, sim_cos = 0;
        float   target_degree = 0.0f;

        /* degree: -180 ~ 179 */
        target_degree = (degree < 180) ? degree : degree - 360;
        target_degree = (0xFFFF * (float)target_degree/360);
        arm_sin_cos_q15((q15_t)target_degree, &sim_sin, &sim_cos);
        #else
        int32_t     radian = 0;
        q15_t       sim_sin = 0, sim_cos = 0;

        radian = (int32_t)((degree/360.0f) * (1<<15));

        sim_cos = arm_cos_q15((q15_t)radian);
        sim_sin = arm_sin_q15((q15_t)radian);
        #endif

        ideal_sin = sin(degree2rad(degree)) * (1 <<15);
        ideal_cos = cos(degree2rad(degree)) * (1 <<15);
    #elif (CONFIG_CORDIC_ROT_FLOAT)
        /* float */
        float32_t   sim_sin = 0.0f, sim_cos = 0.0f;

        arm_sin_cos_f32(degree, &sim_sin, &sim_cos);
        ideal_sin = sin(degree2rad(degree));
        ideal_cos = cos(degree2rad(degree));
    #endif // 1

#if 0
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
#endif

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
#endif // 1

    if( fout )      fclose(fout);
    return 0;
}
