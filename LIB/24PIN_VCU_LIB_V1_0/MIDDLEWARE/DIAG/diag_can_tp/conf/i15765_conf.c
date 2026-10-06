/******************************************************************************
 *    FILENAME    : i15765_conf.c
 *    DESCRIPTION : This file contains configuration structure definition.
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
#include "i15765.h"
#include "diag_sys_conf.h"
#if(TRUE == DIAG_CONF_UDS_SUPPORTED)
	#define CAN_SCHED_CONF_UDS_RX_MSG (CIL_RX_UDS_END - CIL_RX_UDS_START)
#else
	#define CAN_SCHED_CONF_UDS_RX_MSG 0
#endif
/*
 **************************************************************************************************
 *    Defines
 **************************************************************************************************
*/

/*  @code tag     : @[CODE_NAMED_CONSTANTS] @{SDD_NAMED_CONSTANTS} */

#define DCAN_TIME_NAs_NAr       20    /* CAN Message should be transmitted before 25ms (MAX) */
#define DCAN_TIME_NBs           70    /* FC frame should be received before 75 ms (MAX) */
#define DCAN_TIME_NCr           145    /* CF frame should be received before 150 ms (MAX) */
#define DCAN_BSMAX              0    



#define DCAN_CONF_STMIN              5   /* STmin cannot have value from 80 to F0 and FA to FF ; refer ISO section */
#define DCAN_WFTMAX             16

#define DCAN_RX_FUN_ADDR                0x7DF //0x700 //0x18DB33F1
#define DCAN_RX_PHY_TESTER_ADDR         0x7F0//0x7F0//0x7E0
#define DCAN_TX_PHY_TESTER_ADDR         0x7F1//0x7F1//0x150

#define DCAN_RX_PHY_IVN_ADDR         0x6F0
#define DCAN_TX_PHY_IVN_ADDR         0x6F1

#define DCAN_RX_OBD_FUN_ADDR                0x18DB33F1 //0x700 //0x18DB33F1
#define DCAN_RX_PHY_OBD_TESTER_ADDR         0x18DA10F1
#define DCAN_TX_PHY_OBD_TESTER_ADDR         0x18DAF110

// #define DCAN_RX_OBD_FUN_ADDR                0x18DB33F1		//0x18DB33F2 //0x700 //0x18DB33F1
// #define DCAN_RX_PHY_OBD_TESTER_ADDR         0x18DAF110      //0x18DB33F1
// #define DCAN_TX_PHY_OBD_TESTER_ADDR         0x18DA10F1

// #define DCAN_RX_OBD_FUN_ADDR                0x7DF  
// #define DCAN_RX_PHY_OBD_TESTER_ADDR         0x7E0             
// #define DCAN_TX_PHY_OBD_TESTER_ADDR         0x7E8

/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
*/

DCAN_nlTimingCfg_st_t  DCAN_IsoTimings_st[DCAN_TP_INSTANCE_TOTAL_E] =
{
#if(TRUE == DIAG_CONF_UDS_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)

    /* Timing parameters for CAN TP Instance - 0 */
	{
		(DCAN_TIME_NAs_NAr), 		/* Maximum time between the starting of the transmission and */
									/* actual transmission of the message */
		(DCAN_TIME_NBs),     		/* Maximum time between FF/CF(CTS)/FC(WTS) and FC */
		(DCAN_TIME_NCr),     		/* Maximum time between FC(CTS) and CF */
		DCAN_BSMAX,                 /* Maximum number of Consecutive frame to be transmitted as Blocks ( BSMax) */
		DCAN_CONF_STMIN,                 /* Minimum separation time */
		DCAN_WFTMAX                 /* Maximum number of wait to send flow control to be sent or processed */
	},
    // /* Timing parameters for CAN TP Instance - 1 */
	// {
	// 	(DCAN_TIME_NAs_NAr), 		/* Maximum time between the starting of the transmission and */
	// 								/* actual transmission of the message */
	// 	(DCAN_TIME_NBs),     		/* Maximum time between FF/CF(CTS)/FC(WTS) and FC */
	// 	(DCAN_TIME_NCr),     		/* Maximum time between FC(CTS) and CF */
	// 	DCAN_BSMAX,                 /* Maximum number of Consecutive frame to be transmitted as Blocks ( BSMax) */
	// 	DCAN_CONF_STMIN,                 /* Minimum separation time */
	// 	DCAN_WFTMAX                 /* Maximum number of wait to send flow control to be sent or processed */	
	// },
#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
    /* Timing parameters for CAN TP Instance - 2 */
	{
		(DCAN_TIME_NAs_NAr), 		/* Maximum time between the starting of the transmission and */
									/* actual transmission of the message */
		(DCAN_TIME_NBs),     		/* Maximum time between FF/CF(CTS)/FC(WTS) and FC */
		(DCAN_TIME_NCr),     		/* Maximum time between FC(CTS) and CF */
		DCAN_BSMAX,                 /* Maximum number of Consecutive frame to be transmitted as Blocks ( BSMax) */
		DCAN_CONF_STMIN,                 /* Minimum separation time */
		DCAN_WFTMAX                 /* Maximum number of wait to send flow control to be sent or processed */
	}
#endif
};

/* Input vector table for network layer */
DCAN_nlInputVector_st_t  DCAN_nlInputVector_st[DCAN_TP_INSTANCE_TOTAL_E] = 
{
#if(TRUE == DIAG_CONF_UDS_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
    // /* CAN TP Instance - 0 */
	// {
	// 	DCAN_RX_FUN_ADDR,               /* Receive CAN Functional ID */
	// 	DCAN_RX_PHY_IVN_ADDR,           /* Receive CAN Physical ID */
	// 	DCAN_TX_PHY_IVN_ADDR,           /* Transmit CAN ID */
	// 	&DCAN_IsoTimings_st[0],         /* Timing Structure */
	// 	CIL_CANTP_REQ_IVN_RX_E,         /* CAN Rx handle */
	// 	CIL_CANTP_RESP_IVN_TX_E,        /* CAN Tx handle */
	// 	2,                              /* CAN node number */
	// 	TRUE                            /* True = Send with DLC has eight,False = send actual DLC */
	// },
    
    /* CAN TP Instance - 1 */
	{
		DCAN_RX_FUN_ADDR,               /* Receive CAN Functional ID */
		DCAN_RX_PHY_TESTER_ADDR,        /* Receive CAN Physical ID */
		DCAN_TX_PHY_TESTER_ADDR,        /* Transmit CAN ID */
		&DCAN_IsoTimings_st[0],         /* Timing Structure */
		CIL_CANTP_REQ_TESTER_RX_E,      /* CAN Rx handle */
		CIL_CANTP_RESP_TESTER_TX_E,     /* CAN Tx handle */
		2,                              /* CAN node number */
		TRUE                            /* True = Send with DLC has eight,False = send actual DLC */
	},
#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
    /* CAN TP Instance - 2 */
	{
		DCAN_RX_OBD_FUN_ADDR,               /* Receive CAN Functional ID */
		DCAN_RX_PHY_OBD_TESTER_ADDR,        /* Receive CAN Physical ID */
		DCAN_TX_PHY_OBD_TESTER_ADDR,        /* Transmit CAN ID */
		&DCAN_IsoTimings_st[2],         /* Timing Structure */
		CIL_CANTP_REQ_OBD_TESTER_RX_E,      /* CAN Rx handle */
		CIL_CANTP_RESP_OBD_TESTER_TX_E,     /* CAN Tx handle */
		2,                              /* CAN node number */
		TRUE                            /* True = Send with DLC has eight,False = send actual DLC */
	},
#endif

};




DCAN_TPTimingCfg_St_t 	DCAN_TPTiming_St; 


DCAN_TPTimingCfg_St_t  DCAN_TPTimings_aSt[TOTAL_TIMING_PARAMETERS]=
{
	{50U, 2000U}, /*P2 AND P3 DEFAULT VALUES*/
	{50U, 5000U}, /*P2 AND P3 CUSTOMER VALUES*/
};


DCAN_TP_InstanceCANSignConf_St_t  DCAN_TP_InstanceCANSignConf_aSt[DCAN_TP_INSTANCE_TOTAL_E] = 
{
#if(TRUE == DIAG_CONF_UDS_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
    //   {DCAN_TP_INSTANCE_IVN_E,      CIL_CANTP_RESP_IVN_TX_E,        CIL_CANTP_REQ_IVN_RX_E},
      {DCAN_TP_INSTANCE_UDSDIAG_E,  CIL_CANTP_RESP_TESTER_TX_E,     CIL_CANTP_REQ_TESTER_RX_E},
#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED && TRUE == DIAG_CONF_CANTP_SUPPORTED)
      {DCAN_TP_INSTANCE_OBDDIAG_E,  CIL_CANTP_RESP_OBD_TESTER_TX_E,     CIL_CANTP_REQ_OBD_TESTER_RX_E},
#endif
};
/* *****************************************************************************
 End of File
 */

