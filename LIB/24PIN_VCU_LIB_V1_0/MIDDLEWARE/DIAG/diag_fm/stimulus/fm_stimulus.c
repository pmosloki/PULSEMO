
/******************************************************************************
 *    FILENAME    : fm_stimulus.c
 *    DESCRIPTION : fm_stimulus
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     		   Initial version
 ******************************************************************************
*/  


#include "fm_stimulus.h"
#include "fm.h"


/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
 
 /**
*  FUNCTION NAME : FM_Stimulus
*  FILENAME      : stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @param        : srcValue_pu8 pointing to Real data
*  @param        : dstValue_pu8 pointing to global variable
*  @brief        : This function will return the forcevalue or realvalue depends on mode.
*  @return       : Non.                     
*/


void FM_Stimulus(FM_FaultPath_En_t FaultPath_En ,  uint8_t RealFltType);

 /**
*  FUNCTION NAME : SetStimulus
*  FILENAME      : stimulus.c
*  @param        : ForceValue_pu8  pointer to forcevalue 
*  @brief        : This function will set mode to true and assign forcevalue
                   to the respective forcevalues in the structure.
*  @return       : void                     
*/

void  FM_SetStimulus(FM_FaultPath_En_t FaultPath_En ,  uint8_t ForceFltType);


 /**
*  FUNCTION NAME : ResetStimulus
*  FILENAME      : stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @brief        : This function will set mode to false.
*  @return       : void                     
*/
void   FM_ResetStimulus(FM_FaultPath_En_t FaultPath_En);


 /**
*  FUNCTION NAME : Stimulus
*  FILENAME      : stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @param        : srcValue_vp
*  @param        : dstValue_vp 
*  @brief        : This function will return the forcevalue or realvalue depends on mode.
*  @return       : Non.                     
*/

void FM_Stimulus(FM_FaultPath_En_t FaultPath_En ,  uint8_t RealFltType )
{
  
//   uint32_t i=0;
   return;
}



 /**
*  FUNCTION NAME : SetStimulus
*  FILENAME      : stimulus.c
*  @param        : ForceValue_pu8  pointer to forcevalue 
*  @brief        : This function will set mode to true and assign forcevalue
                   to the respective forcevalues in the structure.
*  @return       : void                     
*/


void  FM_SetStimulus(FM_FaultPath_En_t FaultPath_En ,  uint8_t ForceFltType)
{
  
//	uint32_t i=0;
	return;
}
 

 /**
*  FUNCTION NAME : ResetStimulus
*  FILENAME      : stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @brief        : This function will set mode to false.
*  @return       : void                     
*/
void   FM_ResetStimulus(FM_FaultPath_En_t FaultPath_En)
{
	return;
}
