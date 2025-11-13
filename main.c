#define UNICODE
#define _UNICODE


#include <windows.h>
#include <tchar.h>

#undef _tmain
#ifdef _UNICODE
#define _tmain wmain
#else
#define _tmain main
#endif

#ifdef __cplusplus
extern "C"
#endif

void __wgetmainargs(int *, wchar_t ***, wchar_t ***, int, int *);

int wmain(int argc, wchar_t *argv[]);

int _tmain(int argc, TCHAR *argv[])
{
    for(int i = 0; i < argc; i++)
    {
        /**
         *  Use 'cmder' terminal to execute this program
         *  ```
         *  λ .\test_unicode_cli.exe 123 "測試" ab9Q
         *  ```
         *  ps. The terminal of Code::Blcoks is not support unicode
         */
        MessageBox(NULL, argv[i], _T("__wgetmainargs()"), MB_OK);
    }
    return 0;
}

#ifdef _UNICODE
int main()
{
    wchar_t **argv;
    wchar_t **enpv;
    int argc = 0, si = 0;
    __wgetmainargs(&argc, &argv, &enpv, 1, &si);
    return wmain(argc, argv);
}
#endif
