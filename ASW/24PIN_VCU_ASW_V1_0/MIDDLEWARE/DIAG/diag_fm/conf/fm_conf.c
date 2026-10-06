/***************************************************************************************************
 *    FILENAME    :  fm_conf.c
 *
 *    DESCRIPTION : Contains the configuration data for the fault management
 *                  Level 1 and 2. Application user should configure this
 *                  file prior to the compilation
 *
 *    $Id         : $
 *
 ***************************************************************************************************
 * Revision history
 *
 * Ver Author       Date       Description
 * 1   Sloki          25/09/2008
 ***************************************************************************************************
 */

/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
#include "fm.h"
#include "fm_level1.h"
#include "fm_conf.h"

/*
 **************************************************************************************************
 *    "CONFIGURATION DATA TO BE FILLED BY APPLICATION USER"
 *                    FAULT ENTRY CONFIGURATION
 **************************************************************************************************
 */
const FM_APPL_FLTCONF_St_t APPL_FAULTCONF_aSt[NUM_OF_FAULTPATHS_E] =
	{
		/*
			FM configuration
		*/
		/***************************************************************************************/
		/*                                                                                                                             */
		// DTC 1: Communication Error
		{
			FP_COMMUNICATION_ERR_E,
			{2, -2, 0, 0},
			{0, 0, 30},
			FM_PRIO_3_E,
			RDY_EGR_GRP,
			FM_FUN_NW_E,
			FM_ALL_ERR,
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
			false,
#endif
			false,
		},
		// DTC 2: Refrigerant pressure error
		
};

uint16_t RPM_N_u16 = 0;
