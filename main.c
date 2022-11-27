/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/11/27
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <pthread.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static HWND         hWnd;
static UINT_PTR     timer;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static LRESULT CALLBACK
WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    switch (msg)
    {
        case WM_TIMER:
            printf("get timer event\n");
            return 0;

        case WM_KEYDOWN:
            switch( wparam )
            {
                case VK_UP:
                    printf(" key up\n");
                    return 0;

                case VK_DOWN:
                    printf(" key down\n");
                    return 0;

                case VK_LEFT:
                    printf(" key left\n");
                    return 0;
                case VK_RIGHT:
                    printf(" key right\n");
                    return 0;
                case VK_HOME:
                    printf(" key home\n");
                    return 0;
                case VK_END:
                    printf(" key end\n");
                    return 0;
                case VK_SPACE:
                    printf(" key space\n");
                    return 0;
                case VK_RETURN:
                    printf(" key return\n");
                    return 0;

                case VK_NUMPAD0:
                case VK_NUMPAD1:
                case VK_NUMPAD2:
                case VK_NUMPAD3:
                case VK_NUMPAD4:
                case VK_NUMPAD5:
                case VK_NUMPAD6:
                case VK_NUMPAD7:
                case VK_NUMPAD8:
                case VK_NUMPAD9:
                    printf(" keypad num %d\n", wparam - VK_NUMPAD0);
                    return 0;
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    return 0;

                default:
                    break;
            }
            break;

        case WM_CREATE:
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

static void* my_main_proc(void *pArgv)
{
    while( 1 )
    {
        static int  cnt = 0;
        printf("cnt = %d\n", cnt++);


        Sleep(1000);
    }

    pthread_exit(0);
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc,char **argv)
{
    WNDCLASS    wc;
    MSG         msg;
    DWORD       error = 0;

    pthread_t       task;
    pthread_attr_t  attr;

	// create window
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "My Window";

    if( RegisterClass(&wc) == FALSE )
		return -1;

    hWnd = CreateWindow("My Window",
                        "App Simulation", // Window name
                        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        240,
                        320,
                        NULL,
                        NULL,
                        wc.hInstance,
                        NULL);
    if( hWnd == NULL )
    {
        printf("create window fail !\n");
        system("pause");
        return -1;
    }


    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // create main task
	pthread_attr_init(&attr);
    pthread_create(&task, &attr, my_main_proc, NULL);

    // message loop
    while( GetMessage(&msg, NULL, 0, 0) )
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
