/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File         : iso14229_serv2E.c
|    Project      : MIL_PBL_CV
|    Description  : This file contains the export variables and functions to
|                     which can be implemented in the H file.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date            Name                      Company
| --------     ---------------------     ---------------------------------------
|  31/07/2024       Manikandan S            Sloki Software Technologies LLP.
|-------------------------------------------------------------------------------
|******************************************************************************/


/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "iso14229_serv2E.h"
#include "uds_DID.h"
#include "uds_DID_conf.h"
/*******************************************************************************
 *  macros
 ******************************************************************************/
#define SERV2E_HEADER_BYTES 3u
#define DID_OFFSET			1
/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
uint16_t DID_Data_Length_u16 = 0; /* Variable to store DID data length         */

/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
static uint8_t WriteDidSessionCheck(uint16_t DID_u16,DidConf_st_t** DidConfDptr_pSt);
static uint8_t WriteDidAccessTypeCheck(DidConf_st_t* WriteDidConf_pst);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : iso14229_serv2E
 *   Description   : This function will process the service_2E requests(WRITE
 *           DATA BY IDENTIFIER).
 *   Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer
 *              table.
 *   Return Value  : Type of response(Positive Or Negative Or Unknown).
 *******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv2E(UDS_Serv_St_t *UDS_Serv_pSt)
{
	uint16_t RxDID_u16 = 0;		  /* Variable to store DID number         */
	DidConf_st_t *DidConf_pSt = NULL;
	UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_UNKNOWN_E;
	DID_Data_Length_u16 = UDS_Serv_pSt->RxLen_u16 - SERV2E_HEADER_BYTES;


	/* Storing higher byte of DID */
	RxDID_u16 = UDS_Serv_pSt->RxBuff_pu8[ONE] & BIT_U1;
	RxDID_u16 = (RxDID_u16 << EIGHT) | (UDS_Serv_pSt->RxBuff_pu8[TWO]&BIT_U1);

	if(UDS_Serv_pSt->RxLen_u16 < UDS_SID2E_MIN_LEN)
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!WriteDidSessionCheck(RxDID_u16,&DidConf_pSt))
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(DID_Data_Length_u16 != DidConf_pSt->DidLen_u16)
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!iso14229_securitycheck(DidConf_pSt->WriteSecurityLvl_u8))
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SECURITY_ACCESS_DENIED;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!WriteDidAccessTypeCheck(DidConf_pSt))
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if((NULL != DidConf_pSt->CB_outofrange_Fptr) && (TRUE != DidConf_pSt->CB_outofrange_Fptr(RxDID_u16,UDS_Serv_pSt)))
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!DidWrite(DidConf_pSt,&UDS_Serv_pSt->RxBuff_pu8[SERV2E_HEADER_BYTES]))
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_WDBDID;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = GEN_PROG_FAILURE;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else
	{
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_WDBDID;   // to do delete
		UDS_Serv_pSt->TxLen_u16 = TWO;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = (uint8_t)(RxDID_u16 >> EIGHT) & BIT_U1;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = (uint8_t)(RxDID_u16) & BIT_U1;
		Serv_resptype_En = UDS_SERV_RESP_POS_E;
	}

	return Serv_resptype_En;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : AccessTheDid
*   Description   : The function write the DID data to the eeprom.
*   Parameters    : Data pointer,Did table index, access type 
*   Return Value  : true/false  pass/fail
*******************************************************************************/
uint8_t WriteDidSessionCheck(uint16_t DID_u16,DidConf_st_t** DidConfDptr_pSt)
{
	uint16_t LoopCnt_u16 = 0;
	uint8_t State_u8 = false;

	for(LoopCnt_u16 = 0;LoopCnt_u16<TOTAL_DID;LoopCnt_u16++)
	{
		if(DidConf_ast[LoopCnt_u16].DID_u16 == DID_u16)
		{
			*DidConfDptr_pSt = (DidConf_st_t*)&DidConf_ast[LoopCnt_u16];
			if(UdsDiagSessionCheck(DidConf_ast[LoopCnt_u16].WriteDiagSession_u8))
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
/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : WriteDidAccessTypeCheck
*   Description   : The function write the DID data to the eeprom.
*   Parameters    : WriteDidConf_pst 
*   Return Value  : true/false  pass/fail
*******************************************************************************/
uint8_t WriteDidAccessTypeCheck(DidConf_st_t* WriteDidConf_pst)
{
	uint16_t LoopCnt_u16 = 0;
	uint8_t State_u8 = false;

	for(LoopCnt_u16 = 0;LoopCnt_u16<TOTAL_DID;LoopCnt_u16++)
	{
		if(DidConf_ast[LoopCnt_u16].DID_u16 == WriteDidConf_pst->DID_u16)
		{
			if(WriteDidConf_pst->AccessType_u8 == WRITE_ONLY_E || WriteDidConf_pst->AccessType_u8 == READ_WRITE_E)
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
