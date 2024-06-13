#ifndef _ENDIAN_H_INCLUDED_6_27_2014_
#define _ENDIAN_H_INCLUDED_6_27_2014_


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void Int2Byte(const int *pIn, int8_t *pOut, int nLen);
void Byte2Int(const int8_t *pIn, int *pOut, int nLen);

void Int2LByte(const int *pIn, int8_t *pOut, int nLen);
void LByte2Int(const int8_t *pIn, int *pOut, int nLen);

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#endif //#ifndef _ENDIAN_H_INCLUDED_6_27_2014_
