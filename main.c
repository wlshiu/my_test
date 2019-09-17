#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

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
extern sh_io_desc_t    g_sh_io_win;

extern void setupConsole(void);
extern void restoreConsole(void);
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_sh_cmd_help(int argc, char **argv, cb_shell_out_t log, void *pExtra)
{
    log("[%s:%d] help cmd\n", __func__, __LINE__);
    for(int i = 0; i < argc; i++)
        log("argv[%d]:%s\n", i, argv[i]);

    return 0;
}

static sh_cmd_t     g_sh_cmd_help =
{
    .pCmd_name      = "?",
    .cmd_exec       = _sh_cmd_help,
    .pDescription   = "this is 'help' command",
};
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    char        line_buf[256] = {0};
    char        io_cache[32] = {0};
    sh_set_t    sh_set = {0};
    sh_args_t   sh_args = {0};

    sh_set.pLine_buf    = line_buf;
    sh_set.line_buf_len = sizeof(line_buf);
    sh_set.pIO_cache    = io_cache;
    sh_set.io_cache_len = sizeof(io_cache);
    shell_init(&g_sh_io_win, &sh_set);

    shell_register_cmd(&g_sh_cmd_help);

    sh_args.is_blocking = 1;

    setupConsole();
    shell_proc(&sh_args);

//    restoreConsole();
    return 0;
}
