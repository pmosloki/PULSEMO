/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File          : uds_session.c
|    Project      : MIL_PBL_CV
|    Description    : Service description for UDS session
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
/*
****************************************************************************
* Includes
****************************************************************************
*/
#include "str_util.h"
#include "crc_util.h"
#include "diag_typedefs.h"
#include "diag_adapt.h"
#include "uds_session.h"
#include "diag_sys_conf.h"
#include "i15765.h"
#include "uds_conf.h"
#include "fee_adapt.h"
#include "uds_DID.h"
#include"iso14229_serv27.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

static UDS_Serv_resptype_En_t ServDistUDS(uint16_t canTpInstanceId_u16);
UDS_Serv_St_t Serv_databuf_St;

/*  A brief description of a section can be given directly below the section
	banner.
 */

/* ************************************************************************** */

/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
 */

// Global variable that contains the application layer specific data
static ServiceDistUDS_St_t *ServDistUDS_pt = NULL;

/**
 *  Function name  : void uds_session_proc_5ms()
 *  @brief         : Function for uds session process of 5ms.
 *
 *  @param         : none
 *
 *  @return        : none.

 */
void uds_session_proc_5ms(void)
{
	uint16_t canTpInstanceId_u16 = 0;
	uint32_t current_cputimer_u32 = 0;
	UDS_Serv_resptype_En_t resp_type_En = UDS_SERV_RESP_NORESP_E; // To store the response code
	static bool instanceSessTimeout_b = false;

#if (TRUE == DIAG_CONF_CANTP_SUPPORTED)

	static uint8_t instanceSessTimeout_au8[DCAN_TP_INSTANCE_TOTAL_E] /*= {0, 0}*/;

	current_cputimer_u32 = GET_TIME_MS();

	instanceSessTimeout_b = true;
	for (canTpInstanceId_u16 = DCAN_TP_INSTANCE_UDSDIAG_START_E; canTpInstanceId_u16 < DCAN_TP_INSTANCE_UDSDIAG_END_E; canTpInstanceId_u16++)
	{
		if (DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->sessionTimeoutTimer_u32 <= current_cputimer_u32)
		{
			instanceSessTimeout_au8[canTpInstanceId_u16] = true;
			DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->sessionTimeoutTimer_u32 = GET_TIME_MS() + ISO15765_CONF_SESSION_TIMEOUT_DELAY_MS; // todo harsh
		}
		instanceSessTimeout_b = instanceSessTimeout_b && instanceSessTimeout_au8[canTpInstanceId_u16];
	}

	if (instanceSessTimeout_b)
	{
		instanceSessTimeout_b = false;

		/* session timeout when there is no traffic any instances */
		service_distributor_timeout();

		for (canTpInstanceId_u16 = DCAN_TP_INSTANCE_UDSDIAG_START_E; canTpInstanceId_u16 < DCAN_TP_INSTANCE_UDSDIAG_END_E; canTpInstanceId_u16++)
		{
			instanceSessTimeout_au8[canTpInstanceId_u16] = false;
		}
	}

	

	for (canTpInstanceId_u16 = DCAN_TP_INSTANCE_UDSDIAG_START_E; canTpInstanceId_u16 < DCAN_TP_INSTANCE_UDSDIAG_END_E; canTpInstanceId_u16++)
	{
		// Check if AL is in process_request state
		if (DCAN_TP_ST_PROC_REQ_E == DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->al_state_u8)
		{

			// if(canTpInstanceId_u16 == DCAN_TP_INSTANCE_IVN_E)  
			// {
			// 	FunAddessedID_flag = 1;
			// }
			resp_type_En = ServDistUDS(canTpInstanceId_u16);

			if (DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->databuf_pu8[SID_INDEX] != UDS_NEG_RESP_CODE)
			{
				DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->sessionTimeoutTimer_u32 = GET_TIME_MS() + ISO15765_CONF_SESSION_TIMEOUT_DELAY_MS;
				instanceSessTimeout_au8[canTpInstanceId_u16] = false;
			}
			else
			{
				if (DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->databuf_pu8[2] != SERVICE_NOT_SUPPORTED &&
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->databuf_pu8[2] != SERVNOSUPP_IN_ACTIVE_SESS)
				{
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->sessionTimeoutTimer_u32 = GET_TIME_MS() + ISO15765_CONF_SESSION_TIMEOUT_DELAY_MS;
					instanceSessTimeout_au8[canTpInstanceId_u16] = false;
				}
			}

			if ((UDS_SERV_RESP_POS_E == resp_type_En) ||
				(UDS_SERV_RESP_NEG_E == resp_type_En))
			{
				clearBuffer(canTpInstanceId_u16, BUFF1_TP_E);

				if (DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->al_state_u8 != DCAN_TP_ST_TX_INPROG_E)
				{
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_TX_INITIATED_E;
				}
				else
				{
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->tx_prcwait_b = TRUE;
				}
			}

			if (UDS_SERV_RESP_NORESP_E == resp_type_En)
			{
				clearBuffer(canTpInstanceId_u16, BUFF1_TP_E);
			}

			FunAddessedID_flag = 0;
			current_cputimer_u32 = GET_TIME_MS();

			if (((UDS_SERV_RESP_WAITPEND_E == resp_type_En))&&
				(FALSE == DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_P2Ext_b))
			{

				clearBuffer(canTpInstanceId_u16, BUFF1_TP_E);

				DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_P2Ext_b = TRUE;
				/* Load the P2 timer value */
				DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->P2_timings_u32 = DCAN_TPTiming_St.P3_u32;
				/* Start P2 timer */
				DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->timer_u32 = current_cputimer_u32 + DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->P2_timings_u32;

				if (DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->al_state_u8 != DCAN_TP_ST_TX_INPROG_E)
				{
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_TX_INITIATED_E;
				}
				else
				{
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->tx_prcwait_b = TRUE;
				}
			}

			// Check if P2 or extended P2 timer expires
			if (DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->timer_u32 <= current_cputimer_u32)
			{
				if (TRUE == DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->flag_P2Ext_b)
				{
					if (UDS_SERV_RESP_WAITPEND_E == resp_type_En)
					{
						/* Reload extended P2 timer again */
						DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->timer_u32 = current_cputimer_u32 + DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->P2_timings_u32;
						DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->al_state_u8 = DCAN_TP_ST_IDLE_E;

						// Change AL state to transmission initiated
						DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->al_state_u8 = DCAN_TP_ST_TX_INITIATED_E;
					}
				}
				else
				{
					// Change AL state to idle state
					// DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->timer_u32 ->al_state_u8 = DCAN_TP_ST_IDLE_E;
					DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->timer_u32 = 0;
					// clearBuffer(canTpInstanceId_u16,BUFF1_TP_E);
				}
			}
		}
	}
	
	UdsAppProc();
#endif
}

/**
 *  Function name  : void setOBDServDist (ServiceDist_St_t*)
 *  @brief         : Function captures the number of entries in the Distributor .
 *                   table and the configuration structure pointer.

 *  @param         : ServiceDist_St_t* ConfigServDist_pt - pointer to service distributor
 *                                                 table.
 *  @param         : uint32_t numentries_u32 - Number of entries in configuration table.
 *
 *  @return        : none.
 *  @Code tag      : @[CODE_OBD_SERVDIST_3] @{SDD_OBD_SERVDIST_3}
 */
void setUDSServDist(const ServiceDistUDS_St_t *ConfigServDist_pt, uint32_t numentries_u32)
{
	ServDistUDS_pt = ConfigServDist_pt; // Service distributor pointer
	return;
}

/**
 *  Function name  : void servDistSetstr()
 *  @brief         : Function for service distributor setting
 *

 *  @param         : none

 *
 *  @return        : none.

 */
void servDistSetstr(void)
{
#if (TRUE == DIAG_CONF_UDS_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)

	Serv_databuf_St.RxBuff_pu8 = DCAN_tpFrameCtrl_paSt[DCAN_TP_INSTANCE_UDSDIAG_E /*DCAN_TP_INSTANCE_IVN_E*/][BUFF1_TP_E]->databuf_pu8;
	Serv_databuf_St.TxBuff_pu8 = DCAN_tpFrameCtrl_paSt[DCAN_TP_INSTANCE_UDSDIAG_E /*DCAN_TP_INSTANCE_IVN_E*/][BUFF2_TP_E]->databuf_pu8;
	Serv_databuf_St.RxLen_u16 = 0;
	Serv_databuf_St.TxLen_u16 = 0;

#endif
	return;
}

/**
 *  Function name  : void ServDistUDS_init()
 *  @brief         : Function for service distributor initialization.
 *

 *  @param         : none

 *
 *  @return        : none.

 */
void ServDistUDS_init(void)
{
	setUDSServDist((const ServiceDistUDS_St_t *)UDSServDist_apt, sizeof(UDSServDist_apt) / sizeof(UDSServDist_apt[0]));
	servDistSetstr();
}

void UDS_Serv_init(void)
{
	uint8_t idx_u8;
	ReadPblParameter();

	for (idx_u8 = 0; (idx_u8 < UDS_TOTAL_SERVICE_E); idx_u8++)
	{
		// If the PID is found then assert the flag to stop the search.
			if (NULL != ServDistUDS_pt[idx_u8].ServiceInit_Fptr_t)
		{
			ServDistUDS_pt[idx_u8].ServiceInit_Fptr_t();
		}
	}
 	//UpdateNVMblock(PBL_NVM_BLOCK);
	return;
}

/**
 * Function name  : Serv_resptype_En_t ServDist_CAN ()
 * @brief         : Function implements the service distributor for ISO 14229
 * @param         : NULL
 * @return        : Type of response
 * @Code tag      : @[CODE_OBD_SERVDIST_2] @{SDD_OBD_SERVDIST_2}
 */
static UDS_Serv_resptype_En_t ServDistUDS(uint16_t canTpInstanceId_u16)
{
	UDS_Serv_resptype_En_t respCode_E = UDS_SERV_RESP_NORESP_E; // Initialize the response code to no response
	static uint8_t sid_u8 = 0;									// Variable to hold the received SID number
	static bool call_serviceagain_b = FALSE;					// Flag to track the request is called for wait pending CVN
	bool foundService_b = FALSE;								// flag used to trace the SID table
	uint8_t idx_u8 = 0;											// For loop tracing index
#if (TRUE == DIAG_CONF_CANTP_SUPPORTED)

	// Check if the service distributor pointer is valid.
	if (ServDistUDS_pt != NULL)
	{
		Serv_databuf_St.RxBuff_pu8 = DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->databuf_pu8;
		Serv_databuf_St.TxBuff_pu8 = DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->databuf_pu8;

		Serv_databuf_St.TxLen_u16 = 0;
		Serv_databuf_St.RxLen_u16 = DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF1_TP_E]->numbytes_req_u16;

		if (!call_serviceagain_b)
		{
			sid_u8 = Serv_databuf_St.RxBuff_pu8[SID_INDEX];
		}

		// Search for the PID and break in case of a success
		for (idx_u8 = 0; (idx_u8 < UDS_TOTAL_SERVICE_E); idx_u8++)
		{
			// If the PID is found then assert the flag to stop the search.
			if (sid_u8 == ServDistUDS_pt[idx_u8].sid_u8)
			{

				foundService_b = TRUE;
				break;
			}
		}

		if (!foundService_b && (!FunAddessedID_flag))
		{
			// fill the negative response code in the buffer
			Serv_databuf_St.TxBuff_pu8[SID_INDEX] = 0x7F;
			// Copy the SID to the second byte
			Serv_databuf_St.TxBuff_pu8[SID_INDEX + 1] = sid_u8;
			Serv_databuf_St.TxBuff_pu8[2] = SERVICE_NOT_SUPPORTED;
			Serv_databuf_St.TxLen_u16 = 3;
			respCode_E = UDS_SERV_RESP_NEG_E;
		}
		else if (!foundService_b)
		{
			FunAddessedID_flag = 0;
		}
		// Check for the valid index
		else
		{
			if((true == FunAddessedID_flag) && (false == ServDistUDS_pt[idx_u8].FunAddrEnable_u8))
			{
				respCode_E = UDS_SERV_RESP_NORESP_E;
			}
			// Valid service intrepreter exist call the function
			else if (ServDistUDS_pt[idx_u8].Service_Fptr_t)
			{
				ServDistUDS_pt[idx_u8].Session_subid_u8;
				
				if (UdsDiagSessionCheck(ServDistUDS_pt[idx_u8].Session_subid_u8))
				{
					// Call the service functions depends upon the SID.
					respCode_E = ServDistUDS_pt[idx_u8].Service_Fptr_t(&Serv_databuf_St);
				}
				else
				{
					// fill the negative response code in the buffer
					Serv_databuf_St.TxBuff_pu8[SID_INDEX] = 0x7F;
					// Copy the SID to the second byte
					Serv_databuf_St.TxBuff_pu8[SID_INDEX + 1] = sid_u8;
					Serv_databuf_St.TxBuff_pu8[2] = SERVNOSUPP_IN_ACTIVE_SESS;
					Serv_databuf_St.TxLen_u16 = 3;
					respCode_E = UDS_SERV_RESP_NEG_E;
				}

				switch (respCode_E)
				{
				// For positive response
					case UDS_SERV_RESP_POS_E:
					{
						Serv_databuf_St.TxBuff_pu8[SID_INDEX] = (uint8_t)(Serv_databuf_St.RxBuff_pu8[SID_INDEX] + 0x40);
						Serv_databuf_St.TxLen_u16 = (uint16_t)(Serv_databuf_St.TxLen_u16 + 1);
						call_serviceagain_b = FALSE;
						break;
					}
					// For negative response
					case UDS_SERV_RESP_NEG_E:
					{
						if ((Serv_databuf_St.TxBuff_pu8[2] != INVALID_MESSAGE_LENGTH) && (FunAddessedID_flag == 1))
						{
							/*NOTE :- As per ISO14229 NRC should not be sent. If servic is requested from funtional ID
							Refer :- ISO 14229-1:2020(E),
							Section :- 8.7.1
							Page number :-  19*/
							respCode_E = UDS_SERV_RESP_NORESP_E;
						}
						call_serviceagain_b = FALSE;
						break;
					}
					// For waiting response
					case UDS_SERV_RESP_WAITPEND_E:
					{
						// Fill the negative response code in the buffer
						Serv_databuf_St.TxBuff_pu8[SID_INDEX + 1] = Serv_databuf_St.RxBuff_pu8[SID_INDEX];
						Serv_databuf_St.TxBuff_pu8[SID_INDEX] = 0x7F;
						Serv_databuf_St.TxBuff_pu8[SID_INDEX + 2] = 0x78;
						Serv_databuf_St.TxLen_u16 = (uint16_t)(Serv_databuf_St.TxLen_u16 + 3);
						call_serviceagain_b = TRUE;
						break;
					}
					case UDS_SERV_RESP_NORESP_E:
					{
						;
					}
					default:
					{
						call_serviceagain_b = FALSE;
						break;
					}
				}
			}
			else
				{
					respCode_E = UDS_SERV_RESP_NORESP_E;
				}
		}
		DCAN_tpFrameCtrl_paSt[canTpInstanceId_u16][BUFF2_TP_E]->numbytes_resp_u16 = Serv_databuf_St.TxLen_u16;
	}
#endif
	return respCode_E;
}

/**
 * Function name  : void service_distributor_timeout(void)
 * @brief         : Function is call all service timeout function
 * @param         : none
 * @return        : none
 */

void service_distributor_timeout(void)
{
	UDS_SessionTimeout();
}


/* *****************************************************************************
 End of File
  ******************************************************************************/
