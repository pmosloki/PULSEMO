/***************************************************************************************************
 *    FILENAME    :  fmff_conf.c
 *
 *    DESCRIPTION : File contains the configuration table for freeze frames.
 *
 *    $Id         : $
 *
 ***************************************************************************************************
 * Revision history
 *
 * Ver Author       Date       Description
 * 1   Sloki          25/09/2018
 ***************************************************************************************************
 */

#include "fmff_conf.h"
#include "uds_DID_conf.h"
#include "diag_adapt.h"
#include <stdlib.h>
#include "fm_conf.h"
/*
 **************************************************************************************************
 *    "CONFIGURATION DATA TO BE FILLED BY APPLICATION USER"
 *     FREEZE FRAME CONFIGURATION
 **************************************************************************************************
 */
/******************* GLOBAL VARIABLE DECLARATION *****************/
// uint8_t BcsWorkingModeFF_au8[1];
// uint8_t IgnitionStatusFF_au8[1];

// uint8_t Sample1_au8[1] = {2};
// uint8_t Sample2_au8[1] = {3};
// uint8_t Sample3_au8[1] = {4};
// uint8_t Sample4_au8[1] = {5};
// uint8_t Sample5_au8[1] = {6};
// uint8_t Sample6_au8[1] = {7};
/*
 **************************************************************************************************
	The Freeze frame table for OEM specific faults.
 **************************************************************************************************
*/

#if (TRUE == FM_UDS_SUPPORTED)

/**
 * Freeze Frame Parameter Configuration for UDS Specific Protocol
 */
// const FM_FrzFrm_UDS_Data_St_t FM_Global_Snapshot_UDS_aSt[FMFF_CONF_UDS_GLBSS_ENTRIES] =
// 	{
// 		{0xA000, Sample1_au8, 1, NULL},
// 		{0xA001, Sample2_au8, 1, NULL},

// };

const uint16_t FM_Global_Snapshot_UDS_au16[FMFF_CONF_UDS_GLBSS_ENTRIES] = {GLOBAL_FF_DID_1, GLOBAL_FF_DID_2};
const uint16_t FM_FrzFrm_Local01_UDS_au16[FMFF_CONF_UDS_LOCAL_01_ENTRIES] ={SAMPLE_DID_3, SAMPLE_DID_4, SAMPLE_DID_5, SAMPLE_DID_6};
const uint16_t FM_FrzFrm_Local02_UDS_au16[FMFF_CONF_UDS_LOCAL_02_ENTRIES] ={SAMPLE_DID_2};  


// const FM_FrzFrm_UDS_Data_St_t FM_FrzFrm_Local02_UDS_aSt[FMFF_CONF_UDS_SPECIFIC_01_ENTRIES] =
// 	{
// 		{0x7201, (uint8_t *)(BCSPressureFF_au8), 1, NULL},
// 		{0x7202, (uint8_t *)(AmbTempFF_as8), 1, NULL},
// 		{0x7203, (uint8_t *)(Cool_InFF_as8), 1, NULL},
// 		{0x7204, (uint8_t *)(Cool_OutFF_as8), 1, NULL},
// };

const FM_FrzFrmMasterConfig_UDS_St_t FM_FrzFrmMasterConfig_UDS_aSt[NUM_OF_FAULTPATHS_E] =
{
	{FP_COMMUNICATION_ERR_E,        	FM_FrzFrm_Local01_UDS_au16, FMFF_CONF_UDS_LOCAL_01_ENTRIES},
	
};

#endif
