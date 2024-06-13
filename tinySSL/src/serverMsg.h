#ifndef _SERVERMSG_H_INCLUDED_6_28_2014_
#define _SERVERMSG_H_INCLUDED_6_28_2014_


//forward declarations
struct SSL;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int CreateServerHelloMsg(
    struct SSL *pSSL,
    int8_t  *pMsgBuff,
    int   nBuffSize
);

int CreateHelloRequestMsg(
    struct SSL *pSSL,
    int8_t      *pMsgBuff,
    int        nBuffSize
);

int ParseClientKeyExchange(
    struct SSL     *pSSL,
    const int8_t    *pMsg,
    int            nMsgSize
);

int ParseCertificateVerify(
    struct SSL     *pSSL,
    const int8_t    *pMsg,
    int            nMsgSize
);

int ParseClientChangeCipherSpec(
    struct SSL     *pSSL,
    const int8_t    *pMsg,
    int            nMsgSize
);

int VerifyClientMAC(
    struct SSL     *pSSL,
    int8_t           cMsgType,   //Content Type. e.g., CONTENT_HANDSHAKE
    const int8_t    *pMsg,
    int           *pSize
);


int CreateServerChangeCipherMsg(
    struct SSL *pSSL,
    int8_t  *pMsgBuff,
    int   nBuffSize
);


int CreateServerFinishedMsg(
    struct SSL *pSSL,
    int8_t      *pMsgBuff,
    int        nBuffSize
);


#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _SERVERMSG_H_INCLUDED_6_28_2014_
