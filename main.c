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
_sh_cmd_test(int argc, char **argv, cb_shell_out_t log, void *pExtra)
{
    log("[%s:%d] help cmd\n", __func__, __LINE__);
    for(int i = 0; i < argc; i++)
        log("argv[%d]:%s\n", i, argv[i]);

    return 0;
}

static sh_cmd_t     g_sh_cmd_test =
{
    .pCmd_name      = "test",
    .cmd_exec       = _sh_cmd_test,
    .pDescription   = "this is 'test' command",
};

static int
_sh_cmd_test2(int argc, char **argv, cb_shell_out_t log, void *pExtra)
{
    log("[%s:%d] help cmd\n", __func__, __LINE__);
    for(int i = 0; i < argc; i++)
        log("argv[%d]:%s\n", i, argv[i]);

    return 0;
}

sh_cmd_add("test2", "this is 'test2' cmd", _sh_cmd_test2, SH_CMD_REGISTER_PRIORITY_NORMAL);
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    char        line_buf[256] = {0};
    char        io_cache[32] = {0};
    char        history[1024] = {0};
    sh_set_t    sh_set = {0};
    sh_args_t   sh_args = {0};

    sh_set.pLine_buf    = line_buf;
    sh_set.line_buf_len = sizeof(line_buf);
    sh_set.pIO_cache    = io_cache;
    sh_set.io_cache_len = sizeof(io_cache);
    sh_set.pHistory_buf = history;
    sh_set.line_size    = sizeof(line_buf);
    sh_set.cmd_deep     = 3;
    sh_set.history_buf_size = SHELL_CALC_HISTORY_BUFFER(sh_set.line_size, sh_set.cmd_deep);
    shell_init(&g_sh_io_win, &sh_set);

    shell_register_cmd(&g_sh_cmd_test);

    sh_args.is_blocking = 1;

    setupConsole();
    shell_proc(&sh_args);

//    restoreConsole();
    return 0;
}
