/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File          : can_sched_conf.c
|    Project      : MIL_PBL_CV
|    Description    : File contains configuration related to the can schedulers.
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


/* Section: Included Files                                                   */
#include "can_sched.h"
#include "can_sched_conf.h"
#include "cil_can_conf.h"
#include "math_util.h"
#include "diag_appl_test.h"


#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
	#include "j1939.h"
#endif
#if(TRUE == DIAG_CONF_UDS_SUPPORTED)

#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)

#endif

/**************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */

/*
 * @summary:- can structure message buffer  
 */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

 uint32_t CAN_MsgRxBoxTimeOutCnt [CAN_SCHED_CONF_TOTAL_RX_MSG] = {0};
 uint16_t CAN_Transmit_TimerCnt [CAN_SCHED_CONF_TOTAL_TX_MSG]  = {0};

 uint8_t CAN_SCHED_CONF_TOTAL_TX_MSG_u8 = CAN_SCHED_CONF_TOTAL_TX_MSG;
 uint8_t CAN_SCHED_CONF_TOTAL_RX_MSG_u8 = CAN_SCHED_CONF_TOTAL_RX_MSG;

/*  A brief description of a section can be given directly below the section
    banner.
 */
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)

uint32_t CAN_CONFIG_SCHED_ACTIVE_RX_u32;
uint32_t CAN_CONFIG_SCHED_ACTIVE_TX_u32;

#endif


/* ************************************************************************** */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

#if(TRUE == DIAG_CONF_J1939_SUPPORTED)

BOOL IsRXCanMessageActive(UINT32 BitPosCANMessage)
{
    BOOL ret_b = (BOOL)FALSE;
    
    if(BitPosCANMessage == 0xFFFFU)
    {
        ret_b = (BOOL)TRUE;        
    }
    else 
    {            
        ret_b = GETBIT(CAN_CONFIG_SCHED_ACTIVE_RX_u32, BitPosCANMessage);
    }
    
    return ret_b;
}

BOOL IsTXCanMessageActive(UINT32 BitPosCANMessage)
{
    BOOL ret_b = (BOOL)FALSE;
    
    if(BitPosCANMessage == 0xFFFFU)
    {
        ret_b = (BOOL)TRUE;        
    }
    else 
    {    
        ret_b = (BOOL)GETBIT(CAN_CONFIG_SCHED_ACTIVE_TX_u32, BitPosCANMessage);
    }
    
    return ret_b;
}
#endif
/*  A brief description of a section can be given directly below the section
    banner.
 */

// *****************************************************************************


/* count of receive interrupt callbacks" */
//static uint16_t CAN_IntCbCnt_u16 = 0;    

/* Counter for Time out timer */
//static uint32_t CAN_MsgRxBoxTimeOutCnt [CIL_DCAN_TOTAL_RX_E] = { 0 };



/*
 * @summary:-    array of can Application structures    
 */
/* LDRA_EXCLUDE_START 104 S */
/* LDRA_EXCLUDE_START 123 S */
const CANSCHED_RX_Conf_St_t   CANSCHED_RX_Conf_aSt[CIL_CAN_TOTAL_RX_E]=
{
#if (TRUE == DIAG_CONF_UDS_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
        {CIL_CANTP_REQ_IVN_RX_E, PERIODICITY_MS(5), CANSched_RxMsgCallback, NULL},
        {CIL_CANTP_REQ_TESTER_RX_E, PERIODICITY_MS(5), CANSched_RxMsgCallback, NULL},        
        {CIL_CANTP_REQ_FUNC_RX_E, PERIODICITY_MS(5), CANSched_RxMsgCallback, NULL},


#endif
#if (TRUE == DIAG_CONF_OBD2_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
        {CIL_CANTP_REQ_OBD_TESTER_RX_E, PERIODICITY_MS(5), CANSched_RxMsgCallback, NULL},
        {CIL_CANTP_REQ_OBD_FUNC_RX_E, PERIODICITY_MS(5), CANSched_RxMsgCallback, NULL},
#endif
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
        {CIL_J1939_REQ_DA_RX_E, PERIODICITY_MS(20U), J1939_reqPgnClbk, J1939_rqPgnClbk_timeout},
        {CIL_J1939_REQ_DA_BAM_RX_E, PERIODICITY_MS(20U), J1939_reqPgnClbk, J1939_rqPgnClbk_timeout},

        {CIL_J1939_TPCM_RX_E, PERIODICITY_MS(20U), J1939_tpcmClbk, NULL},
        {CIL_J1939_TPCM_BAM_RX_E, PERIODICITY_MS(20U), J1939_tpcmClbk, NULL},
        {CIL_J1939_TPDT_RX_E, PERIODICITY_MS(20U), J1939_tpdtClbk, NULL},
        {CIL_J1939_DM22_RX_E, PERIODICITY_MS(20U), J1939_dm22Clbk, NULL},
        {CIL_J1939_TPDT_BAM_RX_E, PERIODICITY_MS(20U), J1939_tpdtClbk, NULL},
        {CIL_J1939_81_NMAC_RX_E, NO_TIMEOUT, J1939_81_RXNMAC_Callback, NULL},

#endif
#if (TRUE == DIAG_TEST_FM_EEPROM_DEMO)
        {CIL_EEPROM_FAULT_FUNC_RX_E, PERIODICITY_MS(5), NULL, NULL},
#endif
#if (TRUE == DIAG_TEST_FM_DEMO && TRUE == DIAG_CONF_FM_SUPPORTED)
        {CIL_RX_FM_TEST_DEMO_E, PERIODICITY_MS(5), diag_appl_test_fm_drv_Cycle, NULL},
#endif
#if (TRUE == BMS_SUPPORTED)
#endif

#if (TRUE == VEHICLE_SUPPORTED)
#endif

#if (TRUE == MCU_SUPPORTED)   
#endif

#if (TRUE == TFT_SUPPORTED)
#endif

#if (TRUE == EVC2C_SUPPORTED)       
#endif

#if (TRUE == CCS_SUPPORTED)
#endif

};


/*LDRA_INSPECTED 1 X */
const CANSCHED_TX_Conf_St_t CANSCHED_TX_Conf_aSt[CAN_SCHED_CONF_TOTAL_TX_MSG] = {
        // CIL Sig name,			Cycle Time					Offset (Timeslice)			Call Back Function
        {CIL_CANTP_RESP_TESTER_TX_E, PERIODICITY_MS(1000U), PERIODICITY_MS(0U), NULL},
        {CIL_CANTP_RESP_IVN_TX_E, PERIODICITY_MS(1000U), PERIODICITY_MS(0U), NULL},

#if (TRUE == DIAG_CONF_OBD2_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
        {CIL_CANTP_RESP_OBD_TESTER_TX_E, PERIODICITY_MS(1000U), PERIODICITY_MS(0U), NULL},
#endif
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
        {CIL_J1939_71_TEST1_TX_E, PERIODICITY_MS(500U), PERIODICITY_MS(500U), J1939_71_TX_TEST1},
        {CIL_J1939_71_TEST2_TX_E, PERIODICITY_MS(500U), PERIODICITY_MS(500U), J1939_71_TX_TEST2},
        {CIL_J1939_71_TEST3_TX_E, PERIODICITY_MS(500U), PERIODICITY_MS(500U), J1939_71_TX_TEST3},
        {CIL_j1939_tpdt_TX_E, PERIODICITY_MS(60U), PERIODICITY_MS(25U), J1939_tp_txDtManage},
#endif
#if (TRUE == DIAG_TEST_FM_DEMO && TRUE == DIAG_CONF_FM_SUPPORTED)
        {CIL_FM_TEST1_TX_E, PERIODICITY_MS(500U), PERIODICITY_MS(500U), diag_fm_tx_fault1},
        {CIL_FM_TEST1_TX_E, PERIODICITY_MS(500U), PERIODICITY_MS(500U), diag_fm_tx_fault2},
#endif

#if (TRUE == MCU_SUPPORTED) 
      
#endif

#if (TRUE == BMS_SUPPORTED)
#endif

#if (TRUE == TFT_SUPPORTED)      
#endif

#if (TRUE == EVC2C_SUPPORTED)
#endif

#if (TRUE == CCS_SUPPORTED)
#endif
};
/* LDRA_EXCLUDE_END 104 S */
/* LDRA_EXCLUDE_END 123 S */



/* *****************************************************************************
 End of File
 */
