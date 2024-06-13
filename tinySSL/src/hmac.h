#ifndef _HMAC_H_INCLUDED_6_28_2014_
#define _HMAC_H_INCLUDED_6_28_2014_


#define OPAD_CHAR   0x5C
#define IPAD_CHAR   0x36

#define BLOCK_LEN   64  //Block length for HMAC calculation


typedef struct VDATA
{
    const int8_t    *pData;
    int            nSize;
} VDATA_T;

typedef struct HMAC
{
    CTX_T     md5a, md5b;
    CTX_T     sha1a, sha1b;
} HMAC_T;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void HMAC_InitMD5 (HMAC_T *pHMAC, int8_t hashBlock[BLOCK_LEN], const VDATA_T *pKeyBlock);
void HMAC_InitSHA1(HMAC_T *pHMAC, int8_t hashBlock[BLOCK_LEN], const VDATA_T *pKeyBlock);
void HMAC_MD5 (HMAC_T *pHMAC, int8_t md5Hash[MD5_SIZE], const VDATA_T *pDataBlocks);
void HMAC_SHA1(HMAC_T *pHMAC, int8_t shaHash[SHA1_SIZE], const VDATA_T *pDataBlocks);

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _HMAC_H_INCLUDED_6_28_2014_
