/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File            : iso14229_serv83.c
|    Project        : MIL_PBL_CV
|    Description    : This file contains variables and functions of iso14229
|                     service 83.
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


/*******************************************************************************
 *  Includes
 ******************************************************************************/

#include "iso14229_serv83.h"

/*******************************************************************************
 *  Define & Macros
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/
#if (TRUE == UDS_SERVICE83_ENABLE)
/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
static bool Timingparameter_responserecord_b = FALSE;

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  ----------------------------------------------------------------------------
 *  Function Name : iso14229_serv83
 *  Description   : This function will process the service_83 requests
 *  Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer
 *           table.
 *  Return Value  : Type of response.
 *******************************************************************************/

UDS_Serv_resptype_En_t iso14229_serv83(UDS_Serv_St_t *UDS_Serv_pSt)
{
	uint8_t TimingParameter_AccessType_u8;
	UDS_Serv_resptype_En_t serv_83_resp_En;
	uint16_t numofbytesreq_u16 = UDS_Serv_pSt->RxLen_u16;
	TimingParameter_AccessType_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE];
	DCAN_TPTimingCfg_St_t DCAN_TPTimingCfg_St;
	uint8_t Supress_postive_resp_u8 = (UDS_Serv_pSt->RxBuff_pu8[ONE]) & POS_RESP_SUPRESS_VALUE;

	/*checking length*/
	if ((SERV_83_MIN_LEN > numofbytesreq_u16) && (SERV_83_MAX_LEN < numofbytesreq_u16))
	{
		/*invalid message length NRC- 0x13*/
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		serv_83_resp_En = UDS_SERV_RESP_NEG_E;
	}
	else if (EXTENDED_DIAG_SESSION_SUB_ID_E == UDS_GetCurrentSession())
	{
#if (TRUE == UDS83_SECURITY_CHECK)
		if (TRUE == iso14229_securitycheck(SECURITY_EQU_LEVEL_1))
		{
#endif
			switch (TimingParameter_AccessType_u8)
			{
				case READ_EXTENDEDTIMING_PARAMETERSET_E:
				{
					if (numofbytesreq_u16 == TWO)
					{
						UDS_Serv_pSt->TxBuff_pu8[ONE] = READ_EXTENDEDTIMING_PARAMETERSET_E;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = (uint8_t)(DCAN_TPTimings_aSt[CUSTOMER_VALUE].P2_u32 & 0xFF);
						UDS_Serv_pSt->TxBuff_pu8[THREE] = (uint8_t)(DCAN_TPTimings_aSt[CUSTOMER_VALUE].P3_u32 & 0xFF);
						UDS_Serv_pSt->TxBuff_pu8[FOUR] = (uint8_t)((DCAN_TPTimings_aSt[CUSTOMER_VALUE].P3_u32 >> EIGHT) & 0xFF);
						UDS_Serv_pSt->TxLen_u16 = FOUR;
						serv_83_resp_En = UDS_SERV_RESP_POS_E;
						Timingparameter_responserecord_b = TRUE;
					}
					else
					{
						/*invalid message length NRC- 0x13*/
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_83_resp_En = UDS_SERV_RESP_NEG_E;
					}
					break;
				}
				case SET_TIMINGPARAMETER_TODEFAULTVALUES_E:
				{
					if (numofbytesreq_u16 == TWO)
					{
						DCAN_TPTiming_St.P2_u32 = DCAN_TPTimings_aSt[DEFAULT_VALUE].P2_u32;
						DCAN_TPTiming_St.P3_u32 = DCAN_TPTimings_aSt[DEFAULT_VALUE].P3_u32;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SET_TIMINGPARAMETER_TODEFAULTVALUES_E;
						UDS_Serv_pSt->TxLen_u16 = ONE;
						serv_83_resp_En = UDS_SERV_RESP_POS_E;
					}
					else
					{
						/*invalid message length NRC- 0x13*/
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_83_resp_En = UDS_SERV_RESP_NEG_E;
					}

					break;
				}
				case READ_CURRENTLY_ACTIVETIMINGPARAMETERS_E:
				{
					if (numofbytesreq_u16 == TWO)
					{
						UDS_Serv_pSt->TxBuff_pu8[ONE] = READ_CURRENTLY_ACTIVETIMINGPARAMETERS_E;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = (uint8_t)(DCAN_TPTiming_St.P2_u32 & 0xFF);
						UDS_Serv_pSt->TxBuff_pu8[THREE] = (uint8_t)(DCAN_TPTiming_St.P3_u32 & 0xFF);
						UDS_Serv_pSt->TxBuff_pu8[FOUR] = (uint8_t)((DCAN_TPTiming_St.P3_u32 >> 8) & 0xFF);
						UDS_Serv_pSt->TxLen_u16 = FOUR;
						serv_83_resp_En = UDS_SERV_RESP_POS_E;
					}
					else
					{
						/*invalid message length NRC- 0x13*/
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_83_resp_En = UDS_SERV_RESP_NEG_E;
					}

					break;
				}
				case SET_TIMINGPARAMETERS_TOGIVENVALUES_E:
				{
					if (numofbytesreq_u16 == FIVE)
					{
						DCAN_TPTimingCfg_St.P2_u32 = UDS_Serv_pSt->RxBuff_pu8[TWO];
						DCAN_TPTimingCfg_St.P3_u32 = (uint32_t)(UDS_Serv_pSt->RxBuff_pu8[THREE] | (UDS_Serv_pSt->RxBuff_pu8[FOUR] << EIGHT));

						if ((((DCAN_TPTimings_aSt[CUSTOMER_VALUE].P2_u32) == (DCAN_TPTiming_St.P2_u32)) && (DCAN_TPTimingCfg_St.P3_u32 == DCAN_TPTimings_aSt[CUSTOMER_VALUE].P3_u32)) && (TRUE == Timingparameter_responserecord_b))
						{
							Timingparameter_responserecord_b = FALSE;
							DCAN_TPTimingCfg_St.P2_u32 = (DCAN_TPTimings_aSt[CUSTOMER_VALUE].P2_u32);
							DCAN_TPTimingCfg_St.P3_u32 = (DCAN_TPTimings_aSt[CUSTOMER_VALUE].P3_u32);
							UDS_Serv_pSt->TxBuff_pu8[ONE] = SET_TIMINGPARAMETERS_TOGIVENVALUES_E;
							UDS_Serv_pSt->TxLen_u16 = ONE;
							serv_83_resp_En = UDS_SERV_RESP_POS_E;
						}
						else
						{
							/*request out of range NRC- 0x31*/
							UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
							UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
							UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
							UDS_Serv_pSt->TxLen_u16 = THREE;
							serv_83_resp_En = UDS_SERV_RESP_NEG_E;
						}
					}
					else
					{
						/*invalid message length NRC- 0x13*/
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
						UDS_Serv_pSt->TxLen_u16 = THREE;
						serv_83_resp_En = UDS_SERV_RESP_NEG_E;
					}

					break;
				}

				default:
				{
					/*sub function not supported NRC- 0x12*/
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
					UDS_Serv_pSt->TxLen_u16 = THREE;
					serv_83_resp_En = UDS_SERV_RESP_NEG_E;
					break;
				}
			}
#if (TRUE == UDS83_SECURITY_CHECK)
		}
		else
		{
			/*condition not correct NRC- 0x22*/
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
			UDS_Serv_pSt->TxLen_u16 = THREE;
			serv_83_resp_En = UDS_SERV_RESP_NEG_E;
		}
#endif
	}
	else
	{
		/*condition not correct NRC- 0x22*/
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_ATP;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
		UDS_Serv_pSt->TxLen_u16 = THREE;
		serv_83_resp_En = UDS_SERV_RESP_NEG_E;
	}

	if ((POS_RESP_SUPRESS_VALUE == Supress_postive_resp_u8) && (UDS_SERV_RESP_POS_E == serv_83_resp_En))
	{
		/* Independent of the suppressPosRspMsgIndicationBit,
		 * negative response messages are sent by the server */
		serv_83_resp_En = UDS_SERV_RESP_NORESP_E;
	}

	return serv_83_resp_En;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_Serv83_Timeout
 *   Description   : The function Re-initialize the timing parameter to default value
 *                   after timeout
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
void UDS_Serv83_Timeout(void)
{
	DCAN_TPTiming_St.P2_u32 = DCAN_TPTimings_aSt[0].P2_u32;
	DCAN_TPTiming_St.P3_u32 = DCAN_TPTimings_aSt[0].P3_u32;
	Timingparameter_responserecord_b = FALSE;
	return;
}
#endif

/*---------------------- End of File -----------------------------------------*/
