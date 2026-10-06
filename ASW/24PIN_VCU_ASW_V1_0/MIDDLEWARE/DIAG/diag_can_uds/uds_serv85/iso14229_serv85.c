/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File            : iso14229_serv85.c
|    Project        : MIL_PBL_CV
|    Description    : This file contains Control DTC Setting Service(0x85).
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/
    
/**********************************************************************************
                                includes
 *******************************************************************************/
#include "iso14229_serv85.h"
//#include <stdint.h>
#include "diag_typedefs.h"
#include "diag_sys_conf.h"
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
	#include "fm.h"
	#include "fm_level1.h"
	#include "uds_DTC.h"
#endif
/*
 ****************************************************************************************
 *    functions prototype
 ****************************************************************************************
*/

/*
 ****************************************************************************************
 *    Static functions
 ****************************************************************************************
*/


/*
 ****************************************************************************************
 *    Global Variables
 ****************************************************************************************
*/
// FM_DTC_SettingsOff_b = FALSE;


/*
 ****************************************************************************************
 *    Function Definition
 ****************************************************************************************
*/

//fm.c

/*
 *  FUNCTION NAME : iso14229_serv85
 *  FILENAME      : iso14229_serv85.c
 *  @param        : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer table.
 *  @brief        : This function will process the service_85 requests
 *  @return       : Serv_resptype_En_t  
 */
UDS_Serv_resptype_En_t iso14229_serv85(UDS_Serv_St_t*  UDS_Serv_pSt)  
{
  uint16_t numbytes_req_u16 = UDS_Serv_pSt->RxLen_u16;
  uint8_t sub_fun_id_u8     = 0x00u ;
  uint8_t SuppressPosRes_u8 = (UDS_Serv_pSt->RxBuff_pu8[1] & 0x80);
  UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_UNKNOWN_E;
  
  sub_fun_id_u8           = UDS_Serv_pSt->RxBuff_pu8[1];
  

  if( numbytes_req_u16 != SERV85_MIN_LEN )
  {
      UDS_Serv_pSt->TxBuff_pu8[0] = NEGATIVE_RESP;
      UDS_Serv_pSt->TxBuff_pu8[1] = SID_CONTROLDTC;
      UDS_Serv_pSt->TxBuff_pu8[2] = INVALID_MESSAGE_LENGTH;
      UDS_Serv_pSt->TxLen_u16     = LENGTH;
      Serv_resptype_En            = UDS_SERV_RESP_NEG_E;
  }
  else
  {
     sub_fun_id_u8 = (sub_fun_id_u8 & 0x7F);
      switch(sub_fun_id_u8)
      {
		 
          case DTC_SETTINGS_ON_E:
          {
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
        	  FM_DTC_SettingsOff_b        = FALSE;
#endif
              UDS_Serv_pSt->TxBuff_pu8[1] = DTC_SETTINGS_ON_E;
              UDS_Serv_pSt->TxLen_u16     = 1;
              Serv_resptype_En            = UDS_SERV_RESP_POS_E;
              break;
          }
          case DTC_SETTINGS_OFF_E:
          {
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
        	  FM_DTC_SettingsOff_b 	  = TRUE;
#endif
        	 UDS_Serv_pSt->TxBuff_pu8[1] = DTC_SETTINGS_OFF_E;
                 UDS_Serv_pSt->TxLen_u16     = 1;
                 Serv_resptype_En            = UDS_SERV_RESP_POS_E;
                 break;
          }
          default:
          {
              UDS_Serv_pSt->TxBuff_pu8[0] = NEGATIVE_RESP ;
              UDS_Serv_pSt->TxBuff_pu8[1] = SID_CONTROLDTC ;
              UDS_Serv_pSt->TxBuff_pu8[2] = SUB_FUNC_NOT_SUPPORTED ;
              UDS_Serv_pSt->TxLen_u16     = LENGTH;
              Serv_resptype_En            = UDS_SERV_RESP_NEG_E;
          }
      }
	  if(SuppressPosRes_u8 && Serv_resptype_En  == UDS_SERV_RESP_POS_E)
      {
         Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
      }
  }
  return Serv_resptype_En ; 
}
