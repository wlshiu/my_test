#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CONFIG_WATERMARK_PRECISION      512
#define WATERMARK_PATTERN               0x5AA5AA00

static int
_breadcrumb_init(uint8_t *pBuf, uint32_t buf_len)
{
    uint32_t    *pCur = (uint32_t*)pBuf;
    uint32_t    *pEnd = (uint32_t*)(pBuf + buf_len);
    int         cnt = 0;

    if( !pBuf || !buf_len )
        return -1;

    while( pCur < pEnd )
    {
        *pCur = WATERMARK_PATTERN | cnt++;
        pCur = (uint32_t*)((uintptr_t)pCur + CONFIG_WATERMARK_PRECISION);
    }

    return 0;
}

static int
_breadcrumb_navigate(uint8_t *pStart, uint8_t *pEnd)
{
    uint32_t    *pCur = (uint32_t*)pStart;
    int         cnt = 0;

    while( (*pCur & WATERMARK_PATTERN) == WATERMARK_PATTERN )
    {
        cnt = *pCur & 0xFF;
        pCur = (uint32_t*)((uintptr_t)pCur + CONFIG_WATERMARK_PRECISION);
        printf("cnt= %d\n", cnt);
    }

    return cnt;
}

int main()
{
    uint8_t     *pBuf = 0;
    uint32_t    buf_len = 1 << 20;

    do {
        int     idx = -1;
        pBuf = malloc(buf_len);
        memset(pBuf, 0x0, buf_len);

        _breadcrumb_init(pBuf, buf_len);

        memset(pBuf + 2048, 0x0, buf_len - 2048);

        idx = _breadcrumb_navigate(pBuf, pBuf + buf_len);
        printf("keep %d\n", idx * CONFIG_WATERMARK_PRECISION);

    } while(0);

    system("pause");

    return 0;
}
