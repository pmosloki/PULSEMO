/****************************************************************************************
 *    FILENAME    : j1939_73_dmx.c
 *
 *    DESCRIPTION : J1939-73 application diagnostic services
 *
 *    $Id         : $
 *
 *****************************************************************************************
 * Revision history
 *
 * Ver Author         Date          Description
 * 1   Sloki         5/28/2018     Fix for single frame DMx response.
 *****************************************************************************************
 */

/*
******************************************************************************
*    Includes
******************************************************************************
*/
#include "j1939.h"
#include "math_util.h"
#include "cil_can_conf.h"
#include "diag_sys_conf.h"
#include "str_util.h"
/************************************/
/*         module variables         */
/************************************/

J1939_Appl_St_t J1939_appl_St; /* J1939-73 diag appl layer data structure */

/************************************/
/*         Global variables         */
/************************************/

#define DM1_CHANGELIST_LEN 4U           /* for DM1, the 4 past DTC changes are remembered.                        */
#define DM4_FF_LEN 13U                  /* There are 13 bytes of freeze frame data per DTC.                       */
#define DM21_MINWITHMILON_MAX 64255U    /* Max limit for minutes run by engine while MIL is activated.            */
#define DM21_MINSINCEDTCCLR_MAX 64255U  /* Max limit for minutes since DTC were cleared.                          */
#define DM21_DISTWITHMILON_MAX 64255U   /* Max limit for distance in kms run by engine while MIL is activated.    */
#define DM21_DISTSINCEDTCCLR_MAX 64255U /* Max limit for distance in kms since DTC were cleared.                  */

/*****************************************/
/*         module functions prototype    */
/*****************************************/

static void dmx_init(void);
static bool anyChangeInList(uint32_t newValue_u32, uint32_t pastValues_pu32[], uint32_t compareLen_u32);
static J1939Appl_DMxRetStatus_En_t dmx_prepResponse(CAN_MessageFrame_St_t *Can_Applidata_St, J1939DTC_Uni_t J1939DTCs_aUni[], uint8_t NumOfJ1939DTCs_u8, uint16_t pgn_u16, uint8_t SaIndex_u8);
static void getLampStatus(stLamps_Uni_t *statusLamps);
static void dmx_MF_TxType(uint32_t pgn_u32, uint8_t reqSAIndex_u8);
static void dmx_prepResp_SF(CAN_MessageFrame_St_t *Can_Applidata_St, stLamps_Uni_t *statusLamps_Uni, uint8_t NumOfDTCs_u8);
static void dmx_prepResp_multiFrameOverBAM(uint32_t pgn_u32, uint8_t SAIndex_u8);
static void dmx_prepResp_multiFrameOverRTS(uint32_t pgn_u32, uint8_t SAIndex_u8);
static void dmx_prepResp_updateDTC(J1939DTC_Uni_t j1939dtc_Uni);
static void dmx_prepResp_fillFreezeFrame(J1939DTC_Uni_t *FrzFrmJ1939DTCs_pUni, uint8_t NumofJ1939dtc_u8);
static void fillByte(uint8_t data_u8);
uint8_t GetSourceAddrInst(uint8_t ReqSA_u8);
/*****************************************/
/*         Global functions definitions  */
/*****************************************/
/**
 * @brief	: An in-line function to fill a byte to global application buffer.
 *             copy the byte data and increment the application index.
 * @param	: byte value,
 * @return	: none.
 */
void fillByte(uint8_t data_u8)
{
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32] = data_u8;
    J1939_appl_St.respLen_u32++;
}

/**
 *  @brief        : Function to send claimed address and name.
 *                  this function is called when other ECU request for Address Claimed.
 *                  The function sends the response directly.
 *  @param        : Can_Applidata_St - CAN structure to hold CAN data of request and response.
 *  @return       : Type of response.
 *
 */
J1939Appl_DMxRetStatus_En_t J1939_TX_claimedAddress(CAN_MessageFrame_St_t *Can_Applidata_St)
{
    PARAM_NOTUSED(Can_Applidata_St);
    /*Send claimed address and name*/
    J1939_81_SendReqPGNResp();
    return J1939APPL_NO_RESP;
}

/**
 *  @brief        : The task is called every 50ms.
 *                  The task invokes DM1 message transmission if:
 *                  - any change in DTCs in DiaM.
 *                  - 1 second periodicity reached.
 *  @param        : none.
 *  @return       : none
 *
 */
void J1939_DM1_schedTask(void)
{
    /* variable declaration */
    static uint32_t prev_timems_u32 = 0U;
    static uint32_t prevSumDTCs_au32[DM1_CHANGELIST_LEN]; /* Send up-to 4 DTC changes within 1 second.*/
    static uint32_t dtcChangeCnt_u32 = 0U;                /* history counter to keep track DTC changes*/
    static bool initDoneDM1_b = FALSE;
    CAN_MessageFrame_St_t Can_Applidata_St;
    J1939DTC_Uni_t CurrentActiveFaults_aUni[FM_FAULTPATHS];
    uint32_t J1939DTC_au32[FM_FAULTPATHS];
    uint32_t currSumDTCs_u32 = 0U;
    uint8_t NumofPrsntFault_u8 = 0U;
    int8_t pos_s8 = 0;
    bool IsChangeInList_b = FALSE;
    bool IsTxActive_b = FALSE;
    bool triggerTxDM1_b = FALSE;

    if (!initDoneDM1_b)
    {
        initDoneDM1_b = (bool)TRUE;
        /* Capture time */
        prev_timems_u32 = GET_TIME_MS();
    }

    /***************************************************************/
    /*            CHECK WHETHER 1SEC TIMER EXPIRED for DM1         */
    /***************************************************************/

    /* Start T1 timer */
    if ((TIME_DIFF_MS(prev_timems_u32) >= DM1_PERIODICITY_MILLISECS))
    {
        prev_timems_u32 = GET_TIME_MS(); // todo harsh

        /* DM1 timer expired, initiate DM1 transmission */
        triggerTxDM1_b = (bool)TRUE;
    }
    else
    {
        /**************************************************************
            CHECK WHETHER ANY CHANGE IN ACTIVE DTC STATE in DiaM
            The change is identified by additive checksum method
            DM1 sent if "SUM(CURRENT_DTCS) != SUM (PREVIOUS_DTCS)"
            **************************************************************/
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
        NumofPrsntFault_u8 = FM_GetNumberOfPresentFaults();
        /* DTC information from DiaM */
        FM_ReadDTCsOfPresentFaults((uint32_t *)CurrentActiveFaults_aUni, NumofPrsntFault_u8, FM_PROTO_J1939_E);
#endif

        /*Check faults are present to get SPN FMI info*/
        if (NumofPrsntFault_u8 > 0)
        {
            for (pos_s8 = 0; pos_s8 < NumofPrsntFault_u8; pos_s8++)
            {
                J1939DTC_au32[pos_s8] = CurrentActiveFaults_aUni[pos_s8].W_u32;
            }
        }
        currSumDTCs_u32 = CalcAdditiveChecksum((uint32_t)NumofPrsntFault_u8 * sizeof(uint32_t), (uint8_t *)&J1939DTC_au32);
        /* Look for any changes in the past 4 sets of DTC values.
        prevSumDTCs_au32 is a ring buffer, so entire list to be checked. */
        IsChangeInList_b = anyChangeInList(currSumDTCs_u32, prevSumDTCs_au32, DM1_CHANGELIST_LEN);
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
        IsTxActive_b = IsTXCanMessageActive(BITPOSTX_CAN_DM1_EVENT);
#endif
        if (IsChangeInList_b && IsTxActive_b)
        {
            triggerTxDM1_b = (bool)TRUE;
            prevSumDTCs_au32[dtcChangeCnt_u32] = currSumDTCs_u32;
            dtcChangeCnt_u32++;
            dtcChangeCnt_u32 = dtcChangeCnt_u32 % DM1_CHANGELIST_LEN; /* wrap back the index for ring buffer.*/
        }
    }

    if (triggerTxDM1_b)
    {
        int16_t DM1Ret_s16 = 0;
        uint8_t SAInstance_u8 = 0u;
        /* time to send the DM1 message */
        triggerTxDM1_b = FALSE;
        SAInstance_u8 = GetSourceAddrInst(J1939_NMAC_ECU_ClaimedAddr_u8);
        DM1Ret_s16 = J1939_TX_DM1(&Can_Applidata_St,SAInstance_u8);

        /* Process the return values from DMx services */
        if ((int16_t)J1939APPL_SF_RESP == DM1Ret_s16)
        {
            Can_Applidata_St.ID_u32 = (uint32_t)(J1939_BASE_ID | ((uint32_t)J1939_DM1_PGN << 8U) | J1939_NMAC_ECU_ClaimedAddr_u8);
            Can_Applidata_St.DataLength_u8 = J1939_MAXDLC;
            Can_Applidata_St.MessageType_u8 = EXT_E;
#if (TRUE == DIAG_CONF_J1939_SUPPORTED)
            CIL_CAN_Tx_DynamicMsg(CIL_J1939_ACK_TX_E, Can_Applidata_St); // todo harsh
#endif
        }
    }
    return;
}

/**
 *  @brief        : Function to send Active DTC (non-emission).
 *                  Function to prepare response for DM1 message.
 *                  this function is called upon any request or periodically at 1 sec rate or
 *                  any DTC change event.
 *                  The function sends the response directly if it is of a single frame length.
 *                  otherwise sends data bytes to TP for multi-frame transmission.
 *  @param        : Can_Applidata_St - CAN structure to hold CAN data of request and response.
 *  @return       : Type of response.
 *
 */
J1939Appl_DMxRetStatus_En_t J1939_TX_DM1(CAN_MessageFrame_St_t *Can_Applidata_St, uint8_t reqSAIndex_u8)
{
    /* variable declarations */
    J1939DTC_Uni_t CurrentActiveFaults_aUni[FM_FAULTPATHS];
    J1939Appl_DMxRetStatus_En_t J1939Appl_DMxRetStatus_En = J1939APPL_NO_RESP;
    uint8_t NumofCurrentActiveFault_u8 = 0U;
    if(0xFFU != reqSAIndex_u8)
    {
        if ((int16_t)J1939_appl_St.stateAppl_En != (int16_t)J1939_APPL_IDLE_E)
        {
            /* This additional check is done only for DM1, because this is also
            called from J1939_DM1_schedTask(). For other DM services its already taken
            care @dmx_prepResp_multiFrameOverRTS and @dmx_prepResp_multiFrameOverBAM,
            where it will try again once TP layer becomes idle.
    
            J1939 Appl layer is busy with previous request, try later */
            J1939Appl_DMxRetStatus_En = J1939APPL_ACK_ECUBUSY;
        }
        else
        {
    #if (TRUE == DIAG_CONF_FM_SUPPORTED)
            NumofCurrentActiveFault_u8 = FM_GetNumberOfPresentFaults();
            FM_ReadDTCsOfPresentFaults((uint32_t *)CurrentActiveFaults_aUni, NumofCurrentActiveFault_u8, FM_PROTO_J1939_E);
    #elif (TRUE == DIAG_TEST_J1939_DEMO)
            NumofCurrentActiveFault_u8 = 2;
            FM_GetDemo_DTCs((uint32_t *)CurrentActiveFaults_aUni, NumofCurrentActiveFault_u8);
    #endif
            J1939Appl_DMxRetStatus_En = dmx_prepResponse(Can_Applidata_St, CurrentActiveFaults_aUni, NumofCurrentActiveFault_u8, J1939_DM1_PGN,reqSAIndex_u8);
        }
    }
    return J1939Appl_DMxRetStatus_En;
}

/**
 *  @brief        : reading of previously active DTCs (Stage-B confirmed).
 *                  function to prepare response for DM2 message.
 *  @param        : Can_Applidata_St - CAN structure to hold CAN data of request and response.
 *  @return       : Type of response.
 */
J1939Appl_DMxRetStatus_En_t J1939_TX_DM2(CAN_MessageFrame_St_t *Can_Applidata_St, uint8_t reqSAIndex_u8)
{
    /* variable declarations */
    J1939DTC_Uni_t prevActiveFaults_aUni[FM_FAULTPATHS];
    J1939Appl_DMxRetStatus_En_t J1939Appl_DMxRetStatus_En = J1939APPL_NO_RESP;
    uint8_t NumOfPrevActiveFaults_u8 = 0;
    /* DTC information from DiaM */
    if(0xFFU != reqSAIndex_u8)
    {
        #if (TRUE == DIAG_CONF_FM_SUPPORTED)
            NumOfPrevActiveFaults_u8 = FM_GetAllCnfrmDTCs((uint32_t *)prevActiveFaults_aUni, FM_PROTO_J1939_E);
        #elif (TRUE == DIAG_TEST_J1939_DEMO)
            NumOfPrevActiveFaults_u8 = 2;
            FM_GetDemo_DTCs((uint32_t *)prevActiveFaults_aUni, NumOfPrevActiveFaults_u8);
        #endif
            J1939Appl_DMxRetStatus_En = dmx_prepResponse(Can_Applidata_St, prevActiveFaults_aUni, NumOfPrevActiveFaults_u8, J1939_DM2_PGN, reqSAIndex_u8);
            return J1939Appl_DMxRetStatus_En;
    }
}

/**
 *  @brief        : function to clear all Stage-B faults.
 *  @param        : Can_Applidata_St - CAN structure to hold CAN data of request and response.
 *  @return       : Type of response.
 *
 */
J1939Appl_DMxRetStatus_En_t J1939_TX_DM3(CAN_MessageFrame_St_t *Can_Applidata_St, uint8_t reqSAIndex_u8)
{
    J1939Appl_DMxRetStatus_En_t J1939Appl_DMxRetStatus_En = J1939APPL_ACK_ACCESSDENIED;
    PARAM_NOTUSED(Can_Applidata_St);
    /*
    Clear the DiaM memory only if engine is not running.
    */
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
    if (0U == RPM_N_u16)
#else
    if (0U == RPM_DUMMY_u16)
#endif

    {

#if (TRUE == DIAG_CONF_FM_SUPPORTED)

        FM_TurnOffMI();
        FM_ClrAllFaults();
        // FM_ClrPndngOBDFaults();
        FM_ClrRdyMonFlags();
        FM_ClrCommonData();

#endif
        J1939Appl_DMxRetStatus_En = J1939APPL_ACK_POS;
    }
    else
    {
        J1939Appl_DMxRetStatus_En = J1939APPL_ACK_ACCESSDENIED;
    }

    return J1939Appl_DMxRetStatus_En;
}

/**
 *  @brief        : function to prepare response for DM4 message.
 *                  The function reads the freeze frame of active DTCs (Stage-A faults) and
 *                  previously active (Stage-B faults).
 *  @param        : Can_Applidata_St - CAN structure to hold CAN data of request and response.
 *  @return       : Type of response.
 *
 */

J1939Appl_DMxRetStatus_En_t J1939_TX_DM4(CAN_MessageFrame_St_t *Can_Applidata_St, uint8_t reqSAIndex_u8)
{
    uint8_t NumofFaults_u8 = 0U;
    J1939DTC_Uni_t FrzFrmsJ1939DTCs_aUni[FM_FAULTPATHS];
    J1939Appl_DMxRetStatus_En_t J1939Appl_DMxRetStatus_En = J1939APPL_NO_RESP;

    J1939_appl_St.stateAppl_En = J1939_APPL_SERV_INPROGRESS_E;
    if(0xFFU != reqSAIndex_u8)
    {
        #if (TRUE == DIAG_CONF_FM_SUPPORTED)
        
            /* J1939 DTC information from DiaM */
            NumofFaults_u8 = FM_GetNumberOfPresentFaults();
            NumofFaults_u8 = FM_ReadDTCsOfPresentFaults((uint32_t *)FrzFrmsJ1939DTCs_aUni, NumofFaults_u8, FM_PROTO_J1939_E);
        
        #elif (TRUE == DIAG_TEST_J1939_DEMO)
            NumofFaults_u8 = 2;
            FM_GetDemo_DTCs((uint32_t *)FrzFrmsJ1939DTCs_aUni, NumofFaults_u8);
        #endif
            dmx_prepResp_fillFreezeFrame(FrzFrmsJ1939DTCs_aUni, NumofFaults_u8);
        
            /* Read the freeze frames of previously active faults Stage-B */
            NumofFaults_u8 = 0U;
        #if (TRUE == DIAG_CONF_FM_SUPPORTED)
            NumofFaults_u8 = FM_GetAllCnfrmDTCs((uint32_t *)FrzFrmsJ1939DTCs_aUni, FM_PROTO_J1939_E);
        #elif (TRUE == DIAG_TEST_J1939_DEMO)
            NumofFaults_u8 = 2;
            FM_GetDemo_DTCs((uint32_t *)FrzFrmsJ1939DTCs_aUni, NumofFaults_u8);
        #endif
        
            dmx_prepResp_fillFreezeFrame(FrzFrmsJ1939DTCs_aUni, NumofFaults_u8);
            /* End of previously active faults */
        
            /* transmission of response */
            if (J1939_appl_St.respLen_u32 > 8U)
            {
                /* multi packet response, response to be routed via TP layer */
                dmx_MF_TxType(J1939_DM4_PGN, reqSAIndex_u8);
        
                J1939Appl_DMxRetStatus_En = J1939APPL_MF_RESP;
            }
            else
            {
                /* single frame response, the TP layer is not required */
                /* No J1939 DTC is present */
                Can_Applidata_St->DataBytes_au8[0] = 0x0U;
                Can_Applidata_St->DataBytes_au8[1] = 0x0U;
                Can_Applidata_St->DataBytes_au8[2] = 0x0U;
                Can_Applidata_St->DataBytes_au8[3] = 0x0U;
                Can_Applidata_St->DataBytes_au8[4] = 0x0U;
                Can_Applidata_St->DataBytes_au8[5] = 0xFFU;
                Can_Applidata_St->DataBytes_au8[6] = 0xFFU;
                Can_Applidata_St->DataBytes_au8[7] = 0xFFU;
        
                /* Reset the application layer */
                dmx_init();
                J1939Appl_DMxRetStatus_En = J1939APPL_SF_RESP;
            }
    }
    return J1939Appl_DMxRetStatus_En;
}

/**
 *  @brief        : function to read emission relevant pending DTCs (present in current or
 *                  previous driving cycle).
 *  @param        : Can_Applidata_St - CAN structure to hold CAN data of request and response.
 *  @return       : Type of response.
 *
 */
J1939Appl_DMxRetStatus_En_t J1939_TX_DM6(CAN_MessageFrame_St_t *Can_Applidata_St, uint8_t reqSAIndex_u8)
{
    /* variable declarations */
    J1939DTC_Uni_t PndngJ1939DTCs_aUni[FM_L2_ENTRY];
    J1939Appl_DMxRetStatus_En_t J1939Appl_DMxRetStatus_En = J1939APPL_NO_RESP;
    uint8_t NumOfJ1939DTCs_u8 = 0;
    if(0xFFU != reqSAIndex_u8)
    {
        #if (TRUE == DIAG_CONF_FM_SUPPORTED)
            /* DTC information from DiaM */
            // NumOfJ1939DTCs_u8 = _FML2_GetOBDPndng_DTCs((uint16_t*)PndngJ1939DTCs_aUni, FM_L2_ENTRY, FM_PROTO_J1939_E);
        #elif (TRUE == DIAG_TEST_J1939_DEMO)
            NumOfJ1939DTCs_u8 = 2;
            FM_GetDemo_DTCs((uint32_t *)PndngJ1939DTCs_aUni, NumOfJ1939DTCs_u8);
        #endif
            J1939Appl_DMxRetStatus_En = dmx_prepResponse(Can_Applidata_St, PndngJ1939DTCs_aUni, NumOfJ1939DTCs_u8, J1939_DM6_PGN, reqSAIndex_u8);
    }

    return J1939Appl_DMxRetStatus_En;
}

/**
 *  @brief        : The function compares a single value with list of values.
 *                  The size of the length is supplied as parameter.
 *                  If the input value matches with at-least one of the entry of the list, return FALSE.
 *                  If the input values does not match with any value of the list, then return TRUE.
 *  @param        : newValue_u32 - operand1 - single input value.
 *  @param        : pastValues_pu32 - list of values for comparison.
 *  @param        : compareLen_u32 - size of the list.
 *  @return         If the input value matches with at-least one of the entry of the list, return FALSE.
 *                  If the input values does not match with any value of the list, then return TRUE.
 *
 */
static bool anyChangeInList(uint32_t newValue_u32, uint32_t pastValues_pu32[], uint32_t compareLen_u32)
{
    bool state_b = (bool)TRUE;
    while (compareLen_u32 > 0U)
    {
        if (newValue_u32 == pastValues_pu32[compareLen_u32 - 1U])
        {
            /* match found */
            state_b = FALSE;
            break;
        }
        compareLen_u32--;
    }
    /* no match found */
    return state_b;
}

/**
 *  @brief        : get the MIL status from DiaM. All other lamp status are reset to 0.
 *  @param        : structure for lamp status.
 *  @return       : none.
 *
 */
static void getLampStatus(stLamps_Uni_t *statusLamps)
{
    /* reset all lamps */
    statusLamps->all_u32 = 0U;
    /* update the MIL lamp status. we don't have any other lamps */
    // statusLamps->B.MIL_b = DiaM_StatusOf_MI();
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
    statusLamps->B.MIL_b = FM_StatusOf_MI(); // FML2_StatusOf_MI
#endif
    /*If other lamp status supported then update the following as per requirement*/
    /*statusLamps->B.RSL_b = DiaM_StatusOf_RSL();
    statusLamps->B.AWL_b = DiaM_StatusOf_AWL();
    statusLamps->B.PL_b = DiaM_StatusOf_PL();*/
    return;
}

/**
 *  @brif         : reset the state machine
 *  @param        : none
 *  @return       : none
 *
 */
static void dmx_init(void)
{
    /* Reset the application layer */
    J1939_appl_St.stateAppl_En = J1939_APPL_IDLE_E;
    J1939_appl_St.respLen_u32 = 0U;
    J1939_appl_St.isDestSpecific_b = FALSE;
    J1939_appl_St.otherAdd_u8 = 0U;
    return;
}

/**
 * @brief	: Function to populate the application response buffer
 *			  with J1939 specific DTC values.
 * @param	: j1939dtc_Uni - J1939 specific DTC value,
 * @return	: none.
 */
static void dmx_prepResp_updateDTC(J1939DTC_Uni_t j1939dtc_Uni)
{
    /* fill SPN */
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32 + 0U] = (uint8_t)(j1939dtc_Uni.B.SPN_b19);
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32 + 1U] = (uint8_t)(j1939dtc_Uni.B.SPN_b19 >> 8U);
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32 + 2U] = (uint8_t)((j1939dtc_Uni.B.SPN_b19 >> (16U - 5U)) & 0xE0U);
    /* fill FMI */
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32 + 2U] |= (uint8_t)(j1939dtc_Uni.B.FMI_b5 & 0x1FU);

    /* fill CM */
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32 + 3U] = (uint8_t)(j1939dtc_Uni.B.CM_b1 << 7U);

    /* fill OC */
    J1939_appl_St.dataBuff[J1939_appl_St.respLen_u32 + 3U] |= (uint8_t)(j1939dtc_Uni.B.OC_b7 & 0x7FU);
    J1939_appl_St.respLen_u32 += 4U;
    return;
}

/**
 * @brief	: Function to populate the application response buffer
 *			  with multi-frame values.
 * @param	: pgn_u32 - PGN value of the response,
 * @return	: none.
 */
static void dmx_prepResp_multiFrameOverBAM(uint32_t pgn_u32, uint8_t SAInstIndex_u8)
{
    /* multi packet response, response to be routed via TP layer */
    if (J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstIndex_u8]->tl_state_u8 == J1939_TL_ST_IDLE_E)
    {
        /* copy data from appl layer to TP layer */
        J1939_tp_dataCopy(J1939_appl_St.dataBuff, J1939_TpMsg_paSt[J1939_TX_BUFF_E][SAInstIndex_u8]->buffer_pu8, (uint16_t)J1939_appl_St.respLen_u32);

        /* Transmit Broadcast Announce message */
        J1939_tp_txCMbam((uint16_t)J1939_appl_St.respLen_u32, (uint32_t)pgn_u32, SAInstIndex_u8);

        /* The response is handed over to TP layer. The TP will take care of complete transmission.
        Reset the application layer for any new request. */
        dmx_init();
    }
    else
    {
        /* TP is busy, hold on the transmission of the response.
        The TP is checked next during timeout function   */
        J1939_appl_St.stateAppl_En = J1939_APPL_RESP_MF_PROGRESS_E;
        J1939_appl_St.pgn_u32 = pgn_u32;
    }
    return;
}

/**
 * @brief	: Function to populate the application response buffer
 *			  with multi-frame values.
 * @param	: pgn_u16 - PGN value of the response,
 * @return	: none.
 */
static void dmx_prepResp_multiFrameOverRTS(uint32_t pgn_u32, uint8_t SAInstIndex_u8)
{
    if (J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstIndex_u8]->tl_state_u8 == J1939_TL_ST_IDLE_E)
    {
        /*copy data from appl layer to TP layer */
        J1939_tp_dataCopy(J1939_appl_St.dataBuff, J1939_TpMsg_paSt[J1939_RX_BUFF_E][SAInstIndex_u8]->buffer_pu8, (uint16_t)J1939_appl_St.respLen_u32);

        /*Transmit RTS message*/
        J1939_tp_txCMrts((uint16_t)J1939_appl_St.respLen_u32, (uint32_t)pgn_u32, SAInstIndex_u8);

        /* The response is handed over to TP layer. The TP will take care of complete transmission.
        Reset the application layer for any new request. */
        dmx_init();
    }
    else
    {
        /* TP is busy, hold on the transmission of the response.
        The TP is checked next during timeout function   */
        J1939_appl_St.stateAppl_En = J1939_APPL_RESP_MF_PROGRESS_E;
        J1939_appl_St.pgn_u32 = pgn_u32;
    }
}

/**
 * @brief	: Function to populate the application response buffer
 *			  with lamp status and DTC values. The response can be
 *             either single or multi-frame depending on number bytes in response.
 * @param	: Can_Applidata_St - CAN buffer pointer for single frame response.
 * @param	: J1939DTCs_pu16 - list of J1939 DTCs.
 * @param	: NumOfJ1939DTCs_u16 - Number of J1939 DTCs to be added to response buffer,
 * @param	: pgn_u16 - PGN of the request.
 * @return	: closing index.
 */
static J1939Appl_DMxRetStatus_En_t dmx_prepResponse(CAN_MessageFrame_St_t *Can_Applidata_St, J1939DTC_Uni_t J1939DTCs_aUni[], uint8_t NumOfJ1939DTCs_u8, uint16_t pgn_u16,uint8_t reqSAIndex_u8)
{
    stLamps_Uni_t statusLamps_Uni;
    uint32_t i = 0U;
    J1939Appl_DMxRetStatus_En_t J1939Appl_DMxRetStatus_En = J1939APPL_NO_RESP;

    /* LAMP Status */
    getLampStatus(&statusLamps_Uni);
    if ((NumOfJ1939DTCs_u8 > 0U) && (NULL != J1939DTCs_aUni))
    {
    	// fillByte((uint8_t)((statusLamps_Uni.B.MIL_b << 6U) | 0x3FU));   /* 0x3F indicates other lamps are not supported.*/
        fillByte((uint8_t)0xFFU);       /* TODO Sindhuja: As of now all IC related DTCs are non OBD related it doesn't support the lamps */
    	fillByte((uint8_t)(0xFFU));     /* Reserved for SAE assignment Lamp Status = 0xFF */

    	for (i=0U; i < NumOfJ1939DTCs_u8; i++)
        {
            dmx_prepResp_updateDTC(J1939DTCs_aUni[i]);
    	}
    }
    if (J1939_appl_St.respLen_u32 > 8U)
    {
        dmx_MF_TxType((uint32_t)pgn_u16,reqSAIndex_u8);
        J1939Appl_DMxRetStatus_En = J1939APPL_MF_RESP;
    }
    else
    {
        /* single frame response, the TP layer is not required*/
        dmx_prepResp_SF(Can_Applidata_St, &statusLamps_Uni, NumOfJ1939DTCs_u8);
        J1939Appl_DMxRetStatus_En = J1939APPL_SF_RESP;
    }

    return J1939Appl_DMxRetStatus_En;
}

/**
 *  brif          : Check multi-frame transmission option as BAM or RTS
 *  @param        : none
 *  @return       : none
 *
 */
static void dmx_MF_TxType(uint32_t pgn_u32, uint8_t SAInstIndex_u8)
{
    /*BAM and RTS Check*/
    if (J1939_appl_St.isDestSpecific_b)
    {
        dmx_prepResp_multiFrameOverRTS(pgn_u32,SAInstIndex_u8);
    }
    else
    {
        dmx_prepResp_multiFrameOverBAM(pgn_u32,SAInstIndex_u8);
    }
}

/**
 * @brief	: Function to prepare single frame response.
 *             Single frame is sent when there is PGN requst and value is under 8bytes.
 * @param	: Can_Applidata_St - CAN buffer pointer for single frame response.
 * @param	: statusLamps_Uni - current lamp status.
 * @param	: NumOfDTCs_u16 - Number of DTCs to be added to response buffer,
 * @return	: none.
 */
static void pgn_prepResp_SF(CAN_MessageFrame_St_t *Can_Applidata_St)
{
    /* single frame response, the TP layer is not required */
    SetMem(Can_Applidata_St->DataBytes_au8, 0xFFU, (uint32_t)8U);

    /* single frame case */
    CopyData(Can_Applidata_St->DataBytes_au8, J1939_appl_St.respLen_u32, J1939_appl_St.dataBuff);

    /* Reset the application layer */
    dmx_init();
    return;
}

/**
 * @brief	: Function to prepare single frame response.
 *             Single frame is sent when there is single or no DTC available in ECU.
 * @param	: Can_Applidata_St - CAN buffer pointer for single frame response.
 * @param	: statusLamps_Uni - current lamp status.
 * @param	: NumOfDTCs_u16 - Number of DTCs to be added to response buffer,
 * @return	: none.
 */
static void dmx_prepResp_SF(CAN_MessageFrame_St_t *Can_Applidata_St, stLamps_Uni_t *statusLamps_Uni, uint8_t NumOfDTCs_u8)
{
    /* single frame response, the TP layer is not required */
    SetMem(Can_Applidata_St->DataBytes_au8, 0xFFU, (uint32_t)8U);

    if (0U == NumOfDTCs_u8)
    {
        /* No DTC is present */
        // Can_Applidata_St->DataBytes_au8[0] = (uint8_t)(statusLamps_Uni->B.MIL_b << 6U);
        Can_Applidata_St->DataBytes_au8[0] = 0xFFU; /* Not supported in case of Non-OBD related DTCs */
        Can_Applidata_St->DataBytes_au8[1] = 0xFFU; /* Not supported in case of Non-OBD related DTCs */
        Can_Applidata_St->DataBytes_au8[2] = 0x0U;
        Can_Applidata_St->DataBytes_au8[3] = 0x0U;
        Can_Applidata_St->DataBytes_au8[4] = 0x0U;
        Can_Applidata_St->DataBytes_au8[5] = 0x0U;
        Can_Applidata_St->DataBytes_au8[6] = 0xFFU; /* Reserved */
        Can_Applidata_St->DataBytes_au8[7] = 0xFFU; /* Reserved */
    }
    else
    {
        /* single DTC case */
        CopyData(Can_Applidata_St->DataBytes_au8, J1939_appl_St.respLen_u32, J1939_appl_St.dataBuff);
    }
    /* Reset the application layer */
    dmx_init();
    return;
}

/**
 * @brief	: Function to populate the application response buffer
 *			  with freeze frame values.
 *             - Read the freeze frame parameters from DiaM using the J1939 DTC value.
 * @param	: FrzFrmJ1939DTCs_pUni - list of J1939 DTCs,
 * @param	: Numofdtc_u16 - size of J1939 DTC list.
 * @return	: None.
 */
static void dmx_prepResp_fillFreezeFrame(J1939DTC_Uni_t *FrzFrmJ1939DTCs_pUni, uint8_t NumofJ1939dtc_u8)
{
    uint16_t FrzFrm_Parm_au16[FM_FAULTPATHS];
    J1939DTC_Uni_t j1939dtc_Uni;
    uint32_t i = 0U;
    uint16_t J1939_RPM_N_u16 = 0U;
    uint16_t J1939_VEHSPEED_u16 = 0U;
    uint8_t J1939_ECT_u8 = 0U;
    uint8_t J1939_BOOST_u8 = 0U;
    uint8_t J1939_ENGINE_LOAD_u8 = 0U;
    for (i = 0UL; (i < NumofJ1939dtc_u8) && ((J1939_appl_St.respLen_u32 + DM4_FF_LEN) < J1939_73_APPL_BUFF_LEN); i++)
    {
        /* Total 12 bytes of freeze frame parameters per DTC.
        Note: there is no manufacturer specific parameters available.
        if available, then add to the list at end after Vehicle speed and
        update first byte with size info accordingly.
        */
        fillByte((DM4_FF_LEN - 1U)); /* freeze frame length */
                                     //         j1939dtc_u16 = (uint16_t)(FrzFrmJ1939DTCs_pUni+i);  //  : Todo Sandeep
                                     /*Read Freeze  frame*/
//        DiaM_ReadFreezeFrame (FrzFrm_Parm_au16, (uint8_t)i+1U);   :Todo  Sandeep
#if (TRUE == DIAG_CONF_FM_SUPPORTED)

        FM_ReadFrzFrm_ByDTC((uint16_t)(FrzFrmJ1939DTCs_pUni[i].W_u32), (uint16_t *)FrzFrm_Parm_au16, FM_PROTO_J1939_E);

#endif

        j1939dtc_Uni = FrzFrmJ1939DTCs_pUni[i]; //  : Todo Sandeep

        /* SPN:92 - Engine % Load, Size:1byte*/
        J1939_ENGINE_LOAD_u8 = (uint8_t)(FrzFrm_Parm_au16[0]);

        /* SPN:110 - Engine Coolant Temperature, size:1byte */
        J1939_ECT_u8 = (uint8_t)(FrzFrm_Parm_au16[1]);

        /* SPN:102 - Boost. Size:1byte*/
        J1939_BOOST_u8 = (uint8_t)(FrzFrm_Parm_au16[2]);

        /* SPN:190 - Engine Speed, Size:2bytes */
        J1939_RPM_N_u16 = (uint16_t)(FrzFrm_Parm_au16[3]);

        /* SPN84 -  Vehicle Speed, size:2bytes */
        J1939_VEHSPEED_u16 = (uint16_t)(FrzFrm_Parm_au16[4]);

        /*Prepare the response frame*/
        dmx_prepResp_updateDTC(j1939dtc_Uni);

        /* SPN:899 - Engine Torque Mode - Not available, so assign 0x0F(as per J1939-73).
        Bit length-4bits, scale & offset = N/A. */
        fillByte((uint8_t)(0x0F));
        /*Boost */
        fillByte((uint8_t)(J1939_BOOST_u8));
        /*Engine Speed*/
        fillByte((uint8_t)(J1939_RPM_N_u16 & 0xFFU));
        /* SPN:190 - Engine Speed - higher byte */
        fillByte((uint8_t)(J1939_RPM_N_u16 >> 8U));
        /*Engine Load*/
        fillByte((uint8_t)(J1939_ENGINE_LOAD_u8));
        /* Engine coolant temperature */
        fillByte((uint8_t)(J1939_ECT_u8));
        /*Vehicle Speed*/
        fillByte((uint8_t)(J1939_VEHSPEED_u16 & 0xFFU));
        /* SPN84 -  Vehicle Speed - higher byte */
        fillByte((uint8_t)((J1939_VEHSPEED_u16 >> 8U) & 0xFFU));
    }
    return;
}
/**
 * @brief	: Function to populate the application response buffer
 *			  with values. The response can be
 *             either single or multi-frame depending on number bytes in response.
 * @param	: Can_Applidata_St - CAN buffer pointer for single frame response.
 * @param	: J1939DTCs_pu16 - list of J1939 DTCs.
 * @param	: NumOfJ1939DTCs_u16 - Number of J1939 DTCs to be added to response buffer,
 * @param	: pgn_u16 - PGN of the request.
 * @return	: closing index.
 */
static J1939Appl_DMxRetStatus_En_t pgn_prepResponse(CAN_MessageFrame_St_t *Can_Applidata_St, uint16_t pgnIndex_u16, uint8_t SAInstIndex_u8)
{
    J1939Appl_DMxRetStatus_En_t J1939Appl_PgnRetStatus_En = J1939APPL_NO_RESP;
    switch (pgnIndex_u16)
    {
        case 0:
        {
            // uint32_t ReqData_u32 = 0u;
            // ReqData_u32 = Get_Hours_In_Hour();
            // J1939_appl_St.dataBuff[0] = (uint8_t)((ReqData_u32 >> 24) & 0xff);
            // J1939_appl_St.respLen_u32++;
            // J1939_appl_St.dataBuff[1] = (uint8_t)((ReqData_u32 >> 16) & 0xff);
            // J1939_appl_St.respLen_u32++;
            // J1939_appl_St.dataBuff[2] = (uint8_t)((ReqData_u32 >> 8) & 0xff);
            // J1939_appl_St.respLen_u32++;
            // J1939_appl_St.dataBuff[3] = (uint8_t)(ReqData_u32 & 0xff);
            // J1939_appl_St.respLen_u32++;
            break;
        }
        case 1:
        {
            /* To do: Sindhuja; This is added to test */
            // for(int m=0; m<30; m++)
            // {
            //     fillByte((uint8_t)(0x0F));
            // }
            break;
        }
        default:
        {
            break;
        }
    }

    if (J1939_appl_St.respLen_u32 > 8U)
    {
        dmx_MF_TxType((uint32_t)J1939_SPECIFIC_PGN_St[pgnIndex_u16].Req_Type_u16,SAInstIndex_u8);
        J1939Appl_PgnRetStatus_En = J1939APPL_MF_RESP;
    }
    else
    {
        /* single frame response, the TP layer is not required*/
        pgn_prepResp_SF(Can_Applidata_St);
        J1939Appl_PgnRetStatus_En = J1939APPL_SF_RESP;
    }

    return J1939Appl_PgnRetStatus_En;
}
/**
 *  @brief        : This function gives value for Req message
 *  @param        : * ReturnData_u8.
 *  @return       : none
 *
 */
J1939Appl_DMxRetStatus_En_t J1939_rqPgnSample_Callback(CAN_MessageFrame_St_t *Can_Applidata_St, uint32_t pgn_u32, uint8_t SAInstance_u8)
{
    J1939Appl_DMxRetStatus_En_t J1939Appl_ReqPGNRetStatus_En = J1939APPL_NO_RESP;
    if ((int16_t)J1939_appl_St.stateAppl_En != (int16_t)J1939_APPL_IDLE_E)
    {
        /* This additional check is done only for DM1, because this is also
        called from J1939_DM1_schedTask(). For other DM services its already taken
        care @dmx_prepResp_multiFrameOverRTS and @dmx_prepResp_multiFrameOverBAM,
        where it will try again once TP layer becomes idle.

        J1939 Appl layer is busy with previous request, try later */
        J1939Appl_ReqPGNRetStatus_En = J1939APPL_ACK_ECUBUSY;
    }
    else
    {
        J1939Appl_ReqPGNRetStatus_En = pgn_prepResponse(Can_Applidata_St, pgn_u32, SAInstance_u8);
    }
    return J1939Appl_ReqPGNRetStatus_En;
}
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name :
 *   Description   : This function is used fetch the address of the source or destination.
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/
uint8_t GetSourceAddrInst(uint8_t ReqSA_u8)
{
    uint8_t index_u8 = 0u;
    uint8_t ReqSAIndex_u8 = 0xFFu;
    for(index_u8 = 0u; index_u8 < J1939_SA_TOTAL_NUM; index_u8++)
    {
        if(ReqSA_u8 == J1939_Instance_aSt[index_u8].SourceAddress_u8)
        {
            ReqSAIndex_u8 = J1939_Instance_aSt[index_u8].SA_Inst_u8;
            break;
        }
    }
    return ReqSAIndex_u8;
}