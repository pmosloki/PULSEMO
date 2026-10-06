/*
 ****************************************************************************************
 *    FILENAME    : j1939_conf.h
 *
 *    DESCRIPTION : File contains .c related definitions and declarations.
 *
 *    $Id         : $
 *
 ****************************************************************************************
 * Revision history
 *
 * Ver Author              Date              Description
 * 1   Sloki
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 *    Includes
 ****************************************************************************************
 */
#include "j1939_conf.h"
#include "j1939.h"

/*
 *******************************************************************************
 *    Defines
 *******************************************************************************
 */

/*
 *******************************************************************************
 *    Variables
 *******************************************************************************
 */
/*
    Constant structure to hold the transport layer timings specified in
    SAE J1939-21
*/
J1939_tlTimingCfg_St_t J1939_tlTiming_St[TOTAL_CH] =
    {
        {
            J1939_T1,  /* Configuration for T1 timings SAE J1939-21 limits
                                 it to 750ms max */
            J1939_T2,  /* Configuration for T2 timings SAE J1939-21 limits
                          it to 1250ms max */
            J1939_T3,  /* Configuration for T3 timings SAE J1939-21 limits
                          it to 1250ms max */
            J1939_T4,  /* Configuration for T4 timings SAE J1939-21 limits
                            it to 1050ms max */
            J1939_TR,  /* Configuration for Tr timings SAE J1939-21 limits
                          it to 200ms max */
            J1939_TH   /* Configuration for Th timings SAE J1939-21 limits
                                it to 500ms max */
        },
        {
            J1939_T1,

            J1939_T2,

            J1939_T3,

            J1939_T4,

            J1939_TR,

            J1939_TH
        }
    };

/*
    Service distributor for J1939-73 associated Diagnostic message (DM) services.
*/
const J1939_ServDist_St_t J1939_ServDist_aSt[] =
    {
        /*        PGN,            Init Callback,     Service call back,            void data pointer (future use) */
        {J1939_81_REQ_PGN, NULL, J1939_TX_claimedAddress, NULL}, /* REQ_PGN (Request for address claimed)
                                                                   Repetition Rate = On request, PGN = 0xEE00 (60928) */
        {J1939_DM1_PGN, NULL, J1939_TX_DM1, NULL},               /* DM1 (Diagnostic Message 1 - Active Diagnostic Trouble Codes)
                                                                   Repetition Rate = On request, PGN = 0xFECA (65226)*/
        {J1939_DM2_PGN, NULL, J1939_TX_DM2, NULL},               /* DM2 (Diagnostic Message 2 - Previously Active Diagnostic Trouble Codes)
                                                                   Repetition Rate = On request, PGN = 0xFECB (65227) */
        {J1939_DM3_PGN, NULL, J1939_TX_DM3, NULL},               /* DM3 (Diagnostic Message 3 - Diagnostic Data Clear/Reset of Previously Active DTCs)
                                                                   Repetition Rate = On request, PGN = 0xFECC (65228) */
        {J1939_DM4_PGN, NULL, J1939_TX_DM4, NULL},               /* DM4 (Diagnostic Message 4 - Diagnostic Data Active Previously Active DTCs Freeze Frame)
                                                                   Repetition Rate = On request, PGN = 0xFECD (65229)*/
        {J1939_DM6_PGN, NULL, J1939_TX_DM6, NULL},               /* DM6 (Diagnostic Message 6 -Emission Relevant Pending DTCs (present in current or previous driving cycle
                                                                   Repetition Rate = On request, PGN = 0xFECF (65231)*/
};

const J1939_SPECIFIC_PGN_St_t J1939_SPECIFIC_PGN_St[] =
    {
        // PGN                        Function                         Function Parameter
        //  {J1939_SAMPLE1_PGN,    NULL,                                               NULL},
        {J1939_SAMPLE1_PGN, J1939_rqPgnSample_Callback, NULL},
        {J1939_SAMPLE2_PGN, J1939_rqPgnSample_Callback, NULL},
};

/* instances */
const J1939_Instance_St_t J1939_Instance_aSt[J1939_SA_TOTAL_NUM] = 
{
    {J1939_SA_0x27,         EVCU1_ADDR},
    {J1939_SA_0x99,         EVCU2_ADDR},
    {J1939_SA_0xF3,           BMS_ADDR},
    {J1939_SA_0x80,          AUX1_ADDR},
    {J1939_SA_0x81,          AUX2_ADDR},
    {J1939_SA_0x8F,          DCDC_ADDR},
    {J1939_SA_0x90,           TCP_ADDR},
    {J1939_SA_0x05,           DNR_ADDR},
    {J1939_SA_0x82,           BCS_ADDR},
    {J1939_SA_0xEF,           MCU_ADDR},
    {J1939_SA_0x1D,   IMMOBILIZER_ADDR},
    {J1939_SA_0x2E,   E_LUBE_PUMP_ADDR},
    {J1939_SA_0x0B,           EBS_ADDR},
    {J1939_SA_0x19,          HVAC_ADDR},
    {J1939_SA_0x21,          VECU_ADDR},
    {J1939_SA_0x17,            IC_ADDR},
    {J1939_SA_0x47,           DMS_ADDR},
    {J1939_SA_0xEC,           BAC_ADDR},
    {J1939_SA_0xFB,           TCU_ADDR}
};

/*
@@ ELEMENT    = J1939_ADDRESSCLAIMING_ACTIVE_FLAG_b
@@ STRUCTURE  = CalData_St_t
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint8_t$ [0 ... 1]
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "Address claim activation flag"
@@ END
*/
bool J1939_ADDRESSCLAIMING_ACTIVE_FLAG_b = FALSE; // TRUE: if address claiming is required; otherwise FALSE.
