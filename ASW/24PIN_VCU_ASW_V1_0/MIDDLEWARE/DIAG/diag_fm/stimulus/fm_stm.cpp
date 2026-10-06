/*
 * fm_stm.cpp
 *
 *  Created on: Nov 24, 2020
 *      Author: sandeepky
 */

//#include <fm_stm.h>
//#include "fm_conf.h"

/**********************************************************************
 Global variables
**********************************************************************/


/**********************************************************************
        FUNCTION DEFINTIONS
**********************************************************************/
//
///**
//*  @brief     Default Constructor
//*  @param     none
//*  @return    none
//*/
//FM_stm::FM_stm(void)
//{
//   Init();
//}
//
///**
//*  @brief     Constructor with radix information for alignment of set value
//*  @param     none
//*  @return    none
//*/
//void FM_stm::Init (void)
//{
//	;
//}

/**
*  @brief     Stimulates the forced value when mode flag ist set, otherwise
* 			  returns the value value_u16
*  @param     value
*  @return    -
*/
//uint8_t FM_stm::Stimulus(uint8_t FltType_u8)
//{

//    CheckMode();

//    if (MODE_OFF == Mode_u8)
//  	{
//		return FltType_u8;
//  	}
//    return FaultForceValue_u8;

//}

//
//
//
///**
//*  @brief     This function checks if the simulation mode must be changed:
//*             A mode change can be caused by: a) watchdog or b) the callback function
//*             add a)
//*             If watchdog time is zero, the watchdog is not enabled and therefore
//*             not checked.In case that the watchdog expires, the mode is reset to off.
//*             add b)
//*             If the callback function returns FALSE; the mode is set to off and simulation
//*             is cancelled.
//*  @param     -
//*  @return    -
//*/
//void FM_stm::CheckMode(void)
//{
//#if STIMULUS_ACTIVE
//
//
//	if (AbortSimulation_p != 0)
//	{
//		if (AbortSimulation_p())
//		{
//			Mode_u8 = MODE_OFF;
//		}
//	}
//
//#endif
//}
//
//
///**
//*  @brief     Get the mode state value of the stimulus object
//*  @param     -
//*  @return    Mode_u8
//*/
//uint8_t  FM_stm::GetModeState()
//{
//	return Mode_u8;
//}
//
//
///**
//*  @brief     This is an overriding function to enable the stimulus.
//*             The function shall enable the mode and assign the value
//*             to the stimulus variable.
//*  @param     Fault type - uint8_t
//*  @return    none
//*/
//void FM_stm::SetStimulus(uint8_t FltType_u8)
//{
//    Mode_u8 = MODE_ON;
//	FaultForceValue_u8 = FltType_u8;
//    return;
//}
//
///**
//*  @brief     This is an overriding function to disable the stimulus.
//*             The function shall disable the mode variable.
//*  @param     none
//*  @return    none
//*/
//void FM_stm::ResetStimulus()
//{
//    Mode_u8 = MODE_OFF;
//    AbortSimulation_p = 0;
//}




//FM_stm  FM_stm_t[NUM_OF_FAULTPATHS_E];
