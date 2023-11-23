
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#include "Python.h"

#define nullptr     0

static void _Python_Cleanup(void)
{
    Py_Finalize();
    return;
}

static int _Python_Init(void)
{
    int     ret = 0;

    Py_Initialize();

    ret = Py_IsInitialized();
    if( ret == 0 )
    {
        printf("Py_IsInitialized is error.\n");
        return -1;
    }

    PyRun_SimpleString("print('Initialize Python Success from C++ program.')");

    return 0;
}

static PyObject* _Python_ImportModule(const char *pyDir, const char *name)
{
    PyObject    *pModule = 0;

    do {
        char        tempPath[256] = {};

        sprintf(tempPath, "sys.path.append('%s')", pyDir);

        PyRun_SimpleString("import sys");

        PyRun_SimpleString(tempPath);
        PyRun_SimpleString("print('Current sys.path',sys.path)");

        // import module, hello.py
        pModule = PyImport_ImportModule(name);
        if( pModule == 0 )
        {
            PyErr_Print();	// Print stack
            printf("PyImport_ImportModule 'hello.py' not found\n");
            break;
        }
    } while(0);
    return pModule;
}


static int _Call_ModuleHello_Add(PyObject *pModule, int a, int b)
{
    int     rval = 0;

    do {
        PyObject    *pDict = 0;
        PyObject    *pMethod_Add = 0;
        PyObject    *pArg = 0;
        PyObject    *pResult = 0;

        pDict = PyModule_GetDict(pModule);
        if( pDict == 0 )
        {
            PyErr_Print();
            printf("PyModule_GetDict error \n");
            rval = -1;
            break;;
        }

        // get the method of the Module by method name
        pMethod_Add = PyDict_GetItemString(pDict, "add");
        if( pMethod_Add == 0 )
        {
            printf("PyDict_GetItemString 'add' not found.\n");
            rval = -2;
            break;
        }

        // 構造python 函數入參, 接收2
        // See =>  https://docs.python.org/zh-cn/3.7/c-api/arg.html?highlight=pyarg_parse#c.PyArg_Parse

        pArg = Py_BuildValue("(i,i)", a, b);

        // Invoke function, and get python type return value
        pResult = PyEval_CallObject(pMethod_Add, pArg);

        PyArg_Parse(pResult, "i", &rval);
    } while(0);
    return rval;
}

static int _Call_ModuleHello_GetName(PyObject *pModule, char *pFirstName, char *pOut_buf, int out_buf_len)
{
    int     rval = 0;
    do {
        PyObject    *pDict = 0;
        PyObject    *pMethod_GetName = 0;
        PyObject    *pArg = 0;
        PyObject    *pResult = 0;
        char        *pStr = 0;

        pDict = PyModule_GetDict(pModule);
        if( pDict == 0 )
        {
            PyErr_Print();
            printf("PyModule_GetDict Error\n");
            rval = -__LINE__;
            break;
        }

        pMethod_GetName = PyDict_GetItemString(pDict, "get_name");
        if( pMethod_GetName == 0 )
        {
            printf("PyDict_GetItemString 'get_name' not found.\n");
            rval = -__LINE__;
            break;
        }

        pArg = Py_BuildValue("(s)", pFirstName);

        pResult = PyEval_CallObject(pMethod_GetName, pArg);
        if( pResult == 0 )
        {
            printf("Method get_name return NULL\n");
            rval = -__LINE__;
            break;
        }

        // convert PyObject to real data

        PyArg_Parse(pResult, "s", &pStr);

        snprintf(pOut_buf, out_buf_len, "%s", pStr);
    } while(0);
    return rval;
}

int main(void)
{
    PyObject    *pModule_Hello = 0;

    do {
        int     rval = 0;
        char    name[128] = {0};

        // 1. Initialize python
        if( _Python_Init() )
            break;

        PyRun_SimpleString("print('hello world')");

        pModule_Hello = _Python_ImportModule("./", "hello");
        if( pModule_Hello == 0 )
            break;

        printf("\n===== call python methods ==== \n");
        rval = _Call_ModuleHello_Add(pModule_Hello, 1, 3);
        printf("1+3 = %d\n", rval);

        _Call_ModuleHello_GetName(pModule_Hello, "Wei", (char*)&name, sizeof(name));
        printf("%s\n\n", name);

    } while(0);

    _Python_Cleanup();

    system("pause");
    return 0;
}
