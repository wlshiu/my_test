#include <stdio.h>
#include <stdlib.h>

#include "pocketsphinx.h"
#include "util/cmd_ln.h"

#define MODELDIR    "./pocketsphinx/model"

int main(int argc, char *argv[])
{
    ps_decoder_t *ps = NULL;
    cmd_ln_t *config = NULL;

    FILE *fh;
    char const *hyp, *uttid;
    int16 buf[512];
    int rv;
    int32 score;

    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/en-us/en-us",
                         "-lm", MODELDIR "/en-us/en-us.lm.bin",
                         "-dict", MODELDIR "/en-us/cmudict-en-us.dict",
                         NULL);

    if (config == NULL)
    {
        fprintf(stderr, "Failed to create config object, see log for details\n");
        return -1;
    }

    ps = ps_init(config);

    if (ps == NULL)
    {
        fprintf(stderr, "Failed to create recognizer, see log for details\n");
        return -1;
    }

//    char    *pRaw_name = "goforward.raw";
    char    *pRaw_name = "numbers.raw";

    fh = fopen(pRaw_name, "rb");
    if (fh == NULL)
    {
        fprintf(stderr, "Unable to open input file %s\n", pRaw_name);
        return -1;
    }

    rv = ps_start_utt(ps);
    while (!feof(fh))
    {
        size_t nsamp;
        nsamp = fread(buf, 2, 512, fh);
        rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
    }

    rv = ps_end_utt(ps);
    hyp = ps_get_hyp(ps, &score);
    printf("Recognized: %s\n", hyp);

    fclose(fh);
    ps_free(ps);
    cmd_ln_free_r(config);

    system("pause");
    return 0;
}
