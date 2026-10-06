/******************************************************************************
 *    FILENAME    : i15765_tp.c
 *    DESCRIPTION : CAN TP Stack implementation file.
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
*/ 

/* Section: Included Files                                                  */
#include "i15765.h"
#include "i15765_priv.h"
#include "i15765_conf.h"
#include "cil_can_conf.h"
#include "str_util.h"
//#include "app_typedef.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */



/*
 ****************************************************************************************
 *    Variables
 ****************************************************************************************
*/

uint8_t DCAN_TPDataBuffer1_au8[DCAN_TP_INSTANCE_TOTAL_E][MAX_LENGTH_DIAGBUFFER] = {0};

#if (ISO15765_CONF_CH_TYPE == CANTP_FULL_DUPLEX_E)
uint8_t DCAN_TPDataBuffer2_au8[DCAN_TP_INSTANCE_TOTAL_E][MAX_LENGTH_DIAGBUFFER] = {0};
#endif
 
DCAN_tpFrameCtrl_St_t DCAN_TPFrameCtrl1_St[DCAN_TP_INSTANCE_TOTAL_E];  
#if (ISO15765_CONF_CH_TYPE == CANTP_FULL_DUPLEX_E)
DCAN_tpFrameCtrl_St_t DCAN_TPFrameCtrl2_St[DCAN_TP_INSTANCE_TOTAL_E];
#endif

uint8_t* CHBuffer_TP_au8[DCAN_TP_INSTANCE_TOTAL_E][TOTAL_CH] = {0};
DCAN_tpFrameCtrl_St_t *DCAN_tpFrameCtrl_paSt[DCAN_TP_INSTANCE_TOTAL_E][TOTAL_CH] = {NULL};



/*
 ****************************************************************************************
 *    Function prototype
 ****************************************************************************************
*/

/**
* Function name : void DCAN_CommCbk(void)
* @brief        : This is callback function which will in turn calls the application layer function depending on
*                 the CbkType_En.                
* @param        : CbkType_En     Its enum defined in cil_can_conf.h. Defines what type of callback function
*                               gets called.
* @param        : data_pu8      Its data from Network layer
* @param        : length_u16     Length of data.               
* @return       : none
*/
static void DCAN_CommCbk(uint16_t canTpInstanceId_u16,nl2tp_CbkType_En_t CbkType_En, const uint8_t* data_pu8, uint16_t length_u16 );




void i15765_tp_init_buffers(void) // todo-hareesha can be moved to i15765_init.c
{
	int i = 0;
    /*Fill the array with supporting structure.
    Assign the CH1 structure because we can access it in both the channel.*/
	for(i = 0 ; i < DCAN_TP_INSTANCE_TOTAL_E ; i ++)
    {
		DCAN_tpFrameCtrl_paSt[i][BUFF1_TP_E] = &DCAN_TPFrameCtrl1_St[i];
		DCAN_tpFrameCtrl_paSt[i][BUFF2_TP_E] = &DCAN_TPFrameCtrl1_St[i];
		/*Set the buffers for both channel*/
		CHBuffer_TP_au8[i][BUFF1_TP_E] = DCAN_TPDataBuffer1_au8[i];
		CHBuffer_TP_au8[i][BUFF2_TP_E] = DCAN_TPDataBuffer1_au8[i];
		
#if (ISO15765_CONF_CH_TYPE == CANTP_FULL_DUPLEX_E)
		/*Assign the CH2 structure on request of receive structure*/
		DCAN_tpFrameCtrl_paSt[i][BUFF2_TP_E] = &DCAN_TPFrameCtrl2_St[i];
		CHBuffer_TP_au8[i][BUFF2_TP_E] = DCAN_TPDataBuffer2_au8[i];
#endif
	}
	return ;
}

void clearBuffer(uint16_t signal_Index_u16,TP_Buff_En_t TP_Buff_En)
{
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->timer_u32 = 0;
		// DCAN_tpFrameCtrl_paSt[TP_Buff_En]->P2_timings_u32 = 0;
		// DCAN_tpFrameCtrl_paSt[TP_Buff_En]->numbytes_req_u16 = 0;
		// DCAN_tpFrameCtrl_paSt[TP_Buff_En]->numbytes_resp_u16 = 0;
		// DCAN_tpFrameCtrl_paSt[TP_Buff_En]->flag_P2Ext_b = FALSE;
		// Set the application state machine to idle state
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->flag_P2Ext_b = FALSE;
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->al_state_u8 = DCAN_TP_ST_IDLE_E;
		// Set the flags to false
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->flag_FF_rcvd_b = FALSE;
		//DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->flag_new_req_rcvd_b = FALSE;
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->flag_confirm_b = FALSE;
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->flag_err_in_rx = FALSE;
		DCAN_tpFrameCtrl_paSt[signal_Index_u16][TP_Buff_En]->flag_err_in_tx = FALSE;
}

/**
* Function name    : void DCAN_TP_Init(void)
* @brief           : Initialization function for Transport layer process
* @param           : none
* @return          : none
*/

void DCAN_TP_Init (void)
{
	
	uint16_t index_u16 = 0u;
	int  i = 0;
	
  
    /*Fill the array with supporting channel structure*/
    i15765_tp_init_buffers();
	for(i = 0 ; i < DCAN_TP_INSTANCE_TOTAL_E ; i++){
    
		for(index_u16 = 0; index_u16 < TOTAL_CH; index_u16++)
		{
			//DCAN_alFrameCtrl_pSt = &DCAN_alFrameCtrl_St;
			// reset the timer value to 0
			DCAN_tpFrameCtrl_paSt[i][index_u16]->timer_u32 = 0;
			// reset the P2 timings to 0
			DCAN_tpFrameCtrl_paSt[i][index_u16]->P2_timings_u32 = 0;
			// reset the Number of bytes in request
			DCAN_tpFrameCtrl_paSt[i][index_u16]->numbytes_req_u16 = 0;
			// reset the Number of bytes in response
			DCAN_tpFrameCtrl_paSt[i][index_u16]->numbytes_resp_u16 = 0;
			// Initialize the pointer to the main buffer
			DCAN_tpFrameCtrl_paSt[i][index_u16]->databuf_pu8 = CHBuffer_TP_au8[i][index_u16];
			// Reset P2Can extended timing to false
			DCAN_tpFrameCtrl_paSt[i][index_u16]->flag_P2Ext_b = FALSE;
			// Set the application state machine to idle state
			DCAN_tpFrameCtrl_paSt[i][index_u16]->al_state_u8 = DCAN_TP_ST_IDLE_E;
			// Set the flags to false
			DCAN_tpFrameCtrl_paSt[i][index_u16]->flag_FF_rcvd_b = FALSE;
			DCAN_tpFrameCtrl_paSt[i][index_u16]->flag_new_req_rcvd_b = FALSE;
			DCAN_tpFrameCtrl_paSt[i][index_u16]->flag_confirm_b = FALSE;
			DCAN_tpFrameCtrl_paSt[i][index_u16]->flag_err_in_rx = FALSE;
			DCAN_tpFrameCtrl_paSt[i][index_u16]->flag_err_in_tx = FALSE;
			DCAN_tpFrameCtrl_paSt[i][index_u16]->sessionTimeoutTimer_u32 = GET_TIME_MS() + ISO15765_CONF_SESSION_TIMEOUT_DELAY_MS;
		}
	}
	
    DCAN_NL_RegisterCbk(DCAN_CommCbk);
    
    return;
}

/**
* Function name    : void DCAN_TP_Proc_5ms(void)
* @brief           : Transport layer process scheduled periodically
* @param           : none
* @return          : none
* @code tag        : @[CODE_OBD_TIMING_2] @{SDD_OBD_TIMING_2}
*                    @[CODE_AL_01] @{SDD_AL_01}
*/
void DCAN_TP_Proc_5ms(void)
{
    //uint32_t current_cputimer_u32 = 0;    // To store the current CPU time in ticks
    //uint16_t msghandle_u16 = CIL_DCAN_TX_E;    //To store the message handle
    //Serv_resptype_En_t resp_type_En = SERV_RESP_NORESP_E;    // To store the response code
    static uint8_t count_u8 = 0;    // To store the count
    uint8_t TxStatus_u8 = DCAN_MSGNOTSENT;    // To store the transmission status

	uint8_t Index_instance_u8 = 0;
	for( Index_instance_u8 = 0 ; Index_instance_u8 < DCAN_TP_INSTANCE_TOTAL_E ; Index_instance_u8++)
    {
    
        if(NULL == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E] || NULL == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E])
        {
            return;
        }
    
	
        // Check if AL is in idle state
		if (DCAN_TP_ST_IDLE_E == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8)
		{
			// Check if the new request has received
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_new_req_rcvd_b)
			{
				// Change AL state to process request state
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8 = DCAN_TP_ST_PROC_REQ_E;
				//Reset new request received flag
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_new_req_rcvd_b = FALSE;
			}
	
			// Check if FF indication has received
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_FF_rcvd_b)
			{
				// Change AL state to receive in progress
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8 = DCAN_TP_ST_RX_INPROG_E;
			}
	
			// Check if the error in reception has occurred
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_err_in_rx)
			{
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_err_in_rx = FALSE;
			}
		}
	
		// Check if AL is in receive_in_progress state
		if (DCAN_TP_ST_RX_INPROG_E == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8)
		{
			// Check if the new request has received completely
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_new_req_rcvd_b)
			{
				// Change AL state to process request state
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8 = DCAN_TP_ST_PROC_REQ_E;
				//Reset new request received flag
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_new_req_rcvd_b = FALSE;
				//Reset FF received flag
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_FF_rcvd_b = FALSE;
			}
	
			// Check if error in reception has occurred
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_err_in_rx)
			{
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_err_in_rx = FALSE;
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_FF_rcvd_b = FALSE;
	
				// Change AL state to idle state
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8 = DCAN_TP_ST_IDLE_E;
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->timer_u32 = 0;
			}
		}
	
		if( TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->tx_prcwait_b )
        {
			if(DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8 != DCAN_TP_ST_TX_INPROG_E)
            {
                DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_TX_INITIATED_E;
                DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->tx_prcwait_b = FALSE;
			}
		}
	
		// Check if AL is in transmission_initiated state
		if (DCAN_TP_ST_TX_INITIATED_E == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8)
		{

			// Unlock the buffer
			// DCAN_NLUnlockBuffer();	//bufferstate removed
			TxStatus_u8 = DCAN_NL_TX((uint16_t)Index_instance_u8,
										(uint8_t*) DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->databuf_pu8,
										DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->numbytes_resp_u16);
			if (TRUE == TxStatus_u8)
			{
				//Change AL state to transmission in progress
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_TX_INPROG_E;
			}
			else
			{
				// Lock the buffer for Application layer
				// DCAN_NLLockBufferForAL();		//bufferstate removed
				count_u8++;
				
				// Transmission failure is checked for 5 times
				// After that, transmission in AL is aborted
				if (NUM_TIMESLICE_TO_WAIT == count_u8)
				{
					// Change AL state to idle state
					DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_IDLE_E;
					DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->timer_u32 = 0;
					// Unlock the buffer
					// DCAN_NLUnlockBuffer();	//bufferstate removed
					count_u8 = 0;   
				}
			}
		}
	
		// Check if AL is in transmission_in_progress state
		if (DCAN_TP_ST_TX_INPROG_E == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8)
		{
			// Reset the retransmission count to 0
			count_u8 = 0; 
			// Check if the response is transmitted successfully
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->flag_confirm_b)
			{
				DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->flag_confirm_b = FALSE;
				if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_P2Ext_b)
				{
					// Change AL state to process request state
					DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_IDLE_E;
					DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->al_state_u8 = DCAN_TP_ST_PROC_REQ_E;
				}
				else
				{
					// Change AL state to idle state
					// DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_IDLE_E;
					DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->timer_u32 = 0;
					clearBuffer(Index_instance_u8,BUFF2_TP_E);
				}
				// DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->timer_u32 = 0;
				// clearBuffer(Index_instance_u8,BUFF2_TP_E);
			}
	
			// Check if error in transmission has occurred
			if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->flag_err_in_tx)
			{
				// clearBuffer(Index_instance_u8,BUFF2_TP_E);
				// DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->flag_err_in_tx = FALSE;
	
				if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_P2Ext_b)
				{
					DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_P2Ext_b = FALSE;
				}
		
				clearBuffer(Index_instance_u8,BUFF2_TP_E);
			}
		}
		
	}	
    return;
}

/**
* Function name  : void DCAN_TP_FFRecd(void)
* @brief         : This function is called after receiving FF indication from network layer
* @param         : none
* @return        : none
* @ code tag     : @[CODE_AL_03] @{SDD_AL_03}
*/
void DCAN_TP_FFRecd (uint8_t Index_instance_u8)
{
	Diag_Disable_Interrupts ();
    DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_FF_rcvd_b = TRUE;
    if (TRUE == DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_err_in_rx)
    {
        DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF1_TP_E]->flag_err_in_rx = FALSE;
    }
    Diag_Restore_Interrupts ();
    return;
}

/**
* Function name   : void DCAN_TP_NewReqRecd(uint16_t)
* @brief          : This function is called after receiving NL_Indication from network layer
* @param          : uint16_t numbytes_req_u16: Number of bytes present in the request
* @return         : none
* @code tag       : @[CODE_OBD_TIMING_1] @{SDD_OBD_TIMING_1}
*     @[CODE_AL_02] @{SDD_AL_02}
*/
void DCAN_TP_NewReqRecd (uint8_t canTpInstanceId_u16,uint16_t numbytes_req_u16)
{
    uint32_t current_cputimer_u32 = 0;

    Diag_Disable_Interrupts ();
    DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_new_req_rcvd_b = TRUE;
    if (TRUE == DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_err_in_rx)
    {
        DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_err_in_rx = FALSE;
    }
    Diag_Restore_Interrupts ();
    DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->numbytes_req_u16 = numbytes_req_u16;

    /*Capture CPU time */
    current_cputimer_u32 = GET_TIME_MS();
    
    /*Load the P2 timer value */
    DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->P2_timings_u32 = DCAN_TPTiming_St.P2_u32;
    /*Start P2 timer */
    DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->timer_u32 = current_cputimer_u32 + DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->P2_timings_u32;
    return;
}

/**
* Function name   : void DCAN_TP_TxConf(void)
* @brief          : This function is called after receiving NL_Confirm from network layer
* @param          : none
* @return         : none
* @code tag       : @[CODE_AL_04] @{SDD_AL_04}
*/
void DCAN_TP_TxConf (uint8_t Index_instance_u8)
{
	Diag_Disable_Interrupts ();
    DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->flag_confirm_b = TRUE;
    Diag_Restore_Interrupts ();
    return;
}

/**
* Function name   : void DCAN_TP_TxErr(void)
* @brief          : This function is called after receiving NL_Confirm with error from network layer
* @param          : none
* @return         : none
* @code tag       : @[CODE_AL_05] @{SDD_AL_05}
*/
void DCAN_TP_TxErr (uint8_t Index_instance_u8)
{
	Diag_Disable_Interrupts ();
    DCAN_tpFrameCtrl_paSt[Index_instance_u8][BUFF2_TP_E]->flag_err_in_tx = TRUE;
    Diag_Restore_Interrupts ();
    return;
}

/**
* Function name   : void DCAN_TP_RxErr(void)
* @brief          : This function is called after receiving NL_Indication with error
*                   from network layer when AL is in receive_in_progress state
* @param          : none
* @return         : none
*/
void DCAN_TP_RxErr (uint8_t canTpInstanceId_u16)
{
	Diag_Disable_Interrupts ();
    DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_err_in_rx = TRUE;

    if (TRUE == DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_new_req_rcvd_b)
    {
        DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_new_req_rcvd_b = FALSE;
    }

    if (TRUE == DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_FF_rcvd_b)
    {
        DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_FF_rcvd_b = FALSE;
    }

    Diag_Restore_Interrupts ();
    return;
}

/**
* Function name    : void DCAN_CommCbk(void)
* @brief           : This is callback function which will in turn calls the application layer function depending on
*                    the CbkType_En.                
* @param           : CbkType_En     Its enum defined in cil_can_conf.h. Defines what type of callback function
*                                   gets called.
* @param             data_pu8       Its data from Network layer
* @param             length_u16     Length of data.               
* @return          : none
*/
static void DCAN_CommCbk(uint16_t canTpInstanceId_u16, nl2tp_CbkType_En_t CbkType_En, const uint8_t* data_pu8, uint16_t length_u16 )
{
    /*Figure it out which callback type is received*/
    switch(CbkType_En)
    {
        /*Is it new request received*/
        case CIL_NEW_REQ_Rxed_E:
        {
            /*We need the received request buffer in application. Copy the data to application
              global buffer*/
            CopyData(DCAN_TPDataBuffer1_au8[canTpInstanceId_u16], (uint32_t)length_u16, data_pu8);
            // while(1);
            /*Call the handler for New reuest*/
            DCAN_TP_NewReqRecd(canTpInstanceId_u16,length_u16);
            break;
        }
        /*Is it a receive error*/
        case CIL_Rx_ERROR_E:
        {
            /* Error in reception */
            DCAN_TP_RxErr(canTpInstanceId_u16); 
            break;
        }
        /*Is it Transmission confirm*/
        case CIL_TX_CONFRM_E:
        {          
            /* Response message transmitted successfully */
            DCAN_TP_TxConf(canTpInstanceId_u16);
            break;
        }
        /*Is it transmission error*/
        case CIL_TX_ERROR_E:
        {
            /* Error in transmission */
            DCAN_TP_TxErr(canTpInstanceId_u16); 
            break;
        }
        /*Is it FF received*/
        case CIL_FF_RXED_E:
        {
            /*FF indication received*/
            DCAN_TP_FFRecd(canTpInstanceId_u16);  
            break;
        }
    default:
      {
          /*Do nothing*/
          break;
      }
    }
}


/* *****************************************************************************
 End of File
 */















