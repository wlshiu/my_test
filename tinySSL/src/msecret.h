#ifndef _MSECRET_H_INCLUDED_6_28_2014_
#define _MSECRET_H_INCLUDED_6_28_2014_


#define PRE_MASTER_SECRET_LEN   48
#define MASTER_SECRET_LEN       48
#define CLIENT_RANDOM_LEN       32
#define SERVER_RANDOM_LEN       32

//Forward declarations
struct  SSL;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void CalcMasterSecret (
    int8_t       theMasterSecret[MASTER_SECRET_LEN],
    const int8_t preMasterSecret[PRE_MASTER_SECRET_LEN],
    const int8_t pClientRandom[CLIENT_RANDOM_LEN],
    const int8_t pServerRandom[SERVER_RANDOM_LEN]
);
void CalcMasterSecret1(
    int8_t       theMasterSecret[MASTER_SECRET_LEN],
    const int8_t preMasterSecret[PRE_MASTER_SECRET_LEN],
    const int8_t pClientRandom[CLIENT_RANDOM_LEN],
    const int8_t pServerRandom[SERVER_RANDOM_LEN]
);

void CalcKeysFromMaster (struct SSL *pSSL, int bIsClient);
void CalcKeysFromMaster1(struct SSL *pSSL, int bIsClient);


#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _MSECRET_H_INCLUDED_6_28_2014_
