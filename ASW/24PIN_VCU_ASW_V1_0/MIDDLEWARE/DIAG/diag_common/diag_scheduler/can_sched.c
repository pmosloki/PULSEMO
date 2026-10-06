/******************************************************************************
 *    FILENAME    : can_sched.c
 *    DESCRIPTION : description of a can scheduler
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
*/ 



/* Section: Included Files                                                   */
#include "can_sched.h"
#include "diag_sys_conf.h"
#include "cil_can_conf.h"
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
	#include "j1939_81_nmac.h" // todo harsh
#endif
//#include "communicator.h"  
#include "task_scheduler.h"
/**************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */



/*
 * @summary:- can structure message buffer  
 */

CanSchedMsg_St_t *CanSB_St_P;
CanSchedulerState_En_t CanSchedulerState_En = TX_ON_RX_ON_E;
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

// *****************************************************************************


//count of receive interrupt callbacks"
static uint16_t CAN_IntCbCnt_u16 = 0;    

// Counter for Time out timer
static uint32_t CAN_MsgRxBoxTimeOutCnt [CIL_CAN_TOTAL_RX_E] = { 0 };

static uint16_t CAN_Transmit_TimerCnt [CAN_SCHED_CONF_TOTAL_TX_MSG] = { 0 };


/**
 *  FUNCTION NAME : CanSched_Init
 *  @brief        : CAN scheduler init function
 *  @param        : none
 *  @return       : none 
 *  @code tag     : @[CODE_CANSCHED_INIT]@{SDD_CAN_SCHEDULER_INIT}
*/ 
void CanSched_Init (void)
{
    uint16_t n = 0;
    for ( n = 0; n < CAN_SCHED_CONF_TOTAL_TX_MSG; n++)
    {
        CAN_Transmit_TimerCnt[n] = CANSCHED_TX_Conf_aSt[n].Offset_Time_u16;		
    }
 	
    return;
}
/**
 *  @function name : void CanSched_5ms (void)
 *  @brief           Function is working as callback function and assign id and all can related information 
 *  @param           None
 *  @return          None
 *
 */



// void CanSched_5ms(void)
// {
//     uint16_t i;
//     CAN_MessageFrame_St_t tempCanMsg;
//     // RX Scheduler

//     for (i = CIL_CAN_START_E; i < CIL_DCAN_TOTAL_RX_E; i++)
//     {
//         CanSB_St_P = (CanSchedMsg_St_t *)CIL_CAN_GetCilDptr(CANSCHED_RX_Conf_aSt[i].CIL_CAN_SigName_En);
//         if (CanSB_St_P->rxFlag)
//         {
//             // Copy the Message
//             tempCanMsg = CanSB_St_P->msg;

//             CanSB_St_P->rxFlag = false;

//             // scheduler callback function
//             if (CANSCHED_RX_Conf_aSt[i].CANSCHED_CB_Fptr)
//             {
//                 CANSCHED_RX_Conf_aSt[i].CANSCHED_CB_Fptr(i, &tempCanMsg);
//             }
//             CAN_MsgRxBoxTimeOutCnt[i] = 0;
//         }
//         else
//         {
            
//             // no more messages received
//             CAN_MsgRxBoxTimeOutCnt[i]++;
//             if (CAN_MsgRxBoxTimeOutCnt[i] > CANSCHED_RX_Conf_aSt[i].Timeoutms_u32)
//             {
//                 if (CANSCHED_RX_Conf_aSt[i].CANSCHED_TCB_Fptr)
//                 {
//                     // timeout callback function
//                     CANSCHED_RX_Conf_aSt[i].CANSCHED_TCB_Fptr();
//                 }
//                 CAN_MsgRxBoxTimeOutCnt[i] = 0;
//             }
//         }
//     }

// #if (TRUE == DIAG_CONF_J1939_SUPPORTED)
//     // Check Is Address Claim done?
//     if (J1939_NMAC_DONE_E == J1939_81_NMAC_State_En)
// #endif

//     {
//         // TXScheduler
//         if (powerMode_u8 == POWER_MODE_ACTIVE_E)
//         {
//             for (i = 0; i < CAN_SCHED_CONF_TOTAL_TX_MSG; i++)
//             {
//                 if ((CAN_Transmit_TimerCnt[i] % CANSCHED_TX_Conf_aSt[i].Cycle_Time_u16) == 0)
//                 {
//                     if (CANSCHED_TX_Conf_aSt[i].CANSCHED_CB_Fptr)
//                     {
//                         // timer callback function
//                         CANSCHED_TX_Conf_aSt[i].CANSCHED_CB_Fptr();
//                     }
//                     CAN_Transmit_TimerCnt[i] = 0;
//                 }
//                 CAN_Transmit_TimerCnt[i]++;
//             }
//         }
//     }
//     return;
// }

/**
 *  @function name : void CanSched_RxCallBackInt(uint16_t signalName, CAN_MessageFrame_St_t canMsgRx) 
 *  @brief           Function is working as callback function and assign id and all can related information 
 *  @param           uint16_t signalName, CAN_MessageFrame_St_t canMsgRx.
 *  @return          None
 *
 */
uint32_t Timer3_u32;
void CanSched_RxCallBackInt(uint16_t signalName, CAN_MessageFrame_St_t canMsgRx) 
{                                     
    CanSchedMsg_St_t *CanMessageBuffer;    
    CanMessageBuffer = (CanSchedMsg_St_t*) CIL_CAN_GetCilDptr(signalName);
    CAN_IntCbCnt_u16++;

    if (CanMessageBuffer)
    {
        CanMessageBuffer->msg = canMsgRx;
        CanMessageBuffer->rxFlag = true;
        CanMessageBuffer->rxCnt++;
    }
    
}

void CanSched_5ms()
{
    CanRxScheduler();
    CanTxScheduler();
    return;
}

void CanRxScheduler()
{
    uint16_t i;
    CAN_MessageFrame_St_t tempCanMsg;

    for (i = CIL_CAN_START_E; i < CIL_CAN_TOTAL_RX_E; i++)
    {
        CanSB_St_P = (CanSchedMsg_St_t *)CIL_CAN_GetCilDptr(CANSCHED_RX_Conf_aSt[i].CIL_CAN_SigName_En);
        if (CanSB_St_P->rxFlag)
        {
            // Copy the Message
            tempCanMsg = CanSB_St_P->msg;

            CanSB_St_P->rxFlag = false;

            // scheduler callback function
            if (CANSCHED_RX_Conf_aSt[i].CANSCHED_CB_Fptr)
            {
                CANSCHED_RX_Conf_aSt[i].CANSCHED_CB_Fptr(i, &tempCanMsg);
            }
            CAN_MsgRxBoxTimeOutCnt[i] = 0;
        }
        else
        {
            // no more messages received
            CAN_MsgRxBoxTimeOutCnt[i]++;
            if (CAN_MsgRxBoxTimeOutCnt[i] > CANSCHED_RX_Conf_aSt[i].Timeoutms_u32)
            {
                if (CANSCHED_RX_Conf_aSt[i].CANSCHED_TCB_Fptr)
                {
                    // timeout callback function
                    CANSCHED_RX_Conf_aSt[i].CANSCHED_TCB_Fptr();
                }
                CAN_MsgRxBoxTimeOutCnt[i] = 0;
            }
        }
    }

    return;
}

void CanTxScheduler()
{
    uint16_t i;
    // CAN_MessageFrame_St_t tempCanMsg;

#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
    // Check Is Address Claim done?
    if (J1939_NMAC_DONE_E == J1939_81_NMAC_State_En)
#endif

    {
        // TXScheduler
        //if (powerMode_u8 == POWER_MODE_ACTIVE_E)
        {
            for (i = 0; i < CAN_SCHED_CONF_TOTAL_TX_MSG; i++)
            {
                if ((CAN_Transmit_TimerCnt[i] % CANSCHED_TX_Conf_aSt[i].Cycle_Time_u16) == 0)
                {
                    if (CANSCHED_TX_Conf_aSt[i].CANSCHED_CB_Fptr)
                    {
                        // timer callback function
                        CANSCHED_TX_Conf_aSt[i].CANSCHED_CB_Fptr();
                    }
                    CAN_Transmit_TimerCnt[i] = 0;
                }
                CAN_Transmit_TimerCnt[i]++;
            }
        }
    }
    return;
}

void SetCanScheduler(CanSchedulerState_En_t CanSchedulerState)
{
    uint16_t n = 0;
    switch (CanSchedulerState)
    {
    case TX_ON_RX_ON_E:
    {

        if (CanSchedulerState_En != TX_ON_RX_ON_E)
        {
            for (n = 0; n < CAN_SCHED_CONF_TOTAL_TX_MSG; n++)
            {
                CAN_Transmit_TimerCnt[n] = CANSCHED_TX_Conf_aSt[n].Offset_Time_u16;
            }

            for (n = 0; n < CIL_CAN_TOTAL_RX_E; n++)
            {
                CAN_MsgRxBoxTimeOutCnt[n] = 0;
            }
            CanSchedulerState_En = TX_ON_RX_ON_E;
        }
        break;
    }

    case TX_ON_RX_OFF_E:
    {
        if (CanSchedulerState_En != TX_ON_RX_OFF_E)
        {
            for (n = 0; n < CAN_SCHED_CONF_TOTAL_TX_MSG; n++)
            {
                CAN_Transmit_TimerCnt[n] = CANSCHED_TX_Conf_aSt[n].Offset_Time_u16;
            }
            CanSchedulerState_En = TX_ON_RX_OFF_E;
        }
        break;
    }

    case TX_OFF_RX_ON_E:
    {
        if (CanSchedulerState_En != TX_OFF_RX_ON_E)
        {
            for (n = 0; n < CIL_CAN_TOTAL_RX_E; n++)
            {
                CAN_MsgRxBoxTimeOutCnt[n] = 0;
            }
            CanSchedulerState_En = TX_OFF_RX_ON_E;
        }
        break;
    }

    case TX_OFF_RX_OFF_E:
    {
        if (CanSchedulerState_En != TX_OFF_RX_OFF_E)
        {
            CanSchedulerState_En = TX_OFF_RX_OFF_E;
        }
        break;
    }
    default:
    {
        CanSchedulerState_En = TX_ON_RX_ON_E;
        break;
    }
    }
}

void CanSchedulerTxSchdlrReinit(CanMsgType_En_t CanMsgType_En)
{
    uint16_t LoopCnt_u16 = 0;
    for (LoopCnt_u16 = 0; LoopCnt_u16 < CAN_SCHED_CONF_TOTAL_TX_MSG; LoopCnt_u16++)
    {
        if (CanMsgType_En == CIL_CAN_GET_MSG_TYPE(CANSCHED_RX_Conf_aSt[LoopCnt_u16].CIL_CAN_SigName_En))
        {
            CAN_Transmit_TimerCnt[LoopCnt_u16] = CANSCHED_TX_Conf_aSt[LoopCnt_u16].Offset_Time_u16;
        }
    }

    return;
}

void CanSchedulerRxSchdlrReinit(CanMsgType_En_t CanMsgType_En)
{
    uint16_t LoopCnt_u16 = 0;
    for (LoopCnt_u16 = 0; LoopCnt_u16 < CAN_SCHED_CONF_TOTAL_RX_MSG; LoopCnt_u16++)
    {
        if (CanMsgType_En == CIL_CAN_GET_MSG_TYPE(CANSCHED_RX_Conf_aSt[LoopCnt_u16].CIL_CAN_SigName_En))
        {
            CAN_MsgRxBoxTimeOutCnt[LoopCnt_u16] = 0;
        }
    }
    return;
}
/* *****************************************************************************
 End of File
 */
