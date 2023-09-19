/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/08/04
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "comm_dev.h"

#include "timer.h"
#include "xmodem.h"
#include <windows.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ARG_MAX_NUM          3
#define CONFIG_MAX_VAR_NUM          10

#define CONFIG_RX_TIMEOUT_SEC       8

#define NC          "\033[m"
#define RED         "\033[0;32;31m"
#define GREEN       "\033[0;32;32m"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define comm_dev_init(pCfg)                         g_pComm_dev->init(pCfg)
#define comm_dev_send(pHandle, pData, length)       g_pComm_dev->send_bytes(pHandle, pData, length)
#define comm_dev_recv(pHandle, pData, pLength)      g_pComm_dev->recv_bytes(pHandle, pData, pLength)
#define comm_dev_get_state(pHandle, stype)          g_pComm_dev->get_state(pHandle, stype)
#define comm_dev_reset_buf(pHandle, pbuf, len)      g_pComm_dev->reset_buf(pHandle, pbuf, len)
#define comm_dev_ctrl(pHandle, cmd, pExtra)         g_pComm_dev->ctrl(pHandle, cmd, pExtra)
#define comm_dev_deinit(pHandle)                    g_pComm_dev->deinit(pHandle)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct usr_argv
{
    bool        is_running;
} usr_argv_t;

typedef struct loop_var
{
    int     value;
    int     limit;
    int     increment;
    bool    is_hex;
    bool    has_limit;
    char    name[32];
} loop_var_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern comm_dev_desc_t      g_comm_dev_comport;
static comm_dev_desc_t      *g_pComm_dev = &g_comm_dev_comport;


static char                 g_line_buf[1024] = {0};
static char                 g_rx_buf[1024] = {0};
static char                 *g_pFile_name = 0;
static char                 *g_comport = 0;
static char                 *g_outpath = 0;
static uint32_t             g_baudrate = 0;
static FILE                 *g_fout = 0;
static comm_handle_t        g_hComm = 0;
static HANDLE               g_hRecv;
static bool                 g_is_rx_idle = false;

static HANDLE               g_Mutex;
static bool                 g_has_pause_comport_listening = false;
static usr_argv_t           g_usr_argv = {};
static uint32_t             g_tick_ms = 0;

static uint32_t             g_cur_file_pos = 0;

static uint32_t             g_loop_start_pos = 0;
static uint32_t             g_loop_end_pos = 0;

static char                 g_loop_init_str[100] = {0};
static char                 g_loop_cond_str[100] = {0};
static char                 g_loop_update_str[100] = {0};
static char                 g_replace_buf[200] = {0};
static loop_var_t           g_var_symbols[CONFIG_MAX_VAR_NUM];
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_get_params(int argc, char **argv)
{
    if (argc < 2) {
        return -1;
    }

    argv++; argc--;
    while(argc) {
        if (!strcmp(argv[0], "--file")) {
            g_pFile_name = argv[1];
        } else if (!strcmp(argv[0], "--outpath")) {
            argv++; argc--;
            g_outpath = argv[0];
        } else if (!strcmp(argv[0], "--comport")) {
            argv++; argc--;
            g_comport = argv[0];
        } else if (!strcmp(argv[0], "--baudrate")) {
            argv++; argc--;
            g_baudrate = atol(argv[0]);
        }
        else {

        }
        argv++; argc--;
    }

    return 0;
}

static void
_trim_leading_spaces(char* str)
{
    char    *pSrc = 0, *pDest = 0;

    pSrc = pDest = str;

    while( *pSrc == ' ' )
        pSrc++;

    if( pSrc != pDest )
    {
        uint32_t    len = strlen(str) - (int)(pSrc - pDest);
        memmove(pDest, pSrc, len);

        pDest[len] = '\0';
    }

    return;
}

static void
_trim_all_spaces(char* str)
{
    char    *dest = str;
    do {
        while( *dest == ' ' )
            ++dest;
    } while( *str++ = *dest++ );

    return;
}

static int
_strncasecmp(char *s1, char *s2, size_t n)
{
	if (n == 0)
		return 0;

	do {
		unsigned char c1 = (unsigned char) *s1++;
		unsigned char c2 = (unsigned char) *s2++;

		if (c1 != c2) {
			if (c1 >= 'A' && c1 <= 'Z' &&
			    c2 >= 'a' && c2 <= 'z')
				c1 += 'a' - 'A';
			else if (c1 >= 'a' && c1 <= 'z' &&
				 c2 >= 'A' && c2 <= 'Z')
				c2 += 'a' - 'A';
			if (c1 != c2)
				return c1 - c2;
		}
		if (c1 == 0)
			break;
	} while (--n != 0);

	return 0;
}

static uint32_t __stdcall
_comport_recv(PVOID pM)
{
    usr_argv_t      *pArgv = (usr_argv_t*)pM;
    uint32_t        cnt = 0;

    if( g_outpath )
    {
        if( !(g_fout = fopen(g_outpath, "w")) )
        {
            printf("open log file '%s' fail \n", g_outpath);
        }
    }

    while( pArgv->is_running )
    {
        int     sleep_ms = 5;
        int     length = sizeof(g_rx_buf);

        if( g_has_pause_comport_listening == true )
        {
            usleep(500);
            continue;
        }

        memset(g_rx_buf, 0x0, length);
        comm_dev_recv(g_hComm, (uint8_t*)g_rx_buf, &length);
        if( length )
        {
            WaitForSingleObject(g_Mutex,   // handle to mutex
                                INFINITE); // no time-out interval

            fprintf(stdout, "%s", g_rx_buf);
//            printf("%s", g_rx_buf);

            fflush(stdout);
            if( g_fout )
                fprintf(g_fout, "%s", g_rx_buf);

            g_is_rx_idle = false;
            cnt = 0;

            ReleaseMutex(g_Mutex);

            Sleep(10);
        }
        else
        {
            Sleep(sleep_ms);
            cnt++;

            if( sleep_ms * cnt > CONFIG_RX_TIMEOUT_SEC )
                g_is_rx_idle = true;
        }
    }

    return 0;
}

int tx_inbyte(unsigned short timeout) // msec timeout
{
    int         rval = -1;
    uint8_t     value = 0;
    int         len = 1;

    rval = comm_dev_recv(g_hComm, (uint8_t*)&value, &len);
    return (rval < 0) ? -1 : (int)value;
}

void tx_outbyte(int c)
{
    uint8_t     ch = c & 0xFF;
    comm_dev_send(g_hComm, (uint8_t*)&ch, 1);
    return;
}

static void _timer_handler(void)
{
    g_tick_ms++;
    return;
}

static char*
_str_replace(char *src, char *dst, size_t dst_size, char *search, char *replace_with)
{
    char    *replace_buf = g_replace_buf;

//    memset(g_replace_buf, 0x0, sizeof(g_replace_buf));

    if( replace_buf )
    {
        char   *p = (char *)src;
        char   *pos = 0;

        replace_buf[0] = 0;

        while( (pos = strstr(p, search)) )
        {
            size_t      n = (size_t)(pos - p);

            strncat(replace_buf, p, n > dst_size ? dst_size : n);
            strncat(replace_buf, replace_with, dst_size - strlen(replace_buf) - 1);
            p = pos + strlen(search);
        }

        strncat(replace_buf, p, strlen(p));

        snprintf(dst, dst_size, "%s", replace_buf);
    }
    return dst;
}

static int
_parse_init_express(char *pStr)
{
    int     rval = 0;
    char    *pCur = pStr;
    char    *pEnd = 0;

    pEnd = pStr + strlen(pStr);

    for(int i = 0; i < CONFIG_MAX_VAR_NUM; i++)
    {
        char    *pTmp = 0;

        if( pCur > pEnd )
            break;

        pCur = strchr(pCur, '$');
        if( !pCur ) break;

        pTmp = strchr(pCur, '=');
        if( !pTmp )
        {
            rval = -1;
            break;
        }

        *pTmp = '\0';
        snprintf(&g_var_symbols[i].name, sizeof(g_var_symbols[i].name), "%s", pCur);

        pCur = pTmp + 1;
        pTmp = strchr(pCur, ',');
        if( pTmp )
            *pTmp = '\0';

//        g_var_symbols[i].value = (*(pCur + 1) == 'x')
//                                ? strtol(pCur + 2, 0, 16)
//                                : strtol(pCur, 0, 10);

        if( (*(pCur + 1) == 'x') )
        {
            g_var_symbols[i].is_hex = true;
            g_var_symbols[i].value = strtol(pCur + 2, 0, 16);
        }
        else
        {
            g_var_symbols[i].value = strtol(pCur, 0, 10);
        }

        if( pTmp )  pCur = pTmp + 1;
        else        break;
    }

    return rval;
}

static int
_parse_cond_express(char *pStr)
{
    int     rval = 0;
    char    *pCur = pStr;
    char    *pEnd = pStr + strlen(pStr);

    while( pCur < pEnd )
    {
        bool    is_ge = false; // >=
        bool    is_le = false; // <=
        char    *pTmp = 0;

        pCur = strchr(pCur, '$');
        if( !pCur ) break;

        do {
            pTmp = strchr(pCur, '<');
            if( pTmp )
            {
                if( *(pTmp + 1) == '=' )
                    is_le = true;

                break;
            }

            pTmp = strchr(pCur, '>');
            if( pTmp )
            {
                if( *(pTmp + 1) == '=' )
                    is_ge = true;

                break;
            }

            return -1;
        } while(0);


        *pTmp = '\0';
        if( is_ge || is_le )
        {
            *(++pTmp) = '\0';
        }

        for(int i = 0; i < CONFIG_MAX_VAR_NUM; i++)
        {
            if( g_var_symbols[i].name[0] == 0 )
                break;

            if( !strncmp(pCur, g_var_symbols[i].name, strlen(g_var_symbols[i].name)) )
            {
                pCur = pTmp + 1;
                pTmp = strchr(pCur, ',');

                if( pTmp )
                    *pTmp = '\0';

                g_var_symbols[i].has_limit = true;
                g_var_symbols[i].limit = (*(pCur + 1) == 'x')
                                        ? strtol(pCur + 2, 0, 16)
                                        : strtol(pCur, 0, 10);

                if( is_ge == true )
                    g_var_symbols[i].limit--;
                else if( is_le == true )
                    g_var_symbols[i].limit++;

                break;
            }
        }

        if( pTmp )  pCur = pTmp + 1;
        else        break;
    }

    return rval;
}

static int
_parse_update_express(char *pStr)
{
    int     rval = 0;
    char    *pCur = pStr;
    char    *pEnd = pStr + strlen(pStr);

    while( pCur < pEnd )
    {
        bool    is_minus = false; // -=
        char    *pTmp = 0;

        pCur = strchr(pCur, '$');
        if( !pCur ) break;

        pTmp = strstr(pCur, "+=");
        if( !pTmp )
        {
            pTmp = strstr(pCur, "-=");
            if( !pTmp )
            {
                rval = -1;
                break;
            }

            is_minus = true;
        }

        *pTmp++ = '\0';
        *pTmp = '\0';

        for(int i = 0; i < CONFIG_MAX_VAR_NUM; i++)
        {
            if( g_var_symbols[i].name[0] == 0 )
                break;

            if( !strncmp(pCur, g_var_symbols[i].name, strlen(g_var_symbols[i].name)) )
            {
                pCur = pTmp + 1;
                pTmp = strchr(pCur, ',');
                if( pTmp )
                    *pTmp = '\0';

                g_var_symbols[i].increment = (*(pCur + 1) == 'x')
                                            ? strtol(pCur + 2, 0, 16)
                                            : strtol(pCur, 0, 10);
                if( is_minus == true )
                    g_var_symbols[i].increment *= (-1);

                break;
            }
        }

        if( pTmp )  pCur = pTmp + 1;
        else        break;
    }
    return rval;
}

static int
_parse_loop_line(char *pLine_buf)
{
    int     rval = 0;

    do {
        char    *pCur = pLine_buf;
        char    *pEnd = 0;

        pCur = &pLine_buf[strlen("loop ")];
        pEnd = strchr(pCur, ';');
        if( !pEnd || (pEnd - pCur) > sizeof(g_loop_init_str) - 1 )
        {
            printf("loop expression is too long \n");
            break;
        }

        memset(g_loop_init_str, 0x0, sizeof(g_loop_init_str));
        memcpy(g_loop_init_str, pCur, (pEnd - pCur));
        _trim_all_spaces(g_loop_init_str);

        pCur = pEnd + 1;
        pEnd = strchr(pCur, ';');
        if( !pEnd || (pEnd - pCur) > sizeof(g_loop_init_str) - 1 )
        {
            printf("loop expression is too long \n");
            break;
        }
        memset(g_loop_cond_str, 0x0, sizeof(g_loop_cond_str));
        memcpy(g_loop_cond_str, pCur, (pEnd - pCur));
        _trim_all_spaces(g_loop_cond_str);

        pCur = pEnd + 1;
        if( strlen(pCur) > sizeof(g_loop_update_str) - 1 )
        {
            printf("loop expression is too long \n");
            break;
        }
        memset(g_loop_update_str, 0x0, sizeof(g_loop_update_str));
        memcpy(g_loop_update_str, pCur, strlen(pCur));
        _trim_all_spaces(g_loop_update_str);

        rval = _parse_init_express(g_loop_init_str);
        if( rval )  break;

        rval = _parse_cond_express(g_loop_cond_str);
        if( rval )  break;

        rval = _parse_update_express(g_loop_update_str);
        if( rval )  break;

    } while(0);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void usage(char *pProg)
{
    printf("%s [option] [value]\n"
           "  --file        script file\n"
           "  --comport     COM port name\n"
           "  --baudrate    target baudrate\n"
           "  --outpath     save log\n"
           "\n", pProg);

    system("pause");
    exit(-1);
}

int main(int argc, char **argv)
{
    int     rval = 0;
    FILE    *fin = 0;

    do {
        comm_cfg_t  com_cft = {};

        g_outpath = 0;
        memset((void*)&g_var_symbols, 0x0, sizeof(g_var_symbols));

        rval = _get_params(argc, argv);
        if( rval )
        {
            usage(argv[0]);
            break;
        }

        g_Mutex = CreateMutex(NULL,  // default security attributes
                              FALSE, // initially not owned
                              NULL); // unnamed mutex
        if( g_Mutex == NULL )
        {
            printf("CreateMutex() error %u\n", (unsigned int)GetLastError());
            break;
        }

        if( !(fin = fopen(g_pFile_name, "r")) )
        {
            printf("open %s fail !\n", g_pFile_name);
            break;
        }

        com_cft.comport.pComport_name = g_comport;
        com_cft.comport.baudrate      = g_baudrate;
        g_hComm = comm_dev_init(&com_cft);
        if( !g_hComm )
        {
            printf("open COM Port (%s) fail \n", com_cft.comport.pComport_name);
            break;
        }

        comm_dev_ctrl(g_hComm, COMM_CMD_NORMAL_TX, 0);

        printf("\n\n=========================\n");

        g_usr_argv.is_running = true;
        g_hRecv = CreateThread(NULL, 0 /* use default stack size */,
                               (LPTHREAD_START_ROUTINE)_comport_recv,
                               (void*)&g_usr_argv, // argument to thread function
                               0,          // use default creation flags
                               NULL);      // returns the th
        if( g_hRecv == NULL )
        {
            printf("CreateThread fail\n");
            break;
        }

        g_tick_ms = 0;
        if( timer_start(1, &_timer_handler) )
	    {
	        printf("\n timer error\n");
	        break;
	    }

        g_loop_start_pos = ~0u;
        g_loop_end_pos = ~0u;

        g_cur_file_pos = 0;

        while( fgets(g_line_buf, sizeof(g_line_buf), fin) != NULL )
        {
            char    ch = 0;

            g_cur_file_pos += strlen(g_line_buf);

            _trim_leading_spaces(g_line_buf);

            #if 0
            WaitForSingleObject(g_Mutex, INFINITE);

            printf(GREEN "%s" NC, g_line_buf);

            ReleaseMutex(g_Mutex);
            #endif

            ch = g_line_buf[0];
            if( ch == ';' || ch == '//' || ch == '#' ||
                ch == '\r' || ch == '\n'  )
                continue;

//            printf(g_line_buf);

            g_has_pause_comport_listening = false;

            if( g_loop_start_pos == ~0u &&
                !_strncasecmp(g_line_buf, "loop ", strlen("loop ")) )
            {
                g_loop_start_pos = g_cur_file_pos;
                if( _parse_loop_line(g_line_buf) )
                    break;

                continue;
            }


            if( !_strncasecmp(g_line_buf, "endloop", strlen("endloop")) )
            {
                g_loop_end_pos = g_cur_file_pos;

                // check loop condition
                for(int i = 0; i < CONFIG_MAX_VAR_NUM; i++)
                {
                    loop_var_t      *pVar_cur = &g_var_symbols[i];

                    pVar_cur->value += pVar_cur->increment;
                    if( pVar_cur->has_limit == true &&
                        (pVar_cur->increment > 0 &&
                         pVar_cur->value >= pVar_cur->limit) ||
                        (pVar_cur->increment < 0 &&
                         pVar_cur->value <= pVar_cur->limit) )
                    {
                        g_loop_start_pos = ~0u;
                        break;
                    }
                }

                if( g_loop_start_pos != ~0u )
                {
                    fseek(fin, g_loop_start_pos, SEEK_SET);
                    g_cur_file_pos = g_loop_start_pos;
                }

                continue;
            }

            if( g_loop_start_pos != ~0u )
            {
                char    value_str[40] = {0};

                for(int i = 0; i < CONFIG_MAX_VAR_NUM; i++)
                {
                    if( g_var_symbols[i].name[0] == 0 )
                        break;

                    if( g_var_symbols[i].is_hex )
                        snprintf(value_str, sizeof(value_str), "0x%X", g_var_symbols[i].value);
                    else
                        snprintf(value_str, sizeof(value_str), "%d", g_var_symbols[i].value);

                    _str_replace(g_line_buf, g_line_buf, sizeof(g_line_buf), g_var_symbols[i].name, value_str);
                }
            }

            if( !_strncasecmp(g_line_buf, "xmodemsend ", strlen("xmodemsend ")) )
            {
                char    *pCur = &g_line_buf[strlen("send")];
                char    *pTmp = 0;

                _trim_leading_spaces(pCur);

                pTmp = pCur;
                while( 1 )
                {
                    if( *pTmp == '/' )
                        *pTmp = '\\';

                    if( *pTmp == ' ' || *pTmp == '\n' )
                    {
                        *pTmp = 0;
                        break;
                    }

                    pTmp++;
                }

                {
                    FILE        *fdata = 0;
                    int         file_size = 0;
                    int         nbytes = 0;
                    uint8_t     *pBuf = 0;

                    if( !(fdata = fopen(pCur, "rb")) )
                    {
                        printf("Open %s fail !\n", pCur);
                        break;
                    }

                    fseek(fdata, 0, SEEK_END);
                    file_size = ftell(fdata);
                    fseek(fdata, 0, SEEK_SET);

                    if( !(pBuf = malloc(file_size + sizeof(uint32_t))) )
                    {
                        printf("allocate %d bytes fail !\n", file_size);
                        if( fdata )     fclose(fdata);
                        break;
                    }

                    // XmodemSend
                    g_has_pause_comport_listening = true;

                    nbytes = (int)xmodemTransmit(pBuf, file_size);
                    if( nbytes < 0 )
                        printf("xmodem send fail (err: %d)\n", nbytes);
                    else
                        printf("xmodem send %d bytes\n", nbytes);

                    if( pBuf )  free(pBuf);

                    g_has_pause_comport_listening = false;
                }

                continue;
            }

            if( !_strncasecmp(g_line_buf, "send", strlen("send")) )
            {
                char    *pCur = &g_line_buf[strlen("send")];
                char    *pTmp = 0;

                _trim_leading_spaces(pCur);

                pTmp = pCur;
                while( 1 )
                {
                    if( *pTmp == '/' )
                        *pTmp = '\\';

                    if( *pTmp == ' ' || *pTmp == '\n' )
                    {
                        *pTmp = 0;
                        break;
                    }

                    pTmp++;
                }

                {
                    FILE        *fdata = 0;
                    int         file_size = 0;
                    int         cur_idx = 0;
                    uint8_t     *pBuf = 0;
                    uint32_t    start_ms = 0;

                    if( !(fdata = fopen(pCur, "rb")) )
                    {
                        fprintf(stdout, "Open %s fail !\n", pCur);
                        fflush(stdout);
                        break;
                    }

                    fseek(fdata, 0, SEEK_END);
                    file_size = ftell(fdata);
                    fseek(fdata, 0, SEEK_SET);

                    file_size = (file_size + 0x3) & ~0x3;   // 4-align

                    if( !(pBuf = malloc(file_size + sizeof(uint32_t))) )
                    {
                        fprintf(stdout, "allocate %d bytes fail !\n", file_size);
                        fflush(stdout);

                        if( fdata )     fclose(fdata);
                        break;
                    }

                    memset(pBuf, 0xFF, file_size);
                    fread(pBuf, 1, file_size, fdata);
                    fclose(fdata);
                    fdata = 0;

                    pBuf[file_size + 0] = 0xA5;
                    pBuf[file_size + 1] = 0xA5;
                    pBuf[file_size + 2] = 0x5A;
                    pBuf[file_size + 3] = 0x5A;

                    file_size += sizeof(uint32_t);
                    cur_idx = 0;

                    comm_dev_ctrl(g_hComm, COMM_CMD_FORCE_TX, 0);

                    start_ms = g_tick_ms;

                    while( cur_idx < file_size )
                    {
                        comm_dev_send(g_hComm, (uint8_t*)&pBuf[cur_idx], 4);
                        cur_idx += sizeof(uint32_t);

                        #if 0
                        /**
                         *  In baud rate 115200,  8 ~ 9 bytes spend 800 useconds.
                         */
                        usleep(800);
                        #else
//                        usleep(20); // for flash program
//                        for(int i = 100000; i != 0; i--);
                        #endif
                    }

                    comm_dev_ctrl(g_hComm, COMM_CMD_NORMAL_TX, 0);
                    fprintf(stdout, "spent %d ms\n", g_tick_ms - start_ms);
                    fflush(stdout);

                    if( pBuf )  free(pBuf);
                }

                continue;
            }

            if( !_strncasecmp(g_line_buf, "break", strlen("break")) ||
                !_strncasecmp(g_line_buf, "exit", strlen("exit")) ||
                !_strncasecmp(g_line_buf, "return", strlen("return")) )
            {
                break;
            }

            if( !_strncasecmp(g_line_buf, "echo", strlen("echo"))  )
            {
                char        *pCur = &g_line_buf[strlen("echo")];
                char        *pEnd = g_line_buf + strlen(g_line_buf);

                while( pCur < pEnd )
                {
                    if( *pCur == '\"' || *pCur == '\'' )
                    {
                        pCur++;
                        continue;
                    }
                    else if( strncmp(pCur, "\n", strlen("\n")) == 0 )
                    {
                        putchar('\n');
                        pCur += 2;
                        continue;
                    }

                    (*pCur);
                    pCur++;
                }

                continue;
            }

            if( !_strncasecmp(g_line_buf, "wait", strlen("wait")) ||
                !_strncasecmp(g_line_buf, "delay", strlen("delay")) )
            {
                int         arg_cnt = 0;
                char        *pCmd_args[CONFIG_ARG_MAX_NUM] = {0};
                char        *pCur = g_line_buf;
                char        *pEnd = g_line_buf + strlen(g_line_buf);
                uint32_t    is_arg_head = 0;

                is_arg_head = 1;
                while( pCur < pEnd )
                {
                    char    c = *pCur;

                    if( c == ' ' || c == '\t' || c == '\n' )
                    {
                        *pCur++ = '\0';
                        is_arg_head = 1;
                        continue;
                    }

                    if( is_arg_head && arg_cnt < CONFIG_ARG_MAX_NUM &&
                        (((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') || c == '_' || c == '?') || c == '-') )
                    {
                        pCmd_args[arg_cnt++] = pCur;
                        is_arg_head = 0;
                    }

                    pCur++;
                }

                Sleep(atol(pCmd_args[1]));

                continue;
            }

        #if 0 // for debug
            printf("%s", g_line_buf);
        #else
            comm_dev_send(g_hComm, (uint8_t*)g_line_buf, strlen(g_line_buf));
        #endif
            memset(g_line_buf, 0x0, sizeof(g_line_buf));

            usleep(1000);
        }
    } while(0);

    if( fin )   fclose(fin);

//    getch();

    while( g_is_rx_idle == false )
        Sleep(1);

    g_usr_argv.is_running = false;

    // Wait for the thread to finish.
//    WaitForSingleObject(g_hRecv, INFINITE);

    Sleep(200);

    if( g_fout )    fclose(g_fout);

    // Close the thread handle and free memory allocation.
    CloseHandle(g_hRecv);

    comm_dev_deinit(g_hComm);
    g_hComm = 0;

    return 0;
}
