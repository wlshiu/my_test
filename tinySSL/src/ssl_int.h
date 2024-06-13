#ifndef _SSL_INT_H_INCLUDED_6_17_2004_
#define _SSL_INT_H_INCLUDED_6_17_2004_


//Implement secure re-negotiation per RFC5746.


#include "ssl.h"
#include "rc4.h"

#define MD5 CTX
#define SHA CTX


#define ISCLIENT    1
#define ISSERVER    0


#define SSL_VERSION_MAJOR   3
#define SSL_VERSION_MINOR   0
#define SSL_VERSION_MINOR1  1


// The following defines SSL 3.0 content types
#define CONTENT_CHANGECIPHERSPEC    0x14
#define CONTENT_ALERT               0x15
#define CONTENT_HANDSHAKE           0x16
#define CONTENT_APPLICATION_DATA    0x17


//The following defines SSL 3.0/TLS 1.0 Handshake message types
#define MSG_HELLO_REQUEST           0x00
#define MSG_CLIENT_HELLO            0x01
#define MSG_SERVER_HELLO            0x02
#define MSG_CERTIFICATE             0x0B
#define MSG_SERVER_KEY_EXCHANGE     0x0C
#define MSG_CERTIFICATE_REQUEST     0x0D
#define MSG_SERVER_HELLO_DONE       0x0E
#define MSG_CERTIFICATE_VERIFY      0x0F
#define MSG_CLIENT_KEY_EXCHANGE     0x10
#define MSG_FINISHED                0x14

//The followings are used for secured re-negotiation. See RFC5746.
#define MSG_EXTENTION               0xFF
#define MSG_EXTENTION_RENEGOTIATION 0x01

//This is only used in CONTENT_CHANGECIPHERSPEC content type
#define MSG_CHANGE_CIPHER_SPEC      0x01


//The following defines SSL 3.0/TLS 1.0 ALERT message types
//1st byte of ALERT message indicates whether it is a warning or fatal.
#define ALERT_WARNING               0x01
#define ALERT_FATAL                 0x02
//2nd byte of ALERT message indicates the nature of the alert.
#define ALERT_NOTIFY_CLOSE          0x00
#define ALERT_MESSAGE_UNEXPECTED    0x0A
#define ALERT_RECORD_MAC_BAD        0x14
#define ALERT_DECRYPTION_FAILED     0x15
#define ALERT_RECORD_OVERFLOW       0x16
#define ALERT_DECOMPRESSION_FAILED  0x1E
#define ALERT_HANDSHAKE_FAILED      0x28
#define ALERT_CERTIFICATE_BAD       0x2A
#define ALERT_CERTIFICATE_UNSUPPORTED   0x2B
#define ALERT_CERTIFICATE_REVOKED   0x2C
#define ALERT_CERTIFICATE_EXPIRED   0x2D
#define ALERT_CERTIFICATE_UNKNOWN   0x2E
#define ALERT_PARAMETER_ILLEGAL     0x2F
#define ALERT_CA_UNKNOWN            0x30
#define ALERT_ACCESS_DENIED         0x31
#define ALERT_DECODE_ERROR          0x32
#define ALERT_DECRYPT_ERROR         0x33
#define ALERT_EXPORT_RESTRICTION    0x3C
#define ALERT_PROTOCOL_VERSION      0x46
#define ALERT_SECURITY_INSUFFICIENT 0x47
#define ALERT_INTERNAL_ERROR        0x50
#define ALERT_USER_CANCELED         0x5A
#define ALERT_NO_NEGOTIATION        0x64


#define PAD1_BYTE                   0x36
#define PAD2_BYTE                   0x5C
#define PADSIZE_MD5                 0x30
#define PADSIZE_SHA                 0x28
#define MD5_SIZE                    16
#define SHA1_SIZE                   20

//Do not change these values. They are defined by SSL 3.0.
#define CLIENT_RANDOM_SIZE      32
#define SERVER_RANDOM_SIZE      32
#define MASTER_SECRET_LEN       48
#define PRE_MASTER_SECRET_LEN   48

#define MAC_SECRET_LEN          16
#define WRITE_KEY_LEN           16

#define CHALLENGE_LEN           16  //Challenge length of V.20 ClientHello
#define TLS_VERIFY_LEN          12  //Verify block length for TLS 1.0 and later.

typedef enum
{
    CIPHER_NOTSET           = 0,
    CIPHER_RSA_RC4_40_MD5   = 3,
    CIPHER_RSA_RC4_128_MD5  = 4,
    CIPHER_RSA_RC4_128_SHA  = 5
} SSL_CIPHER;

typedef struct CTX
{
    int    data[24];
} CTX_T;

typedef struct SSL
{
    SSL_STATE   eState;
    SSL_CIPHER  ePendingCipher;
    SSL_CIPHER  eClientCipher;
    SSL_CIPHER  eServerCipher;
    int    serverMsgOff;
    int    serverMsgLen;
    int    nNetOutSize;
    int    nAppOutSize;
    int    clientSequenceL;    //Low DWORD. Sequence Number is 64 bits
    int    clientSequenceH;    //High DWORD.Sequence number is 64 bits
    int    serverSequenceL;    //Low DWORD. Sequence Number is 64 bits
    int    serverSequenceH;    //High DWORD.Sequence number is 64 bits

    SSL_RESULT_T      eLastError;         //Last processing error. Used by functions
    struct CERT    *pServerCert;
    const CERTKEY_INFO_T *pCertKey;

    int    nStartTime;         //These times are UNIX TIME. i.e. number of
    int    nCurrentTime;       //seconds since EPOCH, 00:00AM 01/01/1970 UTC

    //SSL_MALLOC  pMallocFunc;
    //SSL_FREE    pFreeFunc;
    //SSL_RANDOM  pRandomFunc;
    int    nSessionIDLen;

    int8_t   sessionID[32];
    int8_t   clientRandom[CLIENT_RANDOM_SIZE];
    int8_t   serverRandom[SERVER_RANDOM_SIZE];

    int8_t   preMasterSecret[PRE_MASTER_SECRET_LEN];
    int8_t   masterSecret[MASTER_SECRET_LEN];
    int8_t   clientMacSecret[20];
    int8_t   serverMacSecret[20];
    int8_t   clientWriteKey[16];
    int8_t   serverWriteKey[16];

    RC4         clientCipher;
    RC4         serverCipher;
    CTX_T       md5Ctx;
    CTX_T       sha1Ctx;

    int8_t  *pTemp;      //These three are temporary variables
    int    nTemp1;     //To be used in any callback functions
    int    nTemp2;     //Do not rely on them being persisted.

    int8_t  *serverMsg;
    int8_t  *appoutMsg;
    int8_t   netoutMsg[8192];
    int8_t   clientVerify[36];
    int8_t   serverVerify[36];

    struct CERT    *pMidCerts;
} SSL_T;


typedef union EBLOCK
{
    int8_t       encryptBlock[256];  //Max RSA key is 2048 bits = 256 bytes
    struct
    {
        CTX_T     md5Hash;
        CTX_T     sha1Hash;
        int8_t   md5Digest[MD5_SIZE];
        int8_t   sha1Digest[SHA1_SIZE];
    };
} EBLOCK_T;


struct CIPHERSET;
typedef struct CIPHERSET CIPHERSET;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern SSL_MALLOC  gfMalloc;
extern SSL_FREE    gfFree;
extern SSL_RANDOM  gfRandom;

extern const CIPHERSET *gpCipherSet;

extern const int8_t PAD1[PADSIZE_MD5];
extern const int8_t PAD2[PADSIZE_MD5];

void DigestInit(struct SSL *pSSL);
void DigestMsg(struct SSL *pSSL, const int8_t *pMsg, int nMsgLen);

void DigestInit1(EBLOCK_T *pBlock);
void DigestInit2(const struct SSL *pSSL, EBLOCK_T *pBlock);
void DigestBlock(EBLOCK_T *pBlock);
void DigestMsg2(EBLOCK_T *pBlock, const int8_t *pMsg, int nMsgLen);
void DigestPad2(EBLOCK_T *pBlock, const int8_t *pPad);
void DigestOut2(EBLOCK_T *pBlock);

int ParseServerMsg(struct SSL *pSSL, const int8_t *pMsg, int nMsgLen);

// These functions are used internally
int ParseHandshake(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int ParseChangeCipherSpec(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int ParseAppData(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int ParseAlertMsg(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);

int ParseServerHello(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int ParseServerHelloDone(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int VerifyServerFinished(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int ParseCertificateMsg(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);
int ParseCertificateRequest(struct SSL *pSSL, const int8_t *pMsg, int nMsgSize);

int CalculateMAC(struct SSL *pSSL, int bIsClient, int8_t *pMac, int8_t cContentType, const int8_t *pMsg, int nMsgSize);
int EncryptWithMAC(struct SSL *pSSL, int bIsClient, int8_t cContentType, int8_t *pMsg, int nMsgSize);

int CreateFinishedMsg(struct SSL *pSSL, int bIsClient, int8_t *pMsgBuff, int nBuffSize);
int CreateAlertMsg(struct SSL *pSSL, int8_t cCategory, int8_t cType);


#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _SSL_INT_H_INCLUDED_6_17_2004_
