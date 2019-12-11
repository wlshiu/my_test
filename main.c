/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/12/06
 * @license
 * @description
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "iniparser.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_CIPHERTEXT_BLOCK_SIZE        10 // total (CONFIG_CIPHERTEXT_BLOCK_SIZE * 2) characters

#define INI_APP_SECTION                     "APP"
#define INI_ITEM_START_CODE                 "APP:start_code"
#define INI_ITEM_BUILD_DATE                 "APP:build_date"
#define INI_ITEM_CHIP_ID                    "APP:chip_device_id"
#define INI_ITEM_PROJECT_NAME               "APP:project_name"
#define INI_ITEM_SDK_VER_MAJOR              "APP:sdk_ver_major"
#define INI_ITEM_SDK_VER_MINOR              "APP:sdk_ver_minor"
#define INI_ITEM_PROJ_VER_MAJOR             "APP:project_ver_major"
#define INI_ITEM_PROJ_VER_MINOR             "APP:project_ver_minor"
#define INI_ITEM_PROJ_DESCRIPTION           "APP:app_description"

#define INI_ITEM_PRIVATE_KEY                "KEY:private_key"
#define INI_ITEM_PUBLIC_KEY                 "KEY:public_key"
#define INI_ITEM_MODULAR_NUM                "KEY:modular"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define _err(str, ...)          do { printf(str, ##__VA_ARGS__); while(1); } while(0)

#define FOURCC(a, b, c, d)      ((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((c) & 0xFF) << 8) | ((d) & 0xFF))
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct key
{
    uint64_t        modular;
    uint64_t        public_key;
    uint64_t        private_key;

} key_t;

typedef struct cypher
{
    uint32_t    data[CONFIG_CIPHERTEXT_BLOCK_SIZE];

} cypher_t;

typedef struct datetime
{
    uint32_t sec   : 5;  // 0 ~ 29 (sec >> 1)
    uint32_t min   : 6;  // 0 ~ 59
    uint32_t hour  : 5;  // 0 ~ 23
    uint32_t day   : 5;  // 1 ~ 31
    uint32_t month : 4;  // 0 ~ 11 + 1
    uint32_t year  : 7;  // 1900 + year
} datetime_t;

// section .verinfo
typedef struct ver_mgt
{
    char        tag[8];  // start code
    uint32_t    lenght;
    uint32_t    type;   // info type

    struct {
        uint64_t    key_modular;
        cypher_t    ciphertext;

        uint16_t    sdk_ver_major;
        uint16_t    sdk_ver_minor;
        datetime_t  build_date;

        char        project_name[100];
        uint16_t    project_ver_major;
        uint16_t    project_ver_minor;

        char        description[];
    } ver_info;

//    uint32_t    crc32;
} ver_mgt_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static unsigned int     g_crc_value = 0xFFFFFFFF;
static char             g_plaintext[CONFIG_CIPHERTEXT_BLOCK_SIZE << 1] = {0};
static int              g_has_decrypt_data = false;

static const unsigned int crc32_table[] =
{
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static unsigned long
_crc32(uint8_t *buf, int len, unsigned int init)
{
    unsigned int crc = init;
    while (len--)
    {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 0xFF];
        buf++;
    }
    return (crc & 0xFFFFFFFF);
}

static uint64_t
_power(uint64_t a, uint64_t k, uint64_t N)
{
    uint64_t     l_out = 1;

    while( k != 0 )
    {
        if( k & 0x01 )
        {
            l_out *= a;

            l_out = l_out % N;
        }
        a *= a;
        a = a % N;
        k = k >> 1;
    }
    return l_out;
}

static int
_encrypt(char *pMsg, key_t *pKey, cypher_t *pCiphertext)
{
    int     rval = 0;
    do {
        int     len = 0;
        if( !pMsg || ((strlen(pMsg) + 1 ) >> 1) > (sizeof(cypher_t) >> 2) )
        {
            rval = -1;
            break;
        }

        len = (strlen(pMsg) + 1) & ~0x1;

        for(int i = 0; i < len; i += 2)
        {
            uint64_t    plaintext = (*(pMsg + i) << 8) | (*(pMsg + i + 1));
            pCiphertext->data[i >> 1] = _power(plaintext, pKey->private_key, pKey->modular);
        }

    } while(0);

    return rval;
}

static int
_decrypt(key_t *pKey, cypher_t *pCiphertext, char *pMsg_buf, int msg_len)
{
    int     rval = 0;
    do {
        int     pos = 0;
        memset(pMsg_buf, 0x0, msg_len);

        for(int i = 0; i < sizeof(cypher_t) >> 2; i++)
        {
            uint64_t    plaintext = 0;

            if( !pCiphertext->data[i] )
                continue;

            plaintext = _power(pCiphertext->data[i], pKey->public_key, pKey->modular);

            pMsg_buf[pos]     = (plaintext >> 8) & 0xFF;
            pMsg_buf[pos + 1] = plaintext & 0xFF;
            pos += 2;
        }

    } while(0);
    return rval;
}

static int
_generate_verinfo_raw(char *pIni_filename, ver_mgt_t **ppVer_mgr)
{
    int         rval = -1;
    dictionary  *pIni = 0;
    ver_mgt_t   *pVer_mgr = 0;

    do {
        pIni = iniparser_load(pIni_filename);

        {
            size_t      len = 0;
            char    *pDesc = (char*)iniparser_getstring(pIni, INI_ITEM_PROJ_DESCRIPTION, NULL);

            len = sizeof(ver_mgt_t) + sizeof(uint32_t);
            if( pDesc )
                len += (strlen(pDesc) + 1);

            len = (len + 0x3) & ~0x3;
            if( !(pVer_mgr = malloc(len)) )
            {
                _err("malloc %d fail !\n", len);
                break;
            }
            memset(pVer_mgr, 0x0, len);
            memcpy(pVer_mgr->ver_info.description, pDesc, strlen(pDesc));

            // length involve all struct ver_mgt_t but no CRC32 member
            pVer_mgr->lenght = len - sizeof(uint32_t);
        }

        {
            time_t      t = time(NULL);
            struct tm   tm = *localtime(&t);

            pVer_mgr->ver_info.build_date.year  = tm.tm_year;
            pVer_mgr->ver_info.build_date.month = tm.tm_mon;
            pVer_mgr->ver_info.build_date.day   = tm.tm_mday;
            pVer_mgr->ver_info.build_date.hour  = tm.tm_hour;
            pVer_mgr->ver_info.build_date.min   = tm.tm_min;
            pVer_mgr->ver_info.build_date.sec   = (tm.tm_sec >> 1);
        }

        pVer_mgr->type   = FOURCC('v', 'i' , 'n', 'f');

        {   // encrypt
            key_t           key = {.modular = 0,};
            const char      *pChip_id = 0;

            pChip_id = iniparser_getstring(pIni, INI_ITEM_CHIP_ID, 0);
            if( !pChip_id )
            {
                _err("No Chip Id !!!\n");
                break;
            }

            key.modular     = iniparser_getint(pIni, INI_ITEM_MODULAR_NUM, 0);
            key.public_key  = iniparser_getint(pIni, INI_ITEM_PUBLIC_KEY, 0);
            key.private_key = iniparser_getint(pIni, INI_ITEM_PRIVATE_KEY, 0);
            if( !key.modular || !key.public_key || !key.private_key )
            {
                _err("No Key info !!!\n");
                break;
            }

            rval = _encrypt((char*)pChip_id, &key, &pVer_mgr->ver_info.ciphertext);
            if( rval )
            {
                _err("encrypt fail \n");
                break;
            }

            rval = _decrypt(&key, &pVer_mgr->ver_info.ciphertext, g_plaintext, sizeof(g_plaintext));
            if( rval )
            {
                _err("decrypt fail \n");
                break;
            }

            if( strncmp(pChip_id, g_plaintext, strlen(pChip_id)) )
            {
                _err("wrong key-pair\n");
                break;
            }

            pVer_mgr->ver_info.key_modular = key.modular;
        }

        snprintf(pVer_mgr->tag, sizeof(pVer_mgr->tag), "%s", iniparser_getstring(pIni, INI_ITEM_START_CODE, NULL));
        snprintf(pVer_mgr->ver_info.project_name, sizeof(pVer_mgr->ver_info.project_name),
                 "%s", iniparser_getstring(pIni, INI_ITEM_PROJECT_NAME, NULL));

        pVer_mgr->ver_info.sdk_ver_major     = iniparser_getint(pIni, INI_ITEM_SDK_VER_MAJOR, 0);
        pVer_mgr->ver_info.sdk_ver_minor     = iniparser_getint(pIni, INI_ITEM_SDK_VER_MINOR, 0);
        pVer_mgr->ver_info.project_ver_major = iniparser_getint(pIni, INI_ITEM_PROJ_VER_MAJOR, 0);
        pVer_mgr->ver_info.project_ver_minor = iniparser_getint(pIni, INI_ITEM_PROJ_VER_MINOR, 0);

        {
            uint32_t    *pCrc32 = (uint32_t*)((uintptr_t)pVer_mgr + pVer_mgr->lenght);

            g_crc_value = 0xFFFFFFFF;
            *pCrc32 = _crc32((uint8_t*)pVer_mgr, pVer_mgr->lenght, g_crc_value);
        }

        *ppVer_mgr = pVer_mgr;
        rval = 0;
    } while(0);

    if( pIni )              iniparser_freedict(pIni);
    if( !(*ppVer_mgr) )     free(pVer_mgr);

    return rval;
}

static int
_parse_verinfo_raw(char *pIni_filename, ver_mgt_t *pVer_mgr, uint64_t public_key)
{
    int         rval = -1;
    dictionary  *pIni = 0;

    do {
        uint32_t    *pCrc32 = 0;

        pCrc32      = (uint32_t*)((uintptr_t)pVer_mgr + pVer_mgr->lenght);
        g_crc_value = 0xFFFFFFFF;

        if( *pCrc32 != _crc32((uint8_t*)pVer_mgr, pVer_mgr->lenght, g_crc_value) )
        {
            _err("CRC not match !\n");
            break;
        }

        // decrypt
        if( g_has_decrypt_data == true )
        {
            key_t   key = {.modular = 0,};

            key.modular     = pVer_mgr->ver_info.key_modular;
            key.public_key  = public_key;

            memset(g_plaintext, 0x0, sizeof(g_plaintext));

            rval = _decrypt(&key, &pVer_mgr->ver_info.ciphertext, g_plaintext, sizeof(g_plaintext));
            if( rval )
            {
                _err("decrypt fail \n");
                break;
            }

            printf("Chip ID: %s\n", g_plaintext);
        }

        {   // output ini file
            char    buf[512] = {0};

            pIni = iniparser_load(pIni_filename);

            iniparser_set(pIni, INI_APP_SECTION, 0);

            snprintf(buf, sizeof(buf), "%d-%02d-%02d %02d:%02d:%02d",
                     pVer_mgr->ver_info.build_date.year + 1900,
                     pVer_mgr->ver_info.build_date.month + 1,
                     pVer_mgr->ver_info.build_date.day,
                     pVer_mgr->ver_info.build_date.hour,
                     pVer_mgr->ver_info.build_date.min,
                     pVer_mgr->ver_info.build_date.sec << 1);
            iniparser_set(pIni, INI_ITEM_BUILD_DATE, buf);

            strncpy(buf, pVer_mgr->tag, sizeof(pVer_mgr->tag));
            iniparser_set(pIni, INI_ITEM_START_CODE, buf);

            strncpy(buf, pVer_mgr->ver_info.project_name, sizeof(pVer_mgr->ver_info.project_name));
            iniparser_set(pIni, INI_ITEM_PROJECT_NAME, buf);

            snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.sdk_ver_major);
            iniparser_set(pIni, INI_ITEM_SDK_VER_MAJOR, buf);

            snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.sdk_ver_minor);
            iniparser_set(pIni, INI_ITEM_SDK_VER_MINOR, buf);

            snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.project_ver_major);
            iniparser_set(pIni, INI_ITEM_PROJ_VER_MAJOR, buf);

            snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.project_ver_minor);
            iniparser_set(pIni, INI_ITEM_PROJ_VER_MINOR, buf);

            iniparser_set(pIni, INI_ITEM_PROJ_DESCRIPTION, pVer_mgr->ver_info.description);

        }

        {
            FILE    *fini = 0;
            if( !(fini = fopen(pIni_filename, "a+w")) )
            {
                _err("open %s fail \n", pIni_filename);
                break;
            }

            iniparser_dump_ini(pIni, fini);

            fclose(fini);
        }

        rval = 0;
    } while(0);

    if( pIni )      iniparser_freedict(pIni);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
static void
usage(char *pProg)
{
    printf("usage: %s [-g/-p] [ini file] [binary file] [(optional) public key]\n"
           "    -g      generate binary data depend on [ini file]\n"
           "    -p      parse binary data and generate [init file]\n"
           "\n", pProg);

    exit(-1);
    return;
}

int main(int argc, char **argv)
{
    int         rval = 0;
    FILE        *fraw = 0;
    FILE        *fini = 0;
    uint8_t     *pRaw = 0;

    do {
        char        *pIni_filename = 0;
        char        *pRaw_filename = 0;
        ver_mgt_t   *pVer_mgr = 0;

        if( argc < 4 )
        {
            usage(argv[0]);
            break;
        }

        g_has_decrypt_data = ( argc == 5) ? true : false;

        pIni_filename = argv[2];
        pRaw_filename = argv[3];

        if( !strncmp(argv[1], "-g", strlen("-g")) )
        {
            size_t      len = 0;

            rval = _generate_verinfo_raw(pIni_filename, &pVer_mgr);
            if( rval )  break;

            pRaw = (uint8_t*)pVer_mgr;

            if( !(fraw = fopen(pRaw_filename, "wb")) )
            {
                _err("open %s fail \n", pRaw_filename);
                break;
            }

            len = fwrite((void*)pVer_mgr, 1, pVer_mgr->lenght + sizeof(uint32_t), fraw);
            if( len != pVer_mgr->lenght + sizeof(uint32_t) )
            {
                _err("output file fail !\n");
                break;
            }
        }
        else if( !strncmp(argv[1], "-p", strlen("-p")) )
        {
            uint32_t    file_size = 0;

            if( !(fraw = fopen(pRaw_filename, "rb")) )
            {
                _err("open %s fail \n", pRaw_filename);
                break;
            }

            fseek(fraw, 0l, SEEK_END);
            file_size = ftell(fraw);
            fseek(fraw, 0l, SEEK_SET);

            if( !(pRaw = malloc(file_size)) )
            {
                _err("malloc %d fail \n", file_size);
                break;
            }

            fread(pRaw, 1, file_size, fraw);
            fclose(fraw);
            fraw = 0;

            if( !(fini = fopen(pIni_filename, "w")) )
            {
                _err("open %s fail \n", pIni_filename);
                break;
            }
            fprintf(fini, "#\n# Automatically generated file;\n#\n\n");
            fclose(fini);
            fini = 0;

            pVer_mgr = (ver_mgt_t*)pRaw;
            rval = _parse_verinfo_raw(pIni_filename, pVer_mgr, atol(argv[4]));
            if( rval )      break;
        }
        else
        {
            usage(argv[0]);
        }
    } while(0);

    if( pRaw )   free(pRaw);
    if( fraw )   fclose(fraw);
    if( fini )   fclose(fini);
    return 0;
}
