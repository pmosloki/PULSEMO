/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File          : can_sched_conf.h
|    Project      : MIL_PBL_CV
|    Description    : This file contains buffer for CIL layer data extraction as  and other structs defined in i15676.h
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

#ifndef CAN_SCHED_CONF_H_
#define CAN_SCHED_CONF_H_

#include "can_if.h"
#include "can_sched.h"
#include "diag_sys_conf.h"
#if(TRUE == DIAG_CONF_CANTP_SUPPORTED)
	#include "i15765.h"
#endif
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */
#define 	CAN_SCHED_CONF_TICK_PERIOD		(5U) 		// in terms of 1ms.

#define    	PERIODICITY_MS(x)  					((x)/CAN_SCHED_CONF_TICK_PERIOD)
#define    	NO_TIMEOUT   						0

#if(TRUE == DIAG_CONF_J1939_SUPPORTED)

#define  BITPOSTX_CAN_DM1_TIME      0U
#define  BITPOSTX_CAN_DM1_EVENT     1U
#define  BITPOSRX_CAN_SPECREQ       2U
#define  BITPOSRX_CAN_TPCMRX        3U
#define  BITPOSRX_CAN_TPCMBAM       4U
#define  BITPOSRX_CAN_TPDTRX        5U
#define  BITPOSRX_CAN_TPDTBAM       6U
#define  BITPOSRX_CAN_DM22          7U
#define  BITPOSRX_CAN_J1939_81_NMAC 8U
#define  BITPOSRX_CAN_J1939_REQ_DA  9U
#define  BITPOSRX_CAN_J1939_REQ_MA  10U

#endif
/*Reception*/

typedef void (*CANSCHED_CB_Fptr_t)( uint16_t sig,  CAN_MessageFrame_St_t* msg_rf );
typedef bool (*CAN_SIGALC_MSGA_Fptr_t)(uint32_t  Bitpos);
typedef void (*CANSCHED_TCB_Fptr_t)(void);
  

/*
 ******************************************************************************
 *    CAN SCHEDULER configuration structure
 ******************************************************************************
*/ 
typedef struct
{
    uint32_t                    CIL_CAN_SigName_En;     /* CIL CAN */
    uint32_t                    Timeoutms_u32;          /* Timeout value in ms */
	/*LDRA_INSPECTED 35 X */
    CANSCHED_CB_Fptr_t          CANSCHED_CB_Fptr;       /* Receive callback function  */
    CANSCHED_TCB_Fptr_t         CANSCHED_TCB_Fptr;      /* Timeout callback function  */
    /* CAN_SIGALC_MSGA_Fptr_t  CAN_SIGNALC_MSGA_Fptr;  Is Signal active CAN Function  */
    /* uint32_t                  Bitpos_u32; */
}CANSCHED_RX_Conf_St_t;

 
typedef struct
{  
    uint32_t                    CIL_CAN_SigName_En;     /* CIL CAN */
    const uint16_t              Cycle_Time_u16;         /* Pointer to constant which contains the cylce time in ms */
    const uint16_t		Offset_Time_u16; 		/* Offset time in ms */
	/*LDRA_INSPECTED 35 X */
    CANSCHED_TCB_Fptr_t         CANSCHED_CB_Fptr;       /* Tx callback function  */
/*    CAN_SIGALC_MSGA_Fptr_t      CAN_SIGNALC_MSGA_Fptr;  Is Signal active CAN Function 
   uint32_t                    Bitpos_u32; */
}CANSCHED_TX_Conf_St_t;

#if(TRUE == DIAG_CONF_UDS_SUPPORTED)
	#define CAN_SCHED_CONF_UDS_RX_MSG (CIL_RX_UDS_END - CIL_RX_UDS_START_E)
#else
	#define CAN_SCHED_CONF_UDS_RX_MSG 0
#endif

#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)
	#define CAN_SCHED_CONF_OBD_RX_MSG (CIL_RX_OBD_END - CIL_RX_OBD_START)
#else
	#define CAN_SCHED_CONF_OBD_RX_MSG 0
#endif

#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
	#define CAN_SCHED_CONF_J1939_RX_MSG (CIL_RX_J1939_END - CIL_RX_J1939_START)
#else
	#define CAN_SCHED_CONF_J1939_RX_MSG 0
#endif

#if (TRUE == DIAG_TEST_FM_DEMO && TRUE == DIAG_CONF_FM_SUPPORTED)
	#define CAN_SCHED_FM_TEST_RX_MSG (CIL_RX_FM_TEST_END_E - CIL_RX_FM_TEST_START_E)
#else
	#define CAN_SCHED_CONF_J1939_RX_MSG 0
#endif

#define CAN_SCHED_CONF_TOTAL_RX_MSG     (CAN_SCHED_CONF_UDS_RX_MSG + CAN_SCHED_CONF_OBD_RX_MSG + CAN_SCHED_CONF_J1939_RX_MSG + CAN_SCHED_FM_TEST_RX_MSG)//3u  // This shall be updated manually by the user depending on number of entries in the Rx conf table.
extern  const CANSCHED_RX_Conf_St_t   CANSCHED_RX_Conf_aSt[CIL_CAN_TOTAL_RX_E];

#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
	#define CAN_SCHED_CONF_J1939_TX_MSG 4U
#else
	#define CAN_SCHED_CONF_J1939_TX_MSG 0U
#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)
	#define CAN_SCHED_CONF_OBD2_TX_MSG 1U
#else
	#define CAN_SCHED_CONF_OBD2_TX_MSG 0U
#endif
#if(TRUE == DIAG_TEST_FM_DEMO && TRUE == DIAG_CONF_FM_SUPPORTED)
	#define CAN_SCHED_CONF_FM_DEMO_TX_MSG  2U
#else
	#define CAN_SCHED_CONF_FM_DEMO_TX_MSG  0U
#endif
#if(TRUE == VCU_CONF_HMI_SUPPORTED)
	#define CAN_SCHED_CONF_HMI_TX_MSG 6
#else
	#define CAN_SCHED_CONF_HMI_TX_MSG 0
#endif
#if(TRUE == MCU_SUPPORTED)
	#define CAN_SCHED_CONF_MC_TX_MSG 3
#else
	#define CAN_SCHED_CONF_MC_TX_MSG 0
#endif

#define CAN_SCHED_CONF_TOTAL_TX_MSG     2U + CAN_SCHED_CONF_J1939_TX_MSG \
					+ CAN_SCHED_CONF_OBD2_TX_MSG \
					+ CAN_SCHED_CONF_FM_DEMO_TX_MSG \
					+ CAN_SCHED_CONF_HMI_TX_MSG     \
					+ CAN_SCHED_CONF_MC_TX_MSG// This shall be updated manually by the user depending on number of entries in the Tx conf table.
extern const CANSCHED_TX_Conf_St_t CANSCHED_TX_Conf_aSt[CAN_SCHED_CONF_TOTAL_TX_MSG];




// extern void CAN_CONFIG_CalcCANConfig(void);
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
extern BOOL IsTXCanMessageActive(UINT32 BitPosCANMessage);
extern BOOL IsRXCanMessageActive(UINT32 BitPosCANMessage);
#endif

#endif

/* *****************************************************************************
 End of File
 */
 
 
