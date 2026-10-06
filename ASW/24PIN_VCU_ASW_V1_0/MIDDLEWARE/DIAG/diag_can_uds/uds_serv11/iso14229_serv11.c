/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv11.c
|    Project      	: MIL_PBL_CV
|    Description    : Service description for UDS service  - ERCReset
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

#ifndef UDS_SERV11_C_
#define UDS_SERV11_C_

/*******************************************************************************
 *  Includes
 ******************************************************************************/

#include "iso14229_serv11.h"
#include"iso14229_serv11_conf.h"
#include"fee_adapt.h"
#include"pal_can_if.h"
#include"pal_can_if.h"
/*******************************************************************************
 *  macros
 ******************************************************************************/
#define ENRAPID_SHUTDOWN_TYPE 0x04
#define RAPID_POWER_SHUTDOWN_TIME       0x10
/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
static uint8_t UdsServ11State_u8 = IDLE_STATE_E;
uint8_t ResetInProgress_u8 = false;
static uint32_t UdsServ11Timer_u32 = 0;
uint8_t Uds11CondNotCorrect_u8 = 0;
uint8_t ResetProcess_u8 = 0;


uint8_t GetEcuResetDescriptor(uint8_t ResetType_u8, UdsEcuReset_St_t **UdsEcuReset_pSt);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : iso14229_serv11
 *   Description   : Function is used to process reset of micro-controller
 *   Parameters    : UDS_Serv_St_t
 *   Return Value  : UDS_Serv_resptype_En_t
 *******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv11(UDS_Serv_St_t *UDS_Serv_pSt)
{
	UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
	UdsEcuReset_St_t*  EcuResetDescriptor_pSt = NULL;
	static uint8_t EcuResetType_u8 = 0;
	static uint8_t Uds11ReqLen_u8 = 0;
	static uint8_t Uds11RespSuppress_u8 = 0;
	uint8_t ResetProcess_u8 = 0;

	if(IDLE_STATE_E == UdsServ11State_u8)
	{
		Uds11ReqLen_u8 = UDS_Serv_pSt->RxLen_u16;
		EcuResetType_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE]&SUB_FUNC_MASK_VALUE;
		Uds11RespSuppress_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE]&POS_RESP_MASK_SUPRESS_VALUE;
	}

	if (WAIT_PENDING_E == UdsServ11State_u8)
	{
		if (UdsServ11Timer_u32 < GET_TIME_MS())
		{
			UDS_Serv_pSt->TxLen_u16 = THREE;
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ECURESET;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = GENERAL_REJECT;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
		else if(true == ResetProcess_u8)
		{
			if (ENRAPID_SHUTDOWN_TYPE == EcuResetDescriptor_pSt->ResetType_u8)
			{
				UDS_Serv_pSt->TxBuff_pu8[ZERO] = UDS_SERVICE11_SID;
				UDS_Serv_pSt->TxBuff_pu8[ONE] = EcuResetDescriptor_pSt->ResetType_u8;
				UDS_Serv_pSt->TxBuff_pu8[TWO] = RAPID_POWER_SHUTDOWN_TIME;
				UDS_Serv_pSt->TxLen_u16 = ONE;
				Serv_resptype_En = UDS_SERV_RESP_POS_E;
			}
			else
			{
				UDS_Serv_pSt->TxBuff_pu8[ZERO] = UDS_SERVICE11_SID;
				UDS_Serv_pSt->TxBuff_pu8[ONE] = EcuResetDescriptor_pSt->ResetType_u8;
				UDS_Serv_pSt->TxLen_u16 = ONE;
				Serv_resptype_En = UDS_SERV_RESP_POS_E;
			}
		}
		else
		{
			Serv_resptype_En = UDS_SERV_RESP_WAITPEND_E;
		}
	}
	else if(false == GetEcuResetDescriptor(EcuResetType_u8, &EcuResetDescriptor_pSt))
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ECURESET;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(ECU_RESET_REQ_LEN != Uds11ReqLen_u8)
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ECURESET;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(false == iso14229_securitycheck(EcuResetDescriptor_pSt->SecurityLvl_u8))    
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ECURESET;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SECURITY_ACCESS_DENIED;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else if(false == EcuResetDescriptor_pSt->callbackFptr)
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ECURESET;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = GENERAL_REJECT;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else 
	{
		EcuResetDescriptor_pSt->callbackFptr();
		if (true == Uds11CondNotCorrect_u8)
		{
			UDS_Serv_pSt->TxLen_u16 = THREE;
			UDS_Serv_pSt->TxBuff_pu8[ZERO] 	= NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] 	= SID_ECURESET;
			UDS_Serv_pSt->TxBuff_pu8[TWO] 	= GENERAL_REJECT;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
		}
		else if(true == ResetInProgress_u8)
		{
			UdsServ11State_u8 = WAIT_PENDING_E;
			UdsServ11Timer_u32 = GET_TIME_MS() + P2_SERVER_MAX;
		}
		else if(POS_RESP_SUPPRESS_VALUE == Uds11RespSuppress_u8)
		{
			Serv_resptype_En = UDS_SERV_RESP_NORESP_E;

		}
		else if(ENRAPID_SHUTDOWN_TYPE == EcuResetDescriptor_pSt->ResetType_u8)
		{
			UDS_Serv_pSt->TxBuff_pu8[ZERO] 	= UDS_SERVICE11_SID;
			UDS_Serv_pSt->TxBuff_pu8[ONE] 	= EcuResetDescriptor_pSt->ResetType_u8;
			UDS_Serv_pSt->TxBuff_pu8[TWO] 	= RAPID_POWER_SHUTDOWN_TIME;
			UDS_Serv_pSt->TxLen_u16 = ONE;
			Serv_resptype_En = UDS_SERV_RESP_POS_E;
		}
		else
		{
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = UDS_SERVICE11_SID;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = EcuResetDescriptor_pSt->ResetType_u8;
			UDS_Serv_pSt->TxLen_u16 = ONE;
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
uint8_t GetEcuResetDescriptor(uint8_t ResetType_u8, UdsEcuReset_St_t **UdsEcuReset_pSt)
{
	uint8_t State_u8 = false;
	uint8_t LoopCnt_u8 = 0;

	for(LoopCnt_u8 = 0;LoopCnt_u8<TOTAL_RESET_TYPE;LoopCnt_u8++)
	{
		if(ResetType_u8 == UdsEcuReset_aSt[LoopCnt_u8].ResetType_u8)
		{
			State_u8 = true;
			*UdsEcuReset_pSt = (UdsEcuReset_St_t *)(&(UdsEcuReset_aSt[LoopCnt_u8]));
			break;
		}
	}

	return State_u8;
}
void Serv11Init(void)
{
	uint8_t Ser11CanData_au8[8] = {0};

	if (PblParam_St.EcuReset_u8 == TRUE)
	{
		Ser11CanData_au8[0] = 0x02;
		Ser11CanData_au8[1] = 0x51;
		Ser11CanData_au8[2] = 0x01;
		Ser11CanData_au8[3] = 0;
		Ser11CanData_au8[4] = 0;
		Ser11CanData_au8[5] = 0;
		Ser11CanData_au8[6] = 0;
		Ser11CanData_au8[7] = 0;
		PblParam_St.EcuReset_u8 = false;
		UpdateNVMblock(PBL_NVM_BLOCK);
		CanMsgTransmit(1, Ser11CanData_au8, 8, 0, 0, 0x7F1);
	}

	return;
}
#endif /* uds_service11.c */
/*---------------------- End of File -----------------------------------------*/
