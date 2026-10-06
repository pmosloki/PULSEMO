/******************************************************************************
 *    FILENAME    : can_if.c
 *    DESCRIPTION : description
 ******************************************************************************
 * Revision history
 *
 * Ver Author       Date               Description
 * 1   Jithesh     18/01/2017		   Initial version
 ******************************************************************************
 */

/* Section: Included Files                                                   */
#include "diag_sys_conf.h"
#if (TRUE == DIAG_CONF_CAN_IF)
// #include"my_can.h"
//#include "can.h"
#endif
#include "cil_can_conf.h"
#include "can_callback.h"
#include "can_if.h"
#include "pal_can_if.h"
#include "comm_cntrl_adapt.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */

// assign number of signals array
static uint32_t CIL_CAN_ConfCANIDs_u32[NUM_CAN_SIGNALS];
uint8_t CanTxRxStatus_au8[TOTAL_CAN_MSG_TYPE_E];

bool CAN_Rx_Flag_b = true;
bool CAN_Tx_Flag_b = true;

uint8_t CIL_CAN_ClaimedECUAddr_pu8 = 0u;

/**
 *  @function name : void CIL_CAN_Init(void)
 *  @brief           Function is working as
 *  @param           None
 *  @return          None
 *
 */
void CIL_CAN_Init(void)
{
    // DRV-CIL Interface
    uint8_t i = 0;

    for (i = 0; i < CIL_DCAN_END_E; i++) // todo harsh
    {
        CIL_CAN_ConfCANIDs_u32[i] = CIL_CAN_Conf_aSt[i].HAL_CAN_MsgConf_St.ID_u32;
    }

    /*Register callback functions*/
    DRV_CAN_Reg_CilCallBack(CIL_CAN_HAL2CIL_IntCallBck);
    // CilCanStatusMaskSet(J1939_CAN_MSG_E,CAN_RX_ONLY);
    // CilCanStatusMaskSet(ISO14229_CAN_MSG_E,CAN_RX_TX_DISABLE);
    CilCanStatusMaskSet(J1939_CAN_MSG_E, CAN_RX_TX_ENABLE);
    CilCanStatusMaskSet(ISO14229_CAN_MSG_E, CAN_RX_TX_ENABLE);
    return;
}

/**
 *  @function name : void CIL_CAN_HAL2CIL_IntCallBck(uint8_t IDE_u8,uint8_t DLC_u8,uint32_t CAN_ID_u32,uint8_t *DataBytes_au8).
 *  @brief           Function is working as
 *  @param           uint8_t IDE_u8,uint8_t DLC_u8,uint32_t CAN_ID_u32,uint8_t *DataBytes_au8.
 *  @return          None
 *
 */
uint32_t CanCnt_u32;
void CIL_CAN_HAL2CIL_IntCallBck(uint8_t IDE_u8, uint8_t DLC_u8, uint8_t CAN_Module_u8, uint32_t CAN_ID_u32, uint8_t *DataBytes_au8)
{
    uint16_t CIL_CANSig_En = 0;
    uint8_t i = 0;                          // Loop count
    uint8_t j = 0;                          // Loop count
    CAN_MessageFrame_St_t Can_Applidata_St; // structure that holds CAN application data
    bool ValidData_b = false;

    /* copy the ID from CAN message structure */
    //    ID_u32 = CAN_ID_u32;

    /* this condition is  required to check whether the id is present in the table or not */
    for (i = CIL_CAN_START_E; i < CIL_CAN_TOTAL_RX_E; i++)
    {
        if (CIL_CAN_ConfCANIDs_u32[i] == (CAN_ID_u32 & CIL_CAN_Conf_aSt[i].HAL_CAN_MsgConf_St.ID_MASK_u32))
        {
            break;
        }
    }

    if ((i < CIL_DCAN_END_E) && (CAN_Module_u8 == CIL_CAN_Conf_aSt[i].HAL_CAN_MsgConf_St.Can_Module_En))
    {
        if(0x00u != (CanTxRxStatus_au8[CIL_CAN_Conf_aSt[i].Communication_Type_En]&CAN_RX_STATUS_MASK()))
        {
            if (DLC_CHECK_ENABLE_E == CIL_CAN_Conf_aSt[i].CAN_DlcEnable_En)
            {
                if (DLC_u8 >= CIL_CAN_Conf_aSt[i].HAL_CAN_MsgConf_St.Dlc_Length_u8)
                {
                    ValidData_b = true;
                }
                else
                {
                    
                }
            }
            if ((true == ValidData_b) || (DLC_CHECK_DISABLE_E == CIL_CAN_Conf_aSt[i].CAN_DlcEnable_En))
            {
                Can_Applidata_St.ID_u32 = CAN_ID_u32;
                Can_Applidata_St.DataLength_u8 = DLC_u8;

                /* copy the data from CAN message structure */
                for (j = 0; j < DLC_u8; j++)
                {
                    Can_Applidata_St.DataBytes_au8[j] = DataBytes_au8[j];
                }
                // CIL_CANSig_En = (uint16_t)(i + CIL_CAN_START_E);
                /* if the user has configured the call back function then call that call back function */
                /* or else it will be null no data will be sent */
                if (NULL != CIL_CAN_Conf_aSt[i].CAN_FnPtr)
                {
                    CIL_CANSig_En = (uint16_t)(i + CIL_CAN_START_E);
                    CIL_CAN_Conf_aSt[i].CAN_FnPtr(CIL_CANSig_En, Can_Applidata_St);
                }
            }
        }
    }
    return;
}

/**
 *  @brief  Function CIL_CAN_Tx_Ack_Msg will copy the data from application and keep
 *  @brief  it in a local structure and will send to  HAL_CAN_Txack_TransmitMessage
 *  @brief  function which is present in HAL_CAN_tx.c.This function will check
 *  @brief  for acknowledgemnt after transmitting CAN data frame and return failure if
 *  @brief  it did not receive acknowledgement in time specified and returns success if
 *  @brief  it receives acknowledgement in time specified.
 *
 *  @param   SignalName
 *
 *  @param   Can_Message_St_t Can_Applidata_St - CAN Application Data Structure
 *
 *  @return CAN_SUCCESS_E          - If the transmission is success
 *  @return CAN_FAILED_E           - If the function won't execute any statement
 *
 *
 */
CAN_RESP_En_t CIL_CAN_Tx_AckMsg(uint16_t SignalName_u16, CAN_MessageFrame_St_t Can_Applidata_St)
{
    CAN_RESP_En_t eCanRetCode = CAN_FAILED_E; // To hold the return value of the function
    PARAM_NOTUSED(SignalName_u16);

#if (TRUE == DIAG_CONF_CAN_IF)
    if (0x00u != (CanTxRxStatus_au8[CIL_CAN_Conf_aSt[SignalName_u16].Communication_Type_En] & CAN_TX_STATUS_MASK()))
    {
        //CanMsgTransmit(0, &Can_Applidata_St.DataBytes_au8[0], Can_Applidata_St.DataLength_u8, 0, CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.IDE_u8, CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.ID_u32);
        CanMsgTransmit(CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.Can_Module_En,
            &Can_Applidata_St.DataBytes_au8[0],Can_Applidata_St.DataLength_u8,
            0U,
            CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.IDE_u8,
            CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.ID_u32);
        // can_tx(Can_Applidata_St.ID_u32,&Can_Applidata_St.DataBytes_au8[0]);
    }
    eCanRetCode = CAN_SUCCESS_E;

//    eCanRetCode = HAL_CAN_SendCANMsg((HAL_CAN_BufferId_En_t)TX_BUFFER_ID, &HAL_CAN_TxMessage_St);
#endif
    return eCanRetCode;
}

/**
 *  @brief Function CIL_CAN_Tx_DynamicMsg will copy the data from application and keep
 *  @brief it in a local structure and will send to  HAL_CAN_Tx_TransmitMessage
 *  @brief function which is present in HAL_CAN_tx.c.This function takes ID from
 *  @brief the structure passed as parameter to this function instead of taking from
 *  @brief configuration table.
 *
 *  @param   SignalName
 *
 *  @param   Can_Message_St_t Can_Applidata_St - CAN Application Data Structure
 *
 *  @return CANRC_SUCCESS_E    - If the transmission is success
 *  @return CANRC_FAILED_E     - If the function won't execute any statement
 *  @return CANRC_INVALID_MODULE_E    - If the CAN module is not valid
 *  @return CANRC_NULL_E       - If the Can_Message_pSt points to null pointer
 *
 */

CAN_RESP_En_t CIL_CAN_Tx_DynamicMsg(uint16_t SignalName_u16, CAN_MessageFrame_St_t Can_Applidata_St)
{
    CAN_RESP_En_t eCanRetCode = CAN_FAILED_E; // To hold the return value of the function
    PARAM_NOTUSED(SignalName_u16);

#if (TRUE == DIAG_CONF_CAN_IF)

    if (0x00u != (CanTxRxStatus_au8[CIL_CAN_Conf_aSt[SignalName_u16].Communication_Type_En] & CAN_TX_STATUS_MASK()))
    {
        //CanMsgTransmit(0, &Can_Applidata_St.DataBytes_au8[0], Can_Applidata_St.DataLength_u8, 0, CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.IDE_u8, CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.ID_u32);
        CanMsgTransmit(CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.Can_Module_En,
            &Can_Applidata_St.DataBytes_au8[0],Can_Applidata_St.DataLength_u8,
            0U,
            CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.IDE_u8,
            CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.ID_u32);
        // can_tx(CIL_CAN_Conf_aSt[SignalName_u16].HAL_CAN_MsgConf_St.ID_u32,&Can_Applidata_St.DataBytes_au8[0]);
    }
    eCanRetCode = CAN_SUCCESS_E;
#endif
    return eCanRetCode;
}

/**
 *  @brief  Function to set claimed address.
 *  @param   Address : Address of variable holding a claimed address.
 *  @return  none
 */
void CIL_CAN_SetClaimedAddr(uint8_t Address)
{
    CIL_CAN_ClaimedECUAddr_pu8 = Address;
}

void CilCanStatusMaskSet(CanMsgType_En_t CommType_En, uint8_t SetMask_u8)
{
    if(CommType_En < TOTAL_CAN_MSG_TYPE_E)
    {
        if(CHECK_FOR_CAN_SCHDLR_INIT(CanTxRxStatus_au8[(uint8_t)CommType_En],SetMask_u8,CAN_TX_STATUS_MASK()))
        {
            CanSchedulerTxSchdlrReinit(CommType_En);
        }

        if(CHECK_FOR_CAN_SCHDLR_INIT(CanTxRxStatus_au8[(uint8_t)CommType_En],SetMask_u8,CAN_RX_STATUS_MASK()))
        {
            CanSchedulerRxSchdlrReinit(CommType_En);
        }

        CanTxRxStatus_au8[(uint8_t)CommType_En] = SetMask_u8;   
    }

    return;
}
/* *****************************************************************************
 End of File
 */
