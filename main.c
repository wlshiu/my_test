/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file buffer.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/12/06
 * @license
 * @description
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "iniparser.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
//=============================================================================
//                  Macro Definition
//=============================================================================
#if 1
#define _err(str, ...)          do { printf(str, ##__VA_ARGS__); while(1); } while(0)
#else
#define _err(str, ...)          do { printf(str, ##__VA_ARGS__); exit(-1); } while(0)
#endif // 1
//=============================================================================
//                  Structure Definition
//=============================================================================
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
    uint32_t    type;  // info type

    struct {
        union {
            uint32_t  number; // 5478
            char      uid_tag[8];
        } chip_id;

        uint64_t    ciphertext;  // encrypt with pubic key
        uint16_t    sdk_ver_major;
        uint16_t    sdk_ver_minor;
        datetime_t  build_date;
        char        project_name[80];
        uint16_t    project_ver_major;
        uint16_t    project_ver_minor;
        char        description[];
    } ver_info;

    uint32_t    crc32;
} ver_mgt_t;


//=============================================================================
//                  Global Data Definition
//=============================================================================
static ver_mgt_t        g_ver_mgr = { .crc32 = 0, };
static unsigned int     g_crc_value = 0xFFFFFFFF;

static const unsigned int crc32_table[] =
{
	0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
	0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
	0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
	0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
	0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
	0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
	0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
	0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
	0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
	0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
	0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
	0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
	0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
	0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
	0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
	0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
	0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
	0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
	0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
	0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
	0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
	0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
	0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
	0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
	0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
	0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
	0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
	0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
	0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
	0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
	0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
	0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
	0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
	0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
	0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
	0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
	0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
	0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
	0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
	0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
	0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
	0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
	0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
	0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
	0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
	0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
	0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
	0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
	0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
	0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
	0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
	0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
	0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
	0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
	0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
	0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
	0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
	0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
	0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
	0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
	0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
	0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
	0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
	0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static unsigned long
_crc32(const unsigned char *buf, int len, unsigned int init)
{
    unsigned int crc = init;
    while (len--)
    {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 0xFF];
        buf++;
    }
    return (crc & 0xFFFFFFFF);
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
static void
usage(char *pProg)
{
    printf("usage: %s [-g/-p] [ini file] [binary file]\n"
           "    -g      generate binary data depend on [ini file]\n"
           "    -p      parse binary data and generate [init file]\n"
           "\n", pProg);

    exit(-1);
    return;
}

int main(int argc, char **argv)
{
    dictionary  *pIni = 0;
    FILE        *fraw = 0;
    FILE        *fini = 0;
    uint8_t     *pRaw = 0;

    do {
        char        *pIni_filename = 0;
        char        *pRaw_filename = 0;
        ver_mgt_t   *pVer_mgr = 0;

        if( argc != 4 )
        {
            usage(argv[0]);
            break;
        }

        pIni_filename = argv[2];
        pRaw_filename = argv[3];

        if( !strncmp(argv[1], "-g", strlen("-g")) )
        {
            size_t      len = 0;

            pIni = iniparser_load(pIni_filename);

            {
                char    *pDesc = iniparser_getstring(pIni, "APP:app_description", NULL);

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
            }

            {
                time_t      t = time(NULL);
                struct tm   tm = *localtime(&t);

                printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                pVer_mgr->ver_info.build_date.year  = tm.tm_year;
                pVer_mgr->ver_info.build_date.month = tm.tm_mon;
                pVer_mgr->ver_info.build_date.day   = tm.tm_mday;
                pVer_mgr->ver_info.build_date.hour  = tm.tm_hour;
                pVer_mgr->ver_info.build_date.min   = tm.tm_min;
                pVer_mgr->ver_info.build_date.sec   = (tm.tm_sec >> 1);
            }

            // length involve all struct ver_mgt_t but no CRC32 member
            pVer_mgr->lenght = len - sizeof(uint32_t);

            snprintf(pVer_mgr->tag, sizeof(pVer_mgr->tag), "%s", iniparser_getstring(pIni, "APP:start_code", NULL));
            snprintf(pVer_mgr->ver_info.chip_id.uid_tag, sizeof(pVer_mgr->ver_info.chip_id.uid_tag), "%s", iniparser_getstring(pIni, "APP:chip_device_tag", NULL));
            snprintf(pVer_mgr->ver_info.project_name, sizeof(pVer_mgr->ver_info.project_name), "%s", iniparser_getstring(pIni, "APP:project_name", NULL));

            pVer_mgr->ver_info.sdk_ver_major     = iniparser_getint(pIni, "APP:sdk_ver_major", 0);
            pVer_mgr->ver_info.sdk_ver_minor     = iniparser_getint(pIni, "APP:sdk_ver_minor", 0);
            pVer_mgr->ver_info.project_ver_major = iniparser_getint(pIni, "APP:project_ver_major", 0);
            pVer_mgr->ver_info.project_ver_minor = iniparser_getint(pIni, "APP:project_ver_minor", 0);

            {
                uint32_t    *pCrc32 = (uint32_t*)((uintptr_t)pVer_mgr + pVer_mgr->lenght);
                *pCrc32 = _crc32((const unsigned char*)pVer_mgr, pVer_mgr->lenght, g_crc_value);
            }

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
            uint32_t    *pCrc32 = 0;

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

            pVer_mgr = (ver_mgt_t*)pRaw;
            pCrc32 = (uint32_t*)(pRaw + pVer_mgr->lenght);

            if( *pCrc32 != _crc32(pRaw, pVer_mgr->lenght, g_crc_value) )
            {
                _err("CRC not match !\n");
                break;
            }

            {   // output ini file
                char    buf[512] = {0};

                if( !(fini = fopen(pIni_filename, "w")) )
                {
                    _err("open %s fail \n", pIni_filename);
                    break;
                }
                fprintf(fini, "#\n# Automatically generated file;\n#\n\n");
                fclose(fini);
                fini = 0;

                pIni = iniparser_load(pIni_filename);

                iniparser_set(pIni, "APP", 0);

                snprintf(buf, sizeof(buf), "%d-%02d-%02d %02d:%02d:%02d",
                         pVer_mgr->ver_info.build_date.year + 1900,
                         pVer_mgr->ver_info.build_date.month + 1,
                         pVer_mgr->ver_info.build_date.day,
                         pVer_mgr->ver_info.build_date.hour,
                         pVer_mgr->ver_info.build_date.min,
                         pVer_mgr->ver_info.build_date.sec << 1);
                iniparser_set(pIni, "APP:build_date", buf);

                strncpy(buf, pVer_mgr->tag, sizeof(pVer_mgr->tag));
                iniparser_set(pIni, "APP:start_code", buf);

                strncpy(buf, pVer_mgr->ver_info.chip_id.uid_tag, sizeof(pVer_mgr->ver_info.chip_id.uid_tag));
                iniparser_set(pIni, "APP:chip_device_tag", buf);

                strncpy(buf, pVer_mgr->ver_info.project_name, sizeof(pVer_mgr->ver_info.project_name));
                iniparser_set(pIni, "APP:project_name", buf);

                snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.sdk_ver_major);
                iniparser_set(pIni, "APP:sdk_ver_major", buf);

                snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.sdk_ver_minor);
                iniparser_set(pIni, "APP:sdk_ver_minor", buf);

                snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.project_ver_major);
                iniparser_set(pIni, "APP:project_ver_major", buf);

                snprintf(buf, sizeof(buf), "%d", pVer_mgr->ver_info.project_ver_minor);
                iniparser_set(pIni, "APP:project_ver_minor", buf);

                iniparser_set(pIni, "APP:app_description", pVer_mgr->ver_info.description);

                if( !(fini = fopen(pIni_filename, "a+w")) )
                {
                    _err("open %s fail \n", pIni_filename);
                    break;
                }

                iniparser_dump_ini(pIni, fini);
            }
        }
        else
        {
            usage(argv[0]);
        }
    } while(0);

    if( pRaw )   free(pRaw);
    if( fraw )   fclose(fraw);
    if( fini )   fclose(fini);
    if( pIni )   iniparser_freedict(pIni);

    printf("------- done\n");
    while(1)
        __asm("nop");

    return 0;
}
