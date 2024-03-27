#include <stdio.h>
#include <stdlib.h>

#include "lru.h"

int main()
{
    lru_list_t  *pHLRU = 0;
    do {
        pHLRU = lru_create(4, 10);
        if( !pHLRU )    break;

        lru_push_node(pHLRU, 1, 11);
        lru_push_node(pHLRU, 2, 12);
        lru_push_node(pHLRU, 3, 13);
        lru_push_node(pHLRU, 1, 21);
        lru_push_node(pHLRU, 4, 14);
        lru_push_node(pHLRU, 10, 15);

        for(int i = 1; i <= 10; i++)
        {
            uint32_t    value = 0;

            lru_get_node_info(pHLRU, i, &value);

            if( value == LRU_NO_DATA )
                printf("Wrong key !!\n");
            else
                printf("key = %d, value = %d\n", i, value);
        }

    } while(0);


    lru_destroy(&pHLRU);

    system("pause");

    return 0;
}
