#ifndef ITP_ARPA_INET_H
#define ITP_ARPA_INET_H

#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t    in_addr_t;


#ifndef ENOTSUP
#define ENOTSUP         129
#endif

/* Extension defined as by report VC 10+ defines error-numbers.  */

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT 102
#endif

#ifndef EADDRINUSE
#define EADDRINUSE 100
#endif

#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL 101
#endif

#ifndef EISCONN
#define EISCONN 113
#endif

#ifndef ENOBUFS
#define ENOBUFS 119
#endif

#ifndef ECONNABORTED
#define ECONNABORTED 106
#endif

#ifndef EALREADY
#define EALREADY 103
#endif

#ifndef ECONNREFUSED
#define ECONNREFUSED 107
#endif

#ifndef ECONNRESET
#define ECONNRESET 108
#endif

#ifndef EDESTADDRREQ
#define EDESTADDRREQ 109
#endif

#ifndef EHOSTUNREACH
#define EHOSTUNREACH 110
#endif

#ifndef EMSGSIZE
#define EMSGSIZE 115
#endif

#ifndef ENETDOWN
#define ENETDOWN 116
#endif

#ifndef ENETRESET
#define ENETRESET 117
#endif

#ifndef ENETUNREACH
#define ENETUNREACH 118
#endif

#ifndef ENOPROTOOPT
#define ENOPROTOOPT 123
#endif

#ifndef ENOTSOCK
#define ENOTSOCK 128
#endif

#ifndef ENOTCONN
#define ENOTCONN 126
#endif

#ifndef ECANCELED
#define ECANCELED 105
#endif

#ifndef EINPROGRESS
#define EINPROGRESS 112
#endif

#ifndef EOPNOTSUPP
#define EOPNOTSUPP 130
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK 140
#endif

#ifndef EOWNERDEAD
#define EOWNERDEAD 133
#endif

#ifndef EPROTO
#define EPROTO 134
#endif

#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT 135
#endif

#ifndef EBADMSG
#define EBADMSG 104
#endif

#ifndef EIDRM
#define EIDRM 111
#endif

#ifndef ENODATA
#define ENODATA 120
#endif

#ifndef ENOLINK
#define ENOLINK 121
#endif

#ifndef ENOMSG
#define ENOMSG 122
#endif

#ifndef ENOSR
#define ENOSR 124
#endif

#ifndef ENOSTR
#define ENOSTR 125
#endif

#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE 127
#endif

#ifndef ETIME
#define ETIME 137
#endif

#ifndef ETXTBSY
#define ETXTBSY 139
#endif

/* Defined as WSAETIMEDOUT.  */
#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif

#ifndef ELOOP
#define ELOOP 114
#endif

#ifndef EPROTOTYPE
#define EPROTOTYPE 136
#endif

#ifndef EOVERFLOW
#define EOVERFLOW 132
#endif


static inline int hexval(unsigned c)
{
    if (c - '0' < 10) return c - '0';
    c |= 32;
    if (c - 'a' < 6) return c - 'a' + 10;
    return -1;
}

static inline int inet_pton(int af, const char *restrict s, void *restrict a0)
{
    uint16_t ip[8];
    unsigned char *a = a0;
    int i, j, v, d, brk = -1, need_v4 = 0;

    if (af == AF_INET)
    {
        for (i = 0; i < 4; i++)
        {
            for (v = j = 0; j < 3 && isdigit(s[j]); j++)
                v = 10 * v + s[j] - '0';
            if (j == 0 || (j > 1 && s[0] == '0') || v > 255)
                return 0;
            a[i] = v;
            if (s[j] == 0 && i == 3)
                return 1;
            if (s[j] != '.')
                return 0;
            s += j + 1;
        }
        return 0;
    }
    else if (af != AF_INET6)
    {
        errno = EAFNOSUPPORT;
        return -1;
    }

    if (*s == ':' && *++s != ':')
        return 0;

    for (i = 0; ; i++)
    {
        if (s[0] == ':' && brk < 0)
        {
            brk = i;
            ip[i & 7] = 0;
            if (!*++s)
                break;
            if (i == 7)
                return 0;
            continue;
        }
        for (v = j = 0; j < 4 && (d = hexval(s[j])) >= 0; j++)
            v = 16 * v + d;
        if (j == 0)
            return 0;
        ip[i & 7] = v;
        if (!s[j] && (brk >= 0 || i == 7))
            break;
        if (i == 7)
            return 0;
        if (s[j] != ':')
        {
            if (s[j] != '.' || (i < 6 && brk < 0))
                return 0;
            need_v4 = 1;
            i++;
            break;
        }
        s += j + 1;
    }
    if (brk >= 0)
    {
        memmove(ip + brk + 7 - i, ip + brk, 2 * (i + 1 - brk));
        for (j = 0; j < 7 - i; j++)
            ip[brk + j] = 0;
    }
    for (j = 0; j < 8; j++)
    {
        *a++ = ip[j] >> 8;
        *a++ = ip[j];
    }
    if (need_v4 && inet_pton(AF_INET, (void *)s, a - 4) <= 0)
        return 0;
    return 1;
}


// const char   *inet_ntop(int af, const void *src, char *dst, socklen_t size);
static inline const char* inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
    const u_char *p = (const u_char *) addrptr;
    if(family == AF_INET)
    {
        char temp[INET_ADDRSTRLEN];
        snprintf(temp, sizeof(temp), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        if (strlen(temp) >= len)
        {
            errno = ENOSPC;
            return (NULL);
        }
        strcpy(strptr, temp);
        return (strptr);
    }
    errno = EAFNOSUPPORT;
    return (NULL);
}

#ifdef __cplusplus
}
#endif

#endif // ITP_ARPA_INET_H
