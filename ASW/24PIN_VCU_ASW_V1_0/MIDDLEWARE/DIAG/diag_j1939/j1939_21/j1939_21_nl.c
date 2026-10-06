/*******************************************************************************
 *    FILENAME    : j1939_interface.c
 *
 *    @brief       : This file contains the routine which receives the transport
 *                    layer CAN frames and analyze the frame type and sends the
 *                    response if required.
 *
 *    $Id         : $
 *******************************************************************************
 * Revision history
 *
 * Ver Author       Date               Description
 * 1   Sloki
 *******************************************************************************
 */

/*
*******************************************************************************
*    Includes
*******************************************************************************
*/
#include "j1939.h"
#include "j1939_conf.h"
#include "can_sched.h"
// #include "hal_can_adapt.h"
#include "j1939_73_dmx.h"
#include "j1939_81_nmac.h"
#include "cil_can_conf.h"
#include "J1939_sched.h"
#include "diag_typedefs.h"
#include "diag_adapt.h"
#include "diag_sys_conf.h"
//#include "candatarx_evcv.h"
/*
*******************************************************************************
*    function definitions
*******************************************************************************
*/

void J1939_interface_init(void);
/*
*******************************************************************************
*    Variables
*******************************************************************************
*/

/*
****************************************************************************************
*    Function Definition
****************************************************************************************
*/

/**
 *  @brief        : Initialization of previous session information to idle state.
 *  @param        : none.
 *  @return       : none
 */
void J1939_interface_init(void)
{
    J1939_appl_St.stateAppl_En = J1939_APPL_IDLE_E;
    J1939_appl_St.respLen_u32 = 0U;
    J1939_appl_St.isDestSpecific_b = FALSE;
    J1939_appl_St.otherAdd_u8 = 0U;
    return;
}

/**
 *  @brief        : This function sends the request identified by the destination
 *                    address and request PGN to be transmitted.
 *  @param        : Destination address, requested Parameter Group Number (PGN)
 *  @return       : TRUE = request sent successfully ; FALSE = request not sent
 */

bool J1939_tx_reqPGN(uint8_t dest_addr_u8, uint32_t req_pgn_u32)
{
    CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t can_id_u32 = 0;
    uint32_t get_cputimer_u32 = 0u;
    bool ret_status = TRUE;
    uint8_t SAIndex_u8 = 0u;
    SAIndex_u8 = GetSourceAddrInst(dest_addr_u8);
    if (0xFFU != SAIndex_u8)
    {
        /* Check for transport layer configuration */
        if (NULL == J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8])
        {
            ret_status = FALSE;
        }
        else
        {
            /* Check if transport layer is idle */
            /* Check if buffer is idle */
            if ((J1939_TL_ST_IDLE_E != J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8]->tl_state_u8) ||
                (J1939_BUFF_IDLE_E != J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8]->bufferstate_u8))
            {
                ret_status = FALSE;
            }
        }

        if (TRUE == ret_status)
        {
            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8]->pgn_u32 = req_pgn_u32;
            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8]->other_ecu_add_u8 = dest_addr_u8;

            can_id_u32 = j1939_REQMSG_BASE_ID;
            can_id_u32 = can_id_u32 + J1939_NMAC_ECU_ClaimedAddr_u8;
            can_id_u32 += ((dest_addr_u8 << 8) & 0xFF00);

            /* Configure ID - Configure source ID according to T6 */
            Can_Applidata_St.ID_u32 = can_id_u32;

            /* Data length for request PGN is 3 bytes */
            Can_Applidata_St.DataLength_u8 = J1939_REQ_PGN_LEN;

            /* PGN Number (LSB) */
            Can_Applidata_St.DataBytes_au8[0] = (uint8_t)(req_pgn_u32 & 0xFF);
            /* PGN Number 2nd BYTE*/
            Can_Applidata_St.DataBytes_au8[1] = (uint8_t)((req_pgn_u32 >> 8) & 0xFF);
            /* PGN Number (MSB) */
            Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((req_pgn_u32 >> 16) & 0xFF);
            /* Fill unused bytes with 0xFF */
            Can_Applidata_St.DataBytes_au8[3] = J1939_UNUSEDBYTES;
            Can_Applidata_St.DataBytes_au8[4] = J1939_UNUSEDBYTES;
            Can_Applidata_St.DataBytes_au8[5] = J1939_UNUSEDBYTES;
            Can_Applidata_St.DataBytes_au8[6] = J1939_UNUSEDBYTES;
            Can_Applidata_St.DataBytes_au8[7] = J1939_UNUSEDBYTES;

#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
            CIL_CAN_Tx_DynamicMsg(CIL_J1939_REQ_TX_E, Can_Applidata_St); // todo harsh
#endif
            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8]->tl_state_u8 = J1939_TL_ST_WAIT_RTS_E;
            /* Get current time */
            get_cputimer_u32 = GET_TIME_MS();
            /* Start T2 timer */
            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAIndex_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[J1939_TX_BUFF_E]->T1_u16;
        }
    }
    return ret_status;
}

/**
 *  @brief        : Receives the transport layer CAN frames and analyze the frame
 *                  type and sends the response if required.
 *                  This is the call back for PGN request 0xEAxx / 0xEAFF
 *  @param        : CIL handle for the received CAN frame.
 *  @param        : CAN frame content.
 *
 *  @return       : none
 */
void J1939_reqPgnClbk(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t *Can_Applidata_St)
{
    uint32_t recd_pgn_u32 = 0U;
    int16_t J1939Appl_DMxRet_s16 = (int16_t)J1939APPL_NO_RESP;
    uint8_t reqDA_u8 = 0U;
    uint8_t reqSA_u8 = 0U;
    uint8_t SAInstance_u8 = 0u;
    bool destSpecificMsg_b = FALSE;
    bool IsActionReqed_b = (bool)TRUE;

    PARAM_NOTUSED(CIL_SigName_En);

    reqDA_u8 = (uint8_t)((Can_Applidata_St->ID_u32 >> 8U) & 0xFFU);
    reqSA_u8 = (uint8_t)((Can_Applidata_St->ID_u32) & 0xFFU);

    SAInstance_u8 = GetSourceAddrInst(reqSA_u8);
    if (0xFF != SAInstance_u8)
    {
        /* Check for transport layer configuration */
        /* Check is it global request */
        /* If it is not global then check is destination address matching with ECU address */
        if (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8] && ((J1939_GLOBAL_REQ == reqDA_u8) || (reqDA_u8 == J1939_NMAC_ECU_ClaimedAddr_u8)))
        {
            if ((J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 == J1939_TL_ST_IDLE_E) &&
                (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferstate_u8 == J1939_BUFF_IDLE_E))
            {

                J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = (uint8_t)(reqSA_u8);
                J1939_appl_St.otherAdd_u8 = (uint8_t)(reqSA_u8);

                if (J1939_GLOBAL_REQ != reqDA_u8)
                {
                    destSpecificMsg_b = (bool)TRUE;
                }

                /*Set is it destination specific*/
                J1939_appl_St.isDestSpecific_b = destSpecificMsg_b;

                /* Extract requested PGN from data field */
                recd_pgn_u32 = Can_Applidata_St->DataBytes_au8[0];
                recd_pgn_u32 += (uint16_t)(((uint16_t)Can_Applidata_St->DataBytes_au8[1] << 8U) & 0xFF00U);
                recd_pgn_u32 += ((Can_Applidata_St->DataBytes_au8[2] & 0xFFFFFFUL) << 16U) & 0xFF0000UL;

                if ((int16_t)J1939_appl_St.stateAppl_En != (int16_t)J1939_APPL_IDLE_E)
                {
                    /* J1939 Appl layer is busy with previous request, try later */
                    J1939Appl_DMxRet_s16 = (int16_t)J1939APPL_ACK_ECUBUSY;
                }
                else
                {
                    uint32_t ReqPGN_Index_u32 = 0;
                    bool dmFound_b = FALSE;
                    /* request answer function */
                    for (ReqPGN_Index_u32 = 0; ReqPGN_Index_u32 < J1939_SPECIFIC_TOTAL_NUM; ReqPGN_Index_u32++)
                    {
                        if ((recd_pgn_u32 & 0xFFFF) == J1939_SPECIFIC_PGN_St[ReqPGN_Index_u32].Req_Type_u16)
                        {
                            dmFound_b = TRUE;
                            if (J1939_SPECIFIC_PGN_St[ReqPGN_Index_u32].Req_Callback_Fptr)
                            {
                                // J1939_SPECIFIC_PGN_St[ReqPGN_Index_u32].Req_Callback_Fptr(*(J1939_SPECIFIC_PGN_St[ReqPGN_Index_u32].Req_FuncData_pu32));
                                J1939Appl_DMxRet_s16 = J1939_SPECIFIC_PGN_St[ReqPGN_Index_u32].Req_Callback_Fptr(Can_Applidata_St, ReqPGN_Index_u32, SAInstance_u8);
                                break;
                            }
                        }
                    }
                    if (ReqPGN_Index_u32 == J1939_SPECIFIC_TOTAL_NUM)
                    {
                        for (ReqPGN_Index_u32 = 0; ReqPGN_Index_u32 < J1939_73_DMx_SERV_TOTAL_NUM; ReqPGN_Index_u32++)
                        {
                            if ((recd_pgn_u32 & 0xFFFFU) == J1939_ServDist_aSt[ReqPGN_Index_u32].PGN_u16)
                            {
                                dmFound_b = TRUE;
                                if (J1939_ServDist_aSt[ReqPGN_Index_u32].Service_Fptr_t)
                                {
                                    J1939Appl_DMxRet_s16 = J1939_ServDist_aSt[ReqPGN_Index_u32].Service_Fptr_t(Can_Applidata_St, SAInstance_u8);
                                    break;
                                }
                            }
                        }
                    }
                    if (!dmFound_b)
                    {
                        /* No acknowledgment (ACK/NACK)is sent on global
                        requests even though requested PGN is not supported */
                        if (destSpecificMsg_b)
                        {
                            /* send the response if the request is destination specific
                            If requested PGN is not supported send NEG ACK if the
                            the request is destination specific.
                            */
                            J1939Appl_DMxRet_s16 = (int16_t)J1939APPL_ACK_NEG;
                        }
                    }
                }

                /*MF is in progress, don't clear the flag*/
                if ((int16_t)J1939_appl_St.stateAppl_En != (int16_t)J1939_APPL_RESP_MF_PROGRESS_E)
                {
                    J1939_appl_St.isDestSpecific_b = FALSE;
                }

                /* Process the return values from DMx services */
                if ((int16_t)J1939APPL_SF_RESP == J1939Appl_DMxRet_s16)
                {
                    uint8_t PduFormat_u8 = (recd_pgn_u32 >> 8);
                    Can_Applidata_St->ID_u32 = (uint32_t)(J1939_BASE_ID | (recd_pgn_u32 << 8) | J1939_NMAC_ECU_ClaimedAddr_u8);

                    /*
                    NOTE: The below case is for PDU-1 format, where the PDU Format (PF) is below 240 (0xF0).
                    Hence according to the J1939-21, the PDU specific (PS) field to be destination address.
                    The destination address is derived from the request.
                    */

                    if (PduFormat_u8 < J1939_PDU1_PF_MAX)
                    {
                        /*PDU1 Formatted frames*/
                        Can_Applidata_St->ID_u32 |= (uint16_t)((uint16_t)reqSA_u8 << 8U);
                    }
                    Can_Applidata_St->DataLength_u8 = J1939_MAXDLC;
                    Can_Applidata_St->MessageType_u8 = EXT_E; // todo harsh
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
                    CIL_CAN_Tx_DynamicMsg(CIL_J1939_ACK_TX_E, *Can_Applidata_St); // todo harsh
#endif
                }
                else
                {
                    switch (J1939Appl_DMxRet_s16)
                    {
                    case J1939APPL_ACK_POS:
                    {
                        Can_Applidata_St->DataBytes_au8[0] = J1939_POSITIVE_ACK_E;
                        break;
                    }
                    case J1939APPL_ACK_NEG:
                    {
                        Can_Applidata_St->DataBytes_au8[0] = J1939_NEGATIVE_ACK_E;
                        break;
                    }
                    case J1939APPL_ACK_ACCESSDENIED:
                    {
                        Can_Applidata_St->DataBytes_au8[0] = J1939_ACCESS_DENIED_E;
                        break;
                    }
                    case J1939APPL_ACK_ECUBUSY:
                    {
                        Can_Applidata_St->DataBytes_au8[0] = J1939_CANNOT_RESPOND_E;
                        break;
                    }
                    default:
                    {
                        /* no action required. return from the function. */
                        IsActionReqed_b = FALSE;
                        break;
                    }
                    }

                    if (IsActionReqed_b == (bool)TRUE)
                    {
                        if (destSpecificMsg_b)
                        {
                            Can_Applidata_St->ID_u32 = (uint32_t)(J1939_ACK_BASE_ID | (uint16_t)((uint16_t)reqSA_u8 << 8U) | J1939_NMAC_ECU_ClaimedAddr_u8);
                        }
                        else
                        {
                            Can_Applidata_St->ID_u32 = (uint32_t)(J1939_GLBACK_BASE_ID | J1939_NMAC_ECU_ClaimedAddr_u8);
                        }

                        Can_Applidata_St->DataLength_u8 = J1939_MAXDLC;
                        Can_Applidata_St->DataBytes_au8[1] = 0U;
                        Can_Applidata_St->DataBytes_au8[2] = J1939_UNUSEDBYTES;
                        Can_Applidata_St->DataBytes_au8[3] = J1939_UNUSEDBYTES;
                        Can_Applidata_St->DataBytes_au8[4] = J1939_UNUSEDBYTES;
                        Can_Applidata_St->DataBytes_au8[5] = (uint8_t)(recd_pgn_u32 & 0xFFU);
                        Can_Applidata_St->DataBytes_au8[6] = (uint8_t)((recd_pgn_u32 >> 8U) & 0xFFU);
                        Can_Applidata_St->DataBytes_au8[7] = (uint8_t)((recd_pgn_u32 >> 16U) & 0xFFU);
                        Can_Applidata_St->MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
                        CIL_CAN_Tx_DynamicMsg(CIL_J1939_ACK_TX_E, *Can_Applidata_St); // Todo: Sandeep K Y
#endif
                    }
                }
            }
        }
    }
    return;
}

/**
 *  @brief        : This is the call back for transport layer connection management frame, PGN: 0xECxx / 0xECFF
 *                  The TPCM message contains the following control byte (1st byte):
 *                  0x10 - RTS
 *                  0x11 - CTS
 *                  0x13 - EOM ACK format
 *                  0x20 - BAM
 *  @param        : CIL handle for the received CAN frame.
 *  @param        : CAN frame content.
 *  @return       : none
 */
void J1939_tpcmClbk(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t *Can_Applidata_St)
{
    uint8_t data_au8[8];
    uint32_t get_cputimer_u32 = 0U;
    uint32_t recd_pgn_u32 = 0U;
    uint8_t reqDA_u8 = 0U;
    uint8_t reqSA_u8 = 0U;
    uint8_t dlc_u8 = Can_Applidata_St->DataLength_u8;
    uint8_t ctrl_byte_u8 = 0U;         /* To store the control byte */
    uint8_t conn_abort_reason_u8 = 0U; /* To store the connection abort reason */
    uint8_t SAInstance_u8 = 0u;
    bool destSpecificMsg_b = FALSE;

    PARAM_NOTUSED(CIL_SigName_En);

    reqDA_u8 = (uint8_t)((Can_Applidata_St->ID_u32 >> 8U) & 0xFFU);
    reqSA_u8 = (uint8_t)((Can_Applidata_St->ID_u32) & 0xFFU);
    SAInstance_u8 = GetSourceAddrInst(reqSA_u8);
    if (0xFFU != SAInstance_u8)
    {
        /* Check for transport layer configuration */
        if ((J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]) && (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]))
        {
            if (J1939_GLOBAL_REQ != reqDA_u8)
            {
                destSpecificMsg_b = (bool)TRUE;
            }

            J1939_tp_dataCopy(Can_Applidata_St->DataBytes_au8, data_au8, (uint16_t)dlc_u8);

            if (destSpecificMsg_b)
            {
                /* Extract control byte */
                ctrl_byte_u8 = data_au8[0];

                /* Extract PGN from data field */
                recd_pgn_u32 = (uint32_t)(data_au8[5]);
                recd_pgn_u32 += (uint16_t)(((uint16_t)data_au8[6] << 8) & 0xFF00U);
                recd_pgn_u32 += ((data_au8[7] & 0xFFFFFFUL) << 16) & 0xFF0000UL;

                switch (ctrl_byte_u8)
                {
                case J1939_TPCM_CTRLBYTE_RTS:
                {
                    if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 == reqSA_u8) &&
                        (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 == recd_pgn_u32))
                    {
                        /* Reinitialize all parameters */
                        J1939_tp_tx_init(SAInstance_u8);

                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = reqSA_u8;
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 = recd_pgn_u32;
                    }

                    /* Extract total message size, number of bytes */
                    /* Extract LSB */
                    J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 = (uint16_t)(data_au8[1]);
                    /* Extract MSB */
                    J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 += (((uint16_t)data_au8[2] << 8) & 0xFF00U);
                    /* Extract total number of packets */
                    J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->total_packets_u8 = data_au8[3];
                    /* Maximum number of packets that can be sent in
                    response to one CTS */
                    J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->max_packets_u8 = data_au8[4];

                    if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 > MAX_BUFFER_SIZE) ||
                        (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_IDLE_E) ||
                        (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferstate_u8 != J1939_BUFF_IDLE_E) ||
                        (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 != recd_pgn_u32))
                    {
                        /* System resources were needed for another task so
                        this connection managed session was terminated */
                        conn_abort_reason_u8 = J1939_NEED_RESOURCES_E;
                        /* Transmit Connection Abort message */
                        J1939_tp_txConnAbort(conn_abort_reason_u8,
                                             recd_pgn_u32, reqSA_u8, J1939_TX_BUFF_E);
                    }
                    else
                    {
                        /* Copy the source address */
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = reqSA_u8;
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferstate_u8 = J1939_BUFF_RTS_CTS_E;
                        /* Send continue to send message */
                        J1939_tp_txCMcts(J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->seqctr_u8, SAInstance_u8);
                    }

                    break;
                }
                case J1939_TPCM_CTRLBYTE_CTS:
                {
                    /* Ignore the message if the connection is not established */
                    if ((J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_IDLE_E) && (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_WAIT_CTS_E))
                    {
                        /* Already in one or more connection managed session
                        and can not support another */
                        conn_abort_reason_u8 = J1939_ALREADY_IN_SESSION_E;
                        /* Transmit Connection Abort message */
                        J1939_tp_txConnAbort(conn_abort_reason_u8, recd_pgn_u32, reqSA_u8, J1939_RX_BUFF_E);
                    }
                    else
                    {
                        /* Get current time */
                        get_cputimer_u32 = GET_TIME_MS(); // todo harsh
                                                          /* Check if T3 timer expires */
                        if (((int32_t)J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                        {
                            /* T3 timeout */
                            conn_abort_reason_u8 = J1939_TIMEOUT_E;
                            /* Transmit Connection Abort message */
                            J1939_tp_txConnAbort(conn_abort_reason_u8,
                                                 recd_pgn_u32, reqSA_u8, J1939_RX_BUFF_E);
                        }
                        else
                        {
                            /* Number of packets that can be sent */
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->num_packets_to_send_u8 = data_au8[1];

                            if (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->num_packets_to_send_u8 > MAX_PACKETS_TO_SEND)
                            {
                                /* System resources were needed for another task so
                                this connection managed session was terminated */
                                conn_abort_reason_u8 = J1939_NEED_RESOURCES_E;
                                /* Transmit Connection Abort message */
                                J1939_tp_txConnAbort(conn_abort_reason_u8,
                                                     recd_pgn_u32, reqSA_u8, J1939_RX_BUFF_E);
                            }
                            else
                            {

                                /* Next packet number to be sent */
                                J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->next_packet_num_u8 = data_au8[2];

                                if (0U == J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->num_packets_to_send_u8)
                                {
                                    /* Get current time */
                                    get_cputimer_u32 = GET_TIME_MS();
                                    /* Start T4 timer */
                                    J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[J1939_RX_BUFF_E]->T4_u16;
                                    /* Change the transport layer state */
                                    J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 = J1939_TL_ST_WAIT_CTS_E;
                                }
                                else
                                {
                                    J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->seqctr_u8 = (uint8_t)(J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->next_packet_num_u8 - 1U);
                                    J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferindex_u16 = (uint16_t)J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->seqctr_u8 * NUM_DATABYTES_MSG;
                                    /* Change the transport state */
                                    J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 = J1939_TL_ST_SEND_DT_E;
                                }
                            }
                        }
                    }
                    break;
                }
                case J1939_TPCM_CTRLBYTE_EOMSG_ACK:
                {
                    if (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_WAIT_EOMSG_ACK_E)
                    {
                        if (reqSA_u8 != J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->other_ecu_add_u8)
                        {
                            /* Already in one or more connection managed session
                            and can not support another */
                            conn_abort_reason_u8 = J1939_ALREADY_IN_SESSION_E;
                            /* Transmit Connection Abort message */
                            J1939_tp_txConnAbort(conn_abort_reason_u8, recd_pgn_u32, reqSA_u8, J1939_RX_BUFF_E);
                        }
                    }
                    else
                    {
                        /* Get current time */
                        get_cputimer_u32 = GET_TIME_MS();
                        /* Check if T3 timer expires */
                        if (((int32_t)J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                        {
                            /* T3 timeout */
                            conn_abort_reason_u8 = J1939_TIMEOUT_E;
                            /* Transmit Connection Abort message */
                            J1939_tp_txConnAbort(conn_abort_reason_u8, recd_pgn_u32, reqSA_u8, J1939_RX_BUFF_E);
                        }
                        else
                        {
                            J1939_tp_rxEndOfMsgAck(SAInstance_u8);
                        }
                    }
                    break;
                }
                case J1939_TPCM_CTRLBYTE_CONN_ABORT:
                {
                    /* Close the established connection */
                    if (reqSA_u8 == J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->other_ecu_add_u8)
                    {
                        /*
                        TBD
                        Issue indication about connection abort to upper layer
                        For future use
                        */

                        /* Reinitialize all parameters */
                        J1939_tp_rx_init(SAInstance_u8);
                    }
                    else if (reqSA_u8 == J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8)
                    {
                        /*
                        TBD
                        Issue indication about connection abort to upper layer
                        For future use
                        */

                        /* Reinitialize all parameters */
                        J1939_tp_tx_init(SAInstance_u8);
                    }
                    else
                    {
                        /* Do Nothing */
                    }

                    break;
                }
                default:
                {
                    break;
                }
                }
            }
            else
            {
                /* GLOBAL request */
                /* Extract control byte */
                ctrl_byte_u8 = data_au8[0];
                if (J1939_TPCM_CTRLBYTE_BAM == ctrl_byte_u8)
                {
                    /* Extract PGN from data field */
                    recd_pgn_u32 = (uint32_t)(data_au8[5]);
                    recd_pgn_u32 += (uint16_t)(((uint16_t)data_au8[6] << 8) & 0xFF00U);
                    recd_pgn_u32 += ((data_au8[7] & 0xFFFFFFUL) << 16) & 0xFF0000UL;

                    if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_IDLE_E) &&
                        (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 == recd_pgn_u32))
                    {
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = (uint8_t)(reqSA_u8);
                        /* Extract total message size, number of bytes */
                        /* Extract LSB */
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 = (uint16_t)(data_au8[1]);
                        /* Extract MSB */
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 += (((uint16_t)data_au8[2] << 8) & 0xFF00U);
                        /* Extract total number of packets */
                        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->total_packets_u8 = data_au8[3];

                        if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 > MAX_BUFFER_SIZE) ||
                            (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferstate_u8 != J1939_BUFF_IDLE_E))
                        {
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 = 0U;
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->total_packets_u8 = 0U;
                        }
                        else
                        {
                            /* Copy the source address */
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = (uint8_t)(reqSA_u8);
                            /* Copy the PGN */
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 = recd_pgn_u32;
                            /* Change the buffer state */
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferstate_u8 = J1939_BUFF_BAM_E;
                            /* Change the transport layer state */
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->tl_state_u8 = J1939_TL_ST_WAIT_DT_E;

                            /* Get current time */
                            get_cputimer_u32 = GET_TIME_MS();
                            /* Start T1 timer */
                            J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[J1939_TX_BUFF_E]->T1_u16;
                        }
                    }
                    else if ((J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 == J1939_TL_ST_IDLE_E) &&
                             (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferstate_u8 == J1939_BUFF_IDLE_E))
                    {
                        J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = (uint8_t)(reqSA_u8);
                        /* Extract total message size, number of bytes */
                        /* Extract LSB */
                        J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferlength_u16 = (uint16_t)(data_au8[1]);
                        /* Extract MSB */
                        J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferlength_u16 += (((uint16_t)data_au8[2] << 8) & 0xFF00U);
                        /* Extract total number of packets */
                        J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->total_packets_u8 = data_au8[3];

                        if ((J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferlength_u16 > MAX_BUFFER_SIZE) ||
                            (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_IDLE_E) ||
                            (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferstate_u8 != J1939_BUFF_IDLE_E))
                        {
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferlength_u16 = 0U;
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->total_packets_u8 = 0U;
                        }
                        else
                        {
                            /*
                            Add code here
                            Check if PGN is supported.
                            If PGN is not supported, ignore the BAM message
                            */

                            /* Copy the source address */
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 = (uint8_t)(reqSA_u8);
                            /* Copy the PGN */
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->pgn_u32 = recd_pgn_u32;
                            /* Change the buffer state */
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->bufferstate_u8 = J1939_BUFF_BAM_E;
                            /* Change the transport layer state */
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->tl_state_u8 = J1939_TL_ST_WAIT_DT_E;

                            /* Get current time */
                            get_cputimer_u32 = GET_TIME_MS();
                            /* Start T1 timer */
                            J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[J1939_RX_BUFF_E]->T1_u16;
                        }
                    }
                    else
                    {
                        /* Do Nothing */
                    }
                }
            }
        }
    }
    return;
}

/**
 *  @brief        : This is the call back for transfer data response (TPDT), PGN: 0xEBxx / 0xEBFF
 *  @param        : CIL handle for the received CAN frame.
 *  @param        : CAN frame content.
 *  @return       : none
 */
void J1939_tpdtClbk(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t *Can_Applidata_St)
{
    uint8_t data_au8[8];
    uint32_t get_cputimer_u32 = 0U;
    uint32_t recd_pgn_u32 = 0U;
    uint8_t reqDA_u8 = 0U;
    uint8_t reqSA_u8 = 0U;
    uint8_t dlc_u8 = Can_Applidata_St->DataLength_u8;
    uint8_t conn_abort_reason_u8 = 0U; /* To store the connection abort reason */
    uint8_t SAInstance_u8 = 0u;
    bool destSpecificMsg_b = FALSE;
    uint8_t BuffIndex_u8 = 0U;

    PARAM_NOTUSED(CIL_SigName_En);

    reqDA_u8 = (uint8_t)((Can_Applidata_St->ID_u32 >> 8U) & 0xFFU);
    reqSA_u8 = (uint8_t)((Can_Applidata_St->ID_u32) & 0xFFU);
    SAInstance_u8 = GetSourceAddrInst(reqSA_u8);
    if (0xFFU != SAInstance_u8)
    {
        /* Check for transport layer configuration */
        if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]) && (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstance_u8]))
        {
            if (J1939_GLOBAL_REQ != reqDA_u8)
            {
                destSpecificMsg_b = (bool)TRUE;
            }

            J1939_tp_dataCopy(Can_Applidata_St->DataBytes_au8, data_au8, (uint16_t)dlc_u8);

            if (destSpecificMsg_b)
            {
                if (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 == (uint8_t)(reqSA_u8))
                {
                    if (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->tl_state_u8 != J1939_TL_ST_WAIT_DT_E)
                    {
                        /* Already in one or more connection managed session
                        and can not support another */
                        conn_abort_reason_u8 = J1939_ALREADY_IN_SESSION_E;
                        /* Transmit Connection Abort message */
                        J1939_tp_txConnAbort(conn_abort_reason_u8,
                                             recd_pgn_u32, reqSA_u8, J1939_TX_BUFF_E);
                    }
                    else
                    {
                        /* Get current time */
                        get_cputimer_u32 = GET_TIME_MS();
                        /* Check if T1 or T2 timer expires */
                        if (((int32_t)J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                        {
                            /* T1 or T2 timeout */
                            conn_abort_reason_u8 = J1939_TIMEOUT_E;
                            /* Transmit Connection Abort message */
                            J1939_tp_txConnAbort(conn_abort_reason_u8, recd_pgn_u32, reqSA_u8, J1939_TX_BUFF_E);
                        }
                        else
                        {
                            J1939_tp_rxdt(data_au8, SAInstance_u8);
                        }
                    }
                }
            }
            else
            {
                for (BuffIndex_u8 = 0u; BuffIndex_u8 < TOTAL_CH; BuffIndex_u8++)
                {
                    if ((J1939_TpMsg_paSt[BuffIndex_u8][SAInstance_u8]->other_ecu_add_u8 == (reqSA_u8)) && (J1939_TL_ST_WAIT_DT_E == J1939_TpMsg_paSt[BuffIndex_u8][SAInstance_u8]->tl_state_u8))
                    {
                        /* Get current time */
                        get_cputimer_u32 = GET_TIME_MS();
                        /* Check if T1 timer expires */
                        if (((int32_t)J1939_TpMsg_paSt[BuffIndex_u8][SAInstance_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                        {
                            if (BuffIndex_u8 == J1939_TX_BUFF_E)
                            {
                                /* Reinitialize all parameters */
                                J1939_tp_tx_init(SAInstance_u8);
                            }
                            else
                            {
                                /* Reinitialize all parameters */
                                J1939_tp_rx_init(SAInstance_u8);
                            }
                        }
                        else
                        {
                            J1939_tp_rxdtbam(BuffIndex_u8, data_au8, SAInstance_u8);
                        }
                    }
                }
            }
        }
    }
    return;
}

/**
 *  @brief        : J1939-73 application diag service function for Diagnostic messages.
 *                  this is timeout callback function.
 *  @param        : none.
 *  @return       : none
 *
 */
void J1939_rqPgnClbk_timeout(void)
{
    int index_u8 = 0U;
    if ((int16_t)J1939_appl_St.stateAppl_En == (int16_t)J1939_APPL_RESP_MF_PROGRESS_E)
    {
        for (index_u8 = 0; index_u8 < J1939_SA_TOTAL_NUM; index_u8++)
        {
            /* multi packet response, response to be routed via TP layer */
            if (J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->tl_state_u8 == J1939_TL_ST_IDLE_E)
            {
                /* copy data from appl layer to TP layer */
                J1939_tp_dataCopy(J1939_appl_St.dataBuff, J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->buffer_pu8, (uint16_t)J1939_appl_St.respLen_u32);

                if (J1939_appl_St.isDestSpecific_b)
                {
                    J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->other_ecu_add_u8 = (uint8_t)(J1939_appl_St.otherAdd_u8);

                    /*Transmit RTS message*/
                    J1939_tp_txCMrts((uint16_t)J1939_appl_St.respLen_u32, (uint32_t)J1939_appl_St.pgn_u32, index_u8);
                }
                else
                {
                    /* Transmit Broadcast Announce message */
                    J1939_tp_txCMbam((uint16_t)J1939_appl_St.respLen_u32, (uint32_t)J1939_appl_St.pgn_u32, index_u8);
                }

                /* The response is handed over to TP layer. The TP will take care of complete transmission.
                Reset the application layer for any new request. */
                J1939_interface_init();
            }
        }
    }

    return;
}
