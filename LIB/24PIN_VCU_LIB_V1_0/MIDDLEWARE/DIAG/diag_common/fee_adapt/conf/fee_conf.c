/******************************************************************************
 *    FILENAME    : fee_conf.c
 *    DESCRIPTION : EEPROM configuration
 ******************************************************************************
 * Revision history
 *
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
 */

#include "fee_conf.h"
#include "uds_conf.h"
#include "nvm_parameter.h"
uint16_t Start_Addr_u16 = 0x0000;

#if (TRUE == DIAG_CONF_FM_SUPPORTED)
/* Maintain Size as multiple of 4 for proper backupram write */
/* The Address is MCU dependent. Size is L2 config table dependent
 *  (mainly depends on number of parameters in Freeze frame table.
 */
const FEE_FMInfo_St_t FEE_FM_Config_aSt[TOTAL_FM_SIGNAL_E] =
{
	{FM_L2_ENTRY1,     FEE_FM_L2_ENTRY1_E},
	{FM_L2_ENTRY2,     FEE_FM_L2_ENTRY2_E},
	{FM_L2_ENTRY3,     FEE_FM_L2_ENTRY3_E},
	{FM_L2_ENTRY4,     FEE_FM_L2_ENTRY4_E},
	{FM_L2_ENTRY5,     FEE_FM_L2_ENTRY5_E},
	{FM_L2_ENTRY6,     FEE_FM_L2_ENTRY6_E},
	{FM_L2_ENTRY7,     FEE_FM_L2_ENTRY7_E},
	{FM_L2_ENTRY8,     FEE_FM_L2_ENTRY8_E},
	{FM_L2_ENTRY9,     FEE_FM_L2_ENTRY9_E},
	{FM_L2_ENTRY10,   FEE_FM_L2_ENTRY10_E},
	{FM_COMMON_DATA, FEE_FM_COMMON_DATA_E},
	{FM_RDYRESULTS,   FEE_FM_RDYRESULTS_E},
	{FM_TFSLC,             FEE_FM_TFSLC_E},
};
#endif

