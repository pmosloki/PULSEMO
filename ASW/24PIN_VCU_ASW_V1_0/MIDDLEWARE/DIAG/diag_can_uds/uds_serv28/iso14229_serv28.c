/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : iso14229_serv28.c
|    Project        : MIL_PBL_CV
|    Description    : Service description for UDS service - Communication
|                     Control.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
|31/07/2024       Manikandan S       Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

#ifndef ISO14229_SERV28_C /* Guard against multiple inclusion */
#define ISO14229_SERV28_C

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "iso14229_serv28.h"
#include "diag_adapt.h"
#include "comm_cntrl_adapt.h"

/*******************************************************************************
 *  Define & Macros
 ******************************************************************************/
Comm_Config_St_t Comm_Config_aSt[TOTAL_COMM_TYPE_E - 1] =
    {
        {NORMAL_COMM_MSG_E, &NormalCommMsg_Adapt},
        {NETWORK_MANAGEMENT_COMM_MSG_E, &NWMngmntCommMsg_Adapt},
        {NETWORK_MANAGEMENT_AND_NORMAL_COMM_MSG_E, &NWMngmnt_NormalCommMsg_Adapt},
};
/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/

/* @Summary : - this boolean for control type */

bool ENABLE_TX_b = TRUE;
bool ENABLE_RX_b = TRUE;
bool DISABLE_TX_b = FALSE;
bool DISABLE_RX_b = FALSE;

/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CommunicationMessage_Fn
*   Description   : This function will process the communication type.
*   Parameters    : uint8_t communicationType_u8 - communication type parameter.
                    bool Rx_Status - control type RX status.
                    bool Tx_Status - control type TX status.
*   Return Value  : None
*******************************************************************************/
static void CommunicationMessage_Fn(uint8_t communicationType_u8, bool Rx_Status, bool Tx_Status);

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CommunicationMessage_Fn
*   Description   : This function will process the communication type.
*   Parameters    : uint8_t communicationType_u8 - communication type parameter.
                    bool Rx_Status - control type RX status.
                    bool Tx_Status - control type TX status.
*   Return Value  : None
*******************************************************************************/
static void CommunicationMessage_Fn(uint8_t communicationType_u8, bool Rx_Status, bool Tx_Status)
{
    uint8_t communicationStatus_u8 = communicationType_u8 - 1;
    (Comm_Config_aSt[communicationStatus_u8].COMM_MSG_Fptr)(Rx_Status, Tx_Status);
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : iso14229_serv28
 *   Description   : This function will process the service_28 requests
 *   Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer table.
 *   Return Value  : Type of response.
 *******************************************************************************/

UDS_Serv_resptype_En_t iso14229_serv28(UDS_Serv_St_t *UDS_Serv_pSt)
{
    uint16_t numbytes_req_u16 = UDS_Serv_pSt->RxLen_u16;
    uint8_t controlType_u8 = UDS_Serv_pSt->RxBuff_pu8[ONE];
    uint8_t communicationType_u8 = UDS_Serv_pSt->RxBuff_pu8[TWO];
    uint8_t SuppressPosRes_u8 = (UDS_Serv_pSt->RxBuff_pu8[ONE] & POS_RESP_MASK_SUPRESS_VALUE);
    bool NoResponse_b = TRUE;
    UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_UNKNOWN_E;

    /* Checks invalid length*/
    if (SERV_28_MIN_LEN != numbytes_req_u16)
    {
        /*invalid message length NRC- 0x13 */
        UDS_Serv_pSt->TxLen_u16 = THREE;
        UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
        UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_COMMUNICATIONCONTROL;
        UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
    }
    /* Check whether the CommunicationType is valid or not*/
    else if ((communicationType_u8 == 0U) || (communicationType_u8 >= (unsigned char)TOTAL_COMM_TYPE_E))
    {
        /*Request out of range NRC- 0x31*/
        UDS_Serv_pSt->TxLen_u16 = THREE;
        UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
        UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_COMMUNICATIONCONTROL;
        UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_OUT_OF_RANGE;
        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
    }
    else
    {
        controlType_u8 = (controlType_u8 & SUB_FUNC_MASK_VALUE);
        switch (controlType_u8)
        {
        case ENABLE_RX_AND_TX_SUB_ID_E:
        {
            /*checking enable rx and tx sub function*/
            CommunicationMessage_Fn(communicationType_u8, ENABLE_RX_b, ENABLE_TX_b);
            UDS_Serv_pSt->TxLen_u16 = ONE;
            UDS_Serv_pSt->TxBuff_pu8[ONE] = controlType_u8;
            Serv_resptype_En = UDS_SERV_RESP_POS_E;
            COMMUNICATION_Status_au8[0] = controlType_u8;
            break;
        }
        case ENABLE_RX_AND_DISABLE_TX_SUB_ID_E:
        {
            /*checking enable rx and disable tx sub function*/
            CommunicationMessage_Fn(communicationType_u8, ENABLE_RX_b, DISABLE_TX_b);
            UDS_Serv_pSt->TxLen_u16 = ONE;
            UDS_Serv_pSt->TxBuff_pu8[ONE] = controlType_u8;
            Serv_resptype_En = UDS_SERV_RESP_POS_E;
            COMMUNICATION_Status_au8[0] = controlType_u8;
            break;
        }
        case DISABLE_RX_AND_ENABLE_TX_SUB_ID_E:
        {
            /*checking disable rx and enable tx sub function*/
            CommunicationMessage_Fn(communicationType_u8, DISABLE_RX_b, ENABLE_TX_b);
            UDS_Serv_pSt->TxLen_u16 = ONE;
            UDS_Serv_pSt->TxBuff_pu8[ONE] = controlType_u8;
            Serv_resptype_En = UDS_SERV_RESP_POS_E;
            COMMUNICATION_Status_au8[0] = controlType_u8;
            break;
        }
        case DISABLE_RX_AND_TX_SUB_ID_E:
        {
            /*checking disable rx and tx sub function*/
            CommunicationMessage_Fn(communicationType_u8, DISABLE_RX_b, DISABLE_TX_b);
            UDS_Serv_pSt->TxLen_u16 = ONE;
            UDS_Serv_pSt->TxBuff_pu8[ONE] = controlType_u8;
            Serv_resptype_En = UDS_SERV_RESP_POS_E;
            COMMUNICATION_Status_au8[0] = controlType_u8;
            break;
        }
        /* Check invalid subfunction id*/
        default:
        {
            /*subfunction not supported NRC- 0x12 */
            NoResponse_b = FALSE;
            UDS_Serv_pSt->TxLen_u16 = THREE;
            UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
            UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_COMMUNICATIONCONTROL;
            UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
            Serv_resptype_En = UDS_SERV_RESP_NEG_E;
        }
        }
        if (SuppressPosRes_u8 && NoResponse_b)
        {
            Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
        }
    }
    return Serv_resptype_En;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UDS_Serv28_Timeout
 *   Description   : This function resets the relevant variables/parameters when session timesout.
 *   Parameters    : none.
 *   Return Value  : none.
 *******************************************************************************/
void UDS_Serv28_Timeout(void)
{
    NWMngmnt_NormalCommMsg_Adapt(TRUE, TRUE); /* Rx Enable, Tx Enable */
    return;
}
#endif /*ISO14229_SERV28_C*/
/*---------------------- End of File -----------------------------------------*/
