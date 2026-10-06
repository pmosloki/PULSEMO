/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File         : iso14229_serv14.c
|    Module       : iso14229_serv14.c file
|    Description  : This file contains variables and functions of iso14229
|                     service 14.
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

#ifndef _UDS_SERV14_C_
#define	_UDS_SERV14_C_

/**********************************************************************************
				 includes
  *******************************************************************************/
#include "iso14229_serv14.h"
#include "diag_typedefs.h"
#include "uds_conf.h"
#include "diag_sys_conf.h"
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
#include "fm.h"
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

uint8_t DTC_ClearedFlag;

/*
 ****************************************************************************************
 *    Function Definition
 ****************************************************************************************
 */

/*
 *  FUNCTION NAME : iso14229_serv14
 *  FILENAME      : iso14229_serv14.c
 *  @param        : UDS_Serv_pSt - pointer to service distributer table.
 *  @brief        :  This function will process the service_14 requests.
 *  @return       : Serv_resptype_En_t
 */
UDS_Serv_resptype_En_t iso14229_serv14(UDS_Serv_St_t *databuffer_pu8)
{
	uint32_t groupDTCID_u32 = 0x00u;
	uint16_t numbytes_req_u16 = databuffer_pu8->RxLen_u16;
	uint8_t i = 0x00u;
	bool found_grp_of_DTC_b = FALSE;
	bool clear_DTC_b = FALSE;
	UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_UNKNOWN_E;

	if (numbytes_req_u16 != SERV14_MIN_LEN)
	{
		databuffer_pu8->TxLen_u16 = 0x03;
		databuffer_pu8->TxBuff_pu8[0] = 0x7F;
		databuffer_pu8->TxBuff_pu8[1] = 0x14;
		databuffer_pu8->TxBuff_pu8[2] = INVALID_MESSAGE_LENGTH;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else
	{
		groupDTCID_u32 = databuffer_pu8->RxBuff_pu8[1];
		groupDTCID_u32 = (groupDTCID_u32 << 8) | databuffer_pu8->RxBuff_pu8[2];
		groupDTCID_u32 = (groupDTCID_u32 << 8) | databuffer_pu8->RxBuff_pu8[3];

		for (i = 0; i < NUMBER_OF_GROUP_DTC_IDs; i++)
		{
			if (groupDTCID_u32 == GroupOfDTCIDs_au32[i])
			{
				found_grp_of_DTC_b = TRUE;
			}
		}
		if (found_grp_of_DTC_b)
		{
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
			clear_DTC_b = FM_ClearDiagnosticInformationForDTCGroup((FM_FunctionalUnit_En_t)groupDTCID_u32);
#endif
			if (clear_DTC_b)
			{
				databuffer_pu8->TxLen_u16 = 0x00;
				Serv_resptype_En = UDS_SERV_RESP_POS_E;
				DTC_ClearedFlag = on;
			}
			else
			{
				databuffer_pu8->TxLen_u16 = 0x03;
				databuffer_pu8->TxBuff_pu8[0] = 0x7F;
				databuffer_pu8->TxBuff_pu8[1] = 0x14;
				databuffer_pu8->TxBuff_pu8[2] = GEN_PROG_FAILURE;
				Serv_resptype_En = UDS_SERV_RESP_NEG_E;
			}
		}
		else
		{
			databuffer_pu8->TxLen_u16 = 0x03;
			databuffer_pu8->TxBuff_pu8[0] = 0x7F;
			databuffer_pu8->TxBuff_pu8[1] = 0x14;
			databuffer_pu8->TxBuff_pu8[2] = REQUEST_OUT_OF_RANGE;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
	}

	return Serv_resptype_En;
}

#endif  /*_UDS_SERV14_C_*/
