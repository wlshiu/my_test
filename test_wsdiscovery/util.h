#ifndef __util_H_wmADoZYh_lMIB_HULj_sLXJ_uXQQpeh2v6bL__
#define __util_H_wmADoZYh_lMIB_HULj_sLXJ_uXQQpeh2v6bL__

#ifdef __cplusplus
extern "C" {
#endif


#include "soapStub.h"
#include "soapH.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 * error message
 */
#ifdef _MSC_VER // WIN32
    #define err_msg(string, ...)                do{ printf(string, __VA_ARGS__); \
                                                    printf("  %s[#%d]\n", __FUNCTION__, __LINE__); \
                                                }while(0)
#else /* _MSC_VER */
    #define err_msg(string, args...)            do{ printf(string, ## args); \
                                                    printf("  %s[#%d]\n", __FUNCTION__, __LINE__); \
                                                }while(0)
#endif /* _MSC_VER */

#if 1 //def ENABLE_TRACE_MSG
    #define enter_trace()                      printf("%s\n",__FUNCTION__)
#else
    #define enter_trace()
#endif

/**
 * handle command not support
 */
#define ACT_NOT_SUPPORT(faultsubcodeQName)                                                  \
    do{ int     result = SOAP_OK;                                                           \
        enter_trace();                                                                      \
        result = soap_receiver_fault_subcode(soap, "ter:ActionNotSupported", NULL, NULL);   \
        if( faultsubcodeQName )                                                             \
            result = soap_receiver_fault_subcode(soap, faultsubcodeQName, NULL, NULL);      \
        return result;                                                                      \
    }while(0)

/**
 * handle onvif SPEC error report
 */
#define RECEIVER_FAULT(ret, err_str, sub_err_str)                              \
    do{ soap_receiver_fault_subcode(soap, err_str, NULL, NULL);                \
        ret = soap_receiver_fault_subcode(soap, sub_err_str, NULL, NULL);      \
        err_msg(" err, %s-> %s !!", err_str, sub_err_str);                     \
    }while(0)

#define SENDER_FAULT(ret, err_str, sub_err_str)                              \
    do{ soap_sender_fault_subcode(soap, err_str, NULL, NULL);                \
        ret = soap_sender_fault_subcode(soap, sub_err_str, NULL, NULL);      \
        err_msg(" err, %s-> %s !!", err_str, sub_err_str);                   \
    }while(0)
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
