/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : pal_can_conf.c
|    Project        : VCU_ASW
|    Description    : The file contains the configration data of CAN module
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 08/12/2023       Jeevan Jestin N             Sloki Software Technologies Pvt Ltd
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"pal_can_conf.h"
#include"AppTest.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/
#define TOTAL_RX_FILTER_CAN_0       3
#define TOTAL_RX_FILTER_CAN_1       4
#define TOTAL_RX_FILTER_CAN_2       1

const uint8_t Can0_Rx_filter_u8 = 3;
const uint8_t Can1_Rx_filter_u8 = 4;
const uint8_t Can2_Rx_filter_u8 = 1;


// CanRxFilterConf_St_t Can0RxFilterConf_aSt[];
// CanRxFilterConf_St_t Can1RxFilterConf_aSt[];
// CanRxFilterConf_St_t Can2RxFilterConf_aSt[];
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/
CanRxFilterConf_St_t Can0RxFilterConf_aSt[TOTAL_RX_FILTER_CAN_0] = 
{
    {0x100     ,0x7FF     ,RX_FIFO_BUFF_E,STD_IDE_E,Call_Back_CAN0_Tx_Test},
    {0x7F0     ,0x7FF     ,RX_FIFO_BUFF_E,STD_IDE_E,NULL},
    {0x7DF     ,0x7FF     ,RX_FIFO_BUFF_E,STD_IDE_E,NULL},
};

CanRxFilterConf_St_t Can1RxFilterConf_aSt[TOTAL_RX_FILTER_CAN_1] = 
{
    {0x200,0x7FF,RX_FIFO_BUFF_E,STD_IDE_E,NULL},   
    {0x7F0,0x7F1,RX_FIFO_BUFF_E,STD_IDE_E,NULL},
    {0x7DF,0x7FF,RX_FIFO_BUFF_E,STD_IDE_E,NULL},
    {0x220,0x7FF,RX_FIFO_BUFF_E,STD_IDE_E,Call_Back_CAN1_RX_Test},
};

CanRxFilterConf_St_t Can2RxFilterConf_aSt[TOTAL_RX_FILTER_CAN_2] = 
{
    {0x103,0x7FF,RX_FIFO_BUFF_E,STD_IDE_E,Call_Back_CAN2_Tx_Test},
};

RSCANchannelConf_St_t RSCANchannelConf_aSt[] = 
{
    /* Note:
            CAN configuration channel number should be in sequential order 
    */
    {true,CAN_FD_MODE_E,CAN_0,TOTAL_RX_FILTER_CAN_0,Can0RxFilterConf_aSt},
    {true,CAN_FD_MODE_E,CAN_1,TOTAL_RX_FILTER_CAN_1,Can1RxFilterConf_aSt},
    {true,CAN_FD_MODE_E,CAN_2,TOTAL_RX_FILTER_CAN_2,Can2RxFilterConf_aSt},
};
RSCANBaudconfg_St_t RSCANbaudconfg_aSt[] =
{
    /* Note:
            CAN Baudrate configuration should be in sequential order 
    */
    {_500K_BAUDRATE_E,_2MBPS_BAUDRATE_E},
    {_500K_BAUDRATE_E,_2MBPS_BAUDRATE_E},
    {_500K_BAUDRATE_E,_2MBPS_BAUDRATE_E},
};

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/

/*---------------------- End of File -----------------------------------------*/