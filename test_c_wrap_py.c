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

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
//=============================================================================
//                Constant Definition
//=============================================================================
#define QAUDCOPTER_MAV_CMD_WAYPOINT             16
#define QAUDCOPTER_MAV_CMD_RETURN_TO_LAUNCH     20
#define QAUDCOPTER_MAV_CMD_TACKOFF              22
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



/**
 *  a waypoint info
 */
typedef struct waypoint
{
    int     seq;
    int     current;
    int     mav_frame;
    int     mav_cmd;
    double  x;
    double  y;
    double  z;
    int     autocontinue;
} waypoint_t;
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
        msg("get value: list type (size =%d)-> not ready !\n", PyList_Size(pObj_value));
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
    PyObject    *self,
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
    PyObject    *self,
    PyObject    *pArgs)
{
    int     shift_north_metres = 0, shift_east_metres = 0, shift_rise_metres = 0;
    double  remain_distance = 0.0;

    PyObject    *pObj_distance, *pObj_lat, *pObj_lon, *pObj_alt;
    if( !PyArg_ParseTuple(pArgs, "OOOO", &pObj_distance, &pObj_lat, &pObj_lon, &pObj_alt) )
    {
        PyErr_Print();
        return Py_BuildValue("iii", shift_north_metres, shift_east_metres, shift_rise_metres);
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
            shift_east_metres  = 5;
            shift_rise_metres  = 6;
            test_shift_event = 1;
        }
    }

    return Py_BuildValue("iii", shift_north_metres, shift_east_metres, shift_rise_metres);
}

static PyObject*
_report_waypoint(
    PyObject    *self,
    PyObject    *pArgs)
{
    PyObject    *pObj_wp = 0, *pObj_tmp;

    if( !PyArg_ParseTuple(pArgs, "O", &pObj_wp) )
        return NULL;

    do{
        if( !PyList_Check(pObj_wp) )
        {
            PyErr_Print();
            break;
        }

        printf("wp: ");
        pObj_tmp = PyList_GetItem(pObj_wp, 0);
        if( PyInt_Check(pObj_tmp) )
            printf("seq: %ld, ", PyInt_AsLong(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 1);
        if( PyInt_Check(pObj_tmp) )
            printf("cur: %ld, ", PyInt_AsLong(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 2);
        if( PyInt_Check(pObj_tmp) )
            printf("mav_frame: %ld, ", PyInt_AsLong(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 3);
        if( PyInt_Check(pObj_tmp) )
            printf("mav_cmd: %ld, ", PyInt_AsLong(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 4);
        if( PyFloat_Check(pObj_tmp) )
            printf("x: %lf, ", PyFloat_AsDouble(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 5);
        if( PyFloat_Check(pObj_tmp) )
            printf("y: %lf, ", PyFloat_AsDouble(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 6);
        if( PyFloat_Check(pObj_tmp) )
            printf("z: %lf, ", PyFloat_AsDouble(pObj_tmp));

        pObj_tmp = PyList_GetItem(pObj_wp, 7);
        if( PyInt_Check(pObj_tmp) )
            printf("autocontinue: %ld, ", PyInt_AsLong(pObj_tmp));

    }while(0);

    return Py_None;
}

static PyObject*
_report_mission(
    PyObject    *self,
    PyObject    *pArgs)
{
    int     shift_north_metres = 0, shift_east_metres = 0, shift_rise_metres = 0;
    int     cur_wp_idx = -1;
    double  remain_distance = 0.0, cur_longitude = 0.0, cur_latitude = 0.0, cur_altitude = 0.0;

    PyObject    *pObj_distance, *pObj_cur_idx, *pObj_lat, *pObj_lon, *pObj_alt;
    if( !PyArg_ParseTuple(pArgs, "OOOOO", &pObj_distance, &pObj_cur_idx, &pObj_lat, &pObj_lon, &pObj_alt) )
    {
        PyErr_Print();
        return Py_BuildValue("iii", shift_north_metres, shift_east_metres, shift_rise_metres);
    }

    cur_wp_idx         = PyInt_AsLong(pObj_cur_idx);
    cur_latitude  = PyFloat_AsDouble(pObj_lat);
    cur_longitude = PyFloat_AsDouble(pObj_lon);
    cur_altitude  = PyFloat_AsDouble(pObj_alt);
    remain_distance = PyFloat_AsDouble(pObj_distance);
    msg(" report pos: %d-th wp, distance %lf, cur gps: %lf, %lf, %lf\n",
        cur_wp_idx, remain_distance, cur_latitude, cur_longitude, cur_altitude);

    { // for test shift event trigger
        static int g_test_shift_event = 0;
        static struct timeval g_start_time = {0};

        struct timeval cur_time = {0};

        if( g_start_time.tv_sec == 0 && g_start_time.tv_usec == 0 )
            gettimeofday(&g_start_time, NULL);

        gettimeofday(&cur_time, NULL);
        printf("==> druation: %ld sec\n", cur_time.tv_sec - g_start_time.tv_sec);

        if( g_test_shift_event == 0 &&
            (cur_time.tv_sec - g_start_time.tv_sec) > 6 /* sec */)
        {
            shift_north_metres = 10;
            shift_east_metres  = 5;
            shift_rise_metres  = 6;
            g_test_shift_event = 1;
        }
    }

    return Py_BuildValue("iii", shift_north_metres, shift_east_metres, shift_rise_metres);
}

static PyMethodDef g_Report_Methods[] =
{
    { "report_altitude", _report_altitude, METH_VARARGS },
    { "report_position", _report_position, METH_VARARGS },
    { "report_waypoint", _report_waypoint, METH_VARARGS },
    { "report_mission", _report_mission, METH_VARARGS },
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
    long            east_metres,
    long            rise_metres)
{
    pymodule_args_t     args = {0};
    PyObject            *pObj_value = 0;

    if( !pHPymodule || !pObj_vehicle )   return -1;

    args.arg_count = 5;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_LONG;
    args.args[1].u.value_long   = north_metres;
    args.args[2].arg_type       = PYMODULE_ARG_LONG;
    args.args[2].u.value_long   = east_metres;
    args.args[3].arg_type       = PYMODULE_ARG_LONG;
    args.args[3].u.value_long   = rise_metres;
    args.args[4].arg_type       = PYMODULE_ARG_CB_FUNC;
    args.args[4].u.pCb_method_def = &g_Report_Methods[1];
    pObj_value = pymodule_exec(pHPymodule, (char*)"quadCopter_goto_relative_position", &args);
    Py_CLEAR(pObj_value);
    return 0;
}

static int
qaudcopter_goto_gps_position(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    double          longitude,
    double          latitude,
    double          altitude)
{
    pymodule_args_t     args = {0};
    PyObject            *pObj_value = 0;

    if( !pHPymodule || !pObj_vehicle )   return -1;

    args.arg_count = 5;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_DOUBLE;
    args.args[1].u.value_double = latitude;
    args.args[2].arg_type       = PYMODULE_ARG_DOUBLE;
    args.args[2].u.value_double = longitude;
    args.args[3].arg_type       = PYMODULE_ARG_DOUBLE;
    args.args[3].u.value_double = altitude;
    args.args[4].arg_type       = PYMODULE_ARG_CB_FUNC;
    args.args[4].u.pCb_method_def = &g_Report_Methods[1];
    pObj_value = pymodule_exec(pHPymodule, (char*)"quadCopter_goto_gps_position", &args);
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

    if( result )    err("get error !!\n");
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

    if( result )    err("get error !!\n");
    return result;
}

static int
qaudcopter_set_yaw(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    int             target_yaw)
{
    int                 result = -1;
    pymodule_args_t     args = {0};
    PyObject            *pObj_values = 0;

    if( !pHPymodule || !pObj_vehicle )
        return result;

    args.arg_count = 3;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_LONG;
    args.args[1].u.value_long   = target_yaw;
    args.args[2].arg_type       = PYMODULE_ARG_LONG;
    args.args[2].u.value_long   = 0;
    pObj_values = pymodule_exec(pHPymodule, (char*)"quadCopter_set_yaw", &args);
    if( pObj_values )
        result = 0;

    Py_CLEAR(pObj_values);

    if( result )    err("get error !!\n");
    return result;
}

static int
qaudcopter_clear_mission(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle)
{
    int                 result = -1;
    pymodule_args_t     args = {0};
    PyObject            *pObj_values = 0;

    if( !pHPymodule || !pObj_vehicle )
        return result;

    args.arg_count = 1;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    pObj_values = pymodule_exec(pHPymodule, (char*)"quadCopter_clear_mission", &args);
    if( pObj_values )
        result = 0;

    Py_CLEAR(pObj_values);

    if( result )    err("get error !!\n");
    return result;
}

static int
qaudcopter_add_one_waypoint(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    long            north_metres,
    long            east_metres,
    long            rise_metres)
{
    int                 result = -1;
    pymodule_args_t     args = {0};
    PyObject            *pObj_values = 0;

    if( !pHPymodule || !pObj_vehicle )
        return result;

    args.arg_count = 4;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_LONG;
    args.args[1].u.value_long   = north_metres;
    args.args[2].arg_type       = PYMODULE_ARG_LONG;
    args.args[2].u.value_long   = east_metres;
    args.args[3].arg_type       = PYMODULE_ARG_LONG;
    args.args[3].u.value_long   = rise_metres;
    pObj_values = pymodule_exec(pHPymodule, (char*)"quadCopter_add_relative_waypoint", &args);
    if( pObj_values )
        result = 0;

    Py_CLEAR(pObj_values);

    if( result )    err("get error !!\n");
    return result;
}

/**
 *  pWaypoints MUST be released by user
 */
static int
qaudcopter_download_curr_mission(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle,
    waypoint_t      **ppWaypoints,
    int             *pWaypoint_cnt)
{
    int                 result = -1;
    PyObject            *pObj_values = 0;
    waypoint_t          *pWaypoints = 0;

    if( !pHPymodule || !pObj_vehicle ||
        !ppWaypoints || !pWaypoint_cnt )
        return result;

    do{
        int                 i, num_wp = 0;
        pymodule_args_t     args = {0};

        args.arg_count = 1;
        args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
        args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
        args.args[1].arg_type       = PYMODULE_ARG_CB_FUNC;
        args.args[1].u.pCb_method_def = &g_Report_Methods[2];
        pObj_values = pymodule_exec(pHPymodule, (char*)"quadCopter_download_curr_mission", &args);
        if( pObj_values )
        {

            if( !PyList_Check(pObj_values) )
            {
                PyErr_Print();
                break;
            }

            if( !(num_wp = PyList_Size(pObj_values)) )
            {
                err("no waypoints !!\n");
                break;
            }

            if( !(pWaypoints = (waypoint_t*)malloc(num_wp*sizeof(waypoint_t))) )
            {
                err("malloc wp list fail !!\n");
                break;
            }
            memset(pWaypoints, 0xFF, num_wp*sizeof(waypoint_t));

            for(i = 0; i < num_wp; i++)
            {
                waypoint_t      *pCur_wp = &pWaypoints[i];
                PyObject        *pObj_wp = 0, *pObj_tmp;
                pObj_wp = PyList_GetItem(pObj_values, i);
                if( !PyList_Check(pObj_wp) )
                {
                    err("obj_wp is wrong type !!\n");
                    break;
                }

                pObj_tmp = PyList_GetItem(pObj_wp, 0);
                if( PyInt_Check(pObj_tmp) )
                    pCur_wp->seq = PyInt_AsLong(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 1);
                if( PyInt_Check(pObj_tmp) )
                    pCur_wp->current = PyInt_AsLong(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 2);
                if( PyInt_Check(pObj_tmp) )
                    pCur_wp->mav_frame = PyInt_AsLong(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 3);
                if( PyInt_Check(pObj_tmp) )
                    pCur_wp->mav_cmd = PyInt_AsLong(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 4);
                if( PyFloat_Check(pObj_tmp) )
                    pCur_wp->x = PyFloat_AsDouble(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 5);
                if( PyFloat_Check(pObj_tmp) )
                    pCur_wp->y = PyFloat_AsDouble(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 6);
                if( PyFloat_Check(pObj_tmp) )
                    pCur_wp->z = PyFloat_AsDouble(pObj_tmp);

                pObj_tmp = PyList_GetItem(pObj_wp, 7);
                if( PyInt_Check(pObj_tmp) )
                    pCur_wp->autocontinue = PyInt_AsLong(pObj_tmp);
            }

            *pWaypoint_cnt = num_wp;
            *ppWaypoints   = pWaypoints;
            result = 0;
        }
    }while(0);

    Py_CLEAR(pObj_values);

    if( result )
    {
        if( pWaypoints )    free(pWaypoints);

        err("get error !!\n");
    }
    return result;
}

static int
qaudcopter_launch_monitor_mission(
    HPymodule_t     *pHPymodule,
    void            *pObj_vehicle)
{
    int                 result = -1;
    pymodule_args_t     args = {0};
    PyObject            *pObj_values = 0;

    if( !pHPymodule || !pObj_vehicle )
        return result;

    args.arg_count = 2;
    args.args[0].arg_type       = PYMODULE_ARG_PYOBJ;
    args.args[0].u.pObj         = (PyObject*)pObj_vehicle;
    args.args[1].arg_type       = PYMODULE_ARG_CB_FUNC;
    args.args[1].u.pCb_method_def = &g_Report_Methods[3];
    pObj_values = pymodule_exec(pHPymodule, (char*)"qaudcopter_launch_monitor_mission", &args);
    if( pObj_values )
        result = 0;

    Py_CLEAR(pObj_values);

    if( result )    err("get error !!\n");
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

static int
test_guid_mode(
    HPymodule_t     *pHPymodule,
    void            *pHCopter)
{
    srand(time (NULL));

    if( qaudcopter_takeoff(pHPymodule, pHCopter, 10) < 0 )
        err("tackoff fail !!!\n");

#if 1
    if( qaudcopter_goto_relative_position(pHPymodule, pHCopter, 20, 30, 5) < 0 )
        err("goto fail !!\n");
#else
    {
        double x = 0.0, y = 0.0, z = 0.0;
        qaudcopter_get_curr_position(pHPymodule, pHCopter, &x, &y, &z);
        msg("****** x=%lf, y= %lf, z = %lf\n", x, y, z);

        if( qaudcopter_goto_gps_position(pHPymodule, pHCopter, x, y + 0.0003, z + 10.0) < 0 )
            err("goto gps fail !!\n");
    }
#endif

    if( qaudcopter_set_yaw(pHPymodule, pHCopter, (rand() % 360)) < 0 )
        err("set yaw fail !\n");

    return 0;
}

static int
test_auto_mode(
    HPymodule_t     *pHPymodule,
    void            *pHCopter)
{
    int             i;
    flight_mode_t   flight_mode = INITIALIZE;

    qaudcopter_clear_mission(pHPymodule, pHCopter);

    qaudcopter_add_one_waypoint(pHPymodule, pHCopter, 10, -10, 10);
    qaudcopter_add_one_waypoint(pHPymodule, pHCopter, 10, 10, 11);
    qaudcopter_add_one_waypoint(pHPymodule, pHCopter, -10, 10, 12);
    qaudcopter_add_one_waypoint(pHPymodule, pHCopter, -10, -10, 13);
    // dummy point
    qaudcopter_add_one_waypoint(pHPymodule, pHCopter, -10, -10, 13);

    {
        int             waypoint_cnt = 0;
        waypoint_t      *pWaypoints = 0;
        qaudcopter_download_curr_mission(pHPymodule, pHCopter, &pWaypoints, &waypoint_cnt);
        if( pWaypoints && waypoint_cnt )
        {
            for(i = 0; i < waypoint_cnt; i++)
            {
                waypoint_t      *pCur_wp = &pWaypoints[i];
                printf("%2d, %3d, %3d, %3d, %4.6lf, %4.6lf, %4.6lf, %2d\n",
                    pCur_wp->seq, pCur_wp->current, pCur_wp->mav_frame, pCur_wp->mav_cmd,
                    pCur_wp->x, pCur_wp->y, pCur_wp->z,
                    pCur_wp->autocontinue);
            }
            free(pWaypoints);
        }
    }

    if( qaudcopter_takeoff(pHPymodule, pHCopter, 10) < 0 )
        err("tackoff fail !!!\n");

    qaudcopter_set_flight_mode(pHPymodule, pHCopter, AUTO);
    flight_mode = qaudcopter_get_flight_mode(pHPymodule, pHCopter);
    i = 0;
    while(1)
    {
        if( flight_mode == g_flight_mode_def_table[i].mode )
        {
            msg("set mode: %s\n", g_flight_mode_def_table[i].pName);
            break;
        }

        if( (int)g_flight_mode_def_table[i].mode == -1 )
        {
            msg("set mode fail !\n");
            break;
        }

        i++;
    }

    qaudcopter_launch_monitor_mission(pHPymodule, pHCopter);

    return 0;
}

int main(int argc, char **argv)
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
#if 0
        pymodule_exec_script((char*)"import sys\n"
                             "sys.path.append('/home/dronekit_wrapper/')\n");
#else
        pymodule_exec_script((char*)"import sys\n"
                             "sys.path.append('./home/dronekit_wrapper/')\n");
#endif
        pymodule_load((char*)"quad_copter", &pHPymodule);

        pHCopter = qaudcopter_create(pHPymodule, argv[1]);
        if( !pHCopter )
            err("create coper fail !!\n");

        if( !strcmp(argv[2], "auto") )
            test_auto_mode(pHPymodule, pHCopter);
        else if( !strcmp(argv[2], "guid") )
            test_guid_mode(pHPymodule, pHCopter);

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


