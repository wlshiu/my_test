/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file pymodule.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/01/08
 * @license
 * @description
 */

#ifndef __pymodule_H_wds6ixBl_lvN8_HkFe_s0DB_uQkr6zG7WgNu__
#define __pymodule_H_wds6ixBl_lvN8_HkFe_s0DB_uQkr6zG7WgNu__

#ifdef __cplusplus
extern "C" {
#endif


#include "Python.h"
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum pymodule_arg_type
{
    PYMODULE_ARG_LONG,
    PYMODULE_ARG_DOUBLE,
    PYMODULE_ARG_STRING,
    PYMODULE_ARG_PYOBJ,
    PYMODULE_ARG_CB_FUNC,

} pymodule_arg_type_t;
//=============================================================================
//                Macro Definition
//=============================================================================
#if defined(OS6BITS)
    typedef unsigned __int64 uint_ptr_t;
#else
    typedef unsigned int uint_ptr_t;
#endif

//=============================================================================
//                Structure Definition
//=============================================================================

/**
 *  the simplest argument unit
 */
typedef struct arg_unit
{
    pymodule_arg_type_t     arg_type;
    union {
        long        value_long;
        double      value_double;
        char        *pString;
        PyObject    *pObj;
        PyMethodDef *pCb_method_def;
    } u;
} arg_unit_t;

/**
 *  arguments of method in a py module
 */
typedef struct pymodule_args
{
    int         arg_count;
    arg_unit_t  args[8];

} pymodule_args_t;


/**
 *  handle of py module
 */
typedef struct HPymodule
{
    char        module_name[32];

} HPymodule_t;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
int
pymodule_init(void);


int
pymodule_deinit(void);


int
pymodule_load(
    char            *pModule_name,
    HPymodule_t     **ppHPymodule);


int
pymodule_unload(
    HPymodule_t     **ppHPymodule);


/**
 *  return PyObject and It MUST destroy by self
 *  e.g. Py_DECREF(pValue);
 */
PyObject*
pymodule_exec(
    HPymodule_t         *pHPymodule,
    char                *pFunc_name,
    pymodule_args_t     *pInput_args);


int
pymodule_exec_script(char *pScript);


int
pymodule_get_attr_string(
    PyObject    *pObj,
    char        *pAttr_name,
    char        *pOut_str,
    int         out_str_len);


int
pymodule_set_attr_string(
    PyObject    *pObj,
    char        *pAttr_name,
    char        *pIn_str);


int
pymodule_get_attr_long(
    PyObject    *pObj,
    char        *pAttr_name,
    long        *pValue);


int
pymodule_set_attr_long(
    PyObject    *pObj,
    char        *pAttr_name,
    long        value);


int
pymodule_get_attr_double(
    PyObject    *pObj,
    char        *pAttr_name,
    double      *pValue);


int
pymodule_set_attr_double(
    PyObject    *pObj,
    char        *pAttr_name,
    double      value);


#ifdef __cplusplus
}
#endif

#endif

