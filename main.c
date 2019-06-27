#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "llist.h"


typedef struct foo
{
    int             num;
    list_head_t     lst_hdr;
} foo_t;

static DECLARE_LIST_HEAD(g_head);

int main()
{
    int             i;
    foo_t           *pNode_cur = 0;
    list_head_t     *pNode_cur_lhdr = 0, *pNode_tmp_lhdr = 0;

    printf("g_head= %p\n", &g_head);
    for(i = 0; i < 10; i++)
    {
        pNode_cur = malloc(sizeof(foo_t));

        printf("+ %p, node= %p\n", &pNode_cur->lst_hdr, pNode_cur);

        pNode_cur->num = i;

        INIT_LIST_HEAD(&pNode_cur->lst_hdr);

        if( i & 0x1)
            list_add(&pNode_cur->lst_hdr, &g_head);
        else
            list_add_tail(&pNode_cur->lst_hdr, &g_head);
    }

    pNode_cur = list_first_entry(&g_head, foo_t, lst_hdr);
    printf("first node: num = %02d\n", pNode_cur->num);

    pNode_cur = list_last_entry(&g_head, foo_t, lst_hdr);
    printf("last node: num = %02d\n", pNode_cur->num);

    list_for_each(pNode_cur_lhdr, &g_head)
    {
        pNode_cur = list_entry(pNode_cur_lhdr, foo_t, lst_hdr);
        printf("num = %02d, %p\n", pNode_cur->num, pNode_cur_lhdr);
    }

    printf("\n\n======\n");
    list_for_each_safe(pNode_cur_lhdr, pNode_tmp_lhdr, &g_head)
    {
        pNode_cur = list_entry(pNode_cur_lhdr, foo_t, lst_hdr);
        printf("num = %02d, %p\n", pNode_cur->num, pNode_cur_lhdr);
        list_del(&pNode_cur->lst_hdr);
        free(pNode_cur);
    }

    return 0;
}
