/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: uds_serv3E.c
|    Project      	: MIL_PBL_CV
|    Description    : Service description for UDS service - Tester present
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
|31/07/2024       Manikandan S         Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

#ifndef UDS_SERV3E_C
#define UDS_SERV3E_C

/*******************************************************************************
 *  Includes
 ******************************************************************************/

#include "iso14229_serv3E.h"

/*******************************************************************************
 *  macros
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : iso14229_serv3E
 *   Description   : This function is a demo function.
 *   Parameters    : UDS_Serv_St_t*
 *   Return Value  : UDS_Serv_resptype_En_t
 *******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv3E(UDS_Serv_St_t *UDS_Serv_pSt)
{
	UDS_Serv_resptype_En_t UDS_Serv_resptype_En;

	if (UDS_SERV3E_LEN == UDS_Serv_pSt->RxLen_u16)
	{
		if (UDS_Serv_pSt->RxBuff_pu8[UDS_SERV3E_IDX_SUB_F] == (0x80))
		{

			/* to do resest timer*/
			UDS_Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
		}
		else if (UDS_Serv_pSt->RxBuff_pu8[UDS_SERV3E_IDX_SUB_F] == (0x00))
		{
			UDS_Serv_pSt->TxBuff_pu8[0] = SID_TESTERPRESENT;
			UDS_Serv_pSt->TxBuff_pu8[1] = 0x00;
			/* to do resest timer*/
			UDS_Serv_pSt->TxLen_u16 = UDS_SERV3E_LEN-1;
			UDS_Serv_resptype_En = UDS_SERV_RESP_POS_E;
		}
		else
		{

			UDS_Serv_pSt->TxBuff_pu8[0] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[1] = SID_TESTERPRESENT;
			UDS_Serv_pSt->TxBuff_pu8[2] = SUB_FUNC_NOT_SUPPORTED;
			UDS_Serv_pSt->TxLen_u16 = 0x03; /* to do resest timer*/
			UDS_Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
	}
	else
	{
		UDS_Serv_pSt->TxBuff_pu8[0] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[1] = SID_TESTERPRESENT;
		UDS_Serv_pSt->TxBuff_pu8[2] = INVALID_MESSAGE_LENGTH;
		UDS_Serv_pSt->TxLen_u16 = 0x03; /* to do resest timer*/
		UDS_Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	return UDS_Serv_resptype_En;
}

void UDS_Serv3E_Timeout(void)
{
	return;
}

#endif /* UDS_SERV3E_C */
/*---------------------- End of File -----------------------------------------*/
