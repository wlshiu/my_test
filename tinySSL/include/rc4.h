#ifndef _RC4_H_INCLUDED_6_27_2014_
#define _RC4_H_INCLUDED_6_27_2014_


//#define TEST_RC4    //Define this to perform algorithm consistency check.


typedef struct RC4
{
    int8_t state[256];
    int8_t x;
    int8_t y;
} RC4;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifdef TEST_RC4
int rc4Test();
#endif //TEST_RC4

void RC4Init(RC4 *pCtx, const int8_t *pKey, int nKeyLen);
void RC4Code(RC4 *pCtx, int8_t *pData, int nDataLen);

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _RC4_H_INCLUDED_6_27_2014_
