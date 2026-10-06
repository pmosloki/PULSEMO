/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File          : iso14229_serv87.c
|    Project      : MIL_PBL_CV
|    Description    : This file contains the variables and functions to
|                     which acn be implemented in the C file.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S        Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

#ifndef ISO14229_SERV87_C
#define ISO14229_SERV87_C

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "iso14229_serv87.h"


#if (TRUE == UDS_SERVICE87_ENABLE)
/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
bool Baud_Verify_b = false; /**/
uds_serv_87_CANbaud_En_t uds_serv_87_CANbaud_En;

/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_Default_Diag_Init
 *   Description   : The function is used to initialize the session  to default
 *          diagnostic session, All flags are reset and all variables
 *          are re-initialized.
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
UDS_Serv_resptype_En_t iso14229_serv87(UDS_Serv_St_t *UDS_Serv_pSt)
{
	uds_serv87_LCTP_En_t LinkControlType_En = (uds_serv87_LCTP_En_t)((UDS_Serv_pSt->RxBuff_pu8[1]) &
																	 LCTP_MASK_VALUE);
	UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
	uint8_t Supress_postive_resp_u8 = (UDS_Serv_pSt->RxBuff_pu8[1]) & POS_RESP_SUPRESS_VALUE;

	if ((SERV_87_MIN_LEN > UDS_Serv_pSt->RxLen_u16) || (SERV_87_MAX_LEN < UDS_Serv_pSt->RxLen_u16))
	{
		/* check whether length should be valid or not */
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = 0x7F;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_LINK_CONTROL;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
	}
	else
	{
#if (SID87_SECURITY_REQUIRED == TRUE)
		if ((PROGRAMMING_SESSION_SUB_ID_E == UDS_GetCurrentSession()) &&
			(true == iso14229_securitycheck(SECURITY_EQU_LEVEL_1)))
		{
#endif
			/*TODO:jeevan*/ /* implement to check security*/
			switch (LinkControlType_En)
			{
				case MODE_TRANSITION_FIXED_PARAMETER_E:
				{
					uds_serv_87_CANbaud_En = (uds_serv_87_CANbaud_En_t)UDS_Serv_pSt->RxBuff_pu8[2];

					switch (uds_serv_87_CANbaud_En)
					{
						case CAN125000BAUD_ID_E:
						{
							UDS_Serv_pSt->TxLen_u16 = ONE;
							UDS_Serv_pSt->TxBuff_pu8[ONE] = LinkControlType_En;
							Serv_resptype_En = UDS_SERV_RESP_POS_E;
							Baud_Verify_b = true;
							break; /* TODO: jeevan */ /* implement to store the baudrate*/
						}

						case CAN250000BAUD_ID_E:
						{
							UDS_Serv_pSt->TxLen_u16 = ONE;
							UDS_Serv_pSt->TxBuff_pu8[ONE] = LinkControlType_En;
							Serv_resptype_En = UDS_SERV_RESP_POS_E;
							Baud_Verify_b = true;
							break;
						}

						case CAN500000BAUD_ID_E:
						{
							UDS_Serv_pSt->TxLen_u16 = ONE;
							UDS_Serv_pSt->TxBuff_pu8[ONE] = LinkControlType_En;
							Serv_resptype_En = UDS_SERV_RESP_POS_E;
							Baud_Verify_b = true;
							break;
						}

						case CAN1000000BAUD_ID_E:
						{
							UDS_Serv_pSt->TxLen_u16 = ONE;
							UDS_Serv_pSt->TxBuff_pu8[ONE] = LinkControlType_En;
							Serv_resptype_En = UDS_SERV_RESP_POS_E;
							Baud_Verify_b = true;
							break;
						}

						default:
						{
							/* The request link control mode identifier( CAN baud) is invalid*/
							UDS_Serv_pSt->TxLen_u16 = THREE;
							UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
							UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_LINK_CONTROL;
							UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
							Baud_Verify_b = false;
							Serv_resptype_En = UDS_SERV_RESP_NEG_E;
						}
					}
					break;
				}

	#if (MODE_TRANSITION_SPECIFIC_PARAMETER_ENABLE == TRUE)
				case MODE_TRANSITION_SPECIFIC_PARAMETER_E:
				{
					CAN_Baudrate_u8 = ((uint32_t)UDS_Serv_pSt->RxBuff_pu8[MPHB] << 16) |
									((uint32_t)UDS_Serv_pSt->RxBuff_pu8[MPMB] << 8) |
									((uint32_t)UDS_Serv_pSt->RxBuff_pu8[MPLB]);
					Baud_Verify_b = true;
					Serv_resptype_En = UDS_SERV_RESP_POS_E;
					break;
				}
	#endif
				case TRANSITION_MODE_E:
				{
					if (POS_RESP_SUPRESS_VALUE != Supress_postive_resp_u8)
					{
						/* The Client should not request the response during transition mode*/
						UDS_Serv_pSt->TxLen_u16 = THREE;
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_LINK_CONTROL;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
						Serv_resptype_En = UDS_SERV_RESP_NEG_E;
					}
					else if (true == Baud_Verify_b)
					{
						/* No response from server, the client and server shall
							transition the baudrate of their communication link*/
						Baud_Verify_b = false;
						Serv_resptype_En = UDS_SERV_RESP_NORESP_E;

						ChangeCANbaudrate(uds_serv_87_CANbaud_En);
						/* TODO: jeevan */ /*Implement to change the baudrate */
										/* Initialize the CAN peripheral with CAN_Baudrate_u8  */
					}
					else
					{
						/* The client request the transition of the mode of operation without
						preceeding verification step*/
						UDS_Serv_pSt->TxLen_u16 = THREE;
						UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
						UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_LINK_CONTROL;
						UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
						Serv_resptype_En = UDS_SERV_RESP_NEG_E;
					}
					break;
				}
				default:
				{
					/* The subfunction paramater is not supported */
					UDS_Serv_pSt->TxLen_u16 = THREE;
					UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
					UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_LINK_CONTROL;
					UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
					Serv_resptype_En = UDS_SERV_RESP_NEG_E;
				}
			}
#if (SID87_SECURITY_REQUIRED == TRUE)
		}

		else
		{
			UDS_Serv_pSt->TxLen_u16 = THREE;
			UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
			UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_LINK_CONTROL;
			UDS_Serv_pSt->TxBuff_pu8[TWO] = CONDITION_NOT_CORRECT;
			Serv_resptype_En = UDS_SERV_RESP_NEG_E;
			/* todo: Jeevan Jestin(write a respective NRC code if required)*/
		}
#endif
	}

	if ((POS_RESP_SUPRESS_VALUE == Supress_postive_resp_u8) && (UDS_SERV_RESP_POS_E == Serv_resptype_En))
	{
		/* Independent of the suppressPosRspMsgIndicationBit,
		 * negative response messages are sent by the server */
		Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
	}
	return Serv_resptype_En;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_Serv87_Timeout
 *   Description   : This function resets the relevant variables/parameters when
 *                   session timesout.
 *   Parameters    : none.
 *   Return Value  : none.
 *******************************************************************************/
void iso14229_serv87_timeout(void)
{
	/*reset the Baudrate verification flag, if it is varified. */
	if (true == Baud_Verify_b)
	{
		Baud_Verify_b = false;
	}
	return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : ChangeCANbaudrate
 *   Description   : The function Re-init CAN baudrate.
 *   Parameters    : none.
 *   Return Value  : none.
 *******************************************************************************/
void ChangeCANbaudrate(uds_serv_87_CANbaud_En_t CANbaud_En)
{
	//Can_ReInit(CANbaud_En);
}

#endif
#endif /* ISO14229_SERV87_C */
/*------------------------------ End of File ---------------------------------*/
