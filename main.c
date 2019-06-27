
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "ring_frame_buf_opt.h"
#include "time.h"

typedef struct frame_info
{
    uint32_t        order;

    uint32_t        frame_start_offset;
    uint32_t        frame_size;
    uint32_t        item_size;

    //uint8_t         *pNext_item_start;
}frame_info_t;

///////////////////////////////////////////////
int _get_item_size(uint8_t *w_ptr, uint8_t *r_ptr, uint32_t *pItem_size)
{
    frame_info_t    *pFrame_info = (frame_info_t*)r_ptr;

    *pItem_size = pFrame_info->item_size;
    return 0;
}

int main()
{
    int             i;
    uint32_t        frame_buf_size = (15 << 10);
    uint8_t         *pFrame_buf = 0;
    uint32_t        item_header_size = 0;
    FILE            *fp_w = 0, *fp_r = 0, *fp_r1 = 0;

    static rb_operator_t   rb_opt = {0};

    srand((unsigned int)time(NULL));

    if( !(pFrame_buf = malloc(frame_buf_size)) )
    {
        printf("malloc fail %u\n", __LINE__);
        return 0;
    }

    memset(pFrame_buf, 0xa, frame_buf_size);

    rb_opt_init(&rb_opt, pFrame_buf, frame_buf_size);

    item_header_size = sizeof(frame_info_t);

    fp_w = fopen("w_info.txt", "wb");
    fp_r = fopen("r_info.txt", "wb");
    fp_r1 = fopen("r1_info.txt", "wb");


    for(i = 1; i < 3000000; i++)
    {
    	int			rst = 0;
        uint8_t     *pItem_buf = 0, *pItem_addr = 0;
        uint32_t    item_size = 0;
        uint32_t    tmp_buf_size = 0;

        do{
			tmp_buf_size = (rand() >> 3) & 0xfff;
        }while( tmp_buf_size == 0 );

        pItem_buf = malloc(tmp_buf_size);
        memset(pItem_buf, (i & 0xF), tmp_buf_size);

        if( rand() & 0x10 )
        {
        	static int		cnt = 0;
            frame_info_t    frm_info = {0};

            frm_info.order              = cnt++;
            frm_info.frame_start_offset = item_header_size;
            frm_info.frame_size         = tmp_buf_size;
            frm_info.item_size          = tmp_buf_size + item_header_size;

			printf(" w: order=%u, item_size=%u, frame_size=%u, data=0x%x \n",
				frm_info.order,
				frm_info.item_size,
				frm_info.frame_size,
				(i & 0xF));

			rst = rb_opt_update_w(&rb_opt,
								  (uint8_t*)&frm_info, item_header_size,
								  pItem_buf, frm_info.frame_size);
			if( rst == 0 )
			{
				fprintf(fp_w, "order=%u, item_size=%u, frame_size=%u, data=0x%x \n",
					frm_info.order,
					frm_info.item_size,
					frm_info.frame_size,
					(i & 0xF));
			}
        }
        else
        {
            frame_info_t    *pFrame_info = 0;

            rb_opt_update_r(&rb_opt, RB_READ_TYPE_PEEK, &pItem_addr, &item_size, _get_item_size);

            pFrame_info = (frame_info_t*)pItem_addr;
            if( pFrame_info )
            {
                printf(" R: order=%u, item_size=%u, frame_size=%u, data=0x%x \n",
                    pFrame_info->order,
                    pFrame_info->item_size,
                    pFrame_info->frame_size,
                    *(pItem_addr + pFrame_info->frame_start_offset) & 0xF);
                fprintf(fp_r, "order=%u, item_size=%u, frame_size=%u, data=0x%x \n",
                    pFrame_info->order,
                    pFrame_info->item_size,
                    pFrame_info->frame_size,
                    *(pItem_addr + pFrame_info->frame_start_offset) & 0xF);
            }

            {// remove
                rb_opt_update_r(&rb_opt, RB_READ_TYPE_REMOVE, &pItem_addr, &item_size, _get_item_size);

                pFrame_info = (frame_info_t*)pItem_addr;
                if( pFrame_info )
                {
                    printf("!R: order=%u, item_size=%u, frame_size=%u, data=0x%x \n",
                        pFrame_info->order,
                        pFrame_info->item_size,
                        pFrame_info->frame_size,
                        *(pItem_addr + pFrame_info->frame_start_offset) & 0xF);
                    fprintf(fp_r1, "order=%u, item_size=%u, frame_size=%u, data=0x%x \n",
                        pFrame_info->order,
                        pFrame_info->item_size,
                        pFrame_info->frame_size,
                        *(pItem_addr + pFrame_info->frame_start_offset) & 0xF);
                }
            }
        }

        if( pItem_buf )  free(pItem_buf);
    }

    fclose(fp_w);
    fclose(fp_r);
    fclose(fp_r1);

    if( pFrame_buf )    free(pFrame_buf);
    return 0;
}
