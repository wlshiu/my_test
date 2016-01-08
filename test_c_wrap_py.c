/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_c_wrap_py.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/01/08
 * @license
 * @description
 */


#include "Python.h"
#include <signal.h>
#include "pymodule.h"
#include <string.h>

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#define err(string, args...)    fprintf(stderr, "%s [#%d] => " string , __FUNCTION__, __LINE__, ## args)
#define msg(string, args...)    fprintf(stderr, "[C] " string , ## args)

#define _str(a)		#a
#define FLIGHT_MODE_INFO(x)     {x, (char*)_str(x)}
//=============================================================================
//                Structure Definition
//=============================================================================
typedef enum flight_mode
{
    INITIALIZE,
    AUTO,
    GUIDED,
    ALT_HOLD,
    LOITER,
    RTL,
    LAND,
} flight_mode_t;

struct flight_mode_def
{
    unsigned int    mode;
    char            *pName;
};
//=============================================================================
//                Global Data Definition
//=============================================================================
static struct flight_mode_def  g_flight_mode_def_table[] =
{
    FLIGHT_MODE_INFO(AUTO),
    FLIGHT_MODE_INFO(GUIDED),
    FLIGHT_MODE_INFO(ALT_HOLD),
    FLIGHT_MODE_INFO(LOITER),
    FLIGHT_MODE_INFO(RTL),
    FLIGHT_MODE_INFO(LAND),
    {(unsigned int)-1, NULL}
};

static double          g_target_longitude = 0.0, g_target_latitude = 0.0, g_target_altitude = 0.0;
//=============================================================================
//                Private Function Definition
//=============================================================================
static int
_pyobj_check(PyObject *pObj_value)
{
    if( !pObj_value )
    {
        err("null objoct !!\n");
        return -1;
    }

    if (PyInt_Check(pObj_value))
    {
        msg("get int value: %ld\n", PyInt_AsLong(pObj_value));
    }
    else if (PyLong_Check(pObj_value))
    {
        msg("get long value: %ld\n", PyLong_AsLong(pObj_value));
    }
    else if (PyFloat_Check(pObj_value))
    {
        msg("get double value: %lf\n", PyFloat_AsDouble(pObj_value));
    }
    else if (PyString_Check(pObj_value))
    {
        msg("get string value: \"%s\"\n", PyString_AsString(pObj_value));
    }
    else if( PyClass_Check(pObj_value) )
    {
        msg("get class type\n");
    }
    else if (PyTuple_Check(pObj_value))
    {
        msg("get value: tuple type -> not ready !\n");
    }
    else if (PyList_Check(pObj_value))
    {
        msg("get value: list type -> not ready !\n");
    }
    else if (PyDict_Check(pObj_value))
    {
        msg("get value: dict type -> not ready !\n");
    }
    else
        err("unknow object type !!!!\n");
    return 0;
}

static PyObject*
_report_altitude(
    PyObject    *pObj,
    PyObject    *pArgs)
{
    PyObject    *pValue = 0;

    if( !PyArg_ParseTuple(pArgs, "O", &pValue) )
        return NULL;

    _pyobj_check(pValue);

    return Py_None;
}

static PyObject*
_report_position(
    PyObject    *pObj,
    PyObject    *pArgs)
{
    int     shift_north_metres = 0, shift_east_metres = 0;
    double  remain_distance = 0.0;

    PyObject    *pObj_distance, *pObj_lat, *pObj_lon, *pObj_alt;
    if( !PyArg_ParseTuple(pArgs, "OOOO", &pObj_distance, &pObj_lat, &pObj_lon, &pObj_alt) )
    {
        PyErr_Print();
        return Py_BuildValue("ii", shift_north_metres, shift_east_metres);
    }

    g_target_latitude  = PyFloat_AsDouble(pObj_lat);
    g_target_longitude = PyFloat_AsDouble(pObj_lon);
    g_target_altitude  = PyFloat_AsDouble(pObj_alt);
    remain_distance = PyFloat_AsDouble(pObj_distance);
    msg(" report pos: distance %lf, gps: %lf, %lf, %lf\n",
        remain_distance, g_target_latitude, g_target_longitude, g_target_altitude);

    { // for test shift event trigger
        static int test_shift_event = 0;

        if( test_shift_event == 0 &&
            remain_distance < 15 )
        {
            shift_north_metres = 10;
            shift_east_metres  = 0;
            test_shift_event = 1;
        }
    }

    return Py_BuildValue("ii", shift_north_metres, shift_east_metres);
}


static PyMethodDef g_Report_Methods[] =
{
    { "report_altitude", _report_altitude, METH_VARARGS },
    { "report_position", _report_position, METH_VARARGS },
    { NULL, NULL }
};

static void*
qaudcopter_create(
    HPymodule_t     *pHPymodule,
    char            *pMaster_ip)
{
    PyObject            *pObj_vehicle = 0;
    pymodule_args_t     args = {0};

    if( !pHPymodule || !pMaster_ip )   return NULL;

    args.arg_count = 1;
    args.args[0].arg_type  = PYMODULE_ARG_STRING;
    args.args[0].u.pString = pMaster_ip;
    pObj_vehicle = pymodule_exec(pHPymodule, (char*)"quadCopter_create_handle", &args);

    return (void*)pObj_vehicle;
}

static void
qaudcopter_destroy(
    void    *pObj_vehicle)
{
    Py_XDECREF((PyObject*)pObj_vehicle);
}

static int
qaudcopter_takeoff(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    long            target_altitude_metres)
{
    pymodule_args_t     args = {0};
    PyObject            *pObj_value = 0;

    if( !pHPymodule || !pObj_vehicle )   return -1;

    args.arg_count = 3;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_LONG;
    args.args[1].u.value_long   = target_altitude_metres;
    args.args[2].arg_type       = PYMODULE_ARG_CB_FUNC;
    args.args[2].u.pCb_method_def = &g_Report_Methods[0];
    pObj_value = pymodule_exec(pHPymodule, (char*)"quadCopter_takeoff", &args);
    if( !pObj_value )   return -2;
    Py_CLEAR(pObj_value);

    return 0;
}

static int
qaudcopter_goto_relative_position(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    long            north_metres,
    long            east_metres)
{
    pymodule_args_t     args = {0};
    PyObject            *pObj_value = 0;

    if( !pHPymodule || !pObj_vehicle )   return -1;

    args.arg_count = 4;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_LONG;
    args.args[1].u.value_long   = north_metres;
    args.args[2].arg_type       = PYMODULE_ARG_LONG;
    args.args[2].u.value_long   = east_metres;
    args.args[3].arg_type       = PYMODULE_ARG_CB_FUNC;
    args.args[3].u.pCb_method_def = &g_Report_Methods[1];
    pObj_value = pymodule_exec(pHPymodule, (char*)"quadCopter_goto_relative_position", &args);
    Py_CLEAR(pObj_value);
    return 0;
}

static flight_mode_t
qaudcopter_get_flight_mode(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle)
{
    flight_mode_t       flight_mode = INITIALIZE;
    pymodule_args_t     args = {0};
    PyObject            *pObj_value = 0;

    if( !pHPymodule || !pObj_vehicle )
        return flight_mode;

    args.arg_count = 1;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    pObj_value = pymodule_exec(pHPymodule, (char*)"quadCopter_get_flight_mode", &args);
    if( PyString_Check(pObj_value) )
    {
        struct flight_mode_def  *pCur_mode_def = &g_flight_mode_def_table[0];
        char                    *pAct_mode_name = PyString_AsString(pObj_value);
        do{
            if( !pCur_mode_def->pName ||
                pCur_mode_def->mode == (unsigned int)-1 )
                break;

            if( !strcmp(pCur_mode_def->pName, pAct_mode_name) )
            {
                flight_mode = (flight_mode_t)pCur_mode_def->mode;
                break;
            }

            pCur_mode_def++;
        }while(1);
    }
    Py_CLEAR(pObj_value);
    return flight_mode;
}

static int
qaudcopter_set_flight_mode(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    flight_mode_t   mode)
{
    int                 result = -1;
    flight_mode_t       flight_mode = INITIALIZE;
    pymodule_args_t     args = {0};
    PyObject            *pObj_value = 0;
    struct flight_mode_def  *pCur_mode_def = &g_flight_mode_def_table[0];

    if( !pHPymodule || !pObj_vehicle )
        return result;

    do{
        if( !pCur_mode_def->pName ||
            pCur_mode_def->mode == (unsigned int)-1 )
            break;

        if( pCur_mode_def->mode == mode )
        {
            flight_mode = (flight_mode_t)pCur_mode_def->mode;
            break;
        }
        pCur_mode_def++;
    }while(1);

    if( flight_mode != INITIALIZE )
    {
        args.arg_count = 2;
        args.args[0].arg_type  = PYMODULE_ARG_PYOBJ;
        args.args[0].u.pObj    = (PyObject*)pObj_vehicle;
        args.args[1].arg_type  = PYMODULE_ARG_STRING;
        args.args[1].u.pString = pCur_mode_def->pName;
        pObj_value = pymodule_exec(pHPymodule, (char*)"quadCopter_set_flight_mode", &args);
        Py_CLEAR(pObj_value);
        result = 0;
    }

    return result;
}

static int
qaudcopter_get_curr_position(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    double          *pLongitude,
    double          *pLatitude,
    double          *pAltitude)
{
    int                 result = -1;
    pymodule_args_t     args = {0};
    PyObject            *pObj_values = 0;

    if( !pHPymodule || !pObj_vehicle )
        return result;

    args.arg_count = 1;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    pObj_values = pymodule_exec(pHPymodule, (char*)"quadCopter_get_curr_position", &args);

    do{
        PyObject    *pObj_lat, *pObj_lon, *pObj_alt;
        if( !PyArg_ParseTuple(pObj_values, "OOO", &pObj_lat, &pObj_lon, &pObj_alt) )
        {
            PyErr_Print();
            break;
        }

        if( pLatitude )     *pLatitude = PyFloat_AsDouble(pObj_lat);
        if( pLongitude )    *pLongitude = PyFloat_AsDouble(pObj_lon);
        if( pAltitude )     *pAltitude = PyFloat_AsDouble(pObj_alt);

        result = 0;

    }while(0);

    Py_CLEAR(pObj_values);
    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
static void
_sig_handler(int sig)
{
    msg("receive signal %d\n", sig);
    switch( sig )
    {
        default:    break;
    }

    exit(1);
}

int main()
{
    int                 ret = 0;
    HPymodule_t         *pHPymodule = 0;

    signal(SIGABRT, _sig_handler);
    signal(SIGTERM, _sig_handler);
    signal(SIGSEGV, _sig_handler);

    do{
        void            *pHCopter = 0;
        flight_mode_t   flight_mode = INITIALIZE;

        ret = pymodule_init();
        if( ret )
        {
            err("init fail !");
            break;
        }

        pymodule_load((char*)"quad_copter", &pHPymodule);

        pHCopter = qaudcopter_create(pHPymodule, (char*)"127.0.0.1:14550");
        if( !pHCopter )
            err("create coper fail !!\n");

        if( qaudcopter_takeoff(pHPymodule, pHCopter, 10) < 0 )
            err("tackoff fail !!!\n");

        {
            double x = 0.0, y = 0.0, z = 0.0;
            qaudcopter_get_curr_position(pHPymodule, pHCopter, &x, &y, &z);
            msg("****** x=%lf, y= %lf, z = %lf\n", x, y, z);
        }


        if( qaudcopter_goto_relative_position(pHPymodule, pHCopter, 0, 30) < 0 )
            err("goto fail !!\n");

        qaudcopter_set_flight_mode(pHPymodule, pHCopter, RTL);
        flight_mode = qaudcopter_get_flight_mode(pHPymodule, pHCopter);
        if( flight_mode == RTL )
            msg("set mode to RTL\n");
        else
            msg("set RTL mode fail (cur=%d) !!\n", flight_mode);

        qaudcopter_destroy(pHCopter);
        pHCopter = 0;

        pymodule_unload(&pHPymodule);
        msg("\n");
    }while(0);

    pymodule_deinit();

    return 0;
}
