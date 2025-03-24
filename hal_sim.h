/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_sim.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/03/03
 * @license
 * @description
 */

#ifndef __hal_sim_H_wCDSUI3G_lQDl_HOTK_smcQ_uyfPQgyw6TvJ__
#define __hal_sim_H_wCDSUI3G_lQDl_HOTK_smcQ_uyfPQgyw6TvJ__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

//=============================================================================
//                  Constant Definition
//=============================================================================

#define __STATIC_FORCEINLINE         static inline
#define __STATIC_INLINE             static inline

#define SYS_HIRC_VALUE      (72 * 1000 * 1000)

#define EPWM                ((TIM_Type*)&g_EPWM)
/* =========================================================================================================================== */
/* ================                                            TIM                                            ================ */
/* =========================================================================================================================== */

/* ==========================================================  CR1  ========================================================== */
#define TIM_CR1_CEN_Pos                   (0UL)                     /*!< CEN (Bit 0)                                           */
#define TIM_CR1_CEN_Msk                   (0x1UL)                   /*!< CEN (Bitfield-Mask: 0x01)                             */
#define TIM_CR1_UDIS_Pos                  (1UL)                     /*!< UDIS (Bit 1)                                          */
#define TIM_CR1_UDIS_Msk                  (0x2UL)                   /*!< UDIS (Bitfield-Mask: 0x01)                            */
#define TIM_CR1_URS_Pos                   (2UL)                     /*!< URS (Bit 2)                                           */
#define TIM_CR1_URS_Msk                   (0x4UL)                   /*!< URS (Bitfield-Mask: 0x01)                             */
#define TIM_CR1_OPM_Pos                   (3UL)                     /*!< OPM (Bit 3)                                           */
#define TIM_CR1_OPM_Msk                   (0x8UL)                   /*!< OPM (Bitfield-Mask: 0x01)                             */
#define TIM_CR1_DIR_Pos                   (4UL)                     /*!< DIR (Bit 4)                                           */
#define TIM_CR1_DIR_Msk                   (0x10UL)                  /*!< DIR (Bitfield-Mask: 0x01)                             */
#define TIM_CR1_CMS_Pos                   (5UL)                     /*!< CMS (Bit 5)                                           */
#define TIM_CR1_CMS_Msk                   (0x60UL)                  /*!< CMS (Bitfield-Mask: 0x03)                             */
#define TIM_CR1_ARPE_Pos                  (7UL)                     /*!< ARPE (Bit 7)                                          */
#define TIM_CR1_ARPE_Msk                  (0x80UL)                  /*!< ARPE (Bitfield-Mask: 0x01)                            */
#define TIM_CR1_CKD_Pos                   (8UL)                     /*!< CKD (Bit 8)                                           */
#define TIM_CR1_CKD_Msk                   (0x300UL)                 /*!< CKD (Bitfield-Mask: 0x03)                             */
#define TIM_CR1_ASYMEN_Pos                (10UL)                    /*!< ASYMEN (Bit 10)                                       */
#define TIM_CR1_ASYMEN_Msk                (0x400UL)                 /*!< ASYMEN (Bitfield-Mask: 0x01)                          */
/* ==========================================================  CR2  ========================================================== */
#define TIM_CR2_CCPC_Pos                  (0UL)                     /*!< CCPC (Bit 0)                                          */
#define TIM_CR2_CCPC_Msk                  (0x1UL)                   /*!< CCPC (Bitfield-Mask: 0x01)                            */
#define TIM_CR2_CCUS_Pos                  (2UL)                     /*!< CCUS (Bit 2)                                          */
#define TIM_CR2_CCUS_Msk                  (0x4UL)                   /*!< CCUS (Bitfield-Mask: 0x01)                            */
#define TIM_CR2_MMS_Pos                   (4UL)                     /*!< MMS (Bit 4)                                           */
#define TIM_CR2_MMS_Msk                   (0x70UL)                  /*!< MMS (Bitfield-Mask: 0x07)                             */
#define TIM_CR2_TI1S_Pos                  (7UL)                     /*!< TI1S (Bit 7)                                          */
#define TIM_CR2_TI1S_Msk                  (0x80UL)                  /*!< TI1S (Bitfield-Mask: 0x01)                            */
#define TIM_CR2_OIS1_Pos                  (8UL)                     /*!< OIS1 (Bit 8)                                          */
#define TIM_CR2_OIS1_Msk                  (0x100UL)                 /*!< OIS1 (Bitfield-Mask: 0x01)                            */
#define TIM_CR2_OIS1N_Pos                 (9UL)                     /*!< OIS1N (Bit 9)                                         */
#define TIM_CR2_OIS1N_Msk                 (0x200UL)                 /*!< OIS1N (Bitfield-Mask: 0x01)                           */
#define TIM_CR2_OIS2_Pos                  (10UL)                    /*!< OIS2 (Bit 10)                                         */
#define TIM_CR2_OIS2_Msk                  (0x400UL)                 /*!< OIS2 (Bitfield-Mask: 0x01)                            */
#define TIM_CR2_OIS2N_Pos                 (11UL)                    /*!< OIS2N (Bit 11)                                        */
#define TIM_CR2_OIS2N_Msk                 (0x800UL)                 /*!< OIS2N (Bitfield-Mask: 0x01)                           */
#define TIM_CR2_OIS3_Pos                  (12UL)                    /*!< OIS3 (Bit 12)                                         */
#define TIM_CR2_OIS3_Msk                  (0x1000UL)                /*!< OIS3 (Bitfield-Mask: 0x01)                            */
#define TIM_CR2_OIS3N_Pos                 (13UL)                    /*!< OIS3N (Bit 13)                                        */
#define TIM_CR2_OIS3N_Msk                 (0x2000UL)                /*!< OIS3N (Bitfield-Mask: 0x01)                           */
#define TIM_CR2_OIS4_Pos                  (14UL)                    /*!< OIS4 (Bit 14)                                         */
#define TIM_CR2_OIS4_Msk                  (0x4000UL)                /*!< OIS4 (Bitfield-Mask: 0x01)                            */
/* =========================================================  SMCR  ========================================================== */
#define TIM_SMCR_SMS_Pos                  (0UL)                     /*!< SMS (Bit 0)                                           */
#define TIM_SMCR_SMS_Msk                  (0x7UL)                   /*!< SMS (Bitfield-Mask: 0x07)                             */
#define TIM_SMCR_TS_Pos                   (4UL)                     /*!< TS (Bit 4)                                            */
#define TIM_SMCR_TS_Msk                   (0x70UL)                  /*!< TS (Bitfield-Mask: 0x07)                              */
#define TIM_SMCR_MSM_Pos                  (7UL)                     /*!< MSM (Bit 7)                                           */
#define TIM_SMCR_MSM_Msk                  (0x80UL)                  /*!< MSM (Bitfield-Mask: 0x01)                             */
#define TIM_SMCR_ETF_Pos                  (8UL)                     /*!< ETF (Bit 8)                                           */
#define TIM_SMCR_ETF_Msk                  (0xf00UL)                 /*!< ETF (Bitfield-Mask: 0x0f)                             */
#define TIM_SMCR_ETPS_Pos                 (12UL)                    /*!< ETPS (Bit 12)                                         */
#define TIM_SMCR_ETPS_Msk                 (0x3000UL)                /*!< ETPS (Bitfield-Mask: 0x03)                            */
#define TIM_SMCR_ECE_Pos                  (14UL)                    /*!< ECE (Bit 14)                                          */
#define TIM_SMCR_ECE_Msk                  (0x4000UL)                /*!< ECE (Bitfield-Mask: 0x01)                             */
#define TIM_SMCR_ETP_Pos                  (15UL)                    /*!< ETP (Bit 15)                                          */
#define TIM_SMCR_ETP_Msk                  (0x8000UL)                /*!< ETP (Bitfield-Mask: 0x01)                             */
/* =========================================================  DIER  ========================================================== */
#define TIM_DIER_UIE_Pos                  (0UL)                     /*!< UIE (Bit 0)                                           */
#define TIM_DIER_UIE_Msk                  (0x1UL)                   /*!< UIE (Bitfield-Mask: 0x01)                             */
#define TIM_DIER_CC1IE_Pos                (1UL)                     /*!< CC1IE (Bit 1)                                         */
#define TIM_DIER_CC1IE_Msk                (0x2UL)                   /*!< CC1IE (Bitfield-Mask: 0x01)                           */
#define TIM_DIER_CC2IE_Pos                (2UL)                     /*!< CC2IE (Bit 2)                                         */
#define TIM_DIER_CC2IE_Msk                (0x4UL)                   /*!< CC2IE (Bitfield-Mask: 0x01)                           */
#define TIM_DIER_CC3IE_Pos                (3UL)                     /*!< CC3IE (Bit 3)                                         */
#define TIM_DIER_CC3IE_Msk                (0x8UL)                   /*!< CC3IE (Bitfield-Mask: 0x01)                           */
#define TIM_DIER_CC4IE_Pos                (4UL)                     /*!< CC4IE (Bit 4)                                         */
#define TIM_DIER_CC4IE_Msk                (0x10UL)                  /*!< CC4IE (Bitfield-Mask: 0x01)                           */
#define TIM_DIER_COMIE_Pos                (5UL)                     /*!< COMIE (Bit 5)                                         */
#define TIM_DIER_COMIE_Msk                (0x20UL)                  /*!< COMIE (Bitfield-Mask: 0x01)                           */
#define TIM_DIER_TIE_Pos                  (6UL)                     /*!< TIE (Bit 6)                                           */
#define TIM_DIER_TIE_Msk                  (0x40UL)                  /*!< TIE (Bitfield-Mask: 0x01)                             */
#define TIM_DIER_BIE_Pos                  (7UL)                     /*!< BIE (Bit 7)                                           */
#define TIM_DIER_BIE_Msk                  (0x80UL)                  /*!< BIE (Bitfield-Mask: 0x01)                             */
/* ==========================================================  SR  =========================================================== */
#define TIM_SR_UIF_Pos                    (0UL)                     /*!< UIF (Bit 0)                                           */
#define TIM_SR_UIF_Msk                    (0x1UL)                   /*!< UIF (Bitfield-Mask: 0x01)                             */
#define TIM_SR_CC1IF_Pos                  (1UL)                     /*!< CC1IF (Bit 1)                                         */
#define TIM_SR_CC1IF_Msk                  (0x2UL)                   /*!< CC1IF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_CC2IF_Pos                  (2UL)                     /*!< CC2IF (Bit 2)                                         */
#define TIM_SR_CC2IF_Msk                  (0x4UL)                   /*!< CC2IF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_CC3IF_Pos                  (3UL)                     /*!< CC3IF (Bit 3)                                         */
#define TIM_SR_CC3IF_Msk                  (0x8UL)                   /*!< CC3IF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_CC4IF_Pos                  (4UL)                     /*!< CC4IF (Bit 4)                                         */
#define TIM_SR_CC4IF_Msk                  (0x10UL)                  /*!< CC4IF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_COMIF_Pos                  (5UL)                     /*!< COMIF (Bit 5)                                         */
#define TIM_SR_COMIF_Msk                  (0x20UL)                  /*!< COMIF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_TIF_Pos                    (6UL)                     /*!< TIF (Bit 6)                                           */
#define TIM_SR_TIF_Msk                    (0x40UL)                  /*!< TIF (Bitfield-Mask: 0x01)                             */
#define TIM_SR_BIF_Pos                    (7UL)                     /*!< BIF (Bit 7)                                           */
#define TIM_SR_BIF_Msk                    (0x80UL)                  /*!< BIF (Bitfield-Mask: 0x01)                             */
#define TIM_SR_CC1OF_Pos                  (9UL)                     /*!< CC1OF (Bit 9)                                         */
#define TIM_SR_CC1OF_Msk                  (0x200UL)                 /*!< CC1OF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_CC2OF_Pos                  (10UL)                    /*!< CC2OF (Bit 10)                                        */
#define TIM_SR_CC2OF_Msk                  (0x400UL)                 /*!< CC2OF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_CC3OF_Pos                  (11UL)                    /*!< CC3OF (Bit 11)                                        */
#define TIM_SR_CC3OF_Msk                  (0x800UL)                 /*!< CC3OF (Bitfield-Mask: 0x01)                           */
#define TIM_SR_CC4OF_Pos                  (12UL)                    /*!< CC4OF (Bit 12)                                        */
#define TIM_SR_CC4OF_Msk                  (0x1000UL)                /*!< CC4OF (Bitfield-Mask: 0x01)                           */
/* ==========================================================  EGR  ========================================================== */
#define TIM_EGR_UG_Pos                    (0UL)                     /*!< UG (Bit 0)                                            */
#define TIM_EGR_UG_Msk                    (0x1UL)                   /*!< UG (Bitfield-Mask: 0x01)                              */
#define TIM_EGR_CC1G_Pos                  (1UL)                     /*!< CC1G (Bit 1)                                          */
#define TIM_EGR_CC1G_Msk                  (0x2UL)                   /*!< CC1G (Bitfield-Mask: 0x01)                            */
#define TIM_EGR_CC2G_Pos                  (2UL)                     /*!< CC2G (Bit 2)                                          */
#define TIM_EGR_CC2G_Msk                  (0x4UL)                   /*!< CC2G (Bitfield-Mask: 0x01)                            */
#define TIM_EGR_CC3G_Pos                  (3UL)                     /*!< CC3G (Bit 3)                                          */
#define TIM_EGR_CC3G_Msk                  (0x8UL)                   /*!< CC3G (Bitfield-Mask: 0x01)                            */
#define TIM_EGR_CC4G_Pos                  (4UL)                     /*!< CC4G (Bit 4)                                          */
#define TIM_EGR_CC4G_Msk                  (0x10UL)                  /*!< CC4G (Bitfield-Mask: 0x01)                            */
#define TIM_EGR_COMG_Pos                  (5UL)                     /*!< COMG (Bit 5)                                          */
#define TIM_EGR_COMG_Msk                  (0x20UL)                  /*!< COMG (Bitfield-Mask: 0x01)                            */
#define TIM_EGR_TG_Pos                    (6UL)                     /*!< TG (Bit 6)                                            */
#define TIM_EGR_TG_Msk                    (0x40UL)                  /*!< TG (Bitfield-Mask: 0x01)                              */
#define TIM_EGR_BG_Pos                    (7UL)                     /*!< BG (Bit 7)                                            */
#define TIM_EGR_BG_Msk                    (0x80UL)                  /*!< BG (Bitfield-Mask: 0x01)                              */
/* =====================================================  CCMR1_OUTPUT  ====================================================== */
#define TIM_CCMR1_OUTPUT_CC1S_Pos         (0UL)                     /*!< CC1S (Bit 0)                                          */
#define TIM_CCMR1_OUTPUT_CC1S_Msk         (0x3UL)                   /*!< CC1S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR1_OUTPUT_OC1FE_Pos        (2UL)                     /*!< OC1FE (Bit 2)                                         */
#define TIM_CCMR1_OUTPUT_OC1FE_Msk        (0x4UL)                   /*!< OC1FE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR1_OUTPUT_OC1PE_Pos        (3UL)                     /*!< OC1PE (Bit 3)                                         */
#define TIM_CCMR1_OUTPUT_OC1PE_Msk        (0x8UL)                   /*!< OC1PE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR1_OUTPUT_OC1M_Pos         (4UL)                     /*!< OC1M (Bit 4)                                          */
#define TIM_CCMR1_OUTPUT_OC1M_Msk         (0x70UL)                  /*!< OC1M (Bitfield-Mask: 0x07)                            */
#define TIM_CCMR1_OUTPUT_OC1CE_Pos        (7UL)                     /*!< OC1CE (Bit 7)                                         */
#define TIM_CCMR1_OUTPUT_OC1CE_Msk        (0x80UL)                  /*!< OC1CE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR1_OUTPUT_CC2S_Pos         (8UL)                     /*!< CC2S (Bit 8)                                          */
#define TIM_CCMR1_OUTPUT_CC2S_Msk         (0x300UL)                 /*!< CC2S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR1_OUTPUT_OC2FE_Pos        (10UL)                    /*!< OC2FE (Bit 10)                                        */
#define TIM_CCMR1_OUTPUT_OC2FE_Msk        (0x400UL)                 /*!< OC2FE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR1_OUTPUT_OC2PE_Pos        (11UL)                    /*!< OC2PE (Bit 11)                                        */
#define TIM_CCMR1_OUTPUT_OC2PE_Msk        (0x800UL)                 /*!< OC2PE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR1_OUTPUT_OC2M_Pos         (12UL)                    /*!< OC2M (Bit 12)                                         */
#define TIM_CCMR1_OUTPUT_OC2M_Msk         (0x7000UL)                /*!< OC2M (Bitfield-Mask: 0x07)                            */
#define TIM_CCMR1_OUTPUT_OC2CE_Pos        (15UL)                    /*!< OC2CE (Bit 15)                                        */
#define TIM_CCMR1_OUTPUT_OC2CE_Msk        (0x8000UL)                /*!< OC2CE (Bitfield-Mask: 0x01)                           */
/* ======================================================  CCMR1_INPUT  ====================================================== */
#define TIM_CCMR1_INPUT_CC1S_Pos          (0UL)                     /*!< CC1S (Bit 0)                                          */
#define TIM_CCMR1_INPUT_CC1S_Msk          (0x3UL)                   /*!< CC1S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR1_INPUT_IC1PSC_Pos        (2UL)                     /*!< IC1PSC (Bit 2)                                        */
#define TIM_CCMR1_INPUT_IC1PSC_Msk        (0xcUL)                   /*!< IC1PSC (Bitfield-Mask: 0x03)                          */
#define TIM_CCMR1_INPUT_IC1F_Pos          (4UL)                     /*!< IC1F (Bit 4)                                          */
#define TIM_CCMR1_INPUT_IC1F_Msk          (0xf0UL)                  /*!< IC1F (Bitfield-Mask: 0x0f)                            */
#define TIM_CCMR1_INPUT_CC2S_Pos          (8UL)                     /*!< CC2S (Bit 8)                                          */
#define TIM_CCMR1_INPUT_CC2S_Msk          (0x300UL)                 /*!< CC2S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR1_INPUT_IC2PSC_Pos        (10UL)                    /*!< IC2PSC (Bit 10)                                       */
#define TIM_CCMR1_INPUT_IC2PSC_Msk        (0xc00UL)                 /*!< IC2PSC (Bitfield-Mask: 0x03)                          */
#define TIM_CCMR1_INPUT_IC2F_Pos          (12UL)                    /*!< IC2F (Bit 12)                                         */
#define TIM_CCMR1_INPUT_IC2F_Msk          (0xf000UL)                /*!< IC2F (Bitfield-Mask: 0x0f)                            */
/* =====================================================  CCMR2_OUTPUT  ====================================================== */
#define TIM_CCMR2_OUTPUT_CC3S_Pos         (0UL)                     /*!< CC3S (Bit 0)                                          */
#define TIM_CCMR2_OUTPUT_CC3S_Msk         (0x3UL)                   /*!< CC3S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR2_OUTPUT_OC3FE_Pos        (2UL)                     /*!< OC3FE (Bit 2)                                         */
#define TIM_CCMR2_OUTPUT_OC3FE_Msk        (0x4UL)                   /*!< OC3FE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR2_OUTPUT_OC3PE_Pos        (3UL)                     /*!< OC3PE (Bit 3)                                         */
#define TIM_CCMR2_OUTPUT_OC3PE_Msk        (0x8UL)                   /*!< OC3PE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR2_OUTPUT_OC3M_Pos         (4UL)                     /*!< OC3M (Bit 4)                                          */
#define TIM_CCMR2_OUTPUT_OC3M_Msk         (0x70UL)                  /*!< OC3M (Bitfield-Mask: 0x07)                            */
#define TIM_CCMR2_OUTPUT_OC3CE_Pos        (7UL)                     /*!< OC3CE (Bit 7)                                         */
#define TIM_CCMR2_OUTPUT_OC3CE_Msk        (0x80UL)                  /*!< OC3CE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR2_OUTPUT_CC4S_Pos         (8UL)                     /*!< CC4S (Bit 8)                                          */
#define TIM_CCMR2_OUTPUT_CC4S_Msk         (0x300UL)                 /*!< CC4S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR2_OUTPUT_OC4FE_Pos        (10UL)                    /*!< OC4FE (Bit 10)                                        */
#define TIM_CCMR2_OUTPUT_OC4FE_Msk        (0x400UL)                 /*!< OC4FE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR2_OUTPUT_OC4PE_Pos        (11UL)                    /*!< OC4PE (Bit 11)                                        */
#define TIM_CCMR2_OUTPUT_OC4PE_Msk        (0x800UL)                 /*!< OC4PE (Bitfield-Mask: 0x01)                           */
#define TIM_CCMR2_OUTPUT_OC4M_Pos         (12UL)                    /*!< OC4M (Bit 12)                                         */
#define TIM_CCMR2_OUTPUT_OC4M_Msk         (0x7000UL)                /*!< OC4M (Bitfield-Mask: 0x07)                            */
#define TIM_CCMR2_OUTPUT_OC4CE_Pos        (15UL)                    /*!< OC4CE (Bit 15)                                        */
#define TIM_CCMR2_OUTPUT_OC4CE_Msk        (0x8000UL)                /*!< OC4CE (Bitfield-Mask: 0x01)                           */
/* ======================================================  CCMR2_INPUT  ====================================================== */
#define TIM_CCMR2_INPUT_CC3S_Pos          (0UL)                     /*!< CC3S (Bit 0)                                          */
#define TIM_CCMR2_INPUT_CC3S_Msk          (0x3UL)                   /*!< CC3S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR2_INPUT_IC3PSC_Pos        (2UL)                     /*!< IC3PSC (Bit 2)                                        */
#define TIM_CCMR2_INPUT_IC3PSC_Msk        (0xcUL)                   /*!< IC3PSC (Bitfield-Mask: 0x03)                          */
#define TIM_CCMR2_INPUT_IC3F_Pos          (4UL)                     /*!< IC3F (Bit 4)                                          */
#define TIM_CCMR2_INPUT_IC3F_Msk          (0xf0UL)                  /*!< IC3F (Bitfield-Mask: 0x0f)                            */
#define TIM_CCMR2_INPUT_CC4S_Pos          (8UL)                     /*!< CC4S (Bit 8)                                          */
#define TIM_CCMR2_INPUT_CC4S_Msk          (0x300UL)                 /*!< CC4S (Bitfield-Mask: 0x03)                            */
#define TIM_CCMR2_INPUT_IC4PSC_Pos        (10UL)                    /*!< IC4PSC (Bit 10)                                       */
#define TIM_CCMR2_INPUT_IC4PSC_Msk        (0xc00UL)                 /*!< IC4PSC (Bitfield-Mask: 0x03)                          */
#define TIM_CCMR2_INPUT_IC4F_Pos          (12UL)                    /*!< IC4F (Bit 12)                                         */
#define TIM_CCMR2_INPUT_IC4F_Msk          (0xf000UL)                /*!< IC4F (Bitfield-Mask: 0x0f)                            */
/* =========================================================  CCER  ========================================================== */
#define TIM_CCER_CC1E_Pos                 (0UL)                     /*!< CC1E (Bit 0)                                          */
#define TIM_CCER_CC1E_Msk                 (0x1UL)                   /*!< CC1E (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC1P_Pos                 (1UL)                     /*!< CC1P (Bit 1)                                          */
#define TIM_CCER_CC1P_Msk                 (0x2UL)                   /*!< CC1P (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC1NE_Pos                (2UL)                     /*!< CC1NE (Bit 2)                                         */
#define TIM_CCER_CC1NE_Msk                (0x4UL)                   /*!< CC1NE (Bitfield-Mask: 0x01)                           */
#define TIM_CCER_CC1NP_Pos                (3UL)                     /*!< CC1NP (Bit 3)                                         */
#define TIM_CCER_CC1NP_Msk                (0x8UL)                   /*!< CC1NP (Bitfield-Mask: 0x01)                           */
#define TIM_CCER_CC2E_Pos                 (4UL)                     /*!< CC2E (Bit 4)                                          */
#define TIM_CCER_CC2E_Msk                 (0x10UL)                  /*!< CC2E (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC2P_Pos                 (5UL)                     /*!< CC2P (Bit 5)                                          */
#define TIM_CCER_CC2P_Msk                 (0x20UL)                  /*!< CC2P (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC2NE_Pos                (6UL)                     /*!< CC2NE (Bit 6)                                         */
#define TIM_CCER_CC2NE_Msk                (0x40UL)                  /*!< CC2NE (Bitfield-Mask: 0x01)                           */
#define TIM_CCER_CC2NP_Pos                (7UL)                     /*!< CC2NP (Bit 7)                                         */
#define TIM_CCER_CC2NP_Msk                (0x80UL)                  /*!< CC2NP (Bitfield-Mask: 0x01)                           */
#define TIM_CCER_CC3E_Pos                 (8UL)                     /*!< CC3E (Bit 8)                                          */
#define TIM_CCER_CC3E_Msk                 (0x100UL)                 /*!< CC3E (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC3P_Pos                 (9UL)                     /*!< CC3P (Bit 9)                                          */
#define TIM_CCER_CC3P_Msk                 (0x200UL)                 /*!< CC3P (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC3NE_Pos                (10UL)                    /*!< CC3NE (Bit 10)                                        */
#define TIM_CCER_CC3NE_Msk                (0x400UL)                 /*!< CC3NE (Bitfield-Mask: 0x01)                           */
#define TIM_CCER_CC3NP_Pos                (11UL)                    /*!< CC3NP (Bit 11)                                        */
#define TIM_CCER_CC3NP_Msk                (0x800UL)                 /*!< CC3NP (Bitfield-Mask: 0x01)                           */
#define TIM_CCER_CC4E_Pos                 (12UL)                    /*!< CC4E (Bit 12)                                         */
#define TIM_CCER_CC4E_Msk                 (0x1000UL)                /*!< CC4E (Bitfield-Mask: 0x01)                            */
#define TIM_CCER_CC4P_Pos                 (13UL)                    /*!< CC4P (Bit 13)                                         */
#define TIM_CCER_CC4P_Msk                 (0x2000UL)                /*!< CC4P (Bitfield-Mask: 0x01)                            */
/* ==========================================================  CNT  ========================================================== */
#define TIM_CNT_CNT_Pos                   (0UL)                     /*!< CNT (Bit 0)                                           */
#define TIM_CNT_CNT_Msk                   (0xffffUL)                /*!< CNT (Bitfield-Mask: 0xffff)                           */
/* ==========================================================  PSC  ========================================================== */
#define TIM_PSC_PSC_Pos                   (0UL)                     /*!< PSC (Bit 0)                                           */
#define TIM_PSC_PSC_Msk                   (0xffffUL)                /*!< PSC (Bitfield-Mask: 0xffff)                           */
/* ==========================================================  ARR  ========================================================== */
#define TIM_ARR_ARR_Pos                   (0UL)                     /*!< ARR (Bit 0)                                           */
#define TIM_ARR_ARR_Msk                   (0xffffUL)                /*!< ARR (Bitfield-Mask: 0xffff)                           */
/* ==========================================================  RCR  ========================================================== */
#define TIM_RCR_REP_Pos                   (0UL)                     /*!< REP (Bit 0)                                           */
#define TIM_RCR_REP_Msk                   (0xffUL)                  /*!< REP (Bitfield-Mask: 0xff)                             */
/* =========================================================  CCR1  ========================================================== */
#define TIM_CCR1_CCR1_Pos                 (0UL)                     /*!< CCR1 (Bit 0)                                          */
#define TIM_CCR1_CCR1_Msk                 (0xfffffUL)               /*!< CCR1 (Bitfield-Mask: 0xfffff)                         */
/* =========================================================  CCR2  ========================================================== */
#define TIM_CCR2_CCR2_Pos                 (0UL)                     /*!< CCR2 (Bit 0)                                          */
#define TIM_CCR2_CCR2_Msk                 (0xfffffUL)               /*!< CCR2 (Bitfield-Mask: 0xfffff)                         */
/* =========================================================  CCR3  ========================================================== */
#define TIM_CCR3_CCR3_Pos                 (0UL)                     /*!< CCR3 (Bit 0)                                          */
#define TIM_CCR3_CCR3_Msk                 (0xfffffUL)               /*!< CCR3 (Bitfield-Mask: 0xfffff)                         */
/* =========================================================  CCR4  ========================================================== */
#define TIM_CCR4_CCR4_Pos                 (0UL)                     /*!< CCR4 (Bit 0)                                          */
#define TIM_CCR4_CCR4_Msk                 (0xfffffUL)               /*!< CCR4 (Bitfield-Mask: 0xfffff)                         */
/* =========================================================  BDTR  ========================================================== */
#define TIM_BDTR_DTG_Pos                  (0UL)                     /*!< DTG (Bit 0)                                           */
#define TIM_BDTR_DTG_Msk                  (0xffUL)                  /*!< DTG (Bitfield-Mask: 0xff)                             */
#define TIM_BDTR_LOCK_Pos                 (8UL)                     /*!< LOCK (Bit 8)                                          */
#define TIM_BDTR_LOCK_Msk                 (0x300UL)                 /*!< LOCK (Bitfield-Mask: 0x03)                            */
#define TIM_BDTR_OSSI_Pos                 (10UL)                    /*!< OSSI (Bit 10)                                         */
#define TIM_BDTR_OSSI_Msk                 (0x400UL)                 /*!< OSSI (Bitfield-Mask: 0x01)                            */
#define TIM_BDTR_OSSR_Pos                 (11UL)                    /*!< OSSR (Bit 11)                                         */
#define TIM_BDTR_OSSR_Msk                 (0x800UL)                 /*!< OSSR (Bitfield-Mask: 0x01)                            */
#define TIM_BDTR_BKE_Pos                  (12UL)                    /*!< BKE (Bit 12)                                          */
#define TIM_BDTR_BKE_Msk                  (0x1000UL)                /*!< BKE (Bitfield-Mask: 0x01)                             */
#define TIM_BDTR_BKP_Pos                  (13UL)                    /*!< BKP (Bit 13)                                          */
#define TIM_BDTR_BKP_Msk                  (0x2000UL)                /*!< BKP (Bitfield-Mask: 0x01)                             */
#define TIM_BDTR_AOE_Pos                  (14UL)                    /*!< AOE (Bit 14)                                          */
#define TIM_BDTR_AOE_Msk                  (0x4000UL)                /*!< AOE (Bitfield-Mask: 0x01)                             */
#define TIM_BDTR_MOE_Pos                  (15UL)                    /*!< MOE (Bit 15)                                          */
#define TIM_BDTR_MOE_Msk                  (0x8000UL)                /*!< MOE (Bitfield-Mask: 0x01)                             */
#define TIM_BDTR_DTGF_Pos                 (16UL)                    /*!< DTGF (Bit 16)                                         */
#define TIM_BDTR_DTGF_Msk                 (0xff0000UL)              /*!< DTGF (Bitfield-Mask: 0xff)                            */
#define TIM_BDTR_DTAE_Pos                 (24UL)                    /*!< DTAE (Bit 24)                                         */
#define TIM_BDTR_DTAE_Msk                 (0x1000000UL)             /*!< DTAE (Bitfield-Mask: 0x01)                            */
/* =========================================================  CCDR1  ========================================================= */
#define TIM_CCDR1_CCR1_Pos                (0UL)                     /*!< CCR1 (Bit 0)                                          */
#define TIM_CCDR1_CCR1_Msk                (0xfffffUL)               /*!< CCR1 (Bitfield-Mask: 0xfffff)                         */
/* =========================================================  CCDR2  ========================================================= */
#define TIM_CCDR2_CCR2_Pos                (0UL)                     /*!< CCR2 (Bit 0)                                          */
#define TIM_CCDR2_CCR2_Msk                (0xfffffUL)               /*!< CCR2 (Bitfield-Mask: 0xfffff)                         */
/* =========================================================  CCDR3  ========================================================= */
#define TIM_CCDR3_CCR3_Pos                (0UL)                     /*!< CCR3 (Bit 0)                                          */
#define TIM_CCDR3_CCR3_Msk                (0xfffffUL)               /*!< CCR3 (Bitfield-Mask: 0xfffff)                         */


//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
#ifndef __IM
    #define __IM
#endif
#ifndef __OM
    #define __OM
#endif
#ifndef __IOM
    #define __IOM
#endif

typedef struct {                                /*!< (@ 0x4000C000) TIM Structure                                              */

  union {
    __IOM uint32_t CR1;                         /*!< (@ 0x00000000) Control Register 1                                         */

    struct {
      __IOM uint32_t CEN        : 1;            /*!< [0..0] Counter enable                                                     */
      __IOM uint32_t UDIS       : 1;            /*!< [1..1] Update disable                                                     */
      __IOM uint32_t URS        : 1;            /*!< [2..2] Update request source                                              */
      __IOM uint32_t OPM        : 1;            /*!< [3..3] One pulse spurce                                                   */
      __IOM uint32_t DIR        : 1;            /*!< [4..4] Direction                                                          */
      __IOM uint32_t CMS        : 2;            /*!< [6..5] Center-aligned mode selection                                      */
      __IOM uint32_t ARPE       : 1;            /*!< [7..7] Auto-reload preload enable                                         */
      __IOM uint32_t CKD        : 2;            /*!< [9..8] Clock division                                                     */
      __IOM uint32_t ASYMEN     : 1;            /*!< [10..10] Asymmetric mode enable                                           */
            uint32_t            : 21;
    } CR1_b;
  } ;

  union {
    __IOM uint32_t CR2;                         /*!< (@ 0x00000004) Control Register 2                                         */

    struct {
      __IOM uint32_t CCPC       : 1;            /*!< [0..0] Capture/compare preloaded control                                  */
            uint32_t            : 1;
      __IOM uint32_t CCUS       : 1;            /*!< [2..2] Capture/compare control update selection                           */
            uint32_t            : 1;
      __IOM uint32_t MMS        : 3;            /*!< [6..4] Master mode selection                                              */
      __IOM uint32_t TI1S       : 1;            /*!< [7..7] TI1 selection                                                      */
      __IOM uint32_t OIS1       : 1;            /*!< [8..8] Output Idle state 1                                                */
      __IOM uint32_t OIS1N      : 1;            /*!< [9..9] Output Idle state 1                                                */
      __IOM uint32_t OIS2       : 1;            /*!< [10..10] Output Idle state 2                                              */
      __IOM uint32_t OIS2N      : 1;            /*!< [11..11] Output Idle state 2                                              */
      __IOM uint32_t OIS3       : 1;            /*!< [12..12] Output Idle state 3                                              */
      __IOM uint32_t OIS3N      : 1;            /*!< [13..13] Output Idle state 3                                              */
      __IOM uint32_t OIS4       : 1;            /*!< [14..14] Output Idle state 4                                              */
            uint32_t            : 17;
    } CR2_b;
  } ;

  union {
    __IOM uint32_t SMCR;                        /*!< (@ 0x00000008) Slave Mode control Register                                */

    struct {
      __IOM uint32_t SMS        : 3;            /*!< [2..0] Slave mode selection                                               */
            uint32_t            : 1;
      __IOM uint32_t TS         : 3;            /*!< [6..4] Trigger selection                                                  */
      __IOM uint32_t MSM        : 1;            /*!< [7..7] Master/slave mode                                                  */
      __IOM uint32_t ETF        : 4;            /*!< [11..8] External trigger filter                                           */
      __IOM uint32_t ETPS       : 2;            /*!< [13..12] External trigger prescaler                                       */
      __IOM uint32_t ECE        : 1;            /*!< [14..14] External clock enable                                            */
      __IOM uint32_t ETP        : 1;            /*!< [15..15] External trigger polarity                                        */
            uint32_t            : 16;
    } SMCR_b;
  } ;

  union {
    __IOM uint32_t DIER;                        /*!< (@ 0x0000000C) interrupt enbale register                                  */

    struct {
      __IOM uint32_t UIE        : 1;            /*!< [0..0] Update interrupt enable                                            */
      __IOM uint32_t CC1IE      : 1;            /*!< [1..1] Capture/Compare 1 interrupt enable                                 */
      __IOM uint32_t CC2IE      : 1;            /*!< [2..2] Capture/Compare 2 interrupt enable                                 */
      __IOM uint32_t CC3IE      : 1;            /*!< [3..3] Capture/Compare 3 interrupt enable                                 */
      __IOM uint32_t CC4IE      : 1;            /*!< [4..4] Capture/Compare 4 interrupt enable                                 */
      __IOM uint32_t COMIE      : 1;            /*!< [5..5] COM interrupt enable                                               */
      __IOM uint32_t TIE        : 1;            /*!< [6..6] Trigger interrupt enable                                           */
      __IOM uint32_t BIE        : 1;            /*!< [7..7] Break interrupt enable                                             */
            uint32_t            : 24;
    } DIER_b;
  } ;

  union {
    __IOM uint32_t SR;                          /*!< (@ 0x00000010) state Register                                             */

    struct {
      __IOM uint32_t UIF        : 1;            /*!< [0..0] Update interrupt flag                                              */
      __IOM uint32_t CC1IF      : 1;            /*!< [1..1] Capture/Compare 1 interrupt flag                                   */
      __IOM uint32_t CC2IF      : 1;            /*!< [2..2] Capture/Compare 2 interrupt flag                                   */
      __IOM uint32_t CC3IF      : 1;            /*!< [3..3] Capture/Compare 3 interrupt flag                                   */
      __IOM uint32_t CC4IF      : 1;            /*!< [4..4] Capture/Compare 4 interrupt flag                                   */
      __IOM uint32_t COMIF      : 1;            /*!< [5..5] COM interrupt flag                                                 */
      __IOM uint32_t TIF        : 1;            /*!< [6..6] Trigger interrupt flag                                             */
      __IOM uint32_t BIF        : 1;            /*!< [7..7] Break interrupt flag                                               */
            uint32_t            : 1;
      __IOM uint32_t CC1OF      : 1;            /*!< [9..9] CC1OF                                                              */
      __IOM uint32_t CC2OF      : 1;            /*!< [10..10] CC2OF                                                            */
      __IOM uint32_t CC3OF      : 1;            /*!< [11..11] CC3OF                                                            */
      __IOM uint32_t CC4OF      : 1;            /*!< [12..12] CC4OF                                                            */
            uint32_t            : 19;
    } SR_b;
  } ;

  union {
    __IOM uint32_t EGR;                         /*!< (@ 0x00000014) event generation Register                                  */

    struct {
      __IOM uint32_t UG         : 1;            /*!< [0..0] Update generation                                                  */
      __IOM uint32_t CC1G       : 1;            /*!< [1..1] Capture/Compare 1 generation                                       */
      __IOM uint32_t CC2G       : 1;            /*!< [2..2] Capture/Compare 2 generation                                       */
      __IOM uint32_t CC3G       : 1;            /*!< [3..3] Capture/Compare 3 generation                                       */
      __IOM uint32_t CC4G       : 1;            /*!< [4..4] Capture/Compare 4 generation                                       */
      __IOM uint32_t COMG       : 1;            /*!< [5..5] Capture/Compare control update generation                          */
      __IOM uint32_t TG         : 1;            /*!< [6..6] Trigger generation                                                 */
      __IOM uint32_t BG         : 1;            /*!< [7..7] Break generation                                                   */
            uint32_t            : 24;
    } EGR_b;
  } ;

  union {
    union {
      __IOM uint32_t CCMR1_OUTPUT;              /*!< (@ 0x00000018) Capture or Compare mode Register (Output mode)             */

      struct {
        __IOM uint32_t CC1S     : 2;            /*!< [1..0] Capture/Compare 1 selection                                        */
        __IOM uint32_t OC1FE    : 1;            /*!< [2..2] Output Compare 1 fast enable                                       */
        __IOM uint32_t OC1PE    : 1;            /*!< [3..3] Output Compare 1 preload enable                                    */
        __IOM uint32_t OC1M     : 3;            /*!< [6..4] Output Compare 1 mode                                              */
        __IOM uint32_t OC1CE    : 1;            /*!< [7..7] Output Compare 1 clear enable                                      */
        __IOM uint32_t CC2S     : 2;            /*!< [9..8] Capture/Compare 2 selection                                        */
        __IOM uint32_t OC2FE    : 1;            /*!< [10..10] Output Compare 2 fast enable                                     */
        __IOM uint32_t OC2PE    : 1;            /*!< [11..11] Output Compare 2 preload enable                                  */
        __IOM uint32_t OC2M     : 3;            /*!< [14..12] Output Compare 2 mode                                            */
        __IOM uint32_t OC2CE    : 1;            /*!< [15..15] Output Compare 2 clear enable                                    */
              uint32_t          : 16;
      } CCMR1_OUTPUT_b;
    } ;

    union {
      __IOM uint32_t CCMR1_INPUT;               /*!< (@ 0x00000018) Capture or Compare mode Register (Input mode)              */

      struct {
        __IOM uint32_t CC1S     : 2;            /*!< [1..0] Capture or Compare 1 Select                                        */
        __IOM uint32_t IC1PSC   : 2;            /*!< [3..2] Input capture 1 prescaler                                          */
        __IOM uint32_t IC1F     : 4;            /*!< [7..4] Input capture 1 filter                                             */
        __IOM uint32_t CC2S     : 2;            /*!< [9..8] Capture or Compare 2 Select                                        */
        __IOM uint32_t IC2PSC   : 2;            /*!< [11..10] Input capture 2 prescaler                                        */
        __IOM uint32_t IC2F     : 4;            /*!< [15..12] Input capture 2 filter                                           */
              uint32_t          : 16;
      } CCMR1_INPUT_b;
    } ;
  };

  union {
    union {
      __IOM uint32_t CCMR2_OUTPUT;              /*!< (@ 0x0000001C) Capture or Compare mode Register (output mode)             */

      struct {
        __IOM uint32_t CC3S     : 2;            /*!< [1..0] Capture/Compare 3 selection                                        */
        __IOM uint32_t OC3FE    : 1;            /*!< [2..2] Output Compare 3 fast enable                                       */
        __IOM uint32_t OC3PE    : 1;            /*!< [3..3] Output Compare 3 preload enable                                    */
        __IOM uint32_t OC3M     : 3;            /*!< [6..4] Output Compare 3 mode                                              */
        __IOM uint32_t OC3CE    : 1;            /*!< [7..7] Output Compare 3 clear enable                                      */
        __IOM uint32_t CC4S     : 2;            /*!< [9..8] Capture/Compare 4 selection                                        */
        __IOM uint32_t OC4FE    : 1;            /*!< [10..10] Output Compare 4 fast enable                                     */
        __IOM uint32_t OC4PE    : 1;            /*!< [11..11] Output Compare 4 preload enable                                  */
        __IOM uint32_t OC4M     : 3;            /*!< [14..12] Output Compare 4 mode                                            */
        __IOM uint32_t OC4CE    : 1;            /*!< [15..15] Output Compare 4 clear enable                                    */
              uint32_t          : 16;
      } CCMR2_OUTPUT_b;
    } ;

    union {
      __IOM uint32_t CCMR2_INPUT;               /*!< (@ 0x0000001C) Capture or Compare mode Register (input mode)              */

      struct {
        __IOM uint32_t CC3S     : 2;            /*!< [1..0] Capture or Compare 3 Select                                        */
        __IOM uint32_t IC3PSC   : 2;            /*!< [3..2] Input capture 3 prescaler                                          */
        __IOM uint32_t IC3F     : 4;            /*!< [7..4] Input capture 3 filter                                             */
        __IOM uint32_t CC4S     : 2;            /*!< [9..8] Capture or Compare 4 Select                                        */
        __IOM uint32_t IC4PSC   : 2;            /*!< [11..10] Input capture 4 prescaler                                        */
        __IOM uint32_t IC4F     : 4;            /*!< [15..12] Input capture 4 filter                                           */
              uint32_t          : 16;
      } CCMR2_INPUT_b;
    } ;
  };

  union {
    __IOM uint32_t CCER;                        /*!< (@ 0x00000020) Capture or Compare enable Register                         */

    struct {
      __IOM uint32_t CC1E       : 1;            /*!< [0..0] Capture/Compare 1 output enable                                    */
      __IOM uint32_t CC1P       : 1;            /*!< [1..1] Capture/Compare 1 output polarity                                  */
      __IOM uint32_t CC1NE      : 1;            /*!< [2..2] Capture/Compare 1 complementary output enable                      */
      __IOM uint32_t CC1NP      : 1;            /*!< [3..3] Capture/Compare 1 complementary output polarity                    */
      __IOM uint32_t CC2E       : 1;            /*!< [4..4] Capture/Compare 2 output enable                                    */
      __IOM uint32_t CC2P       : 1;            /*!< [5..5] Capture/Compare 2 output polarity                                  */
      __IOM uint32_t CC2NE      : 1;            /*!< [6..6] Capture/Compare 2 complementary output enable                      */
      __IOM uint32_t CC2NP      : 1;            /*!< [7..7] Capture/Compare 2 complementary output polarity                    */
      __IOM uint32_t CC3E       : 1;            /*!< [8..8] Capture/Compare 3 output enable                                    */
      __IOM uint32_t CC3P       : 1;            /*!< [9..9] Capture/Compare 3 output polarity                                  */
      __IOM uint32_t CC3NE      : 1;            /*!< [10..10] Capture/Compare 3 complementary output enable                    */
      __IOM uint32_t CC3NP      : 1;            /*!< [11..11] Capture/Compare 3 complementary output polarity                  */
      __IOM uint32_t CC4E       : 1;            /*!< [12..12] Capture/Compare 4 output enable                                  */
      __IOM uint32_t CC4P       : 1;            /*!< [13..13] Capture/Compare 4 output polarity                                */
            uint32_t            : 18;
    } CCER_b;
  } ;

  union {
    __IOM uint32_t CNT;                         /*!< (@ 0x00000024) EPWM counter register                                      */

    struct {
      __IOM uint32_t CNT        : 16;           /*!< [15..0] Counter value                                                     */
            uint32_t            : 16;
    } CNT_b;
  } ;

  union {
    __IOM uint32_t PSC;                         /*!< (@ 0x00000028) Prescaler counter register                                 */

    struct {
      __IOM uint32_t PSC        : 16;           /*!< [15..0] Prescaler value                                                   */
            uint32_t            : 16;
    } PSC_b;
  } ;

  union {
    __IOM uint32_t ARR;                         /*!< (@ 0x0000002C) Auto reload counter register                               */

    struct {
      __IOM uint32_t ARR        : 16;           /*!< [15..0] Auto reload value                                                 */
            uint32_t            : 16;
    } ARR_b;
  } ;

  union {
    __IOM uint32_t RCR;                         /*!< (@ 0x00000030) Repetition counter register                                */

    struct {
      __IOM uint32_t REP        : 8;            /*!< [7..0] Repetition counter value                                           */
            uint32_t            : 24;
    } RCR_b;
  } ;

  union {
    __IOM uint32_t CCR1;                        /*!< (@ 0x00000034) CR1 capture or compare value                               */

    struct {
      __IOM uint32_t CCR1       : 20;           /*!< [19..0] Capture/Compare 1 value                                           */
            uint32_t            : 12;
    } CCR1_b;
  } ;

  union {
    __IOM uint32_t CCR2;                        /*!< (@ 0x00000038) CR2 capture or compare value                               */

    struct {
      __IOM uint32_t CCR2       : 20;           /*!< [19..0] Capture/Compare 2 value                                           */
            uint32_t            : 12;
    } CCR2_b;
  } ;

  union {
    __IOM uint32_t CCR3;                        /*!< (@ 0x0000003C) capture or compare value                                   */

    struct {
      __IOM uint32_t CCR3       : 20;           /*!< [19..0] Capture/Compare 3 value                                           */
            uint32_t            : 12;
    } CCR3_b;
  } ;

  union {
    __IOM uint32_t CCR4;                        /*!< (@ 0x00000040) match output control                                       */

    struct {
      __IOM uint32_t CCR4       : 20;           /*!< [19..0] Capture/Compare 4 value                                           */
            uint32_t            : 12;
    } CCR4_b;
  } ;

  union {
    __IOM uint32_t BDTR;                        /*!< (@ 0x00000044) Braking ande dead zone control Register                    */

    struct {
      __IOM uint32_t DTG        : 8;            /*!< [7..0] Dead-time generator setup                                          */
      __IOM uint32_t LOCK       : 2;            /*!< [9..8] Lock Configuration                                                 */
      __IOM uint32_t OSSI       : 1;            /*!< [10..10] Off-state selection for Idle mode                                */
      __IOM uint32_t OSSR       : 1;            /*!< [11..11] Off-state selection for Run mode                                 */
      __IOM uint32_t BKE        : 1;            /*!< [12..12] Break enable                                                     */
      __IOM uint32_t BKP        : 1;            /*!< [13..13] Break polarity                                                   */
      __IOM uint32_t AOE        : 1;            /*!< [14..14] Automatic output enable                                          */
      __IOM uint32_t MOE        : 1;            /*!< [15..15] Main output enable                                               */
      __IOM uint32_t DTGF       : 8;            /*!< [23..16] Asymmetric Dead-time generator setup                             */
      __IOM uint32_t DTAE       : 1;            /*!< [24..24] Asymmetric Dead-time enable                                      */
            uint32_t            : 7;
    } BDTR_b;
  } ;
  __IM  uint32_t  RESERVED[2];

  union {
    __IOM uint32_t CCDR1;                       /*!< (@ 0x00000050) CR1 down capture or compare value                          */

    struct {
      __IOM uint32_t CCR1       : 20;           /*!< [19..0] Down Capture/Compare 1 value                                      */
            uint32_t            : 12;
    } CCDR1_b;
  } ;

  union {
    __IOM uint32_t CCDR2;                       /*!< (@ 0x00000054) CR2 down capture or compare value                          */

    struct {
      __IOM uint32_t CCR2       : 20;           /*!< [19..0] Down Capture/Compare 2 value                                      */
            uint32_t            : 12;
    } CCDR2_b;
  } ;

  union {
    __IOM uint32_t CCDR3;                       /*!< (@ 0x00000058) CR3 down capture or compare value                          */

    struct {
      __IOM uint32_t CCR3       : 20;           /*!< [19..0] Down Capture/Compare 3 value                                      */
            uint32_t            : 12;
    } CCDR3_b;
  } ;
} TIM_Type;                                     /*!< Size = 92 (0x5c)                                                          */


//=============================================================================
//                  Global Data Definition
//=============================================================================
extern TIM_Type     g_EPWM;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
