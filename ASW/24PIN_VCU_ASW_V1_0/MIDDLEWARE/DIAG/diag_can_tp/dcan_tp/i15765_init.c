/******************************************************************************
 *    FILENAME    : i15765_init.c
 *    DESCRIPTION : This file contains initialization functions.
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
#include "i15765_conf.h"
// #include "i15765_priv.h"
#include "cil_can_conf.h"      
#include "i15765.h"

/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
*/
/*Callback for application layer*/
DCAN_Appl_fcbk DCAN_ApplCbk;

static void i15765_nl_init_buffers(void);


/*Network layer global buffer*/
// uint8_t_t DCAN_CH1_nlDataBuffer[MAX_LENGTH_DIAGBUFFER] = {0};
uint8_t DCAN_CH1_nlDataBuffer[DCAN_TP_INSTANCE_TOTAL_E][MAX_LENGTH_DIAGBUFFER] = {0};

#if (ISO15765_CONF_CH_TYPE == CANTP_FULL_DUPLEX_E)
// uint8_t_t DCAN_CH2_nlDataBuffer[MAX_LENGTH_DIAGBUFFER] = {0};
uint8_t DCAN_CH2_nlDataBuffer[DCAN_TP_INSTANCE_TOTAL_E][MAX_LENGTH_DIAGBUFFER] = {0};
#endif

/* Definition of the message control structure */
// DCAN_nlFrameCtrl_st_t   DCAN_nlFrameCtrl_st;
// DCAN_nlFrameCtrl_st_t   *DCAN_nlFrameCtrl_pst = NULL;

/*Hold the channel one data for transmission*/
// DCAN_nlFrameCtrl_st_t CH1_nlFrameCtrl_st;  
DCAN_nlFrameCtrl_st_t CH1_nlFrameCtrl_st[DCAN_TP_INSTANCE_TOTAL_E];  
#if (ISO15765_CONF_CH_TYPE == CANTP_FULL_DUPLEX_E)
    /*Hold the channel two data for receiption. */
// DCAN_nlFrameCtrl_st_t CH2_nlFrameCtrl_st;
DCAN_nlFrameCtrl_st_t CH2_nlFrameCtrl_st[DCAN_TP_INSTANCE_TOTAL_E];
#endif

// uint8_t_t* CHBuffer_au8[TOTAL_CH] = {0};
// DCAN_nlFrameCtrl_st_t *DCAN_nlFrameCtrl_paSt[TOTAL_CH] = {NULL};
uint8_t* CHBuffer_au8[DCAN_TP_INSTANCE_TOTAL_E][TOTAL_CH] = {0};
DCAN_nlFrameCtrl_st_t *DCAN_nlFrameCtrl_paSt[DCAN_TP_INSTANCE_TOTAL_E][TOTAL_CH] = {NULL};


/**
 *  FUNCTION NAME : void i15765_nl_init_buffers(void)
 *  @brief        : 
 *  @param        : none

 *
 *  @return       : none
 *
 */
static void i15765_nl_init_buffers(void)
{
	int i = 0;
	for(i = 0 ; i < DCAN_TP_INSTANCE_TOTAL_E ; i ++)
	{
		/*Fill the array with supporting structure.
		Assign the CH1 structure because we can access it in both the channel.*/
		DCAN_nlFrameCtrl_paSt[i][BUFF1_NL_E] = &CH1_nlFrameCtrl_st[i];
		DCAN_nlFrameCtrl_paSt[i][BUFF2_NL_E] = &CH1_nlFrameCtrl_st[i];
		/*Set the buffers for both channel*/
		CHBuffer_au8[i][BUFF1_NL_E] = DCAN_CH1_nlDataBuffer[i];
		CHBuffer_au8[i][BUFF2_NL_E] = DCAN_CH1_nlDataBuffer[i];
		
		/*Decide, Is controller supporting dual channel?*/
#if (ISO15765_CONF_CH_TYPE == CANTP_FULL_DUPLEX_E) 
    
		/*Assign the CH2 structure on request of receive structure*/
		DCAN_nlFrameCtrl_paSt[i][BUFF2_NL_E] = &CH2_nlFrameCtrl_st[i];
		CHBuffer_au8[i][BUFF2_NL_E] = DCAN_CH2_nlDataBuffer[i];
#endif
    }
    return ;
}

/**
 *  FUNCTION NAME : DCAN_NL_Init
 *  @param        : none
 *  @brief        : intialise the interal network layer message control register struct
 *                  DCAN_nlFrameCtrl_st
 *  @return       : none
 */
void DCAN_NL_Init (void)
{
    uint16_t index_u16 = 0u;
	uint8_t i  = 0;
    /*Fill the array with supporting channel structure*/
    i15765_nl_init_buffers();
    
    /*Set the default values*/
	for(i = 0 ; i < DCAN_TP_INSTANCE_TOTAL_E ; i++){
		for(index_u16 = 0; index_u16 < TOTAL_CH; index_u16++)
		{
		// DCAN_nlFrameCtrl_paSt[index_u16] = &DCAN_nlFrameCtrl_st;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->buffer_pu8 = CHBuffer_au8[i][index_u16];
			DCAN_nlFrameCtrl_paSt[i][index_u16]->bufferlength_u16 = MAX_LENGTH_DIAGBUFFER;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->timer_u32 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->bufferindex_u16 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->BSmaxCtr_u8 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->BSmax_u8 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->SNCtr_u8 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->wtsfcCtrl_u8 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->activeNlhandle_u8 = 0;
			DCAN_nlFrameCtrl_paSt[i][index_u16]->stmin_u8 = 0;
			/* Active Network layer state to idle */
			DCAN_nlFrameCtrl_paSt[i][index_u16]->nl_state_u8 = DCAN_NL_ST_IDLE_E;
			/* Change the buffer state to idle */
			DCAN_nlFrameCtrl_paSt[i][index_u16]->bufferstate_u8 = DCAN_NL_STBUFF_IDLE_E;
		}
		DCAN_nlInputVector_pst[i] = &DCAN_nlInputVector_st[i]; 
    }
	DCAN_TPTiming_St.P2_u32 = DCAN_TPTimings_aSt[DEFAULT_VALUE].P2_u32;
	DCAN_TPTiming_St.P3_u32 = DCAN_TPTimings_aSt[DEFAULT_VALUE].P3_u32;
    /* Pointer to the input vector table */
	       
    return;
}


/**
 *  FUNCTION NAME : DCAN_NL_RegisterCbk
 *  @param        : none
 *  @brief        : Register the application layer callback function.
 *  @return       : none
 */
void DCAN_NL_RegisterCbk(DCAN_Appl_fcbk Appl_cbk)
{
    DCAN_ApplCbk = Appl_cbk;
}

/* *****************************************************************************
 End of File
 */


