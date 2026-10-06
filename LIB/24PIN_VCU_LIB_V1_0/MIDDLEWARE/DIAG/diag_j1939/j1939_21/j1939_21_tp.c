/*******************************************************************************
 *    FILENAME    : j1939_21_tp.c
 *
 *    @brief       : Contains J1939 -21 Transport protocol
 *                    routines
 *
 *    $Id         : $
 *
 *******************************************************************************
 * Revision history
 *
 * Ver Author       Date       Description
 * 1   Sloki
 *******************************************************************************
 */

/*
 *******************************************************************************
 *    Includes
 *******************************************************************************
 */
#include "j1939.h"
#include "j1939_81_nmac.h"
// #include "hal_can_adapt.h"
#include "cil_can_conf.h"
#include "J1939_sched.h"
#include "diag_sys_conf.h"
//#include "candatarx_evcv.h"
/*
 ****************************************************************************************
 *    functions prototype
 ****************************************************************************************
 */
void J1939_tp_init(void);
void J1939_tp_txDtManage(void);

/*
 ****************************************************************************************
 *    Static Variables
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 *    Static functions
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 *    Global Variables
 ****************************************************************************************
 */
/*Buffer to hold the tp layer data*/
uint8_t txdatabuf_au8[J1939_SA_TOTAL_NUM][MAX_BUFFER_SIZE] = {0};

/*Buffer to hold the tp layer data*/
uint8_t rxdatabuf_au8[J1939_SA_TOTAL_NUM][MAX_BUFFER_SIZE] = {0};

/*Tp layer message */
J1939_TpMsg_St_t J1939_TpMsg_aSt[TOTAL_CH][J1939_SA_TOTAL_NUM];
/* Index 0 -> Tx & Index 1 -> Rx */
J1939_TpMsg_St_t *J1939_TpMsg_paSt[TOTAL_CH][J1939_SA_TOTAL_NUM] = NULL;

/*Tp layer timing */
J1939_tlTimingCfg_St_t *J1939_tlTiming_pSt[TOTAL_CH] = NULL;

DMRxClbk_Fptr_t DMRxClbk_Fptr = NULL;
/*
 ****************************************************************************************
 *    Function Definition
 ****************************************************************************************
 */
/**
 *  @brief        : Register the data receive callback for upper layer
 *  @param        : DMRxClbk Function pointer to upper layer
 *  @return       : none
 */
void J1939_tp_RegDMRxClbk(DMRxClbk_Fptr_t DMRxClbk)
{
    DMRxClbk_Fptr = DMRxClbk;
}

/**
 *  @brief        : Initializes the data structure used by transport layer for Tx
 *  @param        : none
 *  @return       : none
 */
void J1939_tp_tx_init(uint8_t instNum_u8)
{
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8] = &J1939_TpMsg_aSt[J1939_TX_BUFF_E][instNum_u8];
    J1939_tlTiming_pSt[J1939_TX_BUFF_E] = &J1939_tlTiming_St[J1939_TX_BUFF_E];
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->buffer_pu8 = &txdatabuf_au8[instNum_u8][0];
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->bufferindex_u16 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->bufferlength_u16 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->seqctr_u8 = 0U;

    if (J1939_BUFF_AL_E != J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->bufferstate_u8)
    {
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->bufferstate_u8 = J1939_BUFF_IDLE_E;
    }

    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->tl_state_u8 = J1939_TL_ST_IDLE_E;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->timer_u32 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->num_packets_to_send_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->num_packets_to_receive_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->total_packets_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->other_ecu_add_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->pgn_u32 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->num_packet_recd_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->max_packets_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->next_packet_num_u8 = 0U;
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][instNum_u8]->last_CTS_maxPackets_u8 = 0U;
    return;
}

/**
 *  @brief        : Initializes the data structure used by transport layer for Tx
 *  @param        : none
 *  @return       : none
 */
void J1939_tp_rx_init(uint8_t instNum_u8)
{
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8] = &J1939_TpMsg_aSt[J1939_RX_BUFF_E][instNum_u8];
    J1939_tlTiming_pSt[J1939_RX_BUFF_E] = &J1939_tlTiming_St[J1939_RX_BUFF_E];
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->buffer_pu8 = &rxdatabuf_au8[instNum_u8][0];
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->bufferindex_u16 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->bufferlength_u16 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->seqctr_u8 = 0U;

    if (J1939_BUFF_AL_E != J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->bufferstate_u8)
    {
        J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->bufferstate_u8 = J1939_BUFF_IDLE_E;
    }

    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->tl_state_u8 = J1939_TL_ST_IDLE_E;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->timer_u32 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->num_packets_to_send_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->num_packets_to_receive_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->total_packets_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->other_ecu_add_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->pgn_u32 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->num_packet_recd_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->max_packets_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->next_packet_num_u8 = 0U;
    J1939_TpMsg_paSt[J1939_RX_BUFF_E][instNum_u8]->last_CTS_maxPackets_u8 = 0U;
    return;
}

/**
 *  @brief        : Initializes the data structure used by transport layer
 *  @param        : none
 *  @return       : none
 */
void J1939_tp_init(void)
{
    uint8_t InitIndex_u8 = 0u;
    uint8_t InstIndex_u8 = 0u;
    for (InitIndex_u8 = 0; InitIndex_u8 < TOTAL_CH; InitIndex_u8++)
    {
        J1939_tlTiming_pSt[InitIndex_u8] = &J1939_tlTiming_St[InitIndex_u8];
        for (InstIndex_u8 = 0; InstIndex_u8 < J1939_SA_TOTAL_NUM; InstIndex_u8++)
        {
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8] = &J1939_TpMsg_aSt[InitIndex_u8][InstIndex_u8];
            if (0 == InitIndex_u8)
            {
                J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->buffer_pu8 = &txdatabuf_au8[InstIndex_u8][0];
            }
            else
            {
                J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->buffer_pu8 = &rxdatabuf_au8[InstIndex_u8][0];
            }
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->bufferindex_u16 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->bufferlength_u16 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->seqctr_u8 = 0U;

            if (J1939_BUFF_AL_E != J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->bufferstate_u8)
            {
                J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->bufferstate_u8 = J1939_BUFF_IDLE_E;
            }

            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->tl_state_u8 = J1939_TL_ST_IDLE_E;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->timer_u32 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->num_packets_to_send_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->num_packets_to_receive_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->total_packets_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->other_ecu_add_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->pgn_u32 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->num_packet_recd_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->max_packets_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->next_packet_num_u8 = 0U;
            J1939_TpMsg_paSt[InitIndex_u8][InstIndex_u8]->last_CTS_maxPackets_u8 = 0U;
        }
    }
    return;
}

/**
 *  @brief        : Transfers Broadcast Announce Message to inform all nodes of the network
 *                  that a large message is about to be broadcast.
 *                  It defines PGN and number of bytes to be sent.
 *  @param        : Length of data to be sent
 *  @param        : PGN number
 *  @return       : none
 */
void J1939_tp_txCMbam(uint16_t len_u16, uint32_t pgn_u32, uint8_t index_u8)
{
    CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t data_pgn_u32 = 0U;

    /* Check for transport layer configuration */
    /* Check if the transport layer is idle */
    if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8] != NULL) && (J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->tl_state_u8 == J1939_TL_ST_IDLE_E))
    {
        /* Lock the buffer for BAM */
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->bufferstate_u8 = J1939_BUFF_BAM_E;
        /* Copy the length information */
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->bufferlength_u16 = len_u16;

        data_pgn_u32 = pgn_u32;

        /* Configure ID - Configure source ID according to T6 */
        Can_Applidata_St.ID_u32 = J1939_TPCM_GLB_ID;
        Can_Applidata_St.ID_u32 = Can_Applidata_St.ID_u32 + J1939_NMAC_ECU_ClaimedAddr_u8;

        /* Configure data length */
        Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;

        /* Control byte = 32, Broadcast Announce Message */
        Can_Applidata_St.DataBytes_au8[0] = J1939_TPCM_CTRLBYTE_BAM;
        /* Total Message size, number of bytes (LSB) */
        Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(len_u16 & 0xFFU);
        /* Total Message size, number of bytes (MSB) */
        Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((len_u16 >> 8U) & 0xFFU);
        /* Total Number of packets */
        Can_Applidata_St.DataBytes_au8[3] = (uint8_t)((len_u16 + NUM_PACKETS_DT - 1U) / NUM_PACKETS_DT);
        /* Reserved */
        Can_Applidata_St.DataBytes_au8[4] = J1939_UNUSEDBYTES;
        /* PGN Number (LSB) */
        Can_Applidata_St.DataBytes_au8[5] = (uint8_t)(data_pgn_u32 & 0xFFU);
        /* PGN Number 2nd BYTE*/
        Can_Applidata_St.DataBytes_au8[6] = (uint8_t)((data_pgn_u32 >> 8U) & 0xFFU);
        /* PGN Number (MSB) */
        Can_Applidata_St.DataBytes_au8[7] = (uint8_t)((data_pgn_u32 >> 16U) & 0xFFU);

        Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
        CIL_CAN_Tx_DynamicMsg(CIL_J1939_TPCM_TX_E, Can_Applidata_St); // todo harsh
#endif
        /* Change transport layer state */
        if (J1939_TL_ST_IDLE_E == J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->tl_state_u8)
        {
            J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->tl_state_u8 = J1939_TL_ST_WAIT_TO_SEND_DT_E;
        }
    }
    return;
}

/**
 *  @brief        : Transfers Request_To_Send Message. This API informs a node
 *                  that another node on the network wishes to open a virtual connection with it.
 *  @param        : Length of data to be sent
 *  @param        : PGN number
 *  @return       : none
 */
void J1939_tp_txCMrts(uint16_t len_u16, uint32_t pgn_u32, uint8_t index_u8)
{
   CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t get_cputimer_u32 = 0U;
    uint32_t can_id_u32 = 0U;

    /* Check for transport layer configuration */
    if (J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8] != NULL)
    {
        /* Lock the buffer for RTS/CTS */
        J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->bufferstate_u8 = J1939_BUFF_RTS_CTS_E;
        /* Copy the length information */
        J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->bufferlength_u16 = len_u16;
        /* Copy the requested PGN in case of multi-packet response */
        J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->pgn_u32 = pgn_u32;

        /* CAN ID for TP.CM */
        can_id_u32 = J1939_TPCM_BASE_ID;
        can_id_u32 = can_id_u32 + J1939_NMAC_ECU_ClaimedAddr_u8;
        can_id_u32 += (uint16_t)(((uint16_t)J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->other_ecu_add_u8 << 8U) & 0xFF00U);

        /* Configure ID - Configure source ID according to T6 */
        Can_Applidata_St.ID_u32 = can_id_u32;

        /* Configure data length */
        Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;

        /* Control byte = 16, Request_To_Send Message */
        Can_Applidata_St.DataBytes_au8[0] = J1939_TPCM_CTRLBYTE_RTS;
        /* Total Message size, number of bytes (LSB) */
        Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(len_u16 & 0xFFU);
        /* Total Message size, number of bytes (MSB) */
        Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((len_u16 & 0xFF00U) >> 8U);
        /* Total Number of packets */
        Can_Applidata_St.DataBytes_au8[3] = (uint8_t)((len_u16 + NUM_PACKETS_DT - 1U) / NUM_PACKETS_DT);
        /* Max num of packets that can be sent in response to one CTS */
        Can_Applidata_St.DataBytes_au8[4] = MAX_PACKETS_TO_SEND;
        /* PGN Number (LSB) */
        Can_Applidata_St.DataBytes_au8[5] = (uint8_t)(pgn_u32 & 0xFFU);
        /* PGN Number 2nd BYTE*/
        Can_Applidata_St.DataBytes_au8[6] = (uint8_t)((pgn_u32 >> 8U) & 0xFFU);
        /* PGN Number (MSB) */
        Can_Applidata_St.DataBytes_au8[7] = (uint8_t)((pgn_u32 >> 16U) & 0xFFU);

        Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
        CIL_CAN_Tx_DynamicMsg(CIL_J1939_TPCM_TX_E, Can_Applidata_St);
#endif

        /* Get current time */
        get_cputimer_u32 = GET_TIME_MS();

        /* Start T3 timer */
        J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->timer_u32 = get_cputimer_u32 +
                                                                 J1939_tlTiming_pSt[J1939_RX_BUFF_E]->T3_u16;
        /* Change the transport layer state */
        J1939_TpMsg_paSt[J1939_RX_BUFF_E][index_u8]->tl_state_u8 = J1939_TL_ST_WAIT_CTS_E;
    }

    return;
}

/**
 *  @brief        : API used to respond to the Request to send message. CTS should be transmitted by responder only.
 *  @param        : sequence counter
 *  @return       : none
 */
void J1939_tp_txCMcts(uint8_t seq_ctr_u8, uint8_t InstNum_u8)
{
   CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t get_cputimer_u32 = 0U;
    uint32_t can_id_u32 = 0U;
    uint8_t next_packet_num_u8 = 0U;

    next_packet_num_u8 = (uint8_t)(seq_ctr_u8 + 1U);
    if (J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->max_packets_u8 > MAX_PACKETS_TO_SEND)
    {
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->num_packets_to_receive_u8 = MAX_PACKETS_TO_SEND;
    }
    else
    {
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->num_packets_to_receive_u8 = J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->max_packets_u8;
    }

    /* Configure CAN ID */
    can_id_u32 = J1939_TPCM_BASE_ID;
    /* Copy the destination address */
    can_id_u32 += (uint16_t)(((uint16_t)J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->other_ecu_add_u8 << 8U) & 0xFF00U);
    /* Copy the source address */
    can_id_u32 = can_id_u32 + J1939_NMAC_ECU_ClaimedAddr_u8;

    /* Configure ID - Configure source ID according to T6 */
    Can_Applidata_St.ID_u32 = can_id_u32;

    Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;
    /* Copy the control byte */
    Can_Applidata_St.DataBytes_au8[0] = J1939_TPCM_CTRLBYTE_CTS;
    if (seq_ctr_u8 == (J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->seqctr_u8 + 1U))
    {
        /* Number of packets that can be received */
        Can_Applidata_St.DataBytes_au8[1] = J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->num_packets_to_receive_u8;
        /* Next packet number to be received */
        Can_Applidata_St.DataBytes_au8[2] = next_packet_num_u8;
    }
    else
    {
        /* Number of packets that can be sent */
        Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->num_packets_to_receive_u8 -
                                                      J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->num_packet_recd_u8);
        /* Next packet number to be sent */
        Can_Applidata_St.DataBytes_au8[2] = (uint8_t)(J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->seqctr_u8 + 1U);
    }
    /* Reserved bytes */
    Can_Applidata_St.DataBytes_au8[3] = J1939_UNUSEDBYTES;
    Can_Applidata_St.DataBytes_au8[4] = J1939_UNUSEDBYTES;
    /* Parameter Group Number of the packeted message */
    Can_Applidata_St.DataBytes_au8[5] = (uint8_t)(J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->pgn_u32 & 0xFFU);
    Can_Applidata_St.DataBytes_au8[6] = (uint8_t)((J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->pgn_u32 >> 8U) & 0xFFU);
    Can_Applidata_St.DataBytes_au8[7] = (uint8_t)((J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->pgn_u32 >> 16U) & 0xFFU);

    Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
    CIL_CAN_Tx_DynamicMsg(CIL_J1939_TPCM_TX_E, Can_Applidata_St);
#endif

    /* Change the buffer state */
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->bufferstate_u8 = J1939_BUFF_RTS_CTS_E;
    /* Change the transport layer state */
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->tl_state_u8 = J1939_TL_ST_WAIT_DT_E;
    /* Get current time */
    get_cputimer_u32 = GET_TIME_MS();
    /* Start T2 timer */
    J1939_TpMsg_paSt[J1939_TX_BUFF_E][InstNum_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[J1939_TX_BUFF_E]->T2_u16;

    return;
}

/**
 *  @brief        : API used to communicate the data associated with a PGN having
 *                  more than 8 bytes of data.
 *  @param        : none
 *  @return       : none
 */

void J1939_tp_txTpdt(uint8_t BuffIndex_u8, uint8_t SA_index_u8)
{
    CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t get_cputimer_u32 = 0U;
    uint8_t numbytes_u8 = 0U;
    uint8_t idx_u8 = 0U;

    /* Check for transport layer configuration */
    if ((J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8] != NULL) && (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferstate_u8 != J1939_BUFF_IDLE_E))
    {
        /* ensure that the time between the header message and the data-package is between 50ms and 200ms */
        if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 == J1939_TL_ST_WAIT_TO_SEND_DT_E)
        {
            J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 = J1939_TL_ST_SEND_DT_E;
        }
        /* ensure that the time between the last data-package and the next header is between 50 and 200ms */
        else if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 == J1939_TL_ST_WAIT_TO_NEXT_DT_E)
        {
            if (J1939_TX_BUFF_E == BuffIndex_u8)
            {
                /* Reinitialize all parameters */
                J1939_tp_tx_init(SA_index_u8);
            }
            else
            {
                /* Reinitialize all parameters */
                J1939_tp_rx_init(SA_index_u8);
            }
        }
        else if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 == J1939_TL_ST_SEND_DT_E)
        {
            if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferstate_u8 == J1939_BUFF_BAM_E)
            {
                /* Configure CAN ID */
                Can_Applidata_St.ID_u32 = (uint32_t)(j1939_tpdt_GLB_ID + J1939_NMAC_ECU_ClaimedAddr_u8);
            }
            if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferstate_u8 == J1939_BUFF_RTS_CTS_E)
            {
                /* Configure CAN ID */
                Can_Applidata_St.ID_u32 = (uint32_t)(j1939_tpdt_BASE_ID + J1939_NMAC_ECU_ClaimedAddr_u8);
                Can_Applidata_St.ID_u32 += (uint16_t)(((uint16_t)J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->other_ecu_add_u8 << 8U) & 0xFF00U);
            }

            numbytes_u8 = NUM_DATABYTES_MSG;

            /* clearing the buffer contents */
            /* Buffer is initialized with 0xFF for padding */
            for (idx_u8 = J1939_MAXDLC; idx_u8 > 0U; idx_u8--)
            {
                Can_Applidata_St.DataBytes_au8[idx_u8 - 1U] = J1939_UNUSEDBYTES;
            }

            /* Configure data length */
            Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;

            /* Increment the sequence counter */
            J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->seqctr_u8++;

            /* Fill first byte with sequence counter */
            Can_Applidata_St.DataBytes_au8[0] = (uint8_t)J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->seqctr_u8;

            /* Check if the last packet is to be sent */
            if ((J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferindex_u16 + NUM_DATABYTES_MSG) > J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferlength_u16)
            {
                numbytes_u8 = (uint8_t)(J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferlength_u16 - J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferindex_u16);
            }
            if (J1939_TX_BUFF_E == BuffIndex_u8)
            {
                /* Copy data from global buffer to local buffer */
                J1939_tp_dataCopy((const uint8_t *)&txdatabuf_au8[SA_index_u8][J1939_TpMsg_paSt[J1939_TX_BUFF_E][SA_index_u8]->bufferindex_u16],
                                  &Can_Applidata_St.DataBytes_au8[1], (uint16_t)numbytes_u8);
                J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->buffer_pu8 = &txdatabuf_au8[SA_index_u8][0];
            }
            else
            {
                /* Copy data from global buffer to local buffer */
                J1939_tp_dataCopy((const uint8_t *)&rxdatabuf_au8[SA_index_u8][J1939_TpMsg_paSt[J1939_RX_BUFF_E][SA_index_u8]->bufferindex_u16],
                                  &Can_Applidata_St.DataBytes_au8[1], (uint16_t)numbytes_u8);
                J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->buffer_pu8 = &rxdatabuf_au8[SA_index_u8][0];
            }
            /* increment the index */
            J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferindex_u16 += numbytes_u8;

            /* Transmit the data over CAN */
            Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
            CIL_CAN_Tx_DynamicMsg(CIL_j1939_tpdt_TX_E, Can_Applidata_St);
#endif

            if (J1939_BUFF_RTS_CTS_E == J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferstate_u8)
            {
                if ((((J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->seqctr_u8 - J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->last_CTS_maxPackets_u8) % J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->num_packets_to_send_u8) == 0))
                {
                    /*Assign previous CTS total packets counter*/
                    J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->last_CTS_maxPackets_u8 = J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->seqctr_u8;

                    /* Change the transport layer state */
                    J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 = J1939_TL_ST_WAIT_CTS_E;
                    /* Get current time */
                    get_cputimer_u32 = GET_TIME_MS();
                    /* Start T3 timer */
                    J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[BuffIndex_u8]->T3_u16;
                }
            }
            /* reset the variables if all the packets are sent */
            if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferindex_u16 >= J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferlength_u16)
            {
                if (J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->bufferstate_u8 == J1939_BUFF_RTS_CTS_E)
                {
                    /* Change transport layer state */
                    J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 = J1939_TL_ST_WAIT_EOMSG_ACK_E;

                    /* Get current time */
                    get_cputimer_u32 = GET_TIME_MS();
                    /* Start T3 timer */
                    J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->timer_u32 = get_cputimer_u32 +
                                                                             J1939_tlTiming_pSt[BuffIndex_u8]->T3_u16;
                }
                else
                {
                    /* ensure that the time between the last data-package and the next header is between 50 and 200ms */
                    J1939_TpMsg_paSt[BuffIndex_u8][SA_index_u8]->tl_state_u8 = J1939_TL_ST_WAIT_TO_NEXT_DT_E;
                }
            }
        }
        else
        {
            /*Do nothing*/
        }
    }
    return;
}

/**
 *  @brief        : API pass the EOM to the originator indicating that the entire
 *                  message was received and reassembled correctly. EOM should be
 *                  transmitted by responder.
 *  @param        : none
 *  @return       : none
 */
void J1939_tp_txEndOfMsgAck(uint8_t SAInstance_u8)
{
   CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t can_id_u32 = 0U;

    /* Configure CAN ID */
    can_id_u32 = J1939_TPCM_BASE_ID;
    /* Copy the destination address */
    can_id_u32 += (uint16_t)(((uint16_t)J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8 << 8U) & 0xFF00U);
    /* Copy the source address */
    can_id_u32 = can_id_u32 + J1939_NMAC_ECU_ClaimedAddr_u8;

    /* Configure ID - Configure source ID according to T6 */
    Can_Applidata_St.ID_u32 = can_id_u32;

    Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;
    /* Copy control byte */
    Can_Applidata_St.DataBytes_au8[0] = J1939_TPCM_CTRLBYTE_EOMSG_ACK;
    /* Total message size, number of bytes (LSB) */
    Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferindex_u16 & 0xFFU);
    /* Total message size, number of bytes (MSB) */
    Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferindex_u16 >> 8U) & 0xFFU);
    /* Total number of packets */
    Can_Applidata_St.DataBytes_au8[3] = (uint8_t)J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->total_packets_u8;
    /* Reserved */
    Can_Applidata_St.DataBytes_au8[4] = J1939_UNUSEDBYTES;
    /* Parameter Group Number of the packeted message */
    Can_Applidata_St.DataBytes_au8[5] = (uint8_t)(J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 & 0xFFU);
    Can_Applidata_St.DataBytes_au8[6] = (uint8_t)((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 >> 8U) & 0xFFU);
    Can_Applidata_St.DataBytes_au8[7] = (uint8_t)((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32 >> 16U) & 0xFFU);

    Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
    CIL_CAN_Tx_DynamicMsg(CIL_J1939_TPCM_TX_E, Can_Applidata_St);
#endif

    /*Check upper layer callback registered*/
    if (NULL != DMRxClbk_Fptr)
    {
        /*Fetch the required data*/
        DM_DataPackets_St_t DM_DataPackets_St;
        DM_DataPackets_St.len_u16 = J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16;
        DM_DataPackets_St.PGN_u32 = J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->pgn_u32;
        DM_DataPackets_St.SA_u8 = J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->other_ecu_add_u8;
        J1939_tp_dataCopy(J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->buffer_pu8, &DM_DataPackets_St.DMxData[0], J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16);
        /*Inform to upper layer*/
        DMRxClbk_Fptr(&DM_DataPackets_St);
    }

    /* Reinitialize all parameters */
    J1939_tp_tx_init(SAInstance_u8);

    return;
}

/**
 *  @brief        : Nodes involved in a virtual connection can use this API to
 *                  close the connection without completing the transfer of message
 *                  or to prevent a connection from being initialized.
 *                  Timeout between different type of messages resulted in this API.
 *  @param        : Connection Abort Reason
 *  @param        : PGN number
 *  @param        : Destination address of node for which abort to be sent.
 *  @return       : none
 */
void J1939_tp_txConnAbort(uint8_t conn_abort_reason_u8,
                          uint32_t pgn_u32,
                          uint8_t address_u8, J1939_BuffIndex_En_t Buff_En)
{
   CAN_MessageFrame_St_t Can_Applidata_St;
    uint32_t data_pgn_u32 = 0U;
    uint32_t can_id_u32 = 0U;
    uint8_t DAIndex_u8 = 0u;
    DAIndex_u8 = GetSourceAddrInst(address_u8);
    if (0xFFU != DAIndex_u8)
    {
        /* Check for transport layer configuration */
        if (J1939_TpMsg_paSt[Buff_En][DAIndex_u8] != NULL)
        {
            data_pgn_u32 = pgn_u32;
            can_id_u32 = J1939_TPCM_BASE_ID;
            can_id_u32 = can_id_u32 + J1939_NMAC_ECU_ClaimedAddr_u8;
            can_id_u32 += (uint16_t)(((uint16_t)address_u8 << 8U) & 0xFF00U);

            /* Configure ID - Configure source ID according to T6 */
            Can_Applidata_St.ID_u32 = can_id_u32;

            /* Configure data length */
            Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;

            /* Control byte = 255, Request_To_Send Message */
            Can_Applidata_St.DataBytes_au8[0] = J1939_TPCM_CTRLBYTE_CONN_ABORT;
            /* Copy Connection Abort Reason in the 2nd byte of data field */
            Can_Applidata_St.DataBytes_au8[1] = conn_abort_reason_u8;
            /* Reserved bytes */
            Can_Applidata_St.DataBytes_au8[2] = J1939_UNUSEDBYTES;
            Can_Applidata_St.DataBytes_au8[3] = J1939_UNUSEDBYTES;
            Can_Applidata_St.DataBytes_au8[4] = J1939_UNUSEDBYTES;
            /* PGN Number (LSB) */
            Can_Applidata_St.DataBytes_au8[5] = (uint8_t)(data_pgn_u32 & 0xFFU);
            /* PGN Number 2nd BYTE*/
            Can_Applidata_St.DataBytes_au8[6] = (uint8_t)((data_pgn_u32 >> 8U) & 0xFFU);
            /* PGN Number (MSB) */
            Can_Applidata_St.DataBytes_au8[7] = (uint8_t)((data_pgn_u32 >> 16U) & 0xFFU);

            Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
            CIL_CAN_Tx_DynamicMsg(CIL_J1939_TPCM_TX_E, Can_Applidata_St);
#endif

            /* Close the established connection */
            if ((address_u8 == J1939_TpMsg_paSt[Buff_En][DAIndex_u8]->other_ecu_add_u8) && (data_pgn_u32 == J1939_TpMsg_paSt[Buff_En][DAIndex_u8]->pgn_u32))
            {
                /*
                    TBD
                    Issue indication about the connection abort to upper layer
                    For future use
                */
                if (J1939_TX_BUFF_E == Buff_En)
                {
                    /* Reinitialize all parameters */
                    J1939_tp_tx_init(DAIndex_u8);
                }
                else
                {
                    /* Reinitialize all parameters */
                    J1939_tp_rx_init(DAIndex_u8);
                }
            }
        }
    }
    return;
}

/**
 *  @brief        : API get called @J1939_tpdtClbk callback function once
 *                  all demanded data packets received.
 *  @param        : Received data buffer
 *  @return       : none
 */
void J1939_tp_rxdt(const uint8_t buffer_au8[], uint8_t SAInstance_u8)
{
    uint32_t get_cputimer_u32 = 0U;
    uint16_t rem_bytes_u16 = 0U;
    uint16_t num_bytes_recd_u16 = 0U;
    uint8_t seq_ctr_u8 = 0U;

    seq_ctr_u8 = buffer_au8[0];

    /* Check if it is a right sequence counter */
    if (seq_ctr_u8 == (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->seqctr_u8 + 1U))
    {
        rem_bytes_u16 = (uint16_t)(J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferlength_u16 - J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferindex_u16);
        /* Check if the packet received is the last one */
        if (rem_bytes_u16 > NUM_PACKETS_DT)
        {
            num_bytes_recd_u16 = NUM_PACKETS_DT;
        }
        else
        {
            num_bytes_recd_u16 = rem_bytes_u16;
        }

        /* Copy data from local buffer to global buffer */
        J1939_tp_dataCopy(&buffer_au8[1],
                          &txdatabuf_au8[SAInstance_u8][J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferindex_u16],
                          num_bytes_recd_u16);
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->buffer_pu8 = &txdatabuf_au8[SAInstance_u8][0];
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->num_packet_recd_u8++;
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->seqctr_u8++;
        J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->bufferindex_u16 += num_bytes_recd_u16;

        /* Check if the packet received is the last one */
        if (rem_bytes_u16 <= NUM_PACKETS_DT)
        {
            J1939_tp_txEndOfMsgAck(SAInstance_u8);
        }
        else
        {
            if ((J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->num_packets_to_receive_u8 - J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->num_packet_recd_u8) == 0)
            {
                J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->num_packet_recd_u8 = 0U;
                J1939_tp_txCMcts(J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->seqctr_u8, SAInstance_u8);
            }
            else
            {
                /* Get current time */
                get_cputimer_u32 = GET_TIME_MS();
                /* Start T1 timer */
                J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstance_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[J1939_TX_BUFF_E]->T1_u16;
            }
        }
    }
    else
    {
        /* Wrong sequence counter */
        J1939_tp_txCMcts(seq_ctr_u8, SAInstance_u8);
    }

    return;
}

/**
 *  @brief        : API get called @J1939_tpdtClbk callback function after
 *                  receiving broadcast announce message.
 *  @param        : Received data buffer
 *  @return       : none
 */
void J1939_tp_rxdtbam(uint8_t BuffIndex_u8, const uint8_t buffer_au8[], uint8_t ReqSAInstance_u8)
{
    uint32_t get_cputimer_u32 = 0U;
    uint16_t rem_bytes_u16 = 0U;
    uint16_t num_bytes_recd_u16 = 0U;
    uint8_t seq_ctr_u8 = 0U;

    seq_ctr_u8 = buffer_au8[0];

    /* Check if it is a wrong sequence counter */
    if (seq_ctr_u8 == (J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->seqctr_u8 + 1U))
    {
        rem_bytes_u16 = (uint16_t)(J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferlength_u16 - J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferindex_u16);
        /* Check if the packet received is the last one */
        if (rem_bytes_u16 > NUM_PACKETS_DT)
        {
            num_bytes_recd_u16 = NUM_PACKETS_DT;
        }
        else
        {
            num_bytes_recd_u16 = rem_bytes_u16;
        }
        if (BuffIndex_u8 == J1939_TX_BUFF_E)
        {
            /* Copy data from local buffer to global buffer */
            J1939_tp_dataCopy(&buffer_au8[1],
                              &txdatabuf_au8[ReqSAInstance_u8][J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferindex_u16],
                              num_bytes_recd_u16);
            J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->buffer_pu8 = &txdatabuf_au8[ReqSAInstance_u8][0];
        }
        else
        {
            /* Copy data from local buffer to global buffer */
            J1939_tp_dataCopy(&buffer_au8[1],
                              &rxdatabuf_au8[ReqSAInstance_u8][J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferindex_u16],
                              num_bytes_recd_u16);
            J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->buffer_pu8 = &rxdatabuf_au8[ReqSAInstance_u8][0];
        }
        J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->seqctr_u8++;
        J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferindex_u16 += num_bytes_recd_u16;

        /* Check if the packet received is the last one */
        if (rem_bytes_u16 <= NUM_PACKETS_DT)
        {
            if (J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferindex_u16 == J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferlength_u16)
            {
                /* Data has been assembled successfully */
                /* Deliver data to upper layer */
                /* For future use */
                /*Check upper layer callback registered*/
                if (NULL != DMRxClbk_Fptr)
                {
                    /*Fetch the required data*/
                    DM_DataPackets_St_t DM_DataPackets_St;
                    DM_DataPackets_St.len_u16 = J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferlength_u16;
                    DM_DataPackets_St.PGN_u32 = J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->pgn_u32;
                    DM_DataPackets_St.SA_u8 = J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->other_ecu_add_u8;
                    J1939_tp_dataCopy(J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->buffer_pu8, &DM_DataPackets_St.DMxData[0], J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->bufferlength_u16);
                    /*Inform to upper layer*/
                    DMRxClbk_Fptr(&DM_DataPackets_St);
                }
            }
            if (BuffIndex_u8 == J1939_TX_BUFF_E)
            {
                /* Reinitialize all parameters */
                J1939_tp_tx_init(ReqSAInstance_u8);
            }
            else
            {
                /* Reinitialize all parameters */
                J1939_tp_rx_init(ReqSAInstance_u8);
            }
        }
        else
        {
            /* Get current time */
            get_cputimer_u32 = GET_TIME_MS();
            /* Start T1 timer */
            J1939_TpMsg_paSt[BuffIndex_u8][ReqSAInstance_u8]->timer_u32 = get_cputimer_u32 + J1939_tlTiming_pSt[BuffIndex_u8]->T1_u16;
        }
    }
    else
    {
        if (BuffIndex_u8 == J1939_TX_BUFF_E)
        {
            /* Reinitialize all parameters */
            J1939_tp_tx_init(ReqSAInstance_u8);
        }
        else
        {
            /* Reinitialize all parameters */
            J1939_tp_rx_init(ReqSAInstance_u8);
        }
    }

    return;
}

/**
 *  @brief        : Receives transport protocol connection management
 *                    End_of_Message_Acknowledgment message
 *  @param        : Pointer to data buffer, PGN in the data field
 *  @return       : none
 */
void J1939_tp_rxEndOfMsgAck(uint8_t SAIndex_u8)
{
    /*
        TBD
        Indication to the upper layer about successful data delivery
        For future use
    */

    /* Reinitialize all parameters */
    J1939_tp_rx_init(SAIndex_u8);
    return;
}

/**
 *  @brief        : J1939 transport protocol for multi-package messages
 *  @param        : none
 *  @return       : none
 *
 */
void J1939_tp_txDtManage(void)
{
    uint32_t get_cputimer_u32 = 0U;
    uint8_t conn_abort_reason_u8 = 0U;
    uint8_t BuffIndex_u8 = 0U;
    uint8_t index_u8 = 0U;
    /*Transmit the DT messages*/
    for (BuffIndex_u8 = 0u; BuffIndex_u8 < TOTAL_CH; BuffIndex_u8++)
    {
        for (index_u8 = 0u; index_u8 < J1939_SA_TOTAL_NUM; index_u8++)
        {
            if (0xFF != index_u8)
            {
                J1939_tp_txTpdt(BuffIndex_u8, index_u8);
                if (J1939_TL_ST_WAIT_RTS_E == J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->tl_state_u8)
                {
                    /* Get current time */
                    get_cputimer_u32 = GET_TIME_MS();
                    /* Check if T3 timer expires*/
                    if (((int32_t)J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                    {
                        /* T1 timeout */
                        conn_abort_reason_u8 = J1939_TIMEOUT_E;
                        /* Transmit Connection Abort message for sending the multi packet
                        response */
                        J1939_tp_txConnAbort(conn_abort_reason_u8,
                                             J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->pgn_u32,
                                             J1939_TpMsg_paSt[J1939_TX_BUFF_E][index_u8]->other_ecu_add_u8, J1939_TX_BUFF_E);
                    }
                }

                if (J1939_TL_ST_WAIT_CTS_E == J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->tl_state_u8)
                {
                    /* Get current time */
                    get_cputimer_u32 = GET_TIME_MS();
                    /* Check if T3 or T4 timer expires*/
                    if (((int32_t)J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                    {
                        /* T3 or T4 timeout */
                        conn_abort_reason_u8 = J1939_TIMEOUT_E;
                        /* Transmit Connection Abort message for sending the multi-packet
                        response */
                        J1939_tp_txConnAbort(conn_abort_reason_u8,
                                             J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->pgn_u32,
                                             J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->other_ecu_add_u8, BuffIndex_u8);
                    }
                }

                if (J1939_TL_ST_WAIT_DT_E == J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->tl_state_u8)
                {
                    /* Get current time */
                    get_cputimer_u32 = GET_TIME_MS();
                    /* Check if T1 or T2 timer expires*/
                    if (((int32_t)J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                    {
                        if (J1939_BUFF_BAM_E == J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->bufferstate_u8)
                        {
                            if (J1939_TX_BUFF_E == BuffIndex_u8)
                            {
                                /* Reinitialize all parameters */
                                J1939_tp_tx_init(index_u8);
                            }
                            else
                            {
                                /* Reinitialize all parameters */
                                J1939_tp_rx_init(index_u8);
                            }
                        }
                        else
                        {
                            /* T1 or T2 timeout */
                            conn_abort_reason_u8 = J1939_TIMEOUT_E;
                            /* Transmit Connection Abort message for sending the multi packet
                            response */
                            J1939_tp_txConnAbort(conn_abort_reason_u8,
                                                 J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->pgn_u32,
                                                 J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->other_ecu_add_u8, BuffIndex_u8);
                        }
                    }
                }
                if (J1939_TL_ST_WAIT_EOMSG_ACK_E == J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->tl_state_u8)
                {
                    /* Get current time */
                    get_cputimer_u32 = GET_TIME_MS();
                    /* Check if T3 timer expires*/
                    if (((int32_t)J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->timer_u32 - (int32_t)get_cputimer_u32) <= 0)
                    {
                        /* T3 timeout */
                        conn_abort_reason_u8 = J1939_TIMEOUT_E;
                        /* Transmit Connection Abort message for sending the multi packet
                        response */
                        J1939_tp_txConnAbort(conn_abort_reason_u8,
                                             J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->pgn_u32,
                                             J1939_TpMsg_paSt[BuffIndex_u8][index_u8]->other_ecu_add_u8, BuffIndex_u8);
                    }
                }
            }
        }
    }
}

/**
 *  @brief        : copies data from source to destination of given length
 *  @param        : pointer to source buffer; pointer to dest buffer;length
 *  @return       : SUCCESS = 1
 *                  FAIL = 0
 */
uint8_t J1939_tp_dataCopy(const uint8_t srcaddr_pu8[], uint8_t destaddr_pu8[], uint16_t len_u16)
{
    uint16_t idx_u16 = 0U;
    uint8_t retState_u8 = 0U;

    if ((NULL == srcaddr_pu8) || (NULL == destaddr_pu8) || (len_u16 <= 0U))
    {
        /*Do nothing*/
        retState_u8 = 0U;
    }
    else
    {
        /* Copy data from src buffer to destination buffer */
        for (idx_u16 = 0U; idx_u16 < len_u16; idx_u16++)
        {
            destaddr_pu8[idx_u16] = srcaddr_pu8[idx_u16];
        }

        retState_u8 = 1U;
    }

    return retState_u8;
}
/**
 *  @brief        : This function handle the data when Single frame data is received with PGN FECA.
 *  @param        : none.
 *  @return       : none
 *
 */
void J1939_PgnSfClbk(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t *Can_Applidata_St)
{
    uint32_t recd_pgn_u32 = 0U;
    uint8_t reqSA_u8 = 0U;
    uint8_t dlc_u8 = Can_Applidata_St->DataLength_u8;
    uint8_t SAInstance_u8 = 0u;
    reqSA_u8 = (uint8_t)((Can_Applidata_St->ID_u32) & 0xFFU);
    SAInstance_u8 = GetSourceAddrInst(reqSA_u8);

    if (0xFFU != SAInstance_u8)
    {
        uint8_t pf_u8 = (uint8_t)((Can_Applidata_St->ID_u32 >> 16) & 0xFFU);
        uint8_t ps_u8 = (uint8_t)((Can_Applidata_St->ID_u32 >> 8) & 0xFFU);
        uint8_t dp_u8 = (uint8_t)((Can_Applidata_St->ID_u32 >> 24) & 0x01U);
        if(pf_u8 < 240U)
        {
            recd_pgn_u32 = ((uint32_t)dp_u8 << 16U) | ((uint32_t)pf_u8 << 8);
        }
        else
        {
            recd_pgn_u32 = ((uint32_t)dp_u8 << 16U) | ((uint32_t)pf_u8 << 8) | ps_u8;
        }
        if (NULL != DMRxClbk_Fptr)
        {
            /*Fetch the required data*/
            DM_DataPackets_St_t DM_DataPackets_St;
            DM_DataPackets_St.len_u16 = dlc_u8;
            DM_DataPackets_St.PGN_u32 = recd_pgn_u32;
            DM_DataPackets_St.SA_u8 = reqSA_u8;
            J1939_tp_dataCopy(Can_Applidata_St->DataBytes_au8, &DM_DataPackets_St.DMxData[0], dlc_u8);
            /*Inform to upper layer*/
            DMRxClbk_Fptr(&DM_DataPackets_St);
        }
    }
    return;
}