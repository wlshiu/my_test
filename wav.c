/** @file wav.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wav.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)       printf("[error] " str, ##__VA_ARGS__)

#define FOURCC(a, b, c, d)      (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
/**
 *  \brief  Convert seconds into hh:mm:ss format
 *
 *  \param [in] raw_seconds         seconds value
 *  \return
 *      h:m:s - formatted string
 */
static char* seconds_to_time(float raw_seconds)
{
    char *hms;
    int hours, hours_residue, minutes, seconds, milliseconds;

    hms = (char*) malloc(100);

    sprintf(hms, "%f", raw_seconds);

    hours = (int) raw_seconds / 3600;
    hours_residue = (int) raw_seconds % 3600;
    minutes = hours_residue / 60;
    seconds = hours_residue % 60;
    milliseconds = 0;

    // get the decimal part of raw_seconds to get milliseconds
    char *pos;
    pos = strchr(hms, '.');
    int ipos = (int) (pos - hms);
    char decimalpart[15];
    memset(decimalpart, ' ', sizeof(decimalpart));
    strncpy(decimalpart, &hms[ipos + 1], 3);
    milliseconds = atoi(decimalpart);

    sprintf(hms, "%d:%d:%d.%d", hours, minutes, seconds, milliseconds);
    return hms;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int wav_init(char *pWav_name, wav_handle_t *pHWav)
{
    int     rval = 0;
    FILE    *fin = 0;
    uint8_t *pBuf = 0;

    if( !pWav_name || !pHWav )
        return WAV_ERR_NULL_POINT;

    do {
        wav_hdr_t   *pHdr = &pHWav->hdr;

        if( !(fin = fopen(pWav_name, "rb")) )
        {
            err("open %s fail !\n", pWav_name);
            rval = WAV_ERR_OPEN_FAIL;
            break;
        }

        fread(&pHWav->hdr, 1, sizeof(pHWav->hdr), fin);

        if( *((uint32_t*)&pHWav->hdr.riff) != FOURCC('R', 'I', 'F', 'F') )
        {
            err("%s is NOT wav file\n",  pWav_name);
            rval = WAV_ERR_NOT_WAV_FILE;
            break;
        }

        if( !(pBuf = malloc(pHdr->data_size)) )
        {
            err("malloc %d fail !\n", pHdr->data_size);
            rval = WAV_ERR_MALLOC_FAIL;
            break;
        }

        fread(pBuf, 1, pHdr->data_size, fin);

        pHWav->bytes_per_sample = pHdr->bits_per_sample >> 3;

        pHWav->raw_data_nbytes = pHdr->data_size / pHdr->channels;
        if( !(pHWav->raw_data.pData = (uint8_t*)malloc(pHWav->raw_data_nbytes)) )
        {
            err("malloc %d fail !\n", pHdr->data_size / pHdr->channels);
            rval = WAV_ERR_MALLOC_FAIL;
            break;
        }

        pHWav->sample_num = pHWav->raw_data_nbytes / (pHdr->bits_per_sample >> 3);
        for(int i = 0; i < pHWav->sample_num; i++)
        {
            if( pHdr->bits_per_sample == 16 )
            {
                int16_t     *pCur = (int16_t*)((uint32_t)pBuf + i * pHdr->channels * (pHdr->bits_per_sample >> 3));
                pHWav->raw_data.pPCM[i] = *pCur;
            }

            else if( pHdr->bits_per_sample == 32 )
            {
                float     *pCur = (float*)((uint32_t)pBuf + i * pHdr->channels * (pHdr->bits_per_sample >> 3));
                pHWav->raw_data.pFP_data[i] = *pCur;
            }
            else
            {
                char    *pCur = (char*)((uint32_t)pBuf + i * pHdr->channels * (pHdr->bits_per_sample >> 3));
                pHWav->raw_data.pData[i] = (*pCur) - 128; //in wave, 8-bit are unsigned, so shifting to signed
            }
        }
    } while(0);

    if( pBuf )  free(pBuf);

    if( fin )   fclose(fin);

    return rval;
}

int wav_deinit(wav_handle_t *pHWav)
{
    int     rval = 0;

    if( pHWav )
    {
        if( pHWav->raw_data.pData )
            free(pHWav->raw_data.pData);

        memset(pHWav, 0x0, sizeof(wav_handle_t));
    }
    return rval;
}

int wav_dump_info(wav_handle_t *pHWav)
{
    int         rval = 0;
    wav_hdr_t   *pHdr = &pHWav->hdr;

    if( !pHWav )
        return WAV_ERR_NULL_POINT;

    printf("(1-4)   riff: %c%c%c%c\n", pHdr->riff[0], pHdr->riff[1], pHdr->riff[2], pHdr->riff[3]);
    printf("(5-8)   Overall size: bytes=%u (%u KB)\n", pHdr->overall_size, pHdr->overall_size >> 10);
    printf("(9-12)  Wave marker: %c%c%c%c\n", pHdr->wave[0], pHdr->wave[1], pHdr->wave[2], pHdr->wave[3]);
    printf("(13-16) Fmt marker: %c%c%c%c\n", pHdr->fmt_chunk_marker[0], pHdr->fmt_chunk_marker[1], pHdr->fmt_chunk_marker[2], pHdr->fmt_chunk_marker[3]);
    printf("(17-20) Len of Fmt header: %u\n", pHdr->length_of_fmt);
    printf("(21-22) Format type: %u (%s)\n", pHdr->format_type,
           (pHdr->format_type == 1) ? "PCM" :
           (pHdr->format_type == 3) ? "Float" :
           (pHdr->format_type == 6) ? "A-law (8-bits)" :
           (pHdr->format_type == 7) ? "Mu-law (8-bits)" :
           "Not Support");
    printf("(23-24) Channels: %u\n", pHdr->channels);
    printf("(25-28) Sample rate: %u\n", pHdr->sample_rate);
    printf("(29-32) Byte Rate (Per-sec): %u (Bit-Rate=%u)\n", pHdr->byterate, pHdr->byterate * 8);
    printf("(33-34) Block Alignment: %u\n", pHdr->block_align);
    printf("(35-36) Bits per sample: %u\n", pHdr->bits_per_sample);
    printf("(37-40) Data Marker: %c%c%c%c\n", pHdr->data_chunk_header[0], pHdr->data_chunk_header[1], pHdr->data_chunk_header[2], pHdr->data_chunk_header[3]);
    printf("(41-44) Size of data chunk: %u\n", pHdr->data_size);

#if 0
    // dump sample data
    for(int i = 0; i < pHWav->sample_num; i++)
    {
        if( !(i & 0x3) )
            printf("\n x%05X |", i);

        if( pHWav->bytes_per_sample == 2 )
            printf(" %8d", pHWav->raw_data.pPCM[i]);
        else if( pHWav->bytes_per_sample == 4 )
            printf(" %8.6f", pHWav->raw_data.pFP_data[i]);
        else
            printf(" %4d", pHWav->raw_data.pData[i]);
    }

    printf("\n");
#endif
    return rval;
}







#if 0

unsigned char buffer4[4];
unsigned char buffer2[2];


FILE *ptr;
char *filename;
struct HEADER header;

int main(int argc, char **argv)
{
   int read = 0;

// calculate no.of samples
    long num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);
    printf("Number of samples:%lu n", num_samples);

    long size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
    printf("Size of each sample:%ld bytesn", size_of_each_sample);

// calculate duration of file
    float duration_in_seconds = (float) header.overall_size / header.byterate;
    printf("Approx.Duration in seconds=%fn", duration_in_seconds);
    printf("Approx.Duration in h:m:s=%sn", seconds_to_time(duration_in_seconds));



// read each sample from data chunk if PCM
    if (header.format_type == 1)   // PCM
    {
        printf("Dump sample data? Y/N?");
        char c = 'n';
        scanf("%c", &c);
        if (c == 'Y' || c == 'y')
        {
            long i = 0;
            char data_buffer[size_of_each_sample];
            int  size_is_correct = TRUE;

            // make sure that the bytes-per-sample is completely divisible by num.of channels
            long bytes_in_each_channel = (size_of_each_sample / header.channels);
            if ((bytes_in_each_channel  * header.channels) != size_of_each_sample)
            {
                printf("Error: %ld x %ud <> %ldn", bytes_in_each_channel, header.channels, size_of_each_sample);
                size_is_correct = FALSE;
            }

            if (size_is_correct)
            {
                // the valid amplitude range for values based on the bits per sample
                long low_limit = 0l;
                long high_limit = 0l;

                switch (header.bits_per_sample)
                {
                    case 8:
                        low_limit = -128;
                        high_limit = 127;
                        break;
                    case 16:
                        low_limit = -32768;
                        high_limit = 32767;
                        break;
                    case 32:
                        low_limit = -2147483648;
                        high_limit = 2147483647;
                        break;
                }

                printf("nn.Valid range for data values : %ld to %ld n", low_limit, high_limit);
                for (i = 1; i <= num_samples; i++)
                {
                    printf("==========Sample %ld / %ld=============n", i, num_samples);
                    read = fread(data_buffer, sizeof(data_buffer), 1, ptr);
                    if (read == 1)
                    {

                        // dump the data read
                        unsigned int  xchannels = 0;
                        int data_in_channel = 0;
                        int offset = 0; // move the offset for every iteration in the loop below
                        for (xchannels = 0; xchannels < header.channels; xchannels ++ )
                        {
                            printf("Channel#%d : ", (xchannels + 1));
                            // convert data from little endian to big endian based on bytes in each channel sample
                            if (bytes_in_each_channel == 4)
                            {
                                data_in_channel = (data_buffer[offset] & 0x00ff) |
                                                  ((data_buffer[offset + 1] & 0x00ff) << 8) |
                                                  ((data_buffer[offset + 2] & 0x00ff) << 16) |
                                                  (data_buffer[offset + 3] << 24);
                            }
                            else if (bytes_in_each_channel == 2)
                            {
                                data_in_channel = (data_buffer[offset] & 0x00ff) |
                                                  (data_buffer[offset + 1] << 8);
                            }
                            else if (bytes_in_each_channel == 1)
                            {
                                data_in_channel = data_buffer[offset] & 0x00ff;
                                data_in_channel -= 128; //in wave, 8-bit are unsigned, so shifting to signed
                            }

                            offset += bytes_in_each_channel;
                            printf("%d ", data_in_channel);

                            // check if value was in range
                            if (data_in_channel < low_limit || data_in_channel > high_limit)
                                printf("**value out of rangen");

                            printf(" | ");
                        }

                        printf("n");
                    }
                    else
                    {
                        printf("Error reading file. %d bytesn", read);
                        break;
                    }

                } //    for (i =1; i <= num_samples; i++) {

            } //    if (size_is_correct) {

        } // if (c == 'Y' || c == 'y') {
    } //  if (header.format_type == 1) {

    printf("Closing file..n");
    fclose(ptr);

    // cleanup before quitting
    free(filename);
    return 0;

}
#endif // 0
