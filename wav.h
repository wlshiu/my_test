/** @file wav.h
 *
 * @license
 * @description
 */

#ifndef __wav_H_wzGylcHl_l7SB_H7ET_sb7X_ufKZ4723TNYu__
#define __wav_H_wzGylcHl_l7SB_H7ET_sb7X_ufKZ4723TNYu__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum wav_err
{
    WAV_ERR_OK              = 0,
    WAV_ERR_OPEN_FAIL       = -1,
    WAV_ERR_NULL_POINT      = -2,
    WAV_ERR_MALLOC_FAIL     = -3,
    WAV_ERR_NOT_WAV_FILE    = -4,
} wav_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
// WAVE file header format
typedef struct wav_hdr
{
    char        riff[4];                       // RIFF string
    uint32_t    overall_size   ;               // overall size of file in bytes
    char        wave[4];                       // WAVE string
    char        fmt_chunk_marker[4];           // fmt string with trailing null char
    uint32_t    length_of_fmt;                 // length of the format data
    uint16_t    format_type;                   // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    uint16_t    channels;                      // no.of channels
    uint32_t    sample_rate;                   // sampling rate (blocks per second)
    uint32_t    byterate;                      // SampleRate * NumChannels * BitsPerSample/8
    uint16_t    block_align;                   // NumChannels * BitsPerSample/8
    uint16_t    bits_per_sample;               // bits per sample, 8- 8bits, 16- 16 bits etc
    char        data_chunk_header[4];          // DATA string or FLLR string
    uint16_t    data_size;                     // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} wav_hdr_t;

typedef struct wav_handle
{
    wav_hdr_t       hdr;

    char            bytes_per_sample;

    /**
     *  Raw data only support MONO mode
     */
    int             sample_num;         // number of samples
    int             raw_data_nbytes;    // nbytes of one channel

    union {
        int16_t         *pPCM;
        uint8_t         *pData;
        float           *pFP_data;
    } raw_data;



} wav_handle_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int wav_init(char *pWav_name, wav_handle_t *pHWav);

int wav_deinit(wav_handle_t *pHWav);

int wav_dump_info(wav_handle_t *pHWav);

#ifdef __cplusplus
}
#endif

#endif




