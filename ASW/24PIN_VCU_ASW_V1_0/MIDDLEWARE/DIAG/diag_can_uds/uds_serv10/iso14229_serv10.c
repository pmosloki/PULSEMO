/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv10.c
|    Project      	: MIL_PBL_CV
|    Description    : Service description for UDS service  - Session Control
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

#ifndef ISO14229_SID10_C
#define ISO14229_SID10_C
/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "iso14229_serv10.h"
#include "iso14229_serv10_conf.h"
//#include "main.h"
#include "fee_adapt.h"
#include "uds_DID.h"
#include"pal_can_if.h"
#include"pal_can_if.h"
#include"nvm_conf.h"
/*******************************************************************************
 *  macros
 ******************************************************************************/
/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
static uint8_t UdsServ10State_u8 = IDLE_STATE_E;
uint8_t UdsServ10RespPending_u8 = false;
static uint32_t UdsServ10Timer_u32 = 0;
uint8_t Uds10CondNotCorrect_u8 = 0;
static uint8_t ActiveDiagSessType_u8 = DIAG_SESS_TYPE_DEFAULT;
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_Default_Diag_Init
 *   Description   : The function is used to initialize the session  to default
 *          diagnostic session, All flags are reset and all variables are re-initialized.
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
uint8_t GetDiagSessDescriptor(uint8_t SessionType_u8, UdsDiagSession_St_t **UdsDiagSession_pSt);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_SERV10_Session
 *   Description   : The function is used to initialize the session,
 *          All flags are reset and all variables are re-initialized.
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : iso14229_serv10
 *   Description   : The function process the Service_10 request
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv10(UDS_Serv_St_t *UDS_Serv_pSt)
{
	static UdsDiagSession_St_t* DiagSessDescptr_pSt = NULL;
	static uint8_t DiagSessionType_u8 = 0;
	static uint8_t Uds10ReqLen_u8 = 0;
	static uint8_t Uds10RespSuppress_u8 = 0;
	UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
	DidConf_st_t Did_St;

	if(IDLE_STATE_E == UdsServ10State_u8)
	{
		Uds10ReqLen_u8 = UDS_Serv_pSt->RxLen_u16;
		DiagSessionType_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE]&SUB_FUNC_MASK_VALUE;
		Uds10RespSuppress_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE]&POS_RESP_MASK_SUPRESS_VALUE;
	}

	if (WAIT_PENDING_E == UdsServ10State_u8)
	{
		if (UdsServ10Timer_u32 < GET_TIME_MS())
		{
			UdsServ10State_u8 = IDLE_STATE_E;
			UDS_Serv_pSt->TxLen_u16 = THREE;
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SESSIONCONTROL;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = GENERAL_REJECT;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
		else
		{
			Serv_resptype_En = UDS_SERV_RESP_WAITPEND_E;
		}
	}
	else if (!GetDiagSessDescriptor(DiagSessionType_u8, &DiagSessDescptr_pSt))
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SESSIONCONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!DiagSessionCheck(DiagSessDescptr_pSt->SupportedSession_u16))
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SESSIONCONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SUBFUNC_NOSUPP_IN_ACTIVE_SESS;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(UDS_SID10_REQ_LEN != Uds10ReqLen_u8)
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SESSIONCONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else 
	{
		if(NULL != DiagSessDescptr_pSt->callbackFptr);
		{
			DiagSessDescptr_pSt->callbackFptr();
		}

		if (true == Uds10CondNotCorrect_u8)
		{
			Uds10CondNotCorrect_u8 = false;
			UDS_Serv_pSt->TxLen_u16 = THREE;
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SESSIONCONTROL;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
		else if(POS_RESP_SUPPRESS_VALUE == Uds10RespSuppress_u8)
		{
			ActiveDiagSessType_u8 = DiagSessDescptr_pSt->DiagSessionType_u8;
			Did_St.DID_u16 = DID_ACTIVE_DIAG_SESSION;
			Did_St.DidLen_u16 = ACTIVE_DIAG_SESION_SIZE;
			Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
			(void)DidWrite(&Did_St, &ActiveDiagSessType_u8);
			UDS_Security_Lock();
			Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
		}
		else if (true == UdsServ10RespPending_u8)
		{
			ActiveDiagSessType_u8 = DiagSessDescptr_pSt->DiagSessionType_u8;
			UdsServ10RespPending_u8 = false;
			UdsServ10State_u8 = WAIT_PENDING_E;
			UdsServ10Timer_u32 =  GET_TIME_MS() + P2_SERVER_MAX;
			Serv_resptype_En = UDS_SERV_RESP_WAITPEND_E;
		}
		else
		{
			UDS_Serv_pSt->TxLen_u16 = FIVE;
			ActiveDiagSessType_u8 = DiagSessDescptr_pSt->DiagSessionType_u8;
			Did_St.DID_u16 = DID_ACTIVE_DIAG_SESSION;
			Did_St.DidLen_u16 = ACTIVE_DIAG_SESION_SIZE;
			Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
			(void)DidWrite(&Did_St, &ActiveDiagSessType_u8);
			UDS_Serv_pSt->TxBuff_pu8[ONE] = ActiveDiagSessType_u8;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = (uint8_t)DEFAULT_P2_CAN_MAX_HB;
			UDS_Serv_pSt->TxBuff_pu8[THREE] = (uint8_t)DEFAULT_P2_CAN_MAX;
			UDS_Serv_pSt->TxBuff_pu8[FOUR] = (uint8_t)ENHANCED_P2_CAN_MAX_HB;
			UDS_Serv_pSt->TxBuff_pu8[FIVE] = (uint8_t)ENHANCED_P2_CAN_MAX;
			UDS_Security_Lock();
			Serv_resptype_En = UDS_SERV_RESP_POS_E;
		}
	}
	return Serv_resptype_En;
}
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : GetDiagSessDescriptor
 *   Description   : The function provide the Diag session table descriptor
 *   Parameters    : SessionType_u8
 * 					 UdsDiagSession_St
 *   Return Value  : None
 *******************************************************************************/
uint8_t GetDiagSessDescriptor(uint8_t SessionType_u8, UdsDiagSession_St_t **UdsDiagSession_pSt)
{
	uint8_t State_u8 = false;
	uint8_t LoopCnt_u8 = 0;

	for(LoopCnt_u8 = 0;LoopCnt_u8<TOTAL_DIAG_SESSION;LoopCnt_u8++)
	{
		if(SessionType_u8 == UdsDiagSession_aSt[LoopCnt_u8].DiagSessionType_u8)
		{
			*UdsDiagSession_pSt = (UdsDiagSession_St_t *)&UdsDiagSession_aSt[LoopCnt_u8];
			State_u8 = true;
			break;
		}
	}

	return State_u8;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UdsServ10Timeout
 *   Description   : The function provide the Diag session table descriptor
 *   Parameters    : SessionType_u8
 * 					 UdsDiagSession_St
 *   Return Value  : None
 *******************************************************************************/
void UdsServ10Timeout(void)
{
	DidConf_st_t Did_St;
	if(ActiveDiagSessType_u8 != DIAG_SESS_TYPE_DEFAULT)
	{
		ActiveDiagSessType_u8 = DIAG_SESS_TYPE_DEFAULT;
		Did_St.DID_u16 = DID_ACTIVE_DIAG_SESSION;
		Did_St.DidLen_u16 = ACTIVE_DIAG_SESION_SIZE;
		Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
		(void)DidWrite(&Did_St, &ActiveDiagSessType_u8);
	}

	return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UdsServ10Timeout
 *   Description   : The function provide the Diag session table descriptor
 *   Parameters    : SessionType_u8
 * 					 UdsDiagSession_St
 *   Return Value  : None
 *******************************************************************************/
uint8_t DiagSessionCheck(uint16_t ReqSessEquValue_u16)
{
	uint8_t State_u8 = false;
	uint8_t LoopCnt_u8 = 0;

	for(LoopCnt_u8 = 0;LoopCnt_u8<TOTAL_DIAG_SESSION;LoopCnt_u8++)
	{
		if(ActiveDiagSessType_u8 == UdsDiagSession_aSt[LoopCnt_u8].DiagSessionType_u8)
		{
			if(UdsDiagSession_aSt[LoopCnt_u8].DiagSessEquValue_u8&ReqSessEquValue_u16)
			{
				State_u8 = true;
			}
			break;
		}
	}
	if(State_u8 == false)
	{
		State_u8 = false;
	}

	return State_u8;
}

void UDS_Serv10Init(void)
{
	uint8_t DidDataVerfBuff_au8[ACTIVE_DIAG_SESION_SIZE] = {0}, DidDataBuff_au8[ACTIVE_DIAG_SESION_SIZE] = {0x01};
	
	DidConf_st_t Did_St;
	Did_St.DID_u16 = DID_ACTIVE_DIAG_SESSION;
	Did_St.DidLen_u16 = ACTIVE_DIAG_SESION_SIZE;
	Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
	/* Comparing the NVM value with Flash value. If not equal update the NVM value with flash value*/
	(void)DidRead(&Did_St,DidDataVerfBuff_au8);

	if (false == (bool)CompareBuff(DidDataVerfBuff_au8, DidDataBuff_au8, (uint32_t)ACTIVE_DIAG_SESION_SIZE))
	{
		ActiveDiagSessType_u8 = DIAG_SESS_TYPE_DEFAULT;
		Did_St.DID_u16 = DID_ACTIVE_DIAG_SESSION;
		Did_St.DidLen_u16 = ACTIVE_DIAG_SESION_SIZE;
		Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
		(void)DidWrite(&Did_St, &ActiveDiagSessType_u8);
	}
	return;
}
#endif /* ISO14229_SID10_C */
/*---------------------- End of File -----------------------------------------*/
