/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File          : iso14229_serv22.c
|    Project      : MIL_PBL_CV
|    Description    : This file contains the export variables and functions to
|                     which can be implemented in the H file.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date            Name                      Company
| --------     ---------------------     ---------------------------------------
| 31/07/2024       Manikandan S              Sloki Software Technologies LLP.
|-------------------------------------------------------------------------------
|******************************************************************************/


/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "iso14229_serv22.h"
#include "uds_DID.h"
#include "uds_DID_conf.h"
#include"nvm_conf.h"
#include"nvm_parameter.h"
/*******************************************************************************
 *  macros
 ******************************************************************************/
#define DID_VERIFYBUFF_MAX_LEN		50
#define DID_OFFSET					1
#define MAX_BAUDRATE_INDEX   	(4U)
/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
static uint32_t Serv22_Index_u32 = 0u; /* Global Variable for accessing
				   global data buffer */
									   // todo:- LK(remove the dependency on the uds_DID.c)
/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
static uint8_t ReadDidSessionCheck(uint16_t DID_u16,DidConf_st_t** DidConfDptr_pSt);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : iso14229_serv22
 *   Description   : This function will process the service_22 requests.
 *   Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributor
 *           table.
 *   Return Value  : Type of response
 *******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv22(UDS_Serv_St_t *UDS_Serv_pSt)
{
	uint16_t DidLoopCnt_u16 = 0;
	DidConf_st_t *DidConf_pSt = NULL;			  
	uint16_t totalReqDIDs_u16 = 0;				  /* Total number of DIDs
								   requested.                 */
	uint16_t Req_DID_List[MAX_DID_REQ] = {0}; /* List of DIDs requested.    */

	bool Atleast_One_DID_Status_b = FALSE; /* Flag: if True, at-least one
							 DID is supported
										  else, None of the DIDs
										 are supported.         */
	UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_UNKNOWN_E; /*Enum
							 variable is used return a
							 corresponding response.    */
	Serv22_Index_u32 = DID_OFFSET; /* Index variable to access all
							 the data bytes in the
							 requested Frame.       */
	
	totalReqDIDs_u16 = (UDS_Serv_pSt->RxLen_u16 - SID_LEN) / SIZE_OF_ONE_DID;
	

	if ((UDS_Serv_pSt->RxLen_u16 < SERV22_MIN_LEN) || (UDS_Serv_pSt->RxLen_u16 > SERV22_MAX_LEN) || (0 != ((UDS_Serv_pSt->RxLen_u16 - SID_LEN) % SIZE_OF_ONE_DID) ))
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_RDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH; 
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else
	{
		for (DidLoopCnt_u16 = 0; DidLoopCnt_u16 < totalReqDIDs_u16; DidLoopCnt_u16++)
		{
			Req_DID_List[DidLoopCnt_u16] = (uint16_t)UDS_Serv_pSt->RxBuff_pu8[(DidLoopCnt_u16 * TWO) + ONE] << EIGHT;
			Req_DID_List[DidLoopCnt_u16] |= (uint16_t)UDS_Serv_pSt->RxBuff_pu8[(DidLoopCnt_u16 * TWO) + TWO] & BIT_U1;
		}

		/* Loop(multiple DIDs */
		for (DidLoopCnt_u16 = 0; DidLoopCnt_u16 < totalReqDIDs_u16; DidLoopCnt_u16++)
		{
			if(ReadDidSessionCheck( Req_DID_List[DidLoopCnt_u16],&DidConf_pSt))
			{
				Atleast_One_DID_Status_b = true;
				if (!iso14229_securitycheck(DidConf_pSt->ReadSecurityLvl_u8))
				{
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_RDBDID;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = SECURITY_ACCESS_DENIED;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					Serv_resptype_En = UDS_SERV_RESP_NEG_E;
					break;
				}
				else 
				{
					UDS_Serv_pSt->TxBuff_pu8[Serv22_Index_u32++] = (uint8_t)(Req_DID_List[DidLoopCnt_u16] >> EIGHT) & BIT_U1;
					UDS_Serv_pSt->TxBuff_pu8[Serv22_Index_u32++] = (uint8_t)(Req_DID_List[DidLoopCnt_u16]) & BIT_U1;
					/* Call DID read */
					DidRead(DidConf_pSt,&UDS_Serv_pSt->TxBuff_pu8[Serv22_Index_u32]);
					Serv22_Index_u32 += DidConf_pSt->DidLen_u16;
				}
			}
		}
		
		if(FALSE == Atleast_One_DID_Status_b)
		{
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_RDBDID;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
			UDS_Serv_pSt->TxLen_u16 = THREE;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
		/* response length exceeded NRC - 0x14 */
		else if (Serv22_Index_u32 > ISO15765_CONF_NUMDATABYTES)
		{
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_RDBDID;
			UDS_Serv_pSt->TxBuff_pu8[THREE] = RESP_LEN_EXCEEDED;
			UDS_Serv_pSt->TxLen_u16 = THREE;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}

		else
		{
			UDS_Serv_pSt->TxLen_u16 = (Serv22_Index_u32 -1); /* Total Number
									 of Response Bytes. */
			Serv22_Index_u32 = ZERO;
			Serv_resptype_En = UDS_SERV_RESP_POS_E; /* Positive Response */
		}
	}
	return Serv_resptype_En;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_Serv22_Timeout
 *   Description   : Function resets the relevant variables/parameters when
 *           session timeout.
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
void UDS_Serv22_Timeout(void)
{
	return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UdsServ22Init
 *   Description   : The function initialize the 22 related data
 * 					 Intilaize the Boot DID.
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
void UdsServ22Init(void)
{
	return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : AccessTheDid
*   Description   : The function write the DID data to the eeprom.
*   Parameters    : Data pointer,Did table index, access type 
*   Return Value  : true/false  pass/fail
*******************************************************************************/
uint8_t ReadDidSessionCheck(uint16_t DID_u16,DidConf_st_t** DidConfDptr_pSt)
{
	uint16_t LoopCnt_u16 = 0;
	uint8_t State_u8 = false;

	for(LoopCnt_u16 = 0;LoopCnt_u16<TOTAL_DID;LoopCnt_u16++)
	{
		if(DidConf_ast[LoopCnt_u16].DID_u16 == DID_u16)
		{
			*DidConfDptr_pSt = (DidConf_st_t*)&DidConf_ast[LoopCnt_u16];
			if(UdsDiagSessionCheck(DidConf_ast[LoopCnt_u16].ReadDiagSession_u8))
			{
				State_u8 = true;
			}
			else
			{
				State_u8 = false;
			}
			break;
		}
	}	
	return State_u8;
}
/*---------------------- End of File -----------------------------------------*/
