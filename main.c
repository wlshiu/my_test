
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CONFIG_TOTAL_PIN    3

typedef enum ut_pin_type
{
    UT_PIN_TYPE_IGNORE  = 0x0ul,
    UT_PIN_TYPE_GPIO_IO = 0x5,
    UT_PIN_TYPE_UART_TX,
    UT_PIN_TYPE_UART_RX,
    UT_PIN_TYPE_MCO,
} ut_pin_type_t;

typedef struct ut_pin_attr
{
    uint32_t    pin_type;
    int         value;

} ut_pin_attr_t;

const ut_pin_attr_t      g_pin_table[] =
{
    { .pin_type =  UT_PIN_TYPE_GPIO_IO,  .value = 11, },
    { .pin_type =  UT_PIN_TYPE_GPIO_IO,  .value = 21, },
    { .pin_type =  UT_PIN_TYPE_UART_TX,  .value = 31, },
    { .pin_type =  UT_PIN_TYPE_UART_TX,  .value = 41, },
    { .pin_type =  UT_PIN_TYPE_UART_RX,  .value = 51, },
    { .pin_type =  UT_PIN_TYPE_UART_RX,  .value = 61, },
    { .pin_type =      UT_PIN_TYPE_MCO,  .value = 71, },
    { .pin_type =      UT_PIN_TYPE_MCO,  .value = 71, },
    { .pin_type =   UT_PIN_TYPE_IGNORE,  .value = 0, },
};

typedef struct comb_mgr
{
    ut_pin_type_t       target_pin_type[CONFIG_TOTAL_PIN];
    int                 total_pins;

    ut_pin_attr_t       *module_pin_attr[10];

    ut_pin_attr_t       *pPin_attr_list;
} comb_mgr_t;


static int
_my_proc(comb_mgr_t *pMgr)
{
    printf("\n======\n");
    for(int i = 0; i < pMgr->total_pins; i++)
    {
        printf("%d\n", pMgr->module_pin_attr[i]->value);
    }
    printf("\n");
    return 0;
}

int combine(comb_mgr_t *pMgr, int start_idx)
{
    int             rval = 0;
    ut_pin_attr_t   *pAttr_cur = pMgr->pPin_attr_list;

    while( pAttr_cur->pin_type != UT_PIN_TYPE_IGNORE )
    {
        if( pMgr->target_pin_type[start_idx] == pAttr_cur->pin_type )
        {
            pMgr->module_pin_attr[start_idx] = pAttr_cur;

            if( (start_idx + 1) == pMgr->total_pins )
            {
                _my_proc(pMgr);
            }
            else
            {
                combine(pMgr, start_idx + 1);
            }
        }

        pAttr_cur++;
    }

    return rval;
}


int main()
{
    comb_mgr_t      cmb_mgr = {0};

    cmb_mgr.total_pins         = 2;
    cmb_mgr.target_pin_type[0] = UT_PIN_TYPE_UART_RX;
    cmb_mgr.target_pin_type[1] = UT_PIN_TYPE_UART_TX;
    cmb_mgr.target_pin_type[2] = UT_PIN_TYPE_IGNORE;

    cmb_mgr.pPin_attr_list = (ut_pin_attr_t*)&g_pin_table;

    combine(&cmb_mgr, 0);

    system("pause");

    return 0;
}
