#ifndef _BN_H_INCLUDED_6_14_2014_
#define _BN_H_INCLUDED_6_14_2014_

#include <stdint.h>

#define TEST_RSA    //Define this line to compile and run a self-consistency test.


#define BN_SIZE 64  //64 x 32 bits per int represents a total of 2048 bites.

#ifdef WIN32
    #define LITTLE_ENDIAN   1
#else //WIN32
    //Define one of the endianness below, depending on the platform.
    //#define BIG_ENDIAN   1
    //#define LITTLE_ENDIAN   1
#endif //WIN32

typedef int (*FRND)(void);

typedef struct BN
{
    int    data[BN_SIZE];
} BN;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void BN_set(BN *pX, int M);
int BN_modw(const BN *pX, int M);
int BN_getBits(const BN *pX);
int BN_isZero(const BN *pX);
int BN_isOne(const BN *pX);
int BN_isEven(const BN *pX);
int BN_isEqual(const BN *pX, const BN *pY);
int BN_topOff(BN *pX);
int BN_ShiftW(BN *pX, int s);
void BN_ShiftR(BN *pX, int s);
int BN_isNotBigger(const BN *pX, const BN *pY);
int BN_add(BN *pR, const BN *pY);
int BN_sub(BN *pR, const BN *pY);
int BN_sub2(const BN *pX, const BN *pY, BN *pR);
void BN_iadd(BN *pX, int M);
void BN_isub(BN *pX, int M);
void BN_divide(const BN *pX, const BN *pM, BN *pQ, BN *pR);
void BN_mult(const BN *pX, const BN *pY, BN pR[2]);
int BN_multW(const BN *pX, int W, BN *pR);
void BN_multModR(const BN *pX, const BN *pY, const BN *pM, const BN *pR, BN *pResult);
void BN_iModR(BN *pX, const BN *pM, const BN *pR);
void BN_reverse(const BN *pM, BN *pR);
void BN_MInverse(const BN *pX, const BN *pM, BN *pR);
int BN_RMTest(const BN *pX, int testBase);
void BN_MontMult(const BN *pX, const BN *pY, const BN *pM, int w, BN *pResult);
void BN_ExpMod(const BN *pX, const BN *pE, const BN *pM, BN *pResult);

void BN_Random(FRND pfRnd, BN *pR, int nKeyBytes);
void BN_Prime (FRND pfRnd, BN *pR, int nKeyBytes);

void BN_KeyGen(FRND pfRnd, int nPubExp, int nKeyBytes, int8_t *pPubKey, int8_t *pPriKey);
void BN_Encrypt(int8_t *pText, const int8_t *pPubKey, int nPubExp, int nKeyBytes);
void BN_Decrypt(int8_t *pText, const int8_t *pPubKey, const int8_t *pPriKey, int nKeyBytes);

#ifdef TEST_RSA
int DoRSATest();
#endif //TEST_RSA

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _BN_H_INCLUDED_6_14_2014_
