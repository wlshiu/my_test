#ifndef _CIPHER_H_INCLUDED_6_28_2014_
#define _CIPHER_H_INCLUDED_6_28_2014_


typedef enum
{
    CIPHER_NONE,
    CIPHER_CUSTOM,
    CIPHER_RC4,
    CIPHER_MD5,
    CIPHER_SHA1,
    CIPHER_SHA256,
    CIPHER_RSA
} eCipher;


//Forward declarations

//Cipher context
struct CTX;
typedef struct CTX  CTX;

struct CDAT;
typedef struct CDAT  CDAT;


typedef void (*fInit)(CTX *pCtx, const CDAT *pData);
typedef void (*fInput)(CTX *pCtx, const int8_t *pData, int nSize);
typedef void (*fDigest)(CTX *pCtx, int8_t pDigest[]);
typedef void (*fHash)(const int8_t *pData, int nSize, int8_t pDigest[]);

typedef void (*fCode)(CTX *pCtx, int8_t *pData, int nSize);


typedef struct CIPHER
{
    int    eCipher;//Cipher type
    int    cSize;  //Context size
    int    dSize;  //Digest size
    const struct CDAT *pIData;

    fInit   Init;
    union
    {
        fInput  Input;
        fCode   Code;
    };
    fDigest Digest;
    fHash   Hash;
} CIPHER;


typedef struct CIPHERSET
{
    CIPHER  md5;
    CIPHER  sha1;
    CIPHER  sha256;
} CIPHERSET;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern CIPHERSET gCipherSet;

const CIPHERSET *InitCiphers(CIPHERSET *pCipherSet, void *pUserData);

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _CIPHER_H_INCLUDED_6_28_2014_
