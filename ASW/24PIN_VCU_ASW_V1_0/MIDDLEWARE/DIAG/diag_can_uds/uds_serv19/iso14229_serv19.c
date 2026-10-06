/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File         : iso14229_serv19.c
|    Module       : iso14229_serv19.c file
|    Description  : This file contains variables and functions of iso14229
|                     service 19.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
| 06/12/2021        Sandeep KY              Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
************************************************************************** */

#ifndef _UDS_SERV19_C_
#define	_UDS_SERV19_C_
/**********************************************************************************
                includes
 *******************************************************************************/
#include "iso14229_serv19.h"
#include "diag_typedefs.h"
#include "uds_conf.h"
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

/*
 ****************************************************************************************
 *    Function Definition
 ****************************************************************************************
 */

/*
 *  FUNCTION NAME : iso14229_serv19
 *  FILENAME      : iso14229_serv19.c
 *  @param        :
 *  @brief        :
 *  @return       : Serv_resptype_En_t
 */
UDS_Serv_resptype_En_t iso14229_serv19(UDS_Serv_St_t *UDS_Serv_pSt)
{
  int16_t response_length_s16 = 0x00u;
  uint16_t numbytes_req_u16 = UDS_Serv_pSt->RxLen_u16;
  uint8_t ser_buffer_au8[250] = {0};
  uint8_t sub_fun_id_u8 = 0x00u;
  uint8_t i = 0x00;
  bool sub_function_identified_b = FALSE;
  UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_UNKNOWN_E;
  sub_fun_id_u8 = UDS_Serv_pSt->RxBuff_pu8[1];

  for (i = 0; i < (numbytes_req_u16 - 1); i++)
  {
    ser_buffer_au8[i] = UDS_Serv_pSt->RxBuff_pu8[i + 1];
  }

  for (i = 0; i < SERV19_NUMOF_SUB_FUNCTIONS; i++)
  {
    if (sub_fun_id_u8 == Service19_SubFunctionsaSt[i].SUB_FUNCTION_ID)
    {
      sub_function_identified_b = TRUE;
      if (numbytes_req_u16 == Service19_SubFunctionsaSt[i].DTC_SubFun_Valide_Length_u8)
      {
        response_length_s16 = (int16_t)(Service19_SubFunctionsaSt[i].Subfun_Fptr_t)(ser_buffer_au8);
        if (response_length_s16 >= 0)
        {
          for (i = 0; i < response_length_s16; i++)
          {
            UDS_Serv_pSt->TxBuff_pu8[i + 1] = ser_buffer_au8[i];
          }
          UDS_Serv_pSt->TxLen_u16 = response_length_s16;
          Serv_resptype_En = UDS_SERV_RESP_POS_E;
          break;
        }
        else
        {
          UDS_Serv_pSt->TxBuff_pu8[0] = NEGATIVE_RESP;
          UDS_Serv_pSt->TxBuff_pu8[1] = SID_RDTC;
          UDS_Serv_pSt->TxBuff_pu8[2] = REQUEST_OUT_OF_RANGE;
          UDS_Serv_pSt->TxLen_u16 = 3;
          Serv_resptype_En = UDS_SERV_RESP_NEG_E;
        }
      }
      else
      {
        UDS_Serv_pSt->TxBuff_pu8[0] = 0x7F;
        UDS_Serv_pSt->TxBuff_pu8[1] = 0x19;
        UDS_Serv_pSt->TxBuff_pu8[2] = INVALID_MESSAGE_LENGTH;
        UDS_Serv_pSt->TxLen_u16 = 3;
        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
        break;
      }
    }
  }
  if (numbytes_req_u16 < SERV19_MIN_LEN)
  {
    UDS_Serv_pSt->TxBuff_pu8[0] = 0x7F;
    UDS_Serv_pSt->TxBuff_pu8[1] = 0x19;
    UDS_Serv_pSt->TxBuff_pu8[2] = INVALID_MESSAGE_LENGTH;
    UDS_Serv_pSt->TxLen_u16 = 3;
    Serv_resptype_En = UDS_SERV_RESP_NEG_E;
  }

  else if (!(sub_function_identified_b))
  {
    UDS_Serv_pSt->TxBuff_pu8[0] = NEGATIVE_RESP;
    UDS_Serv_pSt->TxBuff_pu8[1] = SID_RDTC;
    UDS_Serv_pSt->TxBuff_pu8[2] = SUB_FUNC_NOT_SUPPORTED;
    UDS_Serv_pSt->TxLen_u16 = 3;
    Serv_resptype_En = UDS_SERV_RESP_NEG_E;
  }
  return Serv_resptype_En;
}

#endif
