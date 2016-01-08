/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file pymodule.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/01/08
 * @license
 * @description
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pymodule.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#if 1
    #define err_msg(string, args...)    do{ fprintf(stderr, "%s [#%d] => ", __FUNCTION__, __LINE__);\
                                            fprintf(stderr, string, ## args); \
                                        }while(0)

    #define dbg_msg(string, args...)    do{ fprintf(stderr, "%s [#%d] => ", __FUNCTION__, __LINE__);\
                                            fprintf(stderr, string, ## args); \
                                        }while(0)

#else
    #define err_msg(string, args...)    do{ fprintf(stderr, "%s [#%d] => ", __FUNCTION__, __LINE__);\
                                            fprintf(stderr, string, ## args); \
                                        }while(0)

    #define dbg_msg(string, args...)
#endif

#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (uint_ptr_t)&(((type *)0)->member)
#endif

#ifndef STRUCTURE_POINTER
    #define STRUCTURE_POINTER(type, ptr, member)    (type*)((uint_ptr_t)ptr - MEMBER_OFFSET(type, member))
#endif
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct pymodule_dev
{
    HPymodule_t     hPymodule;

    PyObject    *pModule;
    PyObject    *pDict;

    // PyObject    *pReturn_values;

} pymodule_dev_t;
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
pymodule_init(void)
{
    int     result = -1;

    Py_Initialize();
    if( Py_IsInitialized() )
        result = 0;

    return result;
}

int
pymodule_deinit(void)
{
    if( Py_IsInitialized() )
        Py_Finalize();
    return 0;
}

int
pymodule_load(
    char            *pModule_name,
    HPymodule_t     **ppHPymodule)
{
    int     result = 0;

    do{
        pymodule_dev_t      *pDev = 0;
        PyObject            *pObj_name = 0;

        if( !ppHPymodule || !pModule_name )
        {
            err_msg("wrong param handle=%p, name=%s!\n",
                    ppHPymodule, pModule_name);
            break;
        }

        if( !Py_IsInitialized() )
        {
            Py_Initialize();
        }

        pDev = malloc(sizeof(pymodule_dev_t));
        if( !pDev )
        {
            err_msg("malloc fail !\n");
            break;
        }
        memset(pDev, 0x0, sizeof(pymodule_dev_t));

        snprintf(pDev->hPymodule.module_name, 32, pModule_name);

        pObj_name       = PyString_FromString(pModule_name);
        pDev->pModule   = PyImport_Import(pObj_name);
        pDev->pDict     = PyModule_GetDict(pDev->pModule);

        Py_XDECREF(pObj_name);

        *ppHPymodule = &pDev->hPymodule;

    }while(0);


    return result;
}

int
pymodule_unload(
    HPymodule_t     **ppHPymodule)
{
    int     result = 0;

    do{
        pymodule_dev_t      *pDev = 0;
        if( !ppHPymodule )
            break;

        pDev = STRUCTURE_POINTER(pymodule_dev_t, (*ppHPymodule), hPymodule);

        if( pDev->pModule )     Py_XDECREF(pDev->pModule);

        free(pDev);
        *ppHPymodule = 0;

    }while(0);
    return result;
}

PyObject*
pymodule_exec(
    HPymodule_t         *pHPymodule,
    char                *pFunc_name,
    pymodule_args_t     *pInput_args)
{
    PyObject    *pReturn_values = 0;
    int         result = 0;

    do{
        int                 i;
        pymodule_dev_t      *pDev = 0;
        PyObject            *pObj_func = 0, *pArg_list = NULL;

        if( !pHPymodule || !pFunc_name || !pInput_args )
        {
            err_msg("wrong param pHPymodule=%p, pFunc_name=%s, pArgs=%p !\n",
                    pHPymodule, pFunc_name, pInput_args);
            break;
        }

        pDev = STRUCTURE_POINTER(pymodule_dev_t, (pHPymodule), hPymodule);

        pObj_func = PyDict_GetItemString(pDev->pDict, pFunc_name);

        if( !PyCallable_Check(pObj_func) )
        {
            err_msg("Can't call func:%s in module %s\n",
                    pFunc_name, pDev->hPymodule.module_name);
            result = __LINE__;
            break;
        }

        if( pInput_args->arg_count )
        {
            pArg_list = PyTuple_New(pInput_args->arg_count);
            if( !pArg_list )
            {
                PyErr_Print();
                result = __LINE__;
                break;
            }

            for(i = 0; i < pInput_args->arg_count; i++)
            {
                arg_unit_t  *pCur_arg = &pInput_args->args[i];
                PyObject    *pArg = 0;

                switch( pCur_arg->arg_type )
                {
                    case PYMODULE_ARG_LONG:
                        pArg = PyInt_FromLong(pCur_arg->u.value_long);
                        break;
                    case PYMODULE_ARG_DOUBLE:
                        pArg = PyFloat_FromDouble(pCur_arg->u.value_double);
                        break;
                    case PYMODULE_ARG_STRING:
                        pArg = PyString_FromString(pCur_arg->u.pString);
                        break;
                    case PYMODULE_ARG_PYOBJ:
                        Py_XINCREF(pCur_arg->u.pObj);
                        PyTuple_SetItem(pArg_list, i, pCur_arg->u.pObj);
                        continue;
                        break;
                    case PYMODULE_ARG_CB_FUNC:
                        pArg = PyCFunction_New(pCur_arg->u.pCb_method_def, NULL);
                        break;
                    default:
                        err_msg("unknow argument type 0x%x !!\n", pCur_arg->arg_type);
                        break;
                }

                if( !pArg )
                {
                    Py_XDECREF(pArg_list);
                    PyErr_Print();
                    result = __LINE__;
                    break;
                }

                PyTuple_SetItem(pArg_list, i, pArg);
            }
        }

        if( result == 0 )
        {
            // excute
            pReturn_values = PyObject_CallObject(pObj_func, pArg_list);
            Py_XDECREF(pArg_list);
            if( !pReturn_values )
                PyErr_Print();
        }

    }while(0);

    if( result )    err_msg("err line:%d\n", result);

    return pReturn_values;
}


int
pymodule_exec_script(
    char    *pScript)
{
    return PyRun_SimpleString(pScript);
}

int
pymodule_get_attr_string(
    PyObject    *pObj,
    char        *pAttr_name,
    char        *pOut_str,
    int         out_str_len)
{
    int     result = -1;

    do{
        PyObject    *pObj_str = 0;
        if( !PyObject_HasAttrString(pObj, pAttr_name) )
            break;

        pObj_str = PyObject_GetAttrString(pObj, pAttr_name);
        if( !PyString_Check(pObj_str) )
            break;

        snprintf(pOut_str, out_str_len, "%s", PyString_AsString(pObj_str));
        Py_XDECREF(pObj_str);
        result = 0;
    }while(0);

    return result;
}

int
pymodule_set_attr_string(
    PyObject    *pObj,
    char        *pAttr_name,
    char        *pIn_str)
{
    int         result = -1;
    PyObject    *pObj_str = 0;

    do{
        pObj_str = PyString_FromString(pIn_str);
        if( !PyObject_HasAttrString(pObj, pAttr_name) )
            break;

        if( PyObject_SetAttrString(pObj, pAttr_name, pObj_str) == -1 )
            break;

        result = 0;
    }while(0);

    if( pObj_str )      Py_XDECREF(pObj_str);

    return result;
}

int
pymodule_get_attr_long(
    PyObject    *pObj,
    char        *pAttr_name,
    long        *pValue)
{
    int     result = -1;

    do{
        PyObject    *pObj_value = 0;
        if( !PyObject_HasAttrString(pObj, pAttr_name) )
            break;

        pObj_value = PyObject_GetAttrString(pObj, pAttr_name);
        if( !PyLong_Check(pObj_value) )
            break;

        if( pValue )
            *pValue = PyLong_AsLong(pObj_value);
        Py_XDECREF(pObj_value);
        result = 0;
    }while(0);

    return result;
}

int
pymodule_set_attr_long(
    PyObject    *pObj,
    char        *pAttr_name,
    long        value)
{
    int         result = -1;
    PyObject    *pObj_value = 0;

    do{
        pObj_value = PyInt_FromLong(value);
        if( !PyObject_HasAttrString(pObj, pAttr_name) )
            break;

        if( PyObject_SetAttrString(pObj, pAttr_name, pObj_value) == -1 )
            break;

        result = 0;
    }while(0);

    if( pObj_value )      Py_XDECREF(pObj_value);

    return result;
}

int
pymodule_get_attr_double(
    PyObject    *pObj,
    char        *pAttr_name,
    double      *pValue)
{
    int     result = -1;

    do{
        PyObject    *pObj_value = 0;
        if( !PyObject_HasAttrString(pObj, pAttr_name) )
            break;

        pObj_value = PyObject_GetAttrString(pObj, pAttr_name);
        if( !PyFloat_Check(pObj_value) )
            break;

        if( pValue )
            *pValue = PyFloat_AsDouble(pObj_value);
        Py_XDECREF(pObj_value);
        result = 0;
    }while(0);

    return result;
}

int
pymodule_set_attr_double(
    PyObject    *pObj,
    char        *pAttr_name,
    double      value)
{
    int         result = -1;
    PyObject    *pObj_value = 0;

    do{
        pObj_value = PyFloat_FromDouble(value);
        if( !PyObject_HasAttrString(pObj, pAttr_name) )
            break;

        if( PyObject_SetAttrString(pObj, pAttr_name, pObj_value) == -1 )
            break;

        result = 0;
    }while(0);

    if( pObj_value )      Py_XDECREF(pObj_value);

    return result;
}
