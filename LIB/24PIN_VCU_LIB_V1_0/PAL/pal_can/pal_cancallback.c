/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : pal_cancallback.c
|    Project        : VCU SDK
|    Description    : The file contains the callback function for the CAN module.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 13/12/2023       Jeevan Jestin N             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "pal_cancallback.h"

// #include "can_callback.h"
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

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CanMsgReceiveCallback
*   Description   : The function is CAN message receive callback
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
void CanMsgReceiveCallback(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
{
    uint8_t data_buff[8] = {0};
    uint8_t i = 0, k;
    for (i = 0; i < 8; i++)
    {
        data_buff[i] = DataBuff_pu8[i];
    }
    /*Call application specific functions*/
    switch (CanNum_u8)
    {
    case CAN_0:
    {
        for (k = 0; k < Can0_Rx_filter_u8; k++)
        {
            if (CanId_u32 == Can0RxFilterConf_aSt[k].CanId_u32)
            {
                if (NULL != Can0RxFilterConf_aSt[k].CanRxCallback_Fptr)
                {
                    Can0RxFilterConf_aSt[k].CanRxCallback_Fptr(UNIT_0, CAN_CHANNEL_1_E, data_buff, Dlc_u8, CanMode_u8, Ide_u8, CanId_u32);
                }
            }
        }
        break;
    }
    case CAN_1:
    {
        for (k = 0; k < Can1_Rx_filter_u8; k++)
        {
            if (CanId_u32 == Can1RxFilterConf_aSt[k].CanId_u32)
            {
                if (NULL != Can1RxFilterConf_aSt[k].CanRxCallback_Fptr)
                {
                    Can1RxFilterConf_aSt[k].CanRxCallback_Fptr(UNIT_0, CAN_CHANNEL_2_E, data_buff, Dlc_u8, CanMode_u8, Ide_u8, CanId_u32);
                }
            }
        }
        break;
    }
    case CAN_2:
    {
        for (k = 0; k < Can2_Rx_filter_u8; k++)
        {
            if (CanId_u32 == Can2RxFilterConf_aSt[k].CanId_u32)
            {
                if (NULL != Can2RxFilterConf_aSt[k].CanRxCallback_Fptr)
                {
                    Can2RxFilterConf_aSt[k].CanRxCallback_Fptr(UNIT_0, CAN_CHANNEL_3_E, data_buff, Dlc_u8, CanMode_u8, Ide_u8, CanId_u32);
                }
            }
        }
        break;
    }
    default:
        break;
    }

    DRV_HAL2CIL_CallBack(Ide_u8, Dlc_u8, CanNum_u8, CanId_u32, &DataBuff_pu8[0]);

    return;
}

/*---------------------- End of File -----------------------------------------*/