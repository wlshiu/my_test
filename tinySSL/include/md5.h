#ifndef _MD5_H_INCLUDED_10_1_2004_
#define _MD5_H_INCLUDED_10_1_2004_


#define TEST_MD5     //Define this to test MD5 against the test suite


#define MD5_DATA    64
#define MD5_SIZE    16

//The MD5 Context
typedef struct MD5
{
    int  ints[16];             // MD5 data buffer in 4 byte integers
    int  state[MD5_SIZE >> 2]; // state (ABCD)
    int  countLo, countHi;     // 64-bit byte count. Must Lo first Hi second
} MD5;

struct CDAT;
struct CIPHER;
typedef struct CDAT     CDAT;
typedef struct CIPHER   CIPHER;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifdef TEST_MD5
int md5Test();
#endif //TEST_MD5

const CDAT *Md5Cd();
void SetMd5(CIPHER *pCipher);

void Md5Init  (MD5 *pMD5, const CDAT *pIData);
void Md5Input (MD5 *pMD5, const int8_t *pData, int nSize);
void Md5Digest(const MD5 *pMD5, int8_t pDigest[MD5_SIZE]);
void Md5Hash(const int8_t *pData, int nSize, int8_t pDigest[MD5_SIZE]);

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _MD5_H_INCLUDED_10_1_2004_
