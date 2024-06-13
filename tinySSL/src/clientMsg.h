#ifndef _CLIENTMSG_H_INCLUDED_6_17_2004_
#define _CLIENTMSG_H_INCLUDED_6_17_2004_


//Forward declaration
struct  SSL;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int CreateClientHelloMsg(
    struct SSL *pSSL,
    int8_t  *pMsgBuff,
    int   nBuffSize
);

int CreateClientKeyExchangeMsg(
    struct SSL *pSSL,
    int8_t  *pMsgBuff,
    int   nBuffSize
);

int CreateChangeCipherSpecMsg(
    struct SSL *pSSL,
    int8_t  *pMsgBuff,
    int   nBuffSize
);

int CreateClientFinishedMsg(
    struct SSL *pSSL,
    int8_t  *pMsgBuff,
    int   nBuffSize
);

int VerifyServerMAC(
    struct SSL     *pSSL,
    int8_t       cMsgType,   //Content Type. e.g., CONTENT_HANDSHAKE
    const int8_t    *pMsg,
    int  *pSize
);


#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _CLIENTMSG_H_INCLUDED_6_17_2004_
