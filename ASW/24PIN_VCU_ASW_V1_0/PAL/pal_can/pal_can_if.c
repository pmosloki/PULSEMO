/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : pal_can_if.c
|    Project        : VCU SDK 
|    Description    : The peripheral abstract layer of the CAN
|    Version        : V1_00
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 08/12/2023       Jeevan Jestin N             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"pal_can_if.h"
#include "uds_DID_conf.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
void UpdateCAN1BaudrateFromDid(void);
void CanMsgRxIntrCallback(uint8_t Unit_u8,uint8_t Channel_u8,uint8_t* DataBuff_pu8,uint8_t Dlc_u8,uint8_t CanMode_u8,uint8_t Ide_u8,uint32_t CanId_u32);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CANinit
*   Description   : The function intialize the VCU CAN
*   Parameters    : NULL
*   Return Value  : NULL
*  ---------------------------------------------------------------------------*/
uint8_t  CANinit(void)
{
    uint8_t State_u8;
    UpdateCAN1BaudrateFromDid();
    CanRxCallback_Fptr = CanMsgRxIntrCallback;
    State_u8 = RSCANinit(CAN_UNIT_0, RSCANchannelConf_aSt,RSCANbaudconfg_aSt);
    return State_u8;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UpdateCAN1BaudrateFromDid
*   Description   : The function intialize the CAN 0 Baudrate Value Using UdsServ22Init 
*   Parameters    : NULL
*   Return Value  : NULL
*  ---------------------------------------------------------------------------*/
void UpdateCAN1BaudrateFromDid(void)
{
    DidConf_st_t Did_St;
    uint8_t Did_Baudrate_Read_u8 = /**(volatile uint8_t *)0xFF2000AA*/0u;
    Did_St.DID_u16 = DID_BAUDRATE_DATA_CODE;
	Did_St.DidLen_u16 = BAUDRATE_DATA_SIZE;
    Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
	(void)DidRead(&Did_St,&Did_Baudrate_Read_u8);
    // Did_Baudrate_Read_u8 = 2u;

    switch(Did_Baudrate_Read_u8)
    {
        case _500KBPS_BUADRATE:
            Did_Baudrate_Read_u8 = _500K_BAUDRATE_E;
            break;
        case _125KBPS_BUADRATE:
            Did_Baudrate_Read_u8 = _125K_BUADRATE_E;
            break;
        case _250KBPS_BUADRATE:
            Did_Baudrate_Read_u8 = _250K_BAUDRATE_E;
            break;
        case _1MBPS_BUADRATE:
            Did_Baudrate_Read_u8 = _1MBPS_BAUDRATE_E;
            break;
        default:
            Did_Baudrate_Read_u8 = _500K_BAUDRATE_E;
            break;
    }
    RSCANbaudconfg_aSt[1].NominalBitrate_En = (RsCanBuadrate_En_t)Did_Baudrate_Read_u8;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CanMsgTransmit
*   Description   : The function intialize the VCU CAN
*   Parameters    : CanNum_u8 : CAN number
*                   DataBuff_pu8 : pointer to the data buffer.
*                   Dlc_u8 : Lemgth of the CAN message.
*                   CanMode_u8 : 0 - Classic CAN
                                 1 - CAN FD
*                   Ide_u8 : 0 - 11bit IDE.
                             1 - 29 bit IDE.
*                   CanId_u32 : message identifier.        
*   Return Value  : NULL
*  ---------------------------------------------------------------------------*/
uint8_t CanMsgTransmit(uint8_t CANNum_u8,uint8_t* DataBuff_pu8,uint8_t Dlc_u8,uint8_t CanMode_u8,uint8_t Ide_u8,uint32_t CanId_u32)
{
    uint8_t State_u8;

    switch(CANNum_u8)
    {
        case CAN_0:
        {
            State_u8 = RSCANtransmit(CAN_UNIT_0,CAN_CHANNEL_1_E,DataBuff_pu8,Dlc_u8,CanMode_u8,Ide_u8,CanId_u32);
            break;
        }
        case CAN_1:
        {
            State_u8 = RSCANtransmit(CAN_UNIT_0,CAN_CHANNEL_2_E,DataBuff_pu8,Dlc_u8,CanMode_u8,Ide_u8,CanId_u32);
            break;
        }
        case CAN_2:
        {
            State_u8 = RSCANtransmit(CAN_UNIT_0,CAN_CHANNEL_3_E,DataBuff_pu8,Dlc_u8,CanMode_u8,Ide_u8,CanId_u32);
            break;
        }
        default:
        {

        }
    }

    return State_u8;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CanMsgTransmit
*   Description   : The function intialize the VCU CAN
*   Parameters    : CanNum_u8 : CAN number
*                   DataBuff_pu8 : pointer to the data buffer.
*                   Dlc_u8 : Lemgth of the CAN message.
*                   CanMode_u8 : 0 - Classic CAN
                                 1 - CAN FD
*                   Ide_u8 : 0 - 11bit IDE.
                             1 - 29 bit IDE.
*                   CanId_u32 : message identifier.        
*   Return Value  : NULL
*  ---------------------------------------------------------------------------*/
void CanMsgRxIntrCallback(uint8_t Unit_u8,uint8_t Channel_u8,uint8_t* DataBuff_pu8,uint8_t Dlc_u8,uint8_t CanMode_u8,uint8_t Ide_u8,uint32_t CanId_u32)
{
    /* Call application specific functions */
    CanMsgReceiveCallback(Channel_u8, DataBuff_pu8, Dlc_u8, CanMode_u8, Ide_u8, CanId_u32);

    return;
}
/*---------------------- End of File -----------------------------------------*/