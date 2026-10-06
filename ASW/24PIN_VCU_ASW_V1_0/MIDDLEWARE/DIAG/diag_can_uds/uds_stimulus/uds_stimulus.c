
/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File          : uds_stimulus.c
|    Project      : MIL_PBL_CV
|    Description    : stimulus functions for handling DID for IOCBLI service.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S        Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/


#include "uds_stimulus.h"


/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
 
/**
*  FUNCTION NAME : UDS_Stimulus
*  FILENAME      : uds_stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @param        : srcValue_pu8 pointing to Real data
*  @param        : dstValue_pu8 pointing to global variable
*  @brief        : This function will return the forcevalue or realvalue depends on mode.
*  @return       : Non.                     
*/


//void UDS_Stimulus(ISO14229_DidList_En_t DidList_En ,  uint8_t *srcValue_pu8 ,  uint8_t *dstValue_pu8 );

/**
*  FUNCTION NAME : UDS_SetStimulus
*  FILENAME      : uds_stimulus.c
*  @param        : ForceValue_pu8  pointer to forcevalue 
*  @brief        : This function will set mode to true and assign forcevalue
                   to the respective forcevalues in the structure.
*  @return       : void                     
*/

//void  UDS_SetStimulus(ISO14229_DidList_En_t DidList_En, uint8_t *ForceValue_pu8);


 /**
*  FUNCTION NAME : UDS_ResetStimulus
*  FILENAME      : uds_stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @brief        : This function will set mode to false.
*  @return       : void                     
*/
//void   UDS_ResetStimulus(ISO14229_DidList_En_t DidList_En);


 /**
*  FUNCTION NAME : UDS_Stimulus
*  FILENAME      : uds_stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @param        : srcValue_vp
*  @param        : dstValue_vp 
*  @brief        : This function will return the forcevalue or realvalue depends on mode.
*  @return       : Non.                     
*/

//void UDS_Stimulus(ISO14229_DidList_En_t DidList_En ,  uint8_t *srcValue_pu8 ,  uint8_t *dstValue_pu8 )
//{
  
//   uint32_t i=0;
//   if(DID_Config_aSt[DidList_En].STM_st != NULL)
//   {
//		if(DID_Config_aSt[DidList_En].STM_st->mode_b)
//		{
			
//			for(i=0;i<DID_Config_aSt[DidList_En].DID_Val_Size;i++) 
//				{
//				*(dstValue_pu8+i) = (*((uint8_t *)(DID_Config_aSt[DidList_En].STM_st->fv_vp)+i));
//				}
//		}
		
//		else
//		{
//			for(i=0;i<DID_Config_aSt[DidList_En].DID_Val_Size;i++)  
//				{
//				*(dstValue_pu8+i) = *(srcValue_pu8+i);
//				}
//		}
//   }
//}



 /**
*  FUNCTION NAME : UDS_SetStimulus
*  FILENAME      : uds_stimulus.c
*  @param        : ForceValue_pu8  pointer to forcevalue 
*  @brief        : This function will set mode to true and assign forcevalue
                   to the respective forcevalues in the structure.
*  @return       : void                     
*/


//void  UDS_SetStimulus(ISO14229_DidList_En_t DidList_En, uint8_t *ForceValue_pu8)
//{
  
//	uint32_t i=0;
//	DID_Value_Status_En_t DID_categary_En = DID_Config_aSt[DidList_En].Type_En;
//	uint8_t ControlByte_u8=ForceValue_pu8[DID_Config_aSt[DidList_En].DID_Val_Size];
	
//	if(DID_Config_aSt[DidList_En].STM_st != NULL) {
//		DID_Config_aSt[DidList_En].STM_st->mode_b = true;
		
//		switch(DID_categary_En) {
//		case BIT_MAPPED_E:
//			for(i=0;i<8;i++) {
//				if(ControlByte_u8 & (0x01 << (7 - i))) {
//					if(ForceValue_pu8[0] & (0x01 << i)) {
//						(*(uint8_t *)(DID_Config_aSt[DidList_En].STM_st->fv_vp)) |= (0x01 << i);
//					} else {
//						(*(uint8_t *)(DID_Config_aSt[DidList_En].STM_st->fv_vp)) &= ~(0x01 << i);
//					}
//				}
//			}
//			break;
			
//		case PACKETED_E:
//		case NUMERIC_E:
//		case STATE_ENCODED_E:
//			for(i=0;i<DID_Config_aSt[DidList_En].DID_Val_Size ;i++)	{   
//				if(ControlByte_u8 & (0x01 << i)) {
//					(*((uint8_t *)(DID_Config_aSt[DidList_En].STM_st->fv_vp)+i))= ForceValue_pu8[i];
//				}
//			}
//			break;
//		}
//		UDS_Stimulus(DID_Config_aSt[DidList_En].DID_Name_En, DID_Config_aSt[DidList_En].Real_Time_Val_u8, DID_Config_aSt[DidList_En].Var_ptr_pu8);
//	}
//}
 

 /**
*  FUNCTION NAME : UDS_ResetStimulus
*  FILENAME      : uds_stimulus.c
*  @param        : DidList_En DidList Enum, 
*  @brief        : This function will set mode to false.
*  @return       : void                     
*/
//void   UDS_ResetStimulus(ISO14229_DidList_En_t DidList_En)
//{
//	if(DID_Config_aSt[DidList_En].STM_st != NULL) 
//    {
//		DID_Config_aSt[DidList_En].STM_st->mode_b = false;
//		UDS_Stimulus(DID_Config_aSt[DidList_En].DID_Name_En, DID_Config_aSt[DidList_En].Real_Time_Val_u8, DID_Config_aSt[DidList_En].Var_ptr_pu8);
//	}
//}
