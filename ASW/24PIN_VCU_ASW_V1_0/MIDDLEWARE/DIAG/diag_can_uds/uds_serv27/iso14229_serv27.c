
/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv27.c
|    Project      	: MIL_PBL_CV
|    Description    : Service description for UDS service  - Security
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
|31/07/2024       Manikandan S         Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/****************************************************************************
 * Includes
 ***************************************************************************/

#include "iso14229_serv27.h"
#include "iso14229_serv27_conf.h"
#include "iso14229_serv10.h"
#include "uds_conf.h"
/*******************************************************************************
 *  Define & Macros
 ******************************************************************************/
#define SENDKEY_SEEDDIFF  0x01U
#define SEED_REQ            0x01U
#define SEND_KEY            0x00U
#define KEY_OFFSET          0x02U

/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/

/* **************************************************************************
 * GLOBAL VARIABLES
 ****************************************************************************/
static uint16_t AccessLvlEqu_u16 = 0;
static uint8_t SecurityAccState_au8[TOTAL_SECURITY_LVL];
static uint8_t CalculatedKey_au8[MAX_KEY_LEN];
static uint8_t GeneratedSeed_au8[MAX_SEED_LEN];
static uint32_t CaptureDelayTimer_au32[TOTAL_SECURITY_LVL];
static uint8_t SecurityFailAttemptCnt_au8[TOTAL_SECURITY_LVL];
static uint32_t DelayTimeoutCnt_au32[TOTAL_SECURITY_LVL];

/*******************************************************************************
 *  FUNCTION PROTOTYPE
 ******************************************************************************/
static void GenerateNullSeed(uint8_t *databuff_pu8,uint8_t Len_u8);
static void GetSecurityTableIndex(uint8_t* TableIndx_u8,UdsSecurityAccess_St_t * UdsSecurityAccess_pSt);
static uint8_t GetSecurityAccessDescriptor(uint8_t AccessType_u8,uint8_t *SubFuncType_u8, UdsSecurityAccess_St_t ** UdsSecurityAccess_pSt);
static uint8_t CompareBuffData(uint8_t *Buffer1_pu8,uint8_t* Buffer2_pu8,uint8_t len_u8);
static void UnlockSecurityLvl(uint8_t SecurityLevel_u8);
/*LDRA_INSPECTED 458 S */
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27
 *  Description   : This function will process the service_27 requests
 *  Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer table.
 *  Return Value  : Type of response.
 *******************************************************************************/
/*LDRA_INSPECTED 76 D */
UDS_Serv_resptype_En_t iso14229_serv27(UDS_Serv_St_t *UDS_Serv_pSt)
{
    uint8_t SecurityAccesType_u8 = UDS_Serv_pSt->RxBuff_pu8[1];
    UDS_Serv_resptype_En_t Serv_resptype_En = UDS_SERV_RESP_NORESP_E;
    uint8_t ReqSubFuncType_u8 = 0U;
    uint8_t Indx_u8 = 0U;
    /* LDRA_INSPECTED 105 D */
    uint8_t delayTimerSel_u8 = 0U;
    UdsSecurityAccess_St_t*  SecAccessDescriptor_pSt = NULL;

    if(UDS_Serv_pSt->RxLen_u16 < (uint16_t)SECURITY_ACCESS_MIN_LEN)
    {
        UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
    }
    /*LDRA_INSPECTED 72 D */
    else if(false == (bool) GetSecurityAccessDescriptor(SecurityAccesType_u8,&ReqSubFuncType_u8,&SecAccessDescriptor_pSt))
    {
        UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SUB_FUNC_NOT_SUPPORTED;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
    }
    else if(!(DiagSessionCheck(SecAccessDescriptor_pSt->SupportedSession_u16)))
	{
		UDS_Serv_pSt->TxLen_u16 = THREE;
		UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
		UDS_Serv_pSt->TxBuff_pu8[TWO] = SUBFUNC_NOSUPP_IN_ACTIVE_SESS;
		Serv_resptype_En = UDS_SERV_RESP_NEG_E;
    }
    else
    {
	    /* LDRA_INSPECTED 135 D */
        GetSecurityTableIndex(&Indx_u8,SecAccessDescriptor_pSt);

        switch(SecurityAccState_au8[Indx_u8])
        {
            case SA_SEED_REQ_E:
            {
                if((unsigned char)SF_TYPE_ODD_E != ReqSubFuncType_u8)
                {
                    UDS_Serv_pSt->TxLen_u16 = THREE;
                    UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                    UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                    UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
                    Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                }
                else if(UDS_Serv_pSt->RxLen_u16 != (uint16_t) SECURITY_ACCESS_SEED_REQ_LEN)
                {
                     UDS_Serv_pSt->TxLen_u16 = THREE;
		             UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		             UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
		             UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		             Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                }
                else
                {
                    UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_SA;
                    UDS_Serv_pSt->TxBuff_pu8[ONE]  = SecurityAccesType_u8;
                    /*LDRA_INSPECTED 496 S */
                    RandomNumGenerator_Fptr(&UDS_Serv_pSt->TxBuff_pu8[TWO],SecAccessDescriptor_pSt->SeedLen_u8,GeneratedSeed_au8);
                    UDS_Serv_pSt->TxLen_u16 = (uint16_t)((uint16_t)TWO + (uint16_t)SecAccessDescriptor_pSt->SeedLen_u8 - (uint16_t)ONE);
                    SecurityFailAttemptCnt_au8[Indx_u8] = 0U;
                    SecurityAccState_au8[Indx_u8] = (uint8_t)SA_RECEIVE_KEY_E; 
                    Serv_resptype_En = UDS_SERV_RESP_POS_E;
                }

                break;
            }
            case SA_RECEIVE_KEY_E:
            {
                if (NULL == KeyGenerator_Fptr)
                {
                    UDS_Serv_pSt->TxLen_u16 = THREE;
                    UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                    UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                    UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_KEY;
                    Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                }
                else if((unsigned char)SF_TYPE_EVEN_E != ReqSubFuncType_u8)
                {
                     UDS_Serv_pSt->TxLen_u16 = THREE;
                    UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                    UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                    UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
                    Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                }
                else if((uint8_t)(UDS_Serv_pSt->RxLen_u16 - 0x02U) != SecAccessDescriptor_pSt->KeyLen_u8)
                {
                     UDS_Serv_pSt->TxLen_u16 = THREE;
		             UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
		             UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
		             UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
		             Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                }
                else
                {
                    /*LDRA_INSPECTED 496 S */
                    KeyGenerator_Fptr(&UDS_Serv_pSt->RxBuff_pu8[KEY_OFFSET],CalculatedKey_au8,SecAccessDescriptor_pSt->KeyLen_u8,
                    SecAccessDescriptor_pSt->SeedLen_u8,SecAccessDescriptor_pSt->SecurityLvl_u8);
                    /*LDRA_INSPECTED 114 S */
                    /*LDRA_INSPECTED 434 S */
                    /*LDRA_INSPECTED 458 S */
                    if (CompareBuffData(CalculatedKey_au8, GeneratedSeed_au8, SecAccessDescriptor_pSt->KeyLen_u8))
                    {
                        /*LDRA_INSPECTED 434 S */
                        /*LDRA_INSPECTED 458 S */
                        UnlockSecurityLvl(SecAccessDescriptor_pSt->SecurityLvl_u8);
                        UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_SA;
                        UDS_Serv_pSt->TxBuff_pu8[ONE] = SecurityAccesType_u8;
                        UDS_Serv_pSt->TxLen_u16 = ONE;
                        Serv_resptype_En = UDS_SERV_RESP_POS_E;
                    }
                    else
                    {
                        SecurityFailAttemptCnt_au8[Indx_u8]++;
                        UDS_Serv_pSt->TxLen_u16 = THREE;
                        UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                        UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                        /*LDRA_INSPECTED 93 S */
                        if(SecurityFailAttemptCnt_au8[Indx_u8] < SecAccessDescriptor_pSt->MaxAttempt_u8)
                        { 
                           // delayTimerSel_u8 =  (uint8_t)((uint8_t)SecurityFailAttemptCnt_au8[Indx_u8] - (uint8_t)SecAccessDescriptor_pSt->MaxAttempt_u8);
                            DelayTimeoutCnt_au32[Indx_u8] = SecAccessDescriptor_pSt->DelayTimerTable_pu8[delayTimerSel_u8];
                            /*LDRA_INSPECTED 496 S */
                            //CaptureDelayTimer_au32[Indx_u8] = GET_TIME_MS();
                            UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_KEY;
                        }
                        else
                        {
                            SecurityAccState_au8[Indx_u8] = (uint8_t)SA_SEED_REQ_E;
                            UDS_Serv_pSt->TxBuff_pu8[TWO] = EXCEEDED_NUMBER_OF_ATTEMPTS;
                            /*LDRA_INSPECTED 93 S */
                            if((true == (bool)SecAccessDescriptor_pSt->DelayTimerEn_u8))
                            {
                                CaptureDelayTimer_au32[Indx_u8] = GET_TIME_MS();
                                SecurityAccState_au8[Indx_u8] = (uint8_t)SA_DELAYTIMER_EN_E;
                                SecurityFailAttemptCnt_au8[Indx_u8] = 0U;
                            }
                        }
                        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                    }   
               }
                
                break;
            }
            case SA_DELAYTIMER_EN_E:
            {
                UDS_Serv_pSt->TxLen_u16 = THREE;
                UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                UDS_Serv_pSt->TxBuff_pu8[TWO] = REQ_TIMEDELAY_NOTEX;
                CaptureDelayTimer_au32[Indx_u8] = GET_TIME_MS();
                Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                break;
            }
            case SA_ACCESS_OK_E:
            {
                if ((unsigned char)SF_TYPE_ODD_E == ReqSubFuncType_u8)
                {
                    if (UDS_Serv_pSt->RxLen_u16 != (uint16_t)SECURITY_ACCESS_SEED_REQ_LEN)
                    {
                        UDS_Serv_pSt->TxLen_u16 = THREE;
                        UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                        UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                        UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
                        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                    }
                    else
                    {
                        UDS_Serv_pSt->TxBuff_pu8[ZERO] = SID_SA;
                        UDS_Serv_pSt->TxBuff_pu8[ONE] = SecurityAccesType_u8;
                        GenerateNullSeed(&UDS_Serv_pSt->TxBuff_pu8[TWO], SecAccessDescriptor_pSt->SeedLen_u8);
                        UDS_Serv_pSt->TxLen_u16 = (uint16_t)(TWO) + (uint16_t)(SecAccessDescriptor_pSt->SeedLen_u8) - 1;
                        Serv_resptype_En = UDS_SERV_RESP_POS_E;
                    }
                }
                else
                {
                    if ((uint8_t)(UDS_Serv_pSt->RxLen_u16 - 0x02U) != SecAccessDescriptor_pSt->KeyLen_u8)
                    {
                        UDS_Serv_pSt->TxLen_u16 = THREE;
                        UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                        UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                        UDS_Serv_pSt->TxBuff_pu8[TWO] = INVALID_MESSAGE_LENGTH;
                        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                    }
                    else
                    {
                        UDS_Serv_pSt->TxLen_u16 = THREE;
                        UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                        UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                        UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
                        Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                    }
                }
                break;
            }
            default:
            {
                UDS_Serv_pSt->TxLen_u16 = THREE;
                UDS_Serv_pSt->TxBuff_pu8[ZERO] = NEGATIVE_RESP;
                UDS_Serv_pSt->TxBuff_pu8[ONE] = SID_SA;
                UDS_Serv_pSt->TxBuff_pu8[TWO] = REQUEST_SEQUENCE_ERR;
                Serv_resptype_En = UDS_SERV_RESP_NEG_E;
                break;
            }
        }
    }
    return Serv_resptype_En;

}


/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27
 *  Description   : This function will process the service_27 requests
 *  Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer table.
 *  Return Value  : Type of response.
 *******************************************************************************/
static void GetSecurityTableIndex(uint8_t* TableIndx_u8,UdsSecurityAccess_St_t * UdsSecurityAccess_pSt)
{
    uint8_t LoopCnt_u8 = 0;

    for (LoopCnt_u8 = 0U; LoopCnt_u8 < (uint8_t)TOTAL_SECURITY_LVL; LoopCnt_u8++)
    {
        if (UdsSecurityAccess_pSt->SecurityLvl_u8 == UdsSecurityAccess_aSt[LoopCnt_u8].SecurityLvl_u8)
        {
            *TableIndx_u8 = LoopCnt_u8;
        }
    }

    return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : Generate_NullSeed
 *   Description   : This function is to return null seed frame.
 *   Parameters    : uint8_t * databuff_pu8
 *   Return Value  : None
 *******************************************************************************/
/*LDRA_EXCLUDE_START 436 S */
static void GenerateNullSeed(uint8_t *databuff_pu8,uint8_t Len_u8)    /* Pointer to an array of bytes */      
{
    uint8_t LoopCnt_u8 = 0U;
    for (LoopCnt_u8 = 0U; LoopCnt_u8 < Len_u8; LoopCnt_u8++)
    {
        /*LDRA_INSPECTED 436 S */
        databuff_pu8[LoopCnt_u8] = 0U;
    }

    return;
}
/*LDRA_EXCLUDE_END 436 S */

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27
 *  Description   : This function will process the service_27 requests
 *  Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer table.
 *  Return Value  : Type of response.
 *******************************************************************************/
static uint8_t GetSecurityAccessDescriptor(uint8_t AccessType_u8,uint8_t *SubFuncType_u8, UdsSecurityAccess_St_t ** UdsSecurityAccess_pSt)
{
    uint8_t State_u8 = false;
    uint8_t LoopCnt_u8 = 0;

    for(LoopCnt_u8 = 0U;LoopCnt_u8<(uint8_t)TOTAL_SECURITY_LVL;LoopCnt_u8++)
    {
        if(UdsSecurityAccess_aSt[LoopCnt_u8].SecurityLvl_u8 == AccessType_u8)
        {
            State_u8 = true;
            *SubFuncType_u8 = (uint8_t)SF_TYPE_ODD_E;

        }
        else if((UdsSecurityAccess_aSt[LoopCnt_u8].SecurityLvl_u8 +SENDKEY_SEEDDIFF) == AccessType_u8)
        {
            State_u8 = true;
            *SubFuncType_u8 = (uint8_t)SF_TYPE_EVEN_E;
        }
        else
        {
            /* Code */
        }

        if(State_u8)
        {
            /*LDRA_INSPECTED 94 S */
            /*LDRA_INSPECTED 95 S */
            /*LDRA_INSPECTED 554 S */
            /*LDRA_INSPECTED 203 S */
            *UdsSecurityAccess_pSt = (UdsSecurityAccess_St_t *)&UdsSecurityAccess_aSt[LoopCnt_u8];
            break;
        }
    }

    return State_u8;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27
 *  Description   : This function will process the service_27 requests
 *  Parameters    : UDS_Serv_St_t* UDS_Serv_pSt - pointer to service distributer table.
 *  Return Value  : Type of response.
 *******************************************************************************/
/*LDRA_EXCLUDE_START 436 S */
/*LDRA_INSPECTED 458 S */
static uint8_t CompareBuffData(uint8_t *Buffer1_pu8,uint8_t* Buffer2_pu8,uint8_t len_u8)   /* Pointer to an array of bytes */
{
    uint8_t Status_u8 = true;
    uint8_t LoopCnt_u8 = 0;

    for(LoopCnt_u8 = 0U;LoopCnt_u8<len_u8;LoopCnt_u8++)
    {
        /*LDRA_INSPECTED 436 S */
        if(Buffer1_pu8[LoopCnt_u8] != Buffer2_pu8[LoopCnt_u8])
        {
            Status_u8 = false;
            break;
        }
    }

    return Status_u8;
}
/*LDRA_EXCLUDE_END 436 S */
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27_securitycheck
 *  Description   : Function checks security is gained or not.
 *  Parameters    : Level.
 *  Return Value  : Type of response.
 *******************************************************************************/
/*LDRA_EXCLUDE_START 458 S */
static void UnlockSecurityLvl(uint8_t SecurityLevel_u8)
{
   uint8_t LoopCnt_u8 = 0;

   for (LoopCnt_u8 = 0U; LoopCnt_u8 < (uint8_t)TOTAL_SECURITY_LVL; LoopCnt_u8++)
   {
       if ((unsigned char)SA_DELAYTIMER_EN_E != SecurityAccState_au8[LoopCnt_u8])
       {
           SecurityAccState_au8[LoopCnt_u8] = (uint8_t)SA_SEED_REQ_E;
       }
   }

    for (LoopCnt_u8 = 0U; LoopCnt_u8 < (uint8_t)TOTAL_SECURITY_LVL; LoopCnt_u8++)
    {
        if (SecurityLevel_u8 == UdsSecurityAccess_aSt[LoopCnt_u8].SecurityLvl_u8)
        {
            AccessLvlEqu_u16 = UdsSecurityAccess_aSt[LoopCnt_u8].LvlEqu_u16;
            PblParam_St.DiagSecurityState_u8 = (uint8_t)AccessLvlEqu_u16;
            SecurityAccState_au8[LoopCnt_u8] = (uint8_t)SA_ACCESS_OK_E;
            break;
        }
    }

    return;
}
/*LDRA_EXCLUDE_END 458 S */
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27_securitycheck
 *  Description   : Function checks security is gained or not.
 *  Parameters    : Level.
 *  Return Value  : Type of response.
 *******************************************************************************/
void DelayTimerCheck(void)
{
    /* LDRA_INSPECTED 105 D */
   uint8_t LoopCnt_u8 = 0;
   uint32_t TimeDiff_u32 = 0;

    for (LoopCnt_u8 = 0U; LoopCnt_u8 < (uint8_t)TOTAL_SECURITY_LVL; LoopCnt_u8++)
    {
        if ((unsigned char)SA_DELAYTIMER_EN_E == SecurityAccState_au8[LoopCnt_u8])
        {
            /*LDRA_INSPECTED 496 S */
            /*LDRA_INSPECTED 434 S */
            /*LDRA_INSPECTED 458 S */
            TimeDiff_u32 = GetTimeDelayDiff(GET_TIME_MS(),CaptureDelayTimer_au32[LoopCnt_u8]);
            if( TimeDiff_u32 >= DelayTimeoutCnt_au32[LoopCnt_u8])
            {
                SecurityAccState_au8[LoopCnt_u8] = (uint8_t)SA_SEED_REQ_E;
            }
        }
    }
    return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27_securitycheck
 *  Description   : Function checks security is gained or not.
 *  Parameters    : Level.
 *  Return Value  : Type of response.
 *******************************************************************************/
uint16_t iso14229_serv27_securitycheck(void)
{
    return AccessLvlEqu_u16;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : iso14229_serv27_timeout
 *  Description   : Function resets the relevant variables/parameters when session times out.
 *  Parameters    : None.
 *  Return Value  : None.
 *******************************************************************************/
/*LDRA_INSPECTED 8 D */
void iso14229_serv27_timeout(void)
{
    uint8_t LoopCnt_u8 = 0;
    CipherText_u32 = 0;
    PlainText_u32 = 0;

    for (LoopCnt_u8 = 0U; LoopCnt_u8 < (uint8_t)TOTAL_SECURITY_LVL; LoopCnt_u8++)
    {
        if ((unsigned char)SA_DELAYTIMER_EN_E != SecurityAccState_au8[LoopCnt_u8])
        {
            SecurityAccState_au8[LoopCnt_u8] = (uint8_t)SA_SEED_REQ_E;  
            if(NO_SECURITY_EQU != AccessLvlEqu_u16)
            {
            AccessLvlEqu_u16 = NO_SECURITY_EQU; 
            PblParam_St.DiagSecurityState_u8 = (uint8_t)AccessLvlEqu_u16;
            SecurityFailAttemptCnt_au8[LoopCnt_u8] = 0U;
                /*LDRA_INSPECTED 128 D */
            UpdateNVMblock(PBL_NVM_BLOCK);
            }
        }
    }
    return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *  Function Name : UDS_Service27Init
 *  Description   : The function initialize the service 27 
 *  Parameters    : None.
 *  Return Value  : None.
 *******************************************************************************/
/*LDRA_INSPECTED 76 D */
void UDS_Service27Init(void)
{
    uint8_t LoopCnt_u8 = 0;
    for (LoopCnt_u8 = 0U; LoopCnt_u8 < (uint8_t)TOTAL_SECURITY_LVL; LoopCnt_u8++)
    {
        DelayTimeoutCnt_au32[LoopCnt_u8] = ZERO;
        CaptureDelayTimer_au32[LoopCnt_u8] = ZERO;
        SecurityAccState_au8[LoopCnt_u8] = (uint8_t)SA_SEED_REQ_E;
    }

    if(UdsServ27InitCallback_Fptr != NULL)
    {
        /*LDRA_INSPECTED 496 S */
        UdsServ27InitCallback_Fptr();
    }
    if (PblParam_St.DiagSecurityState_u8 == SECURITY_EQU_LEVEL_1)
    {
        AccessLvlEqu_u16 = SECURITY_EQU_LEVEL_1;
    }
    return;
}
/*---------------------- End of File -----------------------------------------*/
