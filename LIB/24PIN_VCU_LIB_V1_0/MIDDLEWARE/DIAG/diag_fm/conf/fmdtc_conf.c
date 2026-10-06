/***************************************************************************************************
 *    FILENAME    :  fmdtc_conf.c
 *
 *    DESCRIPTION : File contains the configuration table for DTC values
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

#include "fmdtc_conf.h"
#if (TRUE == FM_UDS_SUPPORTED)

/*
 **************************************************************************************************
 *    "CONFIGURATION DATA TO BE FILLED BY APPLICATION USER"
 *     Mapping of the DTC to Fault Paths for each Error types
 **************************************************************************************************
 */
const uint32_t DTCMappingTable_UDS_aSt[NUM_OF_FAULTPATHS_E][MAX_NUM_ERROR_TYPES - 1] =
{
	/*    Fault         Error           DTC Value
		  Labels,       type,           (24bit)
	*/
	// MAX_ERR,     MIN_ERR,   SIG_ERR,   NPL_ERR,  FER_ERR     		/*     Index - DTC Name		*/
	{0x000000, 		0x000000, 0xE80000, 0x000000, 0x000000}, 	/*	01 - FP_COMMUNICATION_ERR_E  				*/
	
};	

#if (TRUE == FM_J1939_SUPPORTED)

const J1939_DTC_Uni_t J1939_DTC_Conf_aUni[NUM_OF_FAULTPATHS_E] = 
{
	{.DTC_St = {0x7E089,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	01 - ELECTRIC_HAZARD_ELE_FAIL_E  		*/
	{.DTC_St = {0x7E08A,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	02 - AUX_MOTOR_ERROR_ELE_FAIL_E  		*/
	{.DTC_St = {0x7E08B,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	03 - SERVICE_TT_ELE_FAIL_E  			*/
	{.DTC_St = {0x7E08C,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	04 - LIMP_MODE_ELE_FAIL_E	        	*/
	{.DTC_St = {0x7E08D,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	05 - TILT_CAB_ERROR_ELE_FAIL_E  		*/
	{.DTC_St = {0x7E08E,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	06 - BATERY_CHARGE_ERROR_ELE_FAIL_E  	*/
	{.DTC_St = {0x7E08F,	  CURRENT_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	07 - LOW_AIR_PRES_FRNT_OPEN_E			*/
	{.DTC_St = {0x7E090,	  CURRENT_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	08 - LOW_AIR_PRES_REAR_OPEN_E    		*/
	{.DTC_St = {0x7E091,	  VOLTAGE_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	09 - LOW_AIR_PRES_FRNT_SHORT_TO_GND_E	*/
	{.DTC_St = {0x7E092,	  VOLTAGE_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	10 - LOW_AIR_PRES_REAR_SHORT_TO_GND_E  	*/
	{.DTC_St = {0x7E093,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	11 - VACUUM_PUMP_ELE_FAIL_E			  	*/
	{.DTC_St = {0x7E252,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	12 - AEBS_SYSTEM_ELE_FAIL_E  			*/
	{.DTC_St = {0x7E253,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	13 - LDWS_SYSTEM_ELE_FAIL_E  			*/
	{.DTC_St = {0x7E254,	        DATA_INCORRECT,	  1 ,	 0x7F}},	/*	14 - RPM_OUTOFRANGE_E					*/
	{.DTC_St = {0x7E255,	        DATA_INCORRECT,	  1 ,	 0x7F}},	/*	15 - SOC_OUTOFRANGE_E				  	*/
	{.DTC_St = {0x7E256,	        DATA_INCORRECT,	  1 ,	 0x7F}},	/*	16 - BATT_T_OUTOFRANGE_E			   	*/
	{.DTC_St = {0x7E257,	 		DATA_INCORRECT,	  1 ,	 0x7F}},	/*	17 - TRACTION_MOTOR_T_OUTOFRANGE_E	 	*/
	{.DTC_St = {0x7E094,	   INVALID_CALIBRATION,	  1 ,	 0x7F}},	/*	18 - STEERING_SWITCH_OUTOFRANGE_E   	*/
	{.DTC_St = {0x7E258,	 		DATA_INCORRECT,	  1 ,	 0x7F}},	/*	19 - STEERING_SWITCH_STUCK_E   			*/
	{.DTC_St = {0x7E259,	  CURRENT_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	20 - NO_CAMERA_CONNECTION_E   	        */
	{.DTC_St = {0x7E095,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	21 - SPEAKER_FAIL_E  					*/
	{.DTC_St = {0x7E25A,	  CURRENT_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	22 - TFT_BACKLIGHT_DRIVER_ELE_FAIL_E   	*/
	{.DTC_St = {0x7E25B,	 DEVICE_NOT_FUNCTIONAL,	  1 ,	 0x7F}},	/*	23 - MEMORY_FAILURE_E  			   		*/
	{.DTC_St = {0x7E096,	 VOLTAGE_ABOVE_NORMAL,	  1 ,	 0x7F}},	/*	24 - POWER_ISSUE_OVER_VOLTAGE_E  	   	*/
	{.DTC_St = {0x7E097,	 VOLTAGE_BELOW_NORMAL,	  1 ,	 0x7F}},	/*	25 - POWER_ISSUE_UNDER_VOLTAGE_E  	   	*/
	{.DTC_St = {0x7E25D,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	26 - EVCU_NODE_MISSING_E		  	   	*/
	{.DTC_St = {0x7E25E,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	27 - SOC_CAN_TIMEOUT_E			  	   	*/
	{.DTC_St = {0x7E25F,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	28 - RPM_CAN_TIMEOUT_E			  	   	*/
	{.DTC_St = {0x7E260,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	29 - BATT_T_CAN_TIMEOUT_E  		  	   	*/
	{.DTC_St = {0x7E261,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	30 - VCU_NODE_MISSING_E  		  	   	*/
	{.DTC_St = {0x7E262,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	31 - STEERING_MOTOR_TIMEOUT_E	  	   	*/
	{.DTC_St = {0x7E263,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	32 - COMPRESSOR_CAN_TIMEOUT_E	  	   	*/
	{.DTC_St = {0x7E264,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	33 - MCU_CAN_TIMEOUT_E	  	   			*/
	{.DTC_St = {0x7E265,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	34 - AEBS_N_CMS_CAN_TIMEOUT_E	  	   	*/
	{.DTC_St = {0x7E266,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	35 - LDWS_CAN_TIMEOUT_E	  	   			*/
	{.DTC_St = {0x7E267,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	36 - RPM_IMPLAUSIBLE_E	  	   			*/
	{.DTC_St = {0x7E268,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	37 - SOC_IMPLAUSIBLE_E	  	   			*/
	{.DTC_St = {0x7E269,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	38 - BATT_T_IMPLAUSIBLE_E	  	   		*/
	{.DTC_St = {0x7E26A,	 		  DATA_RX_ERR,	  1 ,	 0x7F}},	/*	39 - TRACTION_MOTOR_T_IMPLAUSIBLE_E	  	*/
	{.DTC_St = {0x7E26B,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	40 - CAN_ACTIVE_BUS_OFF_E  		   		*/
	{.DTC_St = {0x7E26C,	  INVALID_UPDATE_RATE,	  1 ,	 0x7F}},	/*	41 - CAN_PASSIVE_BUS_OFF_E  		   	*/

};
#endif
#endif

/* Note addition of DTC will require removing Default DTC masking */
