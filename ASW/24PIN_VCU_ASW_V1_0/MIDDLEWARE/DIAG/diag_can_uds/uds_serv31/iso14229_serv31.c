/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File            : iso14229_serv31.c
|    Project        : MIL_PBL_CV
|    Description      : This file contains details regarding UDS_service31.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S          Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

#ifndef ISO14229_SERV31_C
#define ISO14229_SERV31_C

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "iso14229_serv31.h"
#include"iso14229_serv31_conf.h"
//#include "main.h"

/*******************************************************************************
 *  Define & Macros
 ******************************************************************************/
#define RC_OPT_BYTES_INDX (0x04u)
/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/
uint8_t GetRidDescriptor(uint16_t RoutineId_u16,RIDConf_St_t* * Descriptor_St);
/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
uint8_t UdsServ31State_u8 = IDLE_STATE_E;
uint8_t RCInvalidOptionBytes_u8;
uint8_t RCGeneralProgFail_u8;
uint8_t RCconditionNotCorrect_u8;
Add_Size_St_t Checksum_Add_Size_St;

/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : iso14229_serv31
*   Description   : The function is service Routine control service(0x31)
*   Parameters    : UDS_Serv_pSt Pointer to the recived PDU.
*   Return Value  : Response.
*******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv31(UDS_Serv_St_t*  UDS_Serv_pSt)
{
	uint8_t OptByteCnt_u8 = 0;
	static uint16_t RxRID_u16 = 0;
	static RIDConf_St_t* RIDdescriptor_St = NULL;
	static uint8_t SubFunc_u8 = 0;
	static uint8_t SuppressResp_u8 = 0;
	static uint8_t ReqLen_u8 = 0;
	UDS_Serv_resptype_En_t serv_31_resp_En;

	if(IDLE_STATE_E == UdsServ31State_u8)
	{
		ReqLen_u8 = UDS_Serv_pSt->RxLen_u16;
		SubFunc_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE]&SUB_FUNC_MASK_VALUE;
		SuppressResp_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE]&POS_RESP_MASK_SUPRESS_VALUE;
		RxRID_u16 = (((uint16_t)UDS_Serv_pSt->RxBuff_pu8[TWO]) << EIGHT) | UDS_Serv_pSt->RxBuff_pu8[THREE];
	}

	if(WAIT_PENDING_E == UdsServ31State_u8)
	{
		switch (RIDdescriptor_St->RoutineState_u8)
		{
			case ROUTINE_IDLE_E:
			{
				/* Exception condition  */
				UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
				UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
				UDS_Serv_pSt->TxBuff_pu8[TWO] = GEN_PROG_FAILURE;
				UDS_Serv_pSt->TxLen_u16 = THREE;
				UdsServ31State_u8 = IDLE_STATE_E;
				serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				break;
			}
			case ROUTINE_INPROGRESS_E:
			{
				UDS_Serv_pSt->RxBuff_pu8[ZERO] = SID_ROUTINE_CONTROL;
				RIDdescriptor_St->callbackFptr(&UDS_Serv_pSt->RxBuff_pu8[RC_OPT_BYTES_INDX],RC_START, (uint8_t*)RIDdescriptor_St);
				serv_31_resp_En = UDS_SERV_RESP_WAITPEND_E;
				break;
			}
			case ROUTINE_COMPLETED_E:
			{
				RIDdescriptor_St->callbackFptr(&UDS_Serv_pSt->RxBuff_pu8[RC_OPT_BYTES_INDX], RC_RESULT, (uint8_t*)RIDdescriptor_St);
				UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_ROUTINE_CONTROL;
				UDS_Serv_pSt->TxBuff_pu8[ONE] = RC_START;
				UDS_Serv_pSt->TxBuff_pu8[TWO] = (uint8_t)((RxRID_u16 >> EIGHT) & 0xFF);
				UDS_Serv_pSt->TxBuff_pu8[THREE] = (uint8_t)((RxRID_u16) & 0xFF);

				/* Option Byte filling */
				for (OptByteCnt_u8 = 0; OptByteCnt_u8 < RIDdescriptor_St->StartRespOptionByteLen_u8; OptByteCnt_u8++)
				{
					UDS_Serv_pSt->TxBuff_pu8[OptByteCnt_u8+SERVICE_31_OPT_OFFSET] = RIDdescriptor_St->Result_au8[OptByteCnt_u8];
				}
				UDS_Serv_pSt->TxLen_u16 = RIDdescriptor_St->RespLen_u8-1+RIDdescriptor_St->StartRespOptionByteLen_u8;
				UdsServ31State_u8 = IDLE_STATE_E;
				RIDdescriptor_St->RoutineState_u8 = ROUTINE_IDLE_E;
				serv_31_resp_En = UDS_SERV_RESP_POS_E;
				break;
			}
			default:
			{
				/* Exception condition  */
				UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
				UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
				UDS_Serv_pSt->TxBuff_pu8[TWO] = GEN_PROG_FAILURE;
				UDS_Serv_pSt->TxLen_u16 = THREE;
				UdsServ31State_u8 = IDLE_STATE_E;
				serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				break;
			}
		}
	}
	else if(ReqLen_u8 < SERVICE_31_MIN_LEN)
	{
		/* Minimun length check fail */
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		serv_31_resp_En = UDS_SERV_RESP_NEG_E;
	}
	else if(false == GetRidDescriptor(RxRID_u16,&RIDdescriptor_St))
	{
		/* RID is not available */
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		serv_31_resp_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!UdsDiagSessionCheck(RIDdescriptor_St->DiagSession_u8))
	{
		/* RID is not supported in active session */
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		serv_31_resp_En = UDS_SERV_RESP_NEG_E;
	}
	else if(!iso14229_securitycheck(RIDdescriptor_St->SecurityLvl_u8))
	{
		/* Security fail for RID*/
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SECURITY_ACCESS_DENIED;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		serv_31_resp_En = UDS_SERV_RESP_NEG_E;
	}
	else
	{
		SubFunc_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE];
		switch(SubFunc_u8)
		{
			case RC_START:
			{
				if (ReqLen_u8 != RIDdescriptor_St->StartRoutineReqLen_u8)
				{
					/* RID length invalid */
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else if((ROUTINE_IDLE_E != RIDdescriptor_St->RoutineState_u8))
				{
					/* Invalid sequence */
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else if(NULL == RIDdescriptor_St->callbackFptr)
				{
					/* Funtionality is disabled */
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else 
				{
					RIDdescriptor_St->callbackFptr(&UDS_Serv_pSt->RxBuff_pu8[RC_OPT_BYTES_INDX],RC_START, (uint8_t*)RIDdescriptor_St);
					if (RCconditionNotCorrect_u8)
					{
						/* Condition is met to start the routine */
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_31_resp_En = UDS_SERV_RESP_NEG_E;
					}
					else if (RCInvalidOptionBytes_u8)
					{
						/* Option bye info is invalid or Outof the range */
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_31_resp_En = UDS_SERV_RESP_NEG_E;
					}
					else if (RCGeneralProgFail_u8)
					{
						/* Routine fail */
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = GEN_PROG_FAILURE;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_31_resp_En = UDS_SERV_RESP_NEG_E;
					}
					else if ((ROUTINE_INPROGRESS_E == RIDdescriptor_St->RoutineState_u8)&&
							(true == RIDdescriptor_St->WaitPendingEnable_u8))
					{
						UDS_Serv_pSt->RxBuff_pu8[ZERO] = SID_ROUTINE_CONTROL;
						UdsServ31State_u8 = WAIT_PENDING_E;
						serv_31_resp_En = UDS_SERV_RESP_WAITPEND_E;
					}
					else if(POS_RESP_SUPPRESS_VALUE == SuppressResp_u8)
					{
						serv_31_resp_En = UDS_SERV_RESP_NORESP_E;
					}
					else
					{
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_ROUTINE_CONTROL;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = RC_START;
						UDS_Serv_pSt->TxBuff_pu8[TWO]   = (uint8_t) ((RxRID_u16>>EIGHT)&0xFF);
						UDS_Serv_pSt->TxBuff_pu8[THREE] = (uint8_t) ((RxRID_u16)&0xFF);

						/* Option Byte filling */
						for(OptByteCnt_u8 = 0;OptByteCnt_u8<RIDdescriptor_St->StartRespOptionByteLen_u8;OptByteCnt_u8++)
						{
							UDS_Serv_pSt->TxBuff_pu8[OptByteCnt_u8+SERVICE_31_OPT_OFFSET]  = RIDdescriptor_St->Result_au8[OptByteCnt_u8];
						}
						UDS_Serv_pSt->TxLen_u16 = RIDdescriptor_St->RespLen_u8-1+RIDdescriptor_St->StartRespOptionByteLen_u8;
						serv_31_resp_En = UDS_SERV_RESP_POS_E;
					}
				}
				break;
			}
			case RC_STOP:
			{
				if (ReqLen_u8 != RIDdescriptor_St->StopRoutineReqLen_u8)
				{
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else if(ROUTINE_IDLE_E == RIDdescriptor_St->RoutineState_u8)
				{
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else if (POS_RESP_SUPPRESS_VALUE == SuppressResp_u8)
				{
					serv_31_resp_En = UDS_SERV_RESP_NORESP_E;
				}
				else
				{
					RIDdescriptor_St->callbackFptr(&UDS_Serv_pSt->RxBuff_pu8[RC_OPT_BYTES_INDX],RC_STOP,(uint8_t*)RIDdescriptor_St);
				}

				break;
			}
			case RC_RESULT:
			{
				if (ReqLen_u8 != RIDdescriptor_St->RoutineResultReqLen_u8)
				{
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else if(ROUTINE_IDLE_E == RIDdescriptor_St->RoutineState_u8)
				{
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_31_resp_En = UDS_SERV_RESP_NEG_E;
				}
				else if (POS_RESP_SUPPRESS_VALUE == SuppressResp_u8)
				{
					serv_31_resp_En = UDS_SERV_RESP_NORESP_E;
				}
				else
				{
					RIDdescriptor_St->callbackFptr(&UDS_Serv_pSt->RxBuff_pu8[RC_OPT_BYTES_INDX],RC_RESULT,(uint8_t*)RIDdescriptor_St);

					UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_ROUTINE_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = RC_RESULT;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = (uint8_t)((RxRID_u16 >> EIGHT) & 0xFF);
					UDS_Serv_pSt->TxBuff_pu8[THREE] = (uint8_t)((RxRID_u16) & 0xFF);

					/* Option Byte filling */
					for (OptByteCnt_u8 = 0; OptByteCnt_u8 < RIDdescriptor_St->ResultRespOptionByteLen_u8; OptByteCnt_u8++)
					{
						UDS_Serv_pSt->TxBuff_pu8[OptByteCnt_u8+SERVICE_31_OPT_OFFSET] = RIDdescriptor_St->Result_au8[OptByteCnt_u8];
					}
					UDS_Serv_pSt->TxLen_u16 = RIDdescriptor_St->RespLen_u8-1+RIDdescriptor_St->ResultRespOptionByteLen_u8;
					serv_31_resp_En = UDS_SERV_RESP_POS_E;
				}

				break;
			}
			default:
			{
				UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
				UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ROUTINE_CONTROL;
				UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
				UDS_Serv_pSt->TxLen_u16 = THREE;
				serv_31_resp_En = UDS_SERV_RESP_NEG_E;
			}
		}
	}
	return serv_31_resp_En;
	
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : GetRidDescriptor
*   Description   : The function provides the RID index descriptor
*   Parameters    : Recieved RID
*					Pointer to the descriptor
*   Return Value  : True:- RID available.
*				    False:- RID not available.
*******************************************************************************/
uint8_t GetRidDescriptor(uint16_t RoutineId_u16,RIDConf_St_t* * Descriptor_St)
{
	uint16_t LoopCnt_u16 = 0;
	uint8_t Status_u8 = false;
	

	for (LoopCnt_u16 = 0; LoopCnt_u16 < TOTAL_RID; LoopCnt_u16++)
	{
		if (RoutineId_u16 == RIDConf_aSt[LoopCnt_u16].RID_u16)
		{
			/* Routine available*/
			*Descriptor_St = (RIDConf_St_t*)(& (RIDConf_aSt[LoopCnt_u16]));
			Status_u8 = true;
			break;
		}
	}

	return Status_u8;
}


#endif /* ISO14229_SERV31_C */
/*---------------------- End of File -----------------------------------------*/
