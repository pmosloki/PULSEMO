/******************************************************************************
 *    FILENAME    : i15765_nl.c
 *    DESCRIPTION : This file contains routines for network layer stack.
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
*/ 

/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
*/


#include "i15765.h"
#include "i15765_conf.h"
#include "str_util.h"

/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
*/

DCAN_nlInputVector_st_t *DCAN_nlInputVector_pst[DCAN_TP_INSTANCE_TOTAL_E] = {NULL /*,NULL*/};
uint8_t                 FunAddessedID_flag = FALSE;


/*
 **************************************************************************************************
 *    Function prototype
 **************************************************************************************************
*/
static void DCAN_NLIndication (uint16_t canTpInstanceId_u16 , uint16_t datalength, DCAN_NLResult_en_t nl_result);
static void DCAN_NLConfirm (uint16_t SigName_u16 , DCAN_NLResult_en_t nl_result);
static void DCAN_NLFFIndication (uint16_t SigName_u16 , uint16_t datalength_u16);


/*
 **************************************************************************************************
 *    Function defintions
 **************************************************************************************************
*/
/**
 *  FUNCTION NAME : DCAN_NL_Proc_5ms
 *
 *  FILENAME      : i15765_nl.c
 *
 *  @param        : none
 *
 *  @brief        : This function activates the transmission time related events such as
 *                  sending of flow control message, consecutive frames, overflow control messages,
 *                  sending confirmation status to upper layer and also monitoring of timeouts.
 *
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_NL_PROC] @{SDD_I15765_DCAN_NL_PROC}
 
*/

void  DCAN_NL_Proc_5ms (void)
{
    uint32_t current_cputimer_u32 = 0;
    uint32_t stminti_u32 = 0;       /* holds STmin timer value */
    uint8_t *buffer_pu8 = NULL;      /* points to local buffer  */
    uint32_t txCANID_u32 = 0;     /* Variable holds the CAN ID */
//    uint8_t txDLC_u8 = 0;         /* Variable holds data length to be transmitted */
    uint8_t idx_u8 = 0;           /* index variable for loops */
//    uint8_t buffer_au8[DCAN_MAXDLC] = {0};    /* Local buffer to hold transmitting data */
    uint8_t numbytesTX_u8 = 0;    /* Number of bytes to be transmitted */
    uint8_t msg_tx_status = DCAN_MSGNOTSENT;
	CAN_MessageFrame_St_t Can_Applidata_St; 
	uint8_t buffer_index_u8 = 0;
	uint8_t instanceIndex_u8 = 0;
	uint16_t CIL_CAN_SigName_En = 0;
	
    /* Session is active only if network layer is configured */
    if (DCAN_nlInputVector_pst[0] != NULL)
    {
        /* clearing the buffer contents */
        /* Buffer is initialized with 0xAA for padding */

		for (idx_u8 = 0; idx_u8 < DCAN_MAXDLC; idx_u8++)
        {
            Can_Applidata_St.DataBytes_au8[idx_u8] = ISO15765_CONF_DCAN_RESFILLERBYTE;
        }
		
        buffer_pu8 = Can_Applidata_St.DataBytes_au8;
        current_cputimer_u32 = 0;

        /* Capture CPU time */
        current_cputimer_u32 = GET_TIME_MS();

	for(instanceIndex_u8 = 0 ; instanceIndex_u8 < DCAN_TP_INSTANCE_TOTAL_E ; instanceIndex_u8++)
    {
		for( buffer_index_u8 = 0 ; buffer_index_u8 < TOTAL_CH ; buffer_index_u8++)
        {
        /* Check if time has elapsed. Do network activities only when NL state is not in idle */
			if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 != DCAN_NL_ST_IDLE_E)
            {
              if ((int32_t)(DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) < (int32_t) 0)
              {
				switch (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8)
				{
					/* Send flow control message */
					case DCAN_NL_ST_SENDFC_E:
					{
						/* Check if buffer is locked by the application layer */
						if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 == DCAN_NL_STBUFF_AL_E)
						{
							/* Update the wait to send flow control N_PCI bytes */
							*buffer_pu8 = DCAN_PCI_FC | DCAN_FC_WAIT ;
							buffer_pu8++;
							/* Configure  BSmax */
							*buffer_pu8 = DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_BSmax_u8;
							buffer_pu8++;
							/* Configure SeparationTime(STmin) */
							*buffer_pu8 = DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_STmin_u8;
							txCANID_u32 = DCAN_nlInputVector_pst[instanceIndex_u8]->Tx_CanRespID_32;
							Can_Applidata_St.ID_u32 = txCANID_u32;
							// txDLC_u8 = 0x03;   /* true length */
							Can_Applidata_St.DataLength_u8 = 0x03;
	
							/* Calculate the DLC to be sent */
							if (DCAN_nlInputVector_pst[instanceIndex_u8]->sendDlcEight_b != FALSE)
							{
								// txDLC_u8 = DCAN_MAXDLC;
								Can_Applidata_St.DataLength_u8 = DCAN_MAXDLC;
							}
	
							current_cputimer_u32 = GET_TIME_MS();
							/* Restart the Ar timer */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
															DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_AsArti_u32;
							/* Configure the CAN driver to send the WFS FC frame  */
                            
                            
                            				CIL_CAN_SigName_En = DCAN_TP_InstanceCANSignConf_aSt[instanceIndex_u8].CIL_CAN_TxSignalName_u16;
							msg_tx_status = CIL_CAN_Tx_AckMsg (CIL_CAN_SigName_En, Can_Applidata_St);
							/* Capture CPU time */
							current_cputimer_u32 = GET_TIME_MS();
	
							/* Check if the Ar timer expires */
							if ((int32_t) (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) > (int32_t) 0)
							{
								/* Transmit the message ; */
								if (msg_tx_status == CAN_SUCCESS_E)
								{
									/* Message configured successfully */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->wtsfcCtrl_u8++;
									/* Capture CPU time */
									current_cputimer_u32 = GET_TIME_MS();
									/* Restart the Bs timer */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
																	DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_Bsmaxti_u32;
								}
								else if(msg_tx_status == CAN_INACTIVE_E)
								{
									/* If the CAN Bus is not active abort the communication */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
								}
								else
								{
									/* if previous message has not been transmitted by Data link layer */
									/* resend the frame again */
								}
							}
							else
							{
								DCAN_NLIndication (instanceIndex_u8, DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16, DCAN_NL_TIMEOUT_A_E);
							}
	
							/* Buffer is locked by the application layer,send wait to send flow control */
							if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->wtsfcCtrl_u8 >=
									DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_WFTmax_u8)
							{
								/* Receiver as transmitted WFTmax flow control, the receiver aborts */
								/* message reception and issue an N_WFT_OVRN to upper layer */
								DCAN_NLIndication (instanceIndex_u8 , DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16, DCAN_NL_WFT_OVRN_E);
							}
						}
						else
						{
							/* Update the continue to send flow control N_PCI bytes */
							*buffer_pu8 = DCAN_PCI_FC | DCAN_FC_CTS ;
							buffer_pu8++;
							txCANID_u32 = DCAN_nlInputVector_pst[instanceIndex_u8]->Tx_CanRespID_32;
							// txDLC_u8 = 0x03;   /* true length */
							Can_Applidata_St.DataLength_u8 = 0x03;   /* true length */
							Can_Applidata_St.ID_u32 = txCANID_u32;
							/* Calculate the DLC to be sent */
							if (DCAN_nlInputVector_pst[instanceIndex_u8]->sendDlcEight_b != FALSE)
							{
								// txDLC_u8 = DCAN_MAXDLC;
								Can_Applidata_St.DataLength_u8 = DCAN_MAXDLC;
							}
	
							/* Configure  BSmax */
							*buffer_pu8 = DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_BSmax_u8;
							buffer_pu8++;
							/* Configure SeparationTime(STmin) */
							*buffer_pu8 = DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_STmin_u8;
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
							/* Restart the Ar timer */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
															DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_AsArti_u32;
							/* Configure the CAN driver to send the FC frame  */
							// msg_tx_status = DCAN_TransmitMsg (CIL_DCAN_TX_E, txDLC_u8, (const uint8_t*)buffer_au8);
                            
                            				CIL_CAN_SigName_En = DCAN_TP_InstanceCANSignConf_aSt[instanceIndex_u8].CIL_CAN_TxSignalName_u16;
							msg_tx_status = CIL_CAN_Tx_AckMsg (CIL_CAN_SigName_En, Can_Applidata_St);
				
                            
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
	
							/* Check if the Ar timer expires */
							if ((int32_t) (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) > (int32_t) 0)
							{
								/* Transmit the message ; */
								/* if previous message has not been transmitted by Data link layer */
								/* resend the frame again */
								if (msg_tx_status == CAN_SUCCESS_E)
								{
									/* Message configured successfully */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->wtsfcCtrl_u8 = 0;
									/*Switch network layer state to DCAN_NL_ST_WAITCF_E */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_WAITCF_E;
									/* update the BSmax counter */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmaxCtr_u8 =
										DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_BSmax_u8;
									/* Update the BSmax */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmax_u8 =
										DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_BSmax_u8;
									/* Capture CPU time */
									// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
									//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
									current_cputimer_u32 = GET_TIME_MS();
									/* Start Cr timer */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
																	DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_Crmaxti_u32;
								}
								else if(msg_tx_status == CAN_INACTIVE_E)
								{
									/* If the CAN Bus is not active abort the communication */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
								}
								else
								{
									/* if previous message has not been transmitted by Data link layer */
									/* resend the frame again */
								}
							}
							else
							{
								DCAN_NLIndication (instanceIndex_u8 , DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16, DCAN_NL_TIMEOUT_A_E);
							}
						}
	
						break;
					}
					/* Send overflow control message */
					case DCAN_NL_ST_SENDFC_OVR_E:
					{
						/* Buffer size of the receiver is smaller than the data to receive */
						/* Update the overflow flow control N_PCI bytes */
						*buffer_pu8 = DCAN_PCI_FC | DCAN_FC_OVERFLOW ;
						buffer_pu8++;
						/* Configure  BSmax */
						*buffer_pu8 = DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_BSmax_u8;
						buffer_pu8++;
						/* Configure SeparationTime(STmin) */
						*buffer_pu8 = DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_STmin_u8;
						txCANID_u32 = DCAN_nlInputVector_pst[instanceIndex_u8]->Tx_CanRespID_32;
						// txDLC_u8 = 0x03;   /* true length */
						Can_Applidata_St.DataLength_u8 = 0x03;   /* true length */
						Can_Applidata_St.ID_u32 = txCANID_u32;
						/* Calculate the DLC to be sent */
						if (DCAN_nlInputVector_pst[instanceIndex_u8]->sendDlcEight_b != FALSE)
						{
							// txDLC_u8 = DCAN_MAXDLC;
							Can_Applidata_St.DataLength_u8 = DCAN_MAXDLC;
						}
	
						/* Capture CPU time */
						// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
						//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
						current_cputimer_u32 = GET_TIME_MS();
						/* Restart the Bs timer */
						DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
														DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_AsArti_u32;
						/* Configure the CAN driver to send the OVR FC frame  */
						// msg_tx_status = DCAN_TransmitMsg (CIL_DCAN_TX_E, txDLC_u8, (const uint8_t*)buffer_au8);
						//msg_tx_status = CIL_CAN_Tx_AckMsg (instanceIndex_u8, Can_Applidata_St);
                        
                        			CIL_CAN_SigName_En = DCAN_TP_InstanceCANSignConf_aSt[instanceIndex_u8].CIL_CAN_TxSignalName_u16;
						msg_tx_status = CIL_CAN_Tx_AckMsg (CIL_CAN_SigName_En, Can_Applidata_St);
				
                            
						/* Capture CPU time */
						// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
						//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
						current_cputimer_u32 = GET_TIME_MS();
						/* Check if the Ar timer expires */
						if ((int32_t) (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) > (int32_t) 0)
						{
							/* Transmit the message ; */
							if (msg_tx_status == CAN_SUCCESS_E)
							{
								DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
							}
							else if(msg_tx_status == CAN_INACTIVE_E)
							{
								/* If the CAN Bus is not active abort the communication */
								DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
								DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
							}
							else
							{
								/* if previous message has not been transmitted by Data link layer */
								/* resend the frame again */
							}
						}
						else
						{
							DCAN_NLIndication (instanceIndex_u8 , DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16, DCAN_NL_TIMEOUT_A_E);
						}
	
						break;
					}
					/* Send consecutive frames */
					case DCAN_NL_ST_SENDCF_E:
					{
						/* Configure N_PCI bytes for Consecutive frame */
						*buffer_pu8 = (uint8_t) (DCAN_PCI_CF | DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->SNCtr_u8);
						buffer_pu8++;
						numbytesTX_u8 = 7;
						txCANID_u32 = DCAN_nlInputVector_pst[instanceIndex_u8]->Tx_CanRespID_32;
						// txDLC_u8 = 1;
						Can_Applidata_St.DataLength_u8 = 1;
						Can_Applidata_St.ID_u32 = txCANID_u32;
						/* Check if last frame needs to be transmitted */
						if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->datatoRxTx_u16 <=
								(DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16 + numbytesTX_u8))
						{
							/* Copy the last frame */
							numbytesTX_u8 = (uint8_t) (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->datatoRxTx_u16 -
													DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16);
							
							CopyData (buffer_pu8, numbytesTX_u8, 
									&DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->buffer_pu8[DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16]);
							
							// txDLC_u8 += numbytesTX_u8;
							Can_Applidata_St.DataLength_u8 += numbytesTX_u8;
							/* Calculate the DLC to be sent */
							if (DCAN_nlInputVector_pst[instanceIndex_u8]->sendDlcEight_b != FALSE)
							{
								// txDLC_u8 = DCAN_MAXDLC;
								Can_Applidata_St.DataLength_u8 = DCAN_MAXDLC;
							}
	
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
							/* Restart the As timer */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
															DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_AsArti_u32;
							/* Configure the CAN driver to send the CF frame  */
							// msg_tx_status = DCAN_TransmitMsg (CIL_DCAN_TX_E, txDLC_u8, (const uint8_t*)buffer_au8);
							//msg_tx_status = CIL_CAN_Tx_AckMsg (instanceIndex_u8, Can_Applidata_St);
                            
                            				CIL_CAN_SigName_En = DCAN_TP_InstanceCANSignConf_aSt[instanceIndex_u8].CIL_CAN_TxSignalName_u16;
							msg_tx_status = CIL_CAN_Tx_AckMsg (CIL_CAN_SigName_En, Can_Applidata_St);
				
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
	
							/* Check if the As timer expires */
							if ((int32_t) (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) > (int32_t) 0)
							{
								/* Transmit the message ; */
								if (msg_tx_status == CAN_SUCCESS_E)
								{
									/* Give a confirmation that the last frame has been transmitted */
									DCAN_NLConfirm (instanceIndex_u8 , DCAN_NL_OK_E);
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16 += numbytesTX_u8;
								}
								else if(msg_tx_status == CAN_INACTIVE_E)
								{
									/* If the CAN Bus is not active abort the communication */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
								}
								else
								{
									/* if previous message has not been transmitted by Data link layer */
									/* resend the frame again */
								}
							}
							else
							{
								DCAN_NLConfirm (instanceIndex_u8 , DCAN_NL_TIMEOUT_A_E);
							}
						}
						else
						{
							CopyData (buffer_pu8, numbytesTX_u8, 
									&DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->buffer_pu8[DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16]);
										
							// txDLC_u8 += numbytesTX_u8;
							Can_Applidata_St.DataLength_u8 += numbytesTX_u8;
	
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
							/* Restart the As timer */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
															DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_AsArti_u32;
							/* Configure the CAN driver to send the CF frame  */
							// msg_tx_status = DCAN_TransmitMsg (CIL_DCAN_TX_E, txDLC_u8, (const uint8_t*)buffer_au8);
							//msg_tx_status = CIL_CAN_Tx_AckMsg (instanceIndex_u8, Can_Applidata_St);
                            
                            				CIL_CAN_SigName_En = DCAN_TP_InstanceCANSignConf_aSt[instanceIndex_u8].CIL_CAN_TxSignalName_u16;
							msg_tx_status = CIL_CAN_Tx_AckMsg (CIL_CAN_SigName_En, Can_Applidata_St);
				
                            
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
	
							/* Check if the As timer expires */
							if ((int32_t)(DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) > (int32_t) 0)
							{
								/* Transmit the message ; */
								if (msg_tx_status == CAN_SUCCESS_E)
								{
									/* Increment the sequence counter */
									/* If it reaches to 15, wraparound to zero */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->SNCtr_u8++;
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->SNCtr_u8 &= 0x0F;
									/* update the main buffer index */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16 += numbytesTX_u8;
									/* Decrement Block size counter */
									if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmax_u8 != 0)
									{
										if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmaxCtr_u8 != 0)
										{
											DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmaxCtr_u8--;
										}
									}
	
									/* Check for the STmin value */
									if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->stmin_u8 >= 0xF1)
									{
										/* Units of STmin in the range 0xF1 - 0xF9 are even 100 us, where
										parameter value 0xF1 represents 100 us and 0xF9 represents 900 us */
										stminti_u32 =  ( (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->stmin_u8 & 0x0F) * 100);
									}
									else
									{
										/* absolute milliseconds */
										stminti_u32 =  (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->stmin_u8);
										
									}
	
									/* Capture CPU time */
									// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
									//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
									current_cputimer_u32 = GET_TIME_MS();
									/* Configure SeparatioTime for next CF */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = (uint32_t) (current_cputimer_u32 + stminti_u32);
								}
								else if(msg_tx_status == CAN_INACTIVE_E)
								{
									/* If the CAN Bus is not active abort the communication */
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
									DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
								}
								else
								{
									/* if previous message has not been transmitted by Data link layer */
									/* resend the frame again */
								}
	
							}
							else
							{
								DCAN_NLConfirm (instanceIndex_u8, DCAN_NL_TIMEOUT_A_E);
							}
						}
	
						/* Last Consecutive frame in BS is transmitted ; wait for flow control */
						if ((DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmax_u8 != 0) &&
								(DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmaxCtr_u8 == 0))
						{
							/* Switch the network layer to DCAN_NL_ST_WAITFC_E */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_WAITFC_E;
							/* Capture CPU time */
							// HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
							//APP_TIMER_Capture_Ticks(&current_cputimer_u32);
							current_cputimer_u32 = GET_TIME_MS();
							/* Configure the Bsmax timer */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
															DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_Bsmaxti_u32;
						}
	
						break;
					}
					/* Confirm timeout(Bs) value for FC */
					case DCAN_NL_ST_WAITFC_E:
					{
						/* Flow control N_PDU not received ; abort message transmission and issue */
						/* confirmation with N_TIMEOUT_Bs to upper layer */
						/* Refer table table 18 */
						DCAN_NLConfirm (instanceIndex_u8, DCAN_NL_TIMEOUT_Bs_E);
						break;
					}
					/* Indicate timeout(Cr) value for Consecutive frame */
					case DCAN_NL_ST_WAITCF_E:
					{
						/* ConsecutiveFrame N_PDU not received (lost,overwritten) */
						/* abort the message reception and issue indication with N_TIMEOUT_Cr */
						/* upper layer. refer table 18 */
						DCAN_NLIndication (instanceIndex_u8 , DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16, DCAN_NL_TIMEOUT_Cr_E);
						break;
					}
					default:
					{
						break;
					}
				}
			}
            }
		}
		}
    }
    return;
}

/**
 *  FUNCTION NAME : DCAN_NL_RX
 *
 *  @param        : Message number,CAN ID,CAN DLC,pointer to the Message buffer
 *
 *  @brief        : receives the Network layer CAN frames and analyze the frame type.
 *
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_NLRXFRM] @{SDD_I15765_DCAN_NLRXFRM}
 */
void DCAN_NL_RX( uint32_t CanId_u32, uint8_t dlc_u8, uint8_t *data_pu8)
{
    uint32_t current_cputimer_u32 = 0;
    uint8_t *buffer_pu8 = NULL;
    uint16_t datalenFF_u16 = 0;
    uint16_t idx_u16 = 0;
    uint16_t cfbytesrecv_u16 = 0;
    uint8_t buffer_au8[DCAN_MAXDLC] = {0};
    uint8_t datalen_u8 = 0;
    uint8_t n_PciTyp_u8 = 0;
    uint8_t n_PciLowNib_u8 = 0;
	uint16_t i = 0;
    uint16_t canTpInstanceId_u16 = 0;	

    /* Check for network layer configuration */
    if (DCAN_nlInputVector_pst[0] != NULL)
    {
        buffer_pu8 = buffer_au8;
        cfbytesrecv_u16 = 0;
        current_cputimer_u32 = 0;

        if (dlc_u8 != 8)
        {
            /* In case of DLC less than 8, ignore reception of frame */
            return;
        }
		

        for( i = 0 ; i < DCAN_TP_INSTANCE_TOTAL_E ; i ++)
        {
            if (CanId_u32 == DCAN_nlInputVector_pst[i]->Rx_CanFuncID_u32)
            {
                FunAddessedID_flag = 1;
                break;
            }
            else if(CanId_u32 == DCAN_nlInputVector_pst[i]->Rx_CanPhyID_u32)
            {
                break;
            }
        }
        
        if(i < DCAN_TP_INSTANCE_TOTAL_E)
        {
            datalen_u8 = 7;
            canTpInstanceId_u16 = i ;
        }
        else{
            return ;
        }
        
        if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferstate_u8 == DCAN_NL_STBUFF_AL_E)
        {
            return;
        }

        /* Copy the data from data link layer to local buffer before it over writes again */
        for (idx_u16 = 0; idx_u16 < (uint16_t) dlc_u8; idx_u16++)
        {
            buffer_au8[idx_u16] = *(data_pu8++);
        }
        
        if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 == DCAN_NL_ST_SENDCF_E)
        {
            /* Refer Handling of unexpected frames in document ISO 15765-2 */
            return; 
        }
        
        /* Check if network layer has sent WAITFC */



        /*  N_PCI information  to decide the type of the frame */
        n_PciTyp_u8 = (uint8_t) ((*buffer_pu8) & (uint8_t) 0xF0);
        n_PciLowNib_u8 = (uint8_t) ((*buffer_pu8) & (uint8_t) 0x0F); /* Copy the lower nibble pf N_PCI byte */
        buffer_pu8++;                            /* Point to next data byte */

        if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 == DCAN_NL_ST_WAITFC_E)
        {
            if (n_PciTyp_u8 != DCAN_PCI_FC)
            {
                return; 
            }
        }
        
        /* The type of received frame  is analysed */
        switch (n_PciTyp_u8)
        {
                /* Single frame received */
            case DCAN_PCI_SF:
            {
                /* Check for the valid data length */
                if ((n_PciLowNib_u8 == 0) || (n_PciLowNib_u8 > datalen_u8))
                {
                    /* Invalid data frame ; ignore the received frame */
                    return;
                }
                /* check if the network layer is in the idle state */
                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 != DCAN_NL_ST_IDLE_E)
                {
                    /* Reception in progress, terminate the current reception and report an     */
                    /* indication with NL_UNEXP_PDU to the upper layer and process the SF N_PDU */
                    /* as the start of a new reception                                          */
                    /* Refer to Table 19 Handling of unexpected arrival of a network layer      */
                    /* N_PDU of ISO 15765-2 doc                                                 */
                    DCAN_NLIndication (canTpInstanceId_u16, DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16, DCAN_NL_UNEXP_PDU_E);
                }
                /* Process the Single frame */
                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16 = 0;
                /* Lock the buffer for network layer */
                // DCAN_nlFrameCtrl_paSt[buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_NL; //bufferstate_u8 removed

                /* Copy the contents from local buffer to main buffer */
                for (idx_u16 = 0; idx_u16 < (uint16_t) n_PciLowNib_u8; idx_u16++)
                {
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->buffer_pu8[DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16] = *buffer_pu8;
                    buffer_pu8++;
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16++;
                }
                /* Indicate valid single frame reception to upper layer */
                DCAN_NLIndication (canTpInstanceId_u16,DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16, DCAN_NL_OK_E);
                break;
            }
            /* First frame handling */
            case DCAN_PCI_FF:
            {
                if((FunAddessedID_flag == 1) && (DCAN_TP_INSTANCE_UDSDIAG_E == DCAN_TP_InstanceCANSignConf_aSt[canTpInstanceId_u16].DCAN_TP_Instance_En))
                {
                    FunAddessedID_flag = 0;
                    return;
                }
                
                datalenFF_u16 = (uint16_t)((uint16_t)(n_PciLowNib_u8 << 8) | (*buffer_pu8));
                buffer_pu8++;                
                /* Check if the data length of the first frame is a valid data length */
                /* datalenFF_u16 must have value greater the 7 to recognize it has    */
                /* valid frame Refer ISO15765-2 First frame data length error         */
                /* handling section 7.4.2.1                                           */
                if (datalenFF_u16 <= (uint16_t) datalen_u8)
                {
                    return;
                }

                /* data length in first frame is 6 */
                datalen_u8--;

                /* check if the network layer is in the idle state */
                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 != DCAN_NL_ST_IDLE_E)
                {
                    /* Reception in progress, terminate the current reception and report an     */
                    /* indication with NL_UNEXP_PDU to the upper layer and process the SF N_PDU */
                    /* as the start of a new reception                                          */
                    /* Refer to Table 19 Handling of unexpected arrival of a network layer      */
                    /* N_PDU of ISO 15765-2 doc                                                 */
                    DCAN_NLIndication (canTpInstanceId_u16, DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16, DCAN_NL_UNEXP_PDU_E);
                }

                /* Process the First frame */
                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16 = 0;
                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->datatoRxTx_u16 = datalenFF_u16;
                /* Lock the buffer for network layer */
                // DCAN_nlFrameCtrl_paSt[buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_NL;	//bufferstate_u8 removed
                /* Reset wait to flow control */
                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->wtsfcCtrl_u8 = 0;

                /* Check for overflow */
                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->datatoRxTx_u16 <= DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferlength_u16)
                {
                    /* Copy the contents from local buffer to main buffer */
                    for (idx_u16 = 0; idx_u16 < (uint16_t)datalen_u8; idx_u16++)
                    {
                        DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->buffer_pu8[DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16] = *buffer_pu8;
                        buffer_pu8++;
                        DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16++;
                    }

                    /* Indicate valid single frame reception to upper layer */
                    DCAN_NLFFIndication (canTpInstanceId_u16 , DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->datatoRxTx_u16);
                    /* Switch the network layer to SENDFC */
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 = DCAN_NL_ST_SENDFC_E;
                    /* Set sequence counter to 1 ; FirstFrame will be considered as 0 */
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->SNCtr_u8 = 0x01;
                    /* Capture CPU time */
                    
                    //APP_TIMER_Capture_Ticks(&current_cputimer_u32);
                    current_cputimer_u32 = GET_TIME_MS();
                    //HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
                    /* Transmit flow control (N_Br = 0) */
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->timer_u32 = current_cputimer_u32;
                }
                else
                {   /* Initialize the timer to 0 */
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->timer_u32 = 0;
                    /* Switch the network layer to send OVRFLW control frame */
                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 = DCAN_NL_ST_SENDFC_OVR_E;
                }

                break;
            }
            /* Flow control frame is received */
            case DCAN_PCI_FC:
            {
                //buffer_index_u8 = 1 ;
                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->nl_state_u8 == DCAN_NL_ST_WAITFC_E)
                {
                    if (n_PciLowNib_u8 == DCAN_FC_OVERFLOW)
                    {
                        DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
                        /* Buffer overflow status received ; abort the transmission */
                        /* Refer to table 13 of ISO 15765-2 */
                        DCAN_NLConfirm (canTpInstanceId_u16,DCAN_NL_BUFFER_OVFLW_E);
                    }
                    else if((FunAddessedID_flag == 1) && (DCAN_TP_INSTANCE_UDSDIAG_E == DCAN_TP_InstanceCANSignConf_aSt[canTpInstanceId_u16].DCAN_TP_Instance_En))
                    {
                        FunAddessedID_flag = 0;
                        DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
                        DCAN_NLConfirm (canTpInstanceId_u16,DCAN_NL_INVALID_FS_E);

                    }         //todo yashoda
                    else
                    {
                        if (n_PciLowNib_u8 == DCAN_FC_WAIT)
                        {
                            /* Capture CPU time */
                            
                            //APP_TIMER_Capture_Ticks(&current_cputimer_u32);
                            current_cputimer_u32 = GET_TIME_MS();
                            // HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
                            /* Wait for new flow control N_PDU and restart the N_Bsmax timer */
                            DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->timer_u32 = current_cputimer_u32 +
                                                            DCAN_nlInputVector_pst[canTpInstanceId_u16]->timingData_ps->NL_Bsmaxti_u32;
                        }
                        else
                        {
                            if (n_PciLowNib_u8 == DCAN_FC_CTS)
                            {
                                /* Capture CPU time */
                                
                                //APP_TIMER_Capture_Ticks(&current_cputimer_u32);
                                current_cputimer_u32 = GET_TIME_MS();
                                // HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
                                /* Transmit the consecutive frame */
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->timer_u32 = current_cputimer_u32;
                                /* Get the BlockSize value */
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->BSmax_u8 = *buffer_pu8;
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->BSmaxCtr_u8 = *buffer_pu8;

                                buffer_pu8++;
                                /* Get the SeparationTime value */
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->stmin_u8 = *buffer_pu8;
                                /* Check for valid STmin value */
                                if (( (*buffer_pu8 >= 0x80) && (*buffer_pu8 <= 0xF0)) || (*buffer_pu8 >= 0xFA))
                                {
                                    /* Get the maximum SeparationTime value */
                                    /* refer ISO 15765-2 section ST error handling */
                                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->stmin_u8 = 0x7F;
                                }
                                /* Switch network layer to send consecutive frame */
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_NL_E]->nl_state_u8 = DCAN_NL_ST_SENDCF_E;
                            }
                            else
                            {
                                /* Invalid flow control frame ; ignore*/
                                DCAN_NLConfirm (canTpInstanceId_u16,DCAN_NL_INVALID_FS_E);
                                return ;
                            }
                        }
                    }
                }
                break;
            }
            /*  consecutive Frame handling */
            case DCAN_PCI_CF:
            {
                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 == DCAN_NL_ST_WAITCF_E)
                {
                    if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->SNCtr_u8 != n_PciLowNib_u8)
                    {
                        /* Consecutive frame with wrong sequence number , abort the message reception */
                        /* Network layer makes an indication with NL_WRONG_SN to upper layer */
                        /* See section 7.4.3.1 in ISO 15765 -2 */
                        DCAN_NLIndication (canTpInstanceId_u16 ,DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16, DCAN_NL_WRONG_SN_E);
                    }
                    else
                    {
                        /* check if sequence counter as reached 0xF */
                        DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->SNCtr_u8++;
                        /* Wraparound to zero for the next CF */
                        DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->SNCtr_u8 &= 0x0F;
                        /* Check how many bytes received */
                        cfbytesrecv_u16 = (uint16_t) (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16 + (uint16_t) datalen_u8);
                        /* Check for last frame to be received */
                        if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->datatoRxTx_u16 <= cfbytesrecv_u16)
                        {
                            /* Copy data from local buffer to main buffer */
                            for (idx_u16 = DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16; 
                                idx_u16 < DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->datatoRxTx_u16 ; idx_u16++)
                            {
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->buffer_pu8[DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->
                                                                bufferindex_u16] = *buffer_pu8;
                                buffer_pu8++;
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16++;
                            }
                            /* Indicate valid Consecutive frame reception to upper layer */
                            DCAN_NLIndication (canTpInstanceId_u16 ,DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16, DCAN_NL_OK_E);
                        }
                        else
                        {
                            /* Copy data from local buffer to main buffer */
                            for (idx_u16 = 0; idx_u16 < (uint16_t)datalen_u8; idx_u16++)
                            {
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->buffer_pu8[DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->
                                                                bufferindex_u16] = *buffer_pu8;
                                buffer_pu8++;
                                DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->bufferindex_u16++;
                            }
                            /* Capture CPU time */
                            
                            //APP_TIMER_Capture_Ticks(&current_cputimer_u32);
                            current_cputimer_u32 = GET_TIME_MS();
                            // HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
                            /* Restart the Crmax timer */
                            DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->timer_u32 = current_cputimer_u32 +
                                                            DCAN_nlInputVector_pst[canTpInstanceId_u16]->timingData_ps->NL_Crmaxti_u32;
                                                            
                            /* Decrement BS max counter if it is not zero */
                            if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->BSmax_u8 != 0)
                            {
                                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->BSmaxCtr_u8 != 0)
                                {
                                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->BSmaxCtr_u8--;
                                }
                                if (DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->BSmaxCtr_u8 == 0)
                                {
                                    /* Switch network layer to send flow control frame */
                                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 = DCAN_NL_ST_SENDFC_E;
                                    /* Capture CPU time */
                                    //APP_TIMER_Capture_Ticks(&current_cputimer_u32);
                                    current_cputimer_u32 = GET_TIME_MS();
                                    // HAL_TIMER_WDT_Swtimer_Capture_Ticks (&get_cputimer_u64);
                                    /* reset the timer to transmit the FC immediately */
                                    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->timer_u32 = current_cputimer_u32;
                                }
                            }
                        }						
                    }
                }

                break;
            }
            /* Unknown frame ignore */
            default:
            {
                break;
            }
        }   /* End of switch statement */
    }
    return;
}

/**
 *  FUNCTION NAME : DCAN_NL_TX
 *
 *  @param        : message handle, pointer to buffer, data length
 *
 *  @brief        : The function transmits the response identified by the handle ,the pointer to
 *                  the buffer and the data length to be transmitted. Till the transmission of the
 *                  request is completed or failed the buffer is locked by the network layer.The
 *                  network layer shall signals the completion of the message transmission to the
 *                  service user by means of the function DCAN_NlConfirm
 *
 *  @return       : TRUE = message transmitted successfully ; FALSE = message not transmitted
*/ 

bool DCAN_NL_TX (uint16_t instanceIndex_u8 , uint8_t* databuffer_pu8, uint16_t datalength_u16)
{
    uint32_t current_cputimer_u32 = 0;
    uint8_t* buffer_pu8 = NULL;
    uint32_t txCANID_u32 = 0;     /* Variable holds the CAN ID */
//    uint8_t txDLC_u8 = 0;         /* Variable holds data length to be transmitted */
//    uint8_t buffer_au8[DCAN_MAXDLC] = {0};
    uint8_t idx_u8 = 0;
    uint8_t numbytesTX_u8 = 0;    /* Number of bytes to be transmitted */
    bool retnst_b = FALSE;
    uint8_t msg_tx_status = 0;
    uint16_t CIL_CAN_SigName_En = 0;
	uint8_t buffer_index_u8 = 1; //todo-hareesha - why?

	CAN_MessageFrame_St_t Can_Applidata_St; 
		
    /* Transmit new message only when network layer is in idle state */
		if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 == DCAN_NL_ST_IDLE_E)
		{	
			//for (idx_u8 = 0; idx_u8 < sizeof (Can_Applidata_St.DataBytes_au8); idx_u8++)
			for (idx_u8 = 0; idx_u8 < DCAN_MAXDLC; idx_u8++)
			{
				Can_Applidata_St.DataBytes_au8[idx_u8] = ISO15765_CONF_DCAN_RESFILLERBYTE;
			}
			
			/* initialize the buffer pointer */
			buffer_pu8 = Can_Applidata_St.DataBytes_au8;
			current_cputimer_u32 = 0;

			/* Check for network layer configuration */
			if (DCAN_nlInputVector_pst[instanceIndex_u8] != NULL)
			{
				/* Configure the message frame structure */
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->buffer_pu8 = (uint8_t*) databuffer_pu8;
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->datatoRxTx_u16 = datalength_u16;
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16 = 0;
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmax_u8 = 0;
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->BSmaxCtr_u8 = 0;
				/* Configure sequence counter as 0x01 */
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->SNCtr_u8 = 0x01;
				/* Lock the buffer for application layer */
	
				/* Configure CAN ID and data length */
				txCANID_u32 = DCAN_nlInputVector_pst[instanceIndex_u8]->Tx_CanRespID_32;
				numbytesTX_u8 = 7;
				Can_Applidata_St.DataLength_u8 = 1;
				Can_Applidata_St.ID_u32 = txCANID_u32;
				
				
				/* Check for Single frame or First Frame */
				if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->datatoRxTx_u16 <= (uint16_t) numbytesTX_u8)
				{
					/* Configure Single Frame */
					*buffer_pu8 = (uint8_t) (DCAN_PCI_SF | datalength_u16);
					buffer_pu8++;
					numbytesTX_u8 = (uint8_t) datalength_u16;
	
					/* Calculate the DLC to be sent */
					Can_Applidata_St.DataLength_u8 += (uint8_t) datalength_u16;
					if (DCAN_nlInputVector_pst[instanceIndex_u8]->sendDlcEight_b != FALSE)
					{
						// txDLC_u8 = DCAN_MAXDLC;
						Can_Applidata_St.DataLength_u8 = DCAN_MAXDLC;
					}
				}
				else
				{
					/* Configure for First Frame */
					*buffer_pu8 = (uint8_t) (DCAN_PCI_FF | ((datalength_u16 >> 8) & 0x0F));
					buffer_pu8++;
					*buffer_pu8 = (uint8_t) (datalength_u16 & 0xFF);
					buffer_pu8++;
	
					/* In the first frame only 6 data bytes can be sent */
					numbytesTX_u8--;
					/* Switch netwrok layer to wait for FC */
					DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_WAITFC_E;
					/* Data length for CAN */
					Can_Applidata_St.DataLength_u8 = DCAN_MAXDLC;
				}

				/* Copy the data from main buffer to transmit buffer */
				CopyData (buffer_pu8, numbytesTX_u8, DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->buffer_pu8);
				
				/* Update the buffer index */
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferindex_u16 += numbytesTX_u8;
	
				/* Capture CPU time */
				current_cputimer_u32 = GET_TIME_MS();

				/* Restart the As timer */
				DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
												DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_AsArti_u32;
				/* Configure the CAN driver to send the FF/SF frame  */
				CIL_CAN_SigName_En = DCAN_TP_InstanceCANSignConf_aSt[instanceIndex_u8].CIL_CAN_TxSignalName_u16;
				msg_tx_status = CIL_CAN_Tx_AckMsg (CIL_CAN_SigName_En, Can_Applidata_St);
                
                /* Capture CPU time */
				current_cputimer_u32 = GET_TIME_MS();
				/* Check if the As timer expires */
				if ((int32_t) (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 - current_cputimer_u32) > (int32_t) 0)
				{
					if (msg_tx_status == CAN_SUCCESS_E)
					{
						/* Lock the buffer for network layer */
						// DCAN_nlFrameCtrl_paSt[buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_NL;	//bufferstate_u8 removed
	
						if (DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->datatoRxTx_u16 <= numbytesTX_u8)
						{
							/* Give a confirmation that the Single frame has been transmitted */
							DCAN_NLConfirm (instanceIndex_u8, DCAN_NL_OK_E);
						}
						else
						{
							/* Capture CPU time */
							current_cputimer_u32 = GET_TIME_MS();
							/* start the Bsmax timer */
							DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->timer_u32 = current_cputimer_u32 +
															DCAN_nlInputVector_pst[instanceIndex_u8]->timingData_ps->NL_Bsmaxti_u32;
						}
	
						retnst_b = TRUE;
					}
					else
					{
						/* Message not configured to the CAN correctly retry the transmission */
						/* Restore the state to the IDLE state */
						DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
						DCAN_nlFrameCtrl_paSt[instanceIndex_u8][buffer_index_u8]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
						retnst_b = FALSE;
					}
				}
				else
				{
					DCAN_NLConfirm (instanceIndex_u8 , DCAN_NL_TIMEOUT_A_E);
				}
			}
		}
    return retnst_b;
}

/**
 *  FUNCTION NAME : DCAN_NLIndication
 *
 *  @param        : length of the data ,the result of the received request.
 *
 *  @brief        : This function is called by the network layer, when the complete request either
 *                  single or segmented  is received or while receiving,when there is a Cr timeout or
 *                  wait frame overrun is there or when unexpected first frame or single frame
 *                  is received or a frame with wrong sequence number is received.
 *
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_NLINDICATION] @{SDD_I15765_DCAN_NLINDICATION}

*/
static void DCAN_NLIndication (uint16_t canTpInstanceId_u16 , uint16_t datalength, DCAN_NLResult_en_t nl_result)
{    
    if (nl_result == DCAN_NL_OK_E)
    {
        /* Indicate arrival of valid frame to application layer  to application layer(14229)*/
        DCAN_ApplCbk(canTpInstanceId_u16, CIL_NEW_REQ_Rxed_E, DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->buffer_pu8, datalength);
    }
    else if ((nl_result == DCAN_NL_TIMEOUT_Cr_E) || (nl_result == DCAN_NL_WFT_OVRN_E)
              || (nl_result == DCAN_NL_UNEXP_PDU_E) || (nl_result == DCAN_NL_WRONG_SN_E)
              || (nl_result == DCAN_NL_TIMEOUT_A_E))
    {
        /* indicate to application layer */      
        DCAN_ApplCbk(canTpInstanceId_u16, CIL_Rx_ERROR_E, DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->buffer_pu8, datalength);
    }
    else
    {
        /* do nothing */
    }

    /* Switch network layer to idle state */
    DCAN_nlFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_NL_E]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
    
    return;
}

/**
 *  FUNCTION NAME : DCAN_NLConfirm
 *
 *  FILENAME      : i15765_nl.c
 *
 *  @param        : the result of the received request.
 *
 *  @brief        : This function confirms the completion of the request to transfer the response.
 *
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_NLCONFIRM] @{SDD_I15765_DCAN_NLCONFIRM}
 *
----------------------------------------------------------------------------------------------------
*/ 
static void DCAN_NLConfirm (uint16_t SigName_u16 , DCAN_NLResult_en_t nl_result)
{
    if (nl_result == DCAN_NL_OK_E)
    {
        /* confirm to application layer the completion of the request */
       DCAN_ApplCbk(SigName_u16,CIL_TX_CONFRM_E, DCAN_nlFrameCtrl_paSt[SigName_u16][BUFF2_NL_E]->buffer_pu8, DCAN_nlFrameCtrl_paSt[SigName_u16][BUFF2_NL_E]->bufferlength_u16);
    }
    else if ((nl_result == DCAN_NL_TIMEOUT_Bs_E) || (nl_result == DCAN_NL_BUFFER_OVFLW_E)
              || (nl_result == DCAN_NL_INVALID_FS_E) || (nl_result == DCAN_NL_TIMEOUT_A_E))
    {
        /* The Al state will be in transmit in progress state this is changed
        to idle state so that the application layer is ready to receive the new frame */
        DCAN_ApplCbk(SigName_u16,CIL_TX_ERROR_E, DCAN_nlFrameCtrl_paSt[SigName_u16][BUFF2_NL_E]->buffer_pu8, DCAN_nlFrameCtrl_paSt[SigName_u16][BUFF2_NL_E]->bufferlength_u16);
    }
    else
    {
        /* do nothing */
    }

    /* Switch network layer to idle state */
    DCAN_nlFrameCtrl_paSt[SigName_u16][BUFF2_NL_E]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
    
    return;
}

/**
 *  FUNCTION NAME : DCAN_NLFFIndication
 *  @param        : The length of data in the request message
 *
 *  @brief        : The function switches the network layer state from idle to receive in progress.
 *
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_NLFFINDICATION] @{SDD_I15765_DCAN_NLFFINDICATION}
 *
*/
static void DCAN_NLFFIndication (uint16_t SigName_u16 , uint16_t datalength_u16)
{
    /* To store the request message length.*/
    /* this variable will be used in the future */
    uint16_t req_msg_len_u16 = datalength_u16;

    /* Indicate to application layer */
    DCAN_ApplCbk(SigName_u16,CIL_FF_RXED_E, DCAN_nlFrameCtrl_paSt[SigName_u16][BUFF1_NL_E]->buffer_pu8, req_msg_len_u16);
    
    return;
}


/**
 *  @function name : void CANSched_RxMsgCallback(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t* Can_Applidata_St) 
 *  @brief           Function is working as callback function and assign id and all can related information 
 *  @param           uint16_t CIL_SigName_En, CAN_MessageFrame_St_t* Can_Applidata_St
 *  @return          None
 *
 */
void CANSched_RxMsgCallback(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t* Can_Applidata_St)
{
    uint32_t CanId_u32 = 0;    /* Variable to hold the Received ID */
    uint8_t dlc_u8 = 0;        /* Variable to hold the data length of the received message */
    uint8_t idx_u8 = 0;        /* Index counter */
    uint8_t data_au8[DCAN_MAXDLC] = {0};    /* Variable to hold the received data */
	
    PARAM_NOTUSED (CIL_SigName_En);
    /* Copy the ID from the message structure */
    CanId_u32 = Can_Applidata_St->ID_u32;

    /* Copy the data length from the message structure  */
    dlc_u8 = Can_Applidata_St->DataLength_u8;

    /* Copy the data from the message structure */
    for (idx_u8 = 0; idx_u8 < dlc_u8; idx_u8++)
    {
        data_au8[idx_u8] = Can_Applidata_St->DataBytes_au8[idx_u8];
    }

    /* Send the message handler, can ID, data length and data to the network layer */
    DCAN_NL_RX(CanId_u32, dlc_u8, ( uint8_t*) data_au8);
    return;
}


/* *****************************************************************************
 End of File
 */

