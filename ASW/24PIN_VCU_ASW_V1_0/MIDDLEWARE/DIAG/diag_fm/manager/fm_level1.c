/***************************************************************************************************
 *    FILENAME    : fm_level1.c
 *
 *    DESCRIPTION : File defines all the functions related to Fault manager Level 1
 *
 *    $Id         : $
 *
 ***************************************************************************************************
 * Revision history
 *
 * Ver Author       Date       Description
 * 1                17/09/2008
 ***************************************************************************************************
 */

/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
#include "fm.h"
#include "fm_conf.h"
#include "fm_level1.h"
#include "fm_level2.h"
#include "fee_adapt.h"
#include "fmff_conf.h"

/*ankit*/
#include "diag_appl_test.h"
#include "uds_DTC.h"
// #include "FaultMgmr.h"
/*
 ***************************************************************************************************
 *    Defines
 ***************************************************************************************************
 */
#define NA_STATE 0x80 // Not applicable state
/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
 */

FM_FirstFailedFltsInfo_St_t FM_FirstFailedFltsInfo_St;
bool FML1_FrzFrmUpdated_b = FALSE;

/*
    RAM copy of the fault monitoring structure
*/
FML1_FltMon_St_t FML1_FltMonRam_St;

/*
    Flags to indicate the readiness monitoring
    results for different monitoring groups (FUEL,EGR and CCOMP)
*/
uint8_t FML1_RdyCmpltFUELSys_u8 = 0;
uint8_t FML1_RdyCmpltEGRSys_u8 = 0;
uint8_t FML1_RdyCmpltCCOMPSys_u8 = 0;

/*
    Data structure for Level - 1
*/
FML1_FaultPath_St_t FaultPath_aSt[NUM_OF_FAULTPATHS_E];

/*
 **************************************************************************************************
 *    Static Function prototype
 **************************************************************************************************
 */

static void FML1_RdyInit(void);
static void FML1_Update_RdyMatrix(u16_FaultPath_t);
static void FML1_Report_Error_Fault(uint8_t errortype, u16_FaultPath_t FltPath);
void FML1_SetMaxContPrsnt(u16_FaultPath_t FaultPath_En);
void FML1_SetMinContPrsnt(u16_FaultPath_t FaultPath_En);
void FML1_SetSigContPrsnt(u16_FaultPath_t FaultPath_En);
void FML1_SetNplContPrsnt(u16_FaultPath_t FaultPath_En);
void FML1_SetFerContPrsnt(u16_FaultPath_t FaultPath_En);

void FML1_SetMaxErr(u16_FaultPath_t FaultPath_En);
void FML1_SetMinErr(u16_FaultPath_t FaultPath_En);
void FML1_SetSigErr(u16_FaultPath_t FaultPath_En);
void FML1_SetNplErr(u16_FaultPath_t FaultPath_En);
void FML1_SetFerErr(u16_FaultPath_t FaultPath_En);

typedef void (*Fun_Ptr)(u16_FaultPath_t);

typedef struct
{
    uint8_t errortype_u8; /* HAL specific config table for CAN */
    Fun_Ptr SetContPrsnt; /* Data Pointer*/
    Fun_Ptr Set_Error;    /* Data Pointer*/
} FM_Report_st_t;

FM_Report_st_t FM_Report_ast[5] =
    {
        {FM_MAX_ERR, FML1_SetMaxContPrsnt, FML1_SetMaxErr},
        {FM_MIN_ERR, FML1_SetMinContPrsnt, FML1_SetMinErr},
        {FM_SIG_ERR, FML1_SetSigContPrsnt, FML1_SetSigErr},
        {FM_NPL_ERR, FML1_SetNplContPrsnt, FML1_SetNplErr},
        {FM_FER_ERR, FML1_SetFerContPrsnt, FML1_SetFerErr},
};
// static void FML1_CaptureOBDFrzFrm (u16_FaultPath_t, uint16_t);
// static void FML1_CaptureNonOBDFrzFrm (u16_FaultPath_t , uint16_t);

/*
 **************************************************************************************************
 *    Function definitions
 **************************************************************************************************
 */

/*  Inline function for MAX ERROR
 */

void FML1_SetMaxErr(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrAllErrBits(FaultPath_En);
    FML1_SetMaxErrBit(FaultPath_En);
    FML1_SetErf(FaultPath_En);
    FML1_SetUpt_St(FaultPath_En);
    FML1_SetTst(FaultPath_En);
    FML1_SET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_SET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FM_SetTFSLC_ByFltPathFltType(FaultPath_En, FM_GET_FLTTYPE(0));
}

/*  Inline function for Minimum ERROR
 */
void FML1_SetMinErr(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrAllErrBits(FaultPath_En);
    FML1_SetMinErrBit(FaultPath_En);
    FML1_SetErf(FaultPath_En);
    FML1_SetUpt_St(FaultPath_En);
    FML1_SetTst(FaultPath_En);
    FML1_SET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_SET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FM_SetTFSLC_ByFltPathFltType(FaultPath_En, FM_GET_FLTTYPE(1));
}

/*  Inline function for Signal ERROR
 */
void FML1_SetSigErr(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrAllErrBits(FaultPath_En);
    FML1_SetSigErrBit(FaultPath_En);
    FML1_SetErf(FaultPath_En);
    FML1_SetUpt_St(FaultPath_En);
    FML1_SetTst(FaultPath_En);
    FML1_SET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_SET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FM_SetTFSLC_ByFltPathFltType(FaultPath_En, FM_GET_FLTTYPE(2));
}

/*  Inline function for Non-plausible ERROR
 */
void FML1_SetNplErr(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrAllErrBits(FaultPath_En);
    FML1_SetNplErrBit(FaultPath_En);
    FML1_SetErf(FaultPath_En);
    FML1_SetUpt_St(FaultPath_En);
    FML1_SetTst(FaultPath_En);
    FML1_SET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_SET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FM_SetTFSLC_ByFltPathFltType(FaultPath_En, FM_GET_FLTTYPE(3));
}

/*  Inline function for Functional ERROR
 */
void FML1_SetFerErr(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrAllErrBits(FaultPath_En);
    FML1_SetFerErrBit(FaultPath_En);
    FML1_SetErf(FaultPath_En);
    FML1_SetUpt_St(FaultPath_En);
    FML1_SetTst(FaultPath_En);
    FML1_SET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_SET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FM_SetTFSLC_ByFltPathFltType(FaultPath_En, FM_GET_FLTTYPE(4));
}

/*  Inline function for Clearing all the Error flags.
 */
void FML1_ClrAllErr(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrAllErrBits(FaultPath_En);
    FML1_ClrErf(FaultPath_En);
    FML1_CLR_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
    FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/*
 Function to clear continously present status of functional error
*/
void FML1_ClrFerContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (TRUE == FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.FerCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StFerCp = TRUE;
    }
    else
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StFerCp = FALSE;
    }

    FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.FerCp = FALSE;
}

/*
 Function to clear continously present status of NPL error
*/
void FML1_ClrNplContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (TRUE == FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.NplCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StNplCp = TRUE;
    }
    else
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StNplCp = FALSE;
    }

    FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.NplCp = FALSE;
}

/*
 Function to clear continously present status of sig error
*/
void FML1_ClrSigContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (TRUE == FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.SigCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StSigCp = TRUE;
    }
    else
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StSigCp = FALSE;
    }

    FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.SigCp = FALSE;
}

/*
 Function to clear continously present status of min error
*/
void FML1_ClrMinContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (TRUE == FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.MinCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StMinCp = TRUE;
    }
    else
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StMinCp = FALSE;
    }

    FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.MinCp = FALSE;
}

/*
 Function to clear continously present status of max error
*/
void FML1_ClrMaxContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (TRUE == FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.MaxCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StMaxCp = TRUE;
    }
    else
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StMaxCp = FALSE;
    }

    FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.MaxCp = FALSE;
}

/*
 Function to clear all continously present status
*/
void FML1_ClrAllContPrsnt(u16_FaultPath_t FaultPath_En)
{
    FML1_ClrFerContPrsnt(FaultPath_En);
    FML1_ClrNplContPrsnt(FaultPath_En);
    FML1_ClrSigContPrsnt(FaultPath_En);
    FML1_ClrMinContPrsnt(FaultPath_En);
    FML1_ClrMaxContPrsnt(FaultPath_En);
}

/*
 Function to set continously present status of functional error
*/
void FML1_SetFerContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (!FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StFerCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.FerCp = TRUE;
    }

    FML1_ClrNplContPrsnt(FaultPath_En);
    FML1_ClrSigContPrsnt(FaultPath_En);
    FML1_ClrMinContPrsnt(FaultPath_En);
    FML1_ClrMaxContPrsnt(FaultPath_En);
}

/*
 Function to set continously present status of NPL error
*/
void FML1_SetNplContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (!FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StNplCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.NplCp = TRUE;
    }

    FML1_ClrFerContPrsnt(FaultPath_En);
    FML1_ClrSigContPrsnt(FaultPath_En);
    FML1_ClrMinContPrsnt(FaultPath_En);
    FML1_ClrMaxContPrsnt(FaultPath_En);
}

/*
 Function to set continously present status of sig error
*/
void FML1_SetSigContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (!FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StSigCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.SigCp = TRUE;
    }

    FML1_ClrFerContPrsnt(FaultPath_En);
    FML1_ClrNplContPrsnt(FaultPath_En);
    FML1_ClrMinContPrsnt(FaultPath_En);
    FML1_ClrMaxContPrsnt(FaultPath_En);
}

/*
 Function to set continously present status of min error
*/
void FML1_SetMinContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (!FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StMinCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.MinCp = TRUE;
    }

    FML1_ClrFerContPrsnt(FaultPath_En);
    FML1_ClrNplContPrsnt(FaultPath_En);
    FML1_ClrSigContPrsnt(FaultPath_En);
    FML1_ClrMaxContPrsnt(FaultPath_En);
}

/*
 Function to set continously present status of max error
*/
void FML1_SetMaxContPrsnt(u16_FaultPath_t FaultPath_En)
{
    if (!FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.StMaxCp)
    {
        FML1_FltMonRam_St.FltCp_aSt[FaultPath_En].bf.MaxCp = TRUE;
    }

    FML1_ClrFerContPrsnt(FaultPath_En);
    FML1_ClrNplContPrsnt(FaultPath_En);
    FML1_ClrSigContPrsnt(FaultPath_En);
    FML1_ClrMinContPrsnt(FaultPath_En);
}

/**
 *    @brief   Function to read the Test failed status.
 *
 *    @param   FaultPath_En Fualtpath acts as index for each DTC.
 *    @return  None.
 */
bool FML1_GetTestFailedStatus(u16_FaultPath_t FaultPath_En)
{
    return FML1_GET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/**
 *    @brief   Function to read the Test failed this operation cycle status.
 *
 *    @param   FaultPath_En Fualtpath acts as index for each DTC.
 *    @return  None.
 */
bool FML1_GetTestFailedThisOCStatus(u16_FaultPath_t FaultPath_En)
{
    return FML1_GET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/**
 *    @brief   Function to read the Test not completed since last clear status.
 *
 *    @param   FaultPath_En Fualtpath acts as index for each DTC.
 *    @return  None.
 */
bool FML1_GetTestNotCompletedSLCStatus(u16_FaultPath_t FaultPath_En)
{
    return FML1_GET_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/**
 *    @brief   Function to read the Test failed since last clear status.
 *
 *    @param   FaultPath_En Fualtpath acts as index for each DTC.
 *    @return  None.
 */
bool FML1_GetTestFailedSLCStatus(u16_FaultPath_t FaultPath_En)
{
    return FML1_GET_TFSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/**
 *    @brief   Function to read the Test not completed this operation cycle status.
 *
 *    @param   FaultPath_En Fualtpath acts as index for each DTC.
 *    @return  None.
 */
bool FML1_GetTestNotCompletedThisOCStatus(u16_FaultPath_t FaultPath_En)
{
    return FML1_GET_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/**
 *    @brief   Function to read the Warning Indicator Requested status.
 *
 *    @param   FaultPath_En Fualtpath acts as index for each DTC.
 *    @return  None.
 */
bool FML1_GetWIRStatus(u16_FaultPath_t FaultPath_En)
{
    return FML1_GET_WIR_STATUS_OF_DTC_MAPTBL(FaultPath_En);
}

/**
 *    @brief  The function to report application specific faults to the fault
 *            manager.This API is provided to the application layer.
 *
 *    @param  FaultPath_En - Application specific Fault Paths as configured
 *            by the application user in Fault manager.
 *
 *    @param  ErrorType_En   - Error catgory of the fault (E.g. MAX, MIN,
 *            SIG,etc) Refer to FM_ErrorType_En_t table for proper parameter.
 *
 *    @return None.
 *    @code tag - @[CODE_FML1_REPORTFLT]@{SDD_FML1_REPORTFLT}
 */
void FML1_ReportFault(u16_FaultPath_t FltPath, uint8_t ErrorType_u8)
{
    int16_t deb = 0;
    int16_t debReal = 0;
    // ErrFlagStatus_au8[FltPath] =  FaultPath_aSt[FltPath].L1StReg_Uni.Reg_St.Erf;
    //   Checking_erf_u32 = FaultPath_aSt[2].L1StReg_Uni.Reg_St.Erf;
    //   Checking_erf1_u32 = FaultPath_aSt[12].L1StReg_Uni.Reg_St.Erf;
    //  uint8_t FrzFrmCnt_u8 = 0;
    /*******************************************
    Debounce stratgey for level-1
    ********************************************/
    switch (ErrorType_u8)
    {
    /*******************************************
    Debouncing for Readiness flag
    ********************************************/
    case FM_NO_ERR:
    {
        /* READINESS CHECK */

        // Check if the Readiness check is already done for this fault.
        if (!FML1_GetRdy(FltPath))
        {
            /* READINESS CHECK IS NOT YET OVER */
            if ((FML1_DEBRDY_E == FML1_GetDebStatus(FltPath)) ||
                (FML1_DEBHEAL_E == FML1_GetDebStatus(FltPath)) ||
                (!GET_DEBREADY_CTR(FltPath)))
            {
                /* DEBOUNCING PART */
                if (GET_DEBREADY_CTR(FltPath) <= FML1_GetDebCnt(FltPath))
                {
                    /* DEBOUNCING IS OVER */

                    // Change the state to "Readiness complete"
                    FML1_SetDebStatus(FltPath, FML1_RDYCMPLT_E);

                    // Set the readines flag to TRUE
                    FML1_SetRdy(FltPath);

                    // Update the readiness matrix
                    FML1_Update_RdyMatrix(FltPath);

                    // Update the flag saying Readyness update to NV Memory
                    FML1_SetRdyChanged();

                    // Set the "test flag" to indicate the completion of the test in the current driving cycle
                    FML1_SetTst(FltPath);

                    // Reset the debounce counter to zero
                    FML1_ResetDebCnt(FltPath);

                    // No faults associated with this fault path.
                    // So reset the continously present flag for this fault
                    FML1_ClrAllContPrsnt((u16_FaultPath_t)FltPath);

                    // Clear the other error flags
                    FML1_ClrAllErr((u16_FaultPath_t)FltPath);

                    // updated the status
                    FML1_SetUpt_St(FltPath);
                }
                else
                {
                    /* DEBOUNCING IS IN PROGRESS */

                    // Change the state to as "Debounce to Readiness"
                    FML1_SetDebStatus(FltPath, FML1_DEBRDY_E);

                    // Increment the debounce counter by 1.
                    FML1_IncDebCnt(FltPath);
                }
            }
            else
            {
                /* FIRST OCCURANCE */

                // Change the state to as "Debounce to Ready"
                FML1_SetDebStatus(FltPath, FML1_DEBRDY_E);

                // Reset the debounce counter to zero
                FML1_StartDebCnt(FltPath);

                // Start decrementing the counter
                // FML1_DecDebCnt(FltPath);
            }
        }

        /*******************************************
         Debouncing of Healing
        ********************************************/

        // Check if the error is present in the system or not.
        if (FML1_GetErf(FltPath))
        {
            /* ERROR DOES EXIST - So DEBOUNCING TO HEAL IS REQUIRED */
            // Check if the debouncing has already begun for Healing
            if ((FML1_DEBRDY_E == FML1_GetDebStatus(FltPath)) ||
                (FML1_DEBHEAL_E == FML1_GetDebStatus(FltPath)) ||
                (!GET_DEBHEAL_CTR(FltPath)))
            {
                /* DEBOUNCING PART */

                deb = GET_DEBHEAL_CTR(FltPath);
                debReal = FML1_GetDebCnt(FltPath);
                // Check for the debouncing completion
                if (deb >= debReal)
                {
                    /* DEBOUNCING IS OVER */

                    // Change the state to "HEALED"
                    FML1_SetDebStatus(FltPath, FML1_HEALED_E);

                    // Clear the other error flags
                    FML1_ClrAllErr((u16_FaultPath_t)FltPath);

                    // Set the "test flag" to indicate the completion of the test in the current driving cycle
                    FML1_SetTst(FltPath);

                    // Reset the debounce counter to zero
                    // FML1_ResetDebCnt(FltPath);
                    // Set the FDC count to unmatured state(-128) as per the graph in ISO14229-1 2013 page 379.
                    FML1_SetDebCnt(FltPath, GET_DEBHEAL_CTR(FltPath));

                    // No faults associated with this fault path.
                    // So reset the continously present flag for this fault
                    FML1_ClrAllContPrsnt((u16_FaultPath_t)FltPath);

                    // updated the status
                    FML1_SetUpt_St(FltPath);
                }
                else
                {
                    /* DEBOUNCING IS IN PROGRESS */

                    // Change the state to as "Debounce to Heal"
                    FML1_SetDebStatus(FltPath, FML1_DEBHEAL_E);

                    // Decrement the debounce counter by 1.
                    FML1_DecDebCnt(FltPath);
                }
            }
            else
            {
                /* FIRST OCCURANCE */

                // Change the state to as "Debounce to Heal "
                FML1_SetDebStatus(FltPath, FML1_DEBHEAL_E);

                // Start the debounce counter with Init value (1).
                // FML1_StartDebCnt(FltPath);
                // Start decrementing the counter
                FML1_DecDebCnt(FltPath);
            }

            /*Set Actual debounce count*/
            FML1_SetActDebCnt(FltPath, FML1_GetDebCnt(FltPath));
        }
        else
        {
            /* FIRST OCCURANCE */
            // Change the state to as "Debounce to Heal "
            FML1_SetDebStatus(FltPath, FML1_DEBHEAL_E);

            // Start the debounce counter with Init value (1).
            FML1_StartDebCnt(FltPath);
            // Start decrementing the counter
            // FML1_DecDebCnt(FltPath);
        }
        break;
    }
    case FM_MAX_ERR:
    {
        FML1_Report_Error_Fault(ErrorType_u8, FltPath);
        break;
    }
    case FM_MIN_ERR:
    {
        FML1_Report_Error_Fault(ErrorType_u8, FltPath);
        break;
    }
    case FM_SIG_ERR:
    {
        FML1_Report_Error_Fault(ErrorType_u8, FltPath);
        break;
    }
    case FM_NPL_ERR:
    {
        FML1_Report_Error_Fault(ErrorType_u8, FltPath);
        break;
    }
    case FM_FER_ERR:
    {
        FML1_Report_Error_Fault(ErrorType_u8, FltPath);
        break;
    }
    default:
    {
        /* Do nothing */
        break;
    }
    }

    return;
}

static void FML1_Report_Error_Fault(uint8_t ErrorType_u8, u16_FaultPath_t FltPath)
{
    int32_t j = 0; // loop count
    int32_t i = 0; // loop count
    uint8_t errotIndex = 0;
    uint8_t error_u8 = 0;
    bool FML1_GetErrStatus_b = true;

    /* RESET Debounce counter if debounce to heal counter is active */
    if ((FML1_DEBHEAL_E == FML1_GetDebStatus(FltPath)))
    {
        FML1_StartDebCnt(FltPath);
    }
    error_u8 = FML1_GetErrType(FltPath);
    for (errotIndex = 0; errotIndex < 5; errotIndex++)
    {
        if (((error_u8 >> errotIndex) == 0) && (1 << (7 - errotIndex) == ErrorType_u8))
        {
            if (GET_SUPPORTEDFAULT(FltPath) & ErrorType_u8)
            {
                FML1_GetErrStatus_b = false;
                break;
            }
        }
    }

    if (false == FML1_GetErrStatus_b)
    {
        /* ERROR TYPE DOES NOT EXIST */

        /* Check if the debouncing has already begun for MAX Error type */ 
        if ((FML1_DEBMAXDFCT_E == FML1_GetDebStatus(FltPath)) || (FML1_DEBMINDFCT_E == FML1_GetDebStatus(FltPath)) || (FML1_DEBSIGDFCT_E == FML1_GetDebStatus(FltPath)) ||
        (FML1_DEBNPLDFCT_E == FML1_GetDebStatus(FltPath)) || (FML1_DEBFERDFCT_E == FML1_GetDebStatus(FltPath)) || (!GET_DEBDEFECT_CTR(FltPath)))
        {
            /* DEBOUNCING PART */

            // Check for the debouncing completion
            if (GET_DEBDEFECT_CTR(FltPath) <= FML1_GetDebCnt(FltPath))
            {
                /* DEBOUNCING IS OVER */

                // Capture freeze frame for OBD or NONOBD specific
                // capture the freeze frame (snap shot data)
                uint32_t indexBuff_u32 = 0;
                uint16_t mffIndex_u16 = 0;
                int16_t NumofLocalEntries_s16 = 0;
                int16_t paramSizeFrzFrm_s16 = 0;
                (void)mffIndex_u16;
#if (TRUE == FM_UDS_SUPPORTED)
                // first capture the Global snapshot data.
                for (i = 0; i < FMFF_CONF_UDS_GLBSS_ENTRIES; i++)
                {
                    FreezeFrame_DataLog(FM_Global_Snapshot_UDS_au16[i], &FaultPath_aSt[FltPath].L1FrzFrm_UDS_au8[indexBuff_u32], &indexBuff_u32);

                } /*FM:Jeevan*/
                // then capture the local snapshot data (DTC specific)
                // for (mffIndex_u16 = 0; mffIndex_u16 < NUM_OF_FAULTPATHS_E; mffIndex_u16++)
                //  {
                NumofLocalEntries_s16 = FM_FrzFrmMasterConfig_UDS_aSt[FltPath].NumOfEntries_u16;
                for (i = 0; i < NumofLocalEntries_s16; i++)
                {
                    FreezeFrame_DataLog(FM_FrzFrmMasterConfig_UDS_aSt[FltPath].FM_FrzFrm_LocalEntries[i], &FaultPath_aSt[FltPath].L1FrzFrm_UDS_au8[indexBuff_u32], &indexBuff_u32);
                }
                // }
#endif // FM_UDS_SUPPORTED

#if (TRUE == FM_OBD_SUPPORTED)

                indexBuff_u32 = 0;
                // first capture the Global snapshot data.
                for (i = 0; i < FMFF_CONF_OBD_GLBSS_ENTRIES; i++)
                {
                    paramSizeFrzFrm_s16 = FM_Global_Snapshot_OBD_aSt[i].ParamSize_u16;
                    for (j = 0; j < paramSizeFrzFrm_s16; j++)
                    {
                        FaultPath_aSt[FltPath].L1FrzFrm_OBD_au8[indexBuff_u32] = FM_Global_Snapshot_OBD_aSt[i].ParamBuffer_pu8[j];
                        indexBuff_u32++;
                    }
                }
                // then capture the local snapshot data (DTC specific)
                for (mffIndex_u16 = 0; mffIndex_u16 < NUM_OF_FAULTPATHS_E; mffIndex_u16++)
                {
                    NumofLocalEntries_s16 = FM_FrzFrmMasterConfig_OBD_aSt[mffIndex_u16].NumOfEntries_u16;
                    for (i = 0; i < NumofLocalEntries_s16; i++)
                    {
                        paramSizeFrzFrm_s16 = FM_FrzFrmMasterConfig_OBD_aSt[mffIndex_u16].FrzFrm_Data_pSt[i].ParamSize_u16;
                        for (j = 0; j < paramSizeFrzFrm_s16; j++)
                        {
                            FaultPath_aSt[FltPath].L1FrzFrm_OBD_au8[indexBuff_u32] = FM_FrzFrmMasterConfig_OBD_aSt[mffIndex_u16].FrzFrm_Data_pSt[i].ParamBuffer_pu8[j];
                            indexBuff_u32++;
                        }
                    }
                }
#endif // FM_OBD_SUPPORTED

#if (TRUE == FM_J1939_SUPPORTED)
////////////// To do : For this project freeze frames are not required. So commented this for J1939.
                // indexBuff_u32 = 0;
                // // first capture the Global snapshot data.
                // for (i = 0; i < FMFF_CONF_J1939_GLBSS_ENTRIES; i++)
                // {
                //     paramSizeFrzFrm_s16 = FM_Global_Snapshot_J1939_aSt[i].ParamSize_u16;
                //     for (j = 0; j < paramSizeFrzFrm_s16; j++)
                //     {
                //         FaultPath_aSt[FltPath].L1FrzFrm_J1939_au8[indexBuff_u32] = FM_Global_Snapshot_J1939_aSt[i].ParamBuffer_pu8[j];
                //         indexBuff_u32++;
                //     }
                // }
                // // then capture the local snapshot data (DTC specific)
                // for (mffIndex_u16 = 0; mffIndex_u16 < NUM_OF_FAULTPATHS_E; mffIndex_u16++)
                // {
                //     NumofLocalEntries_s16 = FM_FrzFrmMasterConfig_J1939_aSt[mffIndex_u16].NumOfEntries_u16;
                //     for (i = 0; i < NumofLocalEntries_s16; i++)
                //     {
                //         paramSizeFrzFrm_s16 = FM_FrzFrmMasterConfig_J1939_aSt[mffIndex_u16].FrzFrm_Data_pSt[i].ParamSize_u16;
                //         for (j = 0; j < paramSizeFrzFrm_s16; j++)
                //         {
                //             FaultPath_aSt[FltPath].L1FrzFrm_J1939_au8[indexBuff_u32] = FM_FrzFrmMasterConfig_J1939_aSt[mffIndex_u16].FrzFrm_Data_pSt[i].ParamBuffer_pu8[j];
                //             indexBuff_u32++;
                //         }
                //     }
                // }
#endif // FM_J1939_SUPPORTED

                FML1_FrzFrmUpdated_b = TRUE;
                // FML1_SET_TFTOC_STATUS_OF_DTC_MAPTBL(FltPath);
                //  Change the state to "DEBOUNCE TO MAX DEFECT"
                FML1_SetDebStatus(FltPath, FML1_DFCTCNFRMD_E);
                // Set the "test flag" to indicate the completion of the test in the current driving cycle
                // FML1_SetTst(FltPath);
                // Reset the debounce counter to zero
                // FML1_ResetDebCnt(FltPath);
                // Set the FDC count to matured state(127) as per the graph in ISO14229-1 2013 page 379.
                FML1_SetDebCnt(FltPath, GET_DEBDEFECT_CTR(FltPath));
                // Max fault is confirmed. This means other faults are not 
                // continuously present. So reset the other fault flags. 
//                for(i=0;i<5;i++)
//                {
//                    if(FM_Report_ast[i].errortype_u8 == ErrorType_u8)
//                    {
//                        FM_Report_ast[i].SetContPrsnt((u16_FaultPath_t)FltPath);
//                        FM_Report_ast[i].Set_Error((u16_FaultPath_t)FltPath);
//                    }
//                }
                switch(ErrorType_u8)
                {
                    case FM_MAX_ERR:
                    {
                    	FML1_SetMaxContPrsnt((u16_FaultPath_t)FltPath);
                    	FML1_SetMaxErr((u16_FaultPath_t)FltPath);
                        break;
                    }
                    case FM_MIN_ERR:
                    {
                    	FML1_SetMinContPrsnt((u16_FaultPath_t)FltPath);
                    	FML1_SetMinErr((u16_FaultPath_t)FltPath);
                        break;
                    }
                    case FM_SIG_ERR:
                    {
                    	FML1_SetSigContPrsnt((u16_FaultPath_t)FltPath);
                    	FML1_SetSigErr((u16_FaultPath_t)FltPath);
                        break;
                    }
                    case FM_NPL_ERR:
                    {
                    	FML1_SetNplContPrsnt((u16_FaultPath_t)FltPath);
                    	FML1_SetNplErr((u16_FaultPath_t)FltPath);
                        break;
                    }
                    case FM_FER_ERR:
                    {
                    	FML1_SetFerContPrsnt((u16_FaultPath_t)FltPath);
                    	FML1_SetFerErr((u16_FaultPath_t)FltPath);
                        break;
                    }
                    default:
                    {
                         /* Do nothing */
                         break;
                    }
                }

                // Check for DTC clear flags for L1 and update the first Fault since last DTC clear
                if (TRUE == FM_FirstFailedFltsInfo_St.DTCClearedIndicator[DTCCLR_IND_FOR_ACT_FLT])
                {
                    // Clear the corresponding flag
                    FM_FirstFailedFltsInfo_St.DTCClearedIndicator[DTCCLR_IND_FOR_ACT_FLT] = FALSE;
                    // get the DTC from the DTC table
                    FM_FirstFailedFltsInfo_St.ActiveDTC_u32 = GetDTCofFltPathFltType(FltPath, ErrorType_u8, FM_PROTO_UDS_E);
                }
            }
            else
            {
            	switch(ErrorType_u8)
            	{
            		case FM_MAX_ERR:
            		{
            			/* DEBOUNCING IS IN PROGRESS */

            			// Change the state to as "Debounce to MAX Defect"
            			FML1_SetDebStatus(FltPath, FML1_DEBMAXDFCT_E);
            			// Increment the debounce counter by 1.
            			FML1_IncDebCnt(FltPath);
            			break;
            		}
            		case FM_MIN_ERR:
            		{
            			/* DEBOUNCING IS IN PROGRESS */

            		    // Change the state to as "Debounce to MAX Defect"
            			FML1_SetDebStatus(FltPath, FML1_DEBMINDFCT_E);
            			// Increment the debounce counter by 1.
            			FML1_IncDebCnt(FltPath);
            			break;
            		}
            		case FM_SIG_ERR:
            		{
            		    /* DEBOUNCING IS IN PROGRESS */

            		    // Change the state to as "Debounce to MAX Defect"
            		    FML1_SetDebStatus(FltPath, FML1_DEBSIGDFCT_E);
            		    // Increment the debounce counter by 1.
            		    FML1_IncDebCnt(FltPath);
            		    break;
            		}
            		case FM_NPL_ERR:
            		{
            		    /* DEBOUNCING IS IN PROGRESS */

            		    // Change the state to as "Debounce to MAX Defect"
            		    FML1_SetDebStatus(FltPath, FML1_DEBNPLDFCT_E);
            		    // Increment the debounce counter by 1.
            		    FML1_IncDebCnt(FltPath);
            		    break;
            		}
            		case FM_FER_ERR:
            		{
            		    /* DEBOUNCING IS IN PROGRESS */

            		    // Change the state to as "Debounce to MAX Defect"
            		    FML1_SetDebStatus(FltPath, FML1_DEBFERDFCT_E);
            		    // Increment the debounce counter by 1.
            		    FML1_IncDebCnt(FltPath);
            		    break;
            		}
            		default:
            		{
            		     /* Do nothing */
            		     break;
            		}
            	}

            }
       }
       else
       {

       	switch(ErrorType_u8)
       	{
       		case FM_MAX_ERR:
       		{
       			/* FIRST OCCURANCE */
       			// Change the state to as "Debounce to MAX Defect "
       			FML1_SetDebStatus(FltPath, FML1_DEBMAXDFCT_E);

       			// Start the debounce counter with Init value (1).
       			FML1_StartDebCnt(FltPath);
       			break;
       		}
       		case FM_MIN_ERR:
       		{
       			/* FIRST OCCURANCE */
       			// Change the state to as "Debounce to MIN Defect "
       			FML1_SetDebStatus(FltPath, FML1_DEBMINDFCT_E);

       			// Start the debounce counter with Init value (1).
       			FML1_StartDebCnt(FltPath);
       			break;
       		}
       		case FM_SIG_ERR:
       		{
       			/* FIRST OCCURANCE */
       			// Change the state to as "Debounce to SIG Defect "
       			FML1_SetDebStatus(FltPath, FML1_DEBSIGDFCT_E);

       			// Start the debounce counter with Init value (1).
       			FML1_StartDebCnt(FltPath);
       			break;
       		}
       		case FM_NPL_ERR:
       		{
       			/* FIRST OCCURANCE */
       			// Change the state to as "Debounce to NPL Defect "
       			FML1_SetDebStatus(FltPath, FML1_DEBNPLDFCT_E);

       			// Start the debounce counter with Init value (1).
       			FML1_StartDebCnt(FltPath);
       			break;
       		}
       		case FM_FER_ERR:
       		{
       			/* FIRST OCCURANCE */
       			// Change the state to as "Debounce to NPL Defect "
       			FML1_SetDebStatus(FltPath, FML1_DEBFERDFCT_E);

       			// Start the debounce counter with Init value (1).
       			FML1_StartDebCnt(FltPath);
       			break;
       		}
       		default:
       		{
       		     /* Do nothing */
       		     break;
       		}
       	 }
       }
    } 
}

/**
 *    @brief   Handle readiness Task. This task looks for the readiness status
 *             from all the faults and updates the monitoring group flag.
 *
 *             1:The task looks at the debounced readiness flag from each of the
 *               fault path
 *             2:If all the readiness flags associated to a monitoring group is
 *               indicating "complete" (i.e. logic HIGH), then result for the
 *               corresponding group will report to the tester as "Complete" when
 *               requested
 *
 *    @param   None.
 *    @return  None.
 *    @code tag - @[CODE_FML1_HANDLERDY]@{SDD_FML1_HANDLERDY}
 */
void FML1_HndlRdy(void)
{
    u16_FaultPath_t FltPath = 0;
    bool MonitorFuelGrp_b = TRUE;
    bool MonitorEGRGrp_b = TRUE;
    bool MonitorCCompGrp_b = TRUE;

    /*
        Check the readiness for the different groups
    */
    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
        {
            switch (FML1_GetRdyGroup(FltPath))
            {
            /*   READINESS GROUP FOR FUEL SYSTEM
             */
            case RDY_FUEL_GRP:
            {
                if (MonitorFuelGrp_b)
                {
                    MonitorFuelGrp_b &= FML1_GetRdy(FltPath);
                }
                break;
            }

            /*   READINESS GROUP FOR EGR SYSTEM
             */
            case RDY_EGR_GRP:
            {
                if (MonitorEGRGrp_b)
                {
                    MonitorEGRGrp_b &= FML1_GetRdy(FltPath);
                }
                break;
            }

            /*   READINESS GROUP FOR COMPREHENSIVE COMPONENT SYSTEM
             */
            case RDY_CCOMP_GRP:
            {
                if (MonitorCCompGrp_b)
                {
                    MonitorCCompGrp_b &= FML1_GetRdy(FltPath);
                }
                break;
            }

            default:
            {
                /* Do nothing */
                break;
            }
            }
        }
    }

    /*
        End of for loop. Update the global readiness
        flags using the monitoring flags status.
    */
    if (MonitorFuelGrp_b)
    {
        // Set the readiness flag of FUEL System group
        FML1_SETRdyCmpltFUELSys();
    }
    if (MonitorEGRGrp_b)
    {
        // Set the readiness flag of EGR System group
        FML1_SETRdyCmpltEGRSys();
    }
    if (MonitorCCompGrp_b)
    {
        // Set the readiness flag of Comprehensive component System group
        FML1_SETRdyCmpltCCOMPSys();
    }

    return;
}

/**
 *    @brief   Returns the status of readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FML1_DERIVED_ISRDY_COMPLETE]@{SDD_FML1_DERIVED_ISRDY_COMPLETE}
 */
bool FML1_IsRdyComplete(void)
{
    // updating the readiness status
    FML1_HndlRdy();

    if (FML1_GETRdyCmpltFUELSys() && FML1_GETRdyCmpltEGRSys() && FML1_GETRdyCmpltCCOMPSys())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 *    @brief   Returns the status of Fuel system readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FML1_DERIVED_ISFUEL_SYSRDY_COMPLETE]@{SDD_FML1_DERIVED_ISFUEL_SYSRDY_COMPLETE}
 */
bool FML1_IsFUELSysRdyComplete(void)
{
    // updating the readiness status
    FML1_HndlRdy();

    if (FML1_GETRdyCmpltFUELSys())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 *    @brief   Returns the status of EGR system readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FML1_DERIVED_ISEGR_SYSRDY_COMPLETE]@{SDD_FML1_DERIVED_ISEGR_SYSRDY_COMPLETE}
 */
bool FML1_IsEGRSysRdyComplete(void)
{
    // updating the readiness status
    FML1_HndlRdy();

    if (FML1_GETRdyCmpltEGRSys())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 *    @brief   Returns the status of CCOMP system readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FML1_DERIVED_ISCCOMP_SYSRDY_COMPLETE]@{SDD_FML1_DERIVED_ISCCOMP_SYSRDY_COMPLETE}
 */
bool FML1_IsCCOPMSysRdyComplete(void)
{
    // updating the readiness status
    FML1_HndlRdy();

    if (FML1_GETRdyCmpltCCOMPSys())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 *    @brief   Update the NV memory with the readiness monitoring test results
 *             of the current driving cycle during power off
 *
 *    @param   None.
 *    @return  None.
 *    @code tag - @[CODE_FML1_RDYPWROFF]@{SDD_FML1_RDYPWROFF}
 */
void FML1_Rdy_pwroff(void)
{
    // update readiness matrix to flash
    FML1_UpdtNVMem_proc();

    return;
}

/**
 *    @brief   Update the NV memory with the readiness monitoring test results
 *             of the current driving cycle.
 *
 *    @param   None.
 *    @return  None.
 *    @code tag - @[CODE_FML1_UPT_NVMEM]@{SDD_FML1_UPT_NVMEM}
 */
void FML1_UpdtNVMem_proc(void)
{
    // Store the Readiness monitoring results into NV memory if
    // there are some updates in the current driving cycle.
    /*FF variable validation*/ // ankit
                               //    BCSPressureFF_au8[0] = (uint8_t)BcsInp_St.Pressure_f;
                               //    AmbTempFF_as8[0] = BcsInp_St.Ambient_Temp_s8;
                               //    Cool_InFF_as8[0] = BcsInp_St.Coolant_In_Temp_s8;
                               //    Cool_OutFF_as8[0] = BcsInp_St.Coolant_Out_Temp_s8;

    if (FML1_IsRdyChanged())
    {
        FEE_Write(FM_RDYRESULTS, (uint8_t *)&FML1_FltMonRam_St.RdyMonMat_au8[0]);
        // FDL_write(FEE_AddrConfig_aSt[FM_RDYRESULTS].StartAddress_u32, FEE_AddrConfig_aSt[FM_RDYRESULTS].Size_u16, (uint8_t*)&FML1_FltMonRam_St.RdyMonMat_au8[0]);
        // There is no updates to be copied in to NV memory
        FML1_ClrRdyChanged();
    }

    FEE_Write(FM_TFSLC, (uint8_t *)&FML1_FltMonRam_St.TFSLC_Mat_au8[0]);
    //  FDL_write(FEE_AddrConfig_aSt[FM_TFSLC].StartAddress_u32, FEE_AddrConfig_aSt[FM_TFSLC].Size_u16, (uint8_t*)&FML1_FltMonRam_St.TFSLC_Mat_au8[0]);

    return;
}

/**
 *    @brief   Function to copy the configuration parameter from the config
 *             table to the status register.
 *
 *             Initialization function for Fault manager level-1.
 *
 *             1:Initialize the readiness-monitoring matrix from the NV memory
 *
 *    @param   None
 *    @return  None
 *    @code tag - @[CODE_FML1_INIT]@{SDD_FML1_INIT}
 */
void FML1_Init(void)
{
    u16_FaultPath_t FltPath = 0;

    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        // Initialising all the error in the L1 data structure with false
        FML1_ClrStReg(FltPath);

        // copy priority from configuration structure to L1 data structure
        FML1_SetPriority(FltPath, GET_FAULT_PRIORITY(FltPath));

        // Reset the debounce count to zero
        FML1_ResetDebCnt(FltPath);

        // Set all the flags of the fault in the continuously monitored
        FML1_RSTContPrsnt((u16_FaultPath_t)FltPath);

        FML1_CLR_TF_STATUS_OF_DTC_MAPTBL(FltPath);
        FML1_CLR_TFTOC_STATUS_OF_DTC_MAPTBL(FltPath);
        FML1_CLR_PDTC_STATUS_OF_DTC_MAPTBL(FltPath);
        FML1_CLR_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
        FML1_CLR_WIR_STATUS_OF_DTC_MAPTBL(FltPath);
        FML1_SET_TNCSLC_STATUS_OF_DTC_MAPTBL(FltPath);
        // FML1_CLR_TFSLC_STATUS_OF_DTC_MAPTBL(FltPath);
        //  set the flag "Test not completed in this operating cycle).
        FML1_SET_TNCTOC_STATUS_OF_DTC_MAPTBL(FltPath);
        // Reset the flag "Warning indicator requested.
        // This flag is for OEM specific faults, generally set=0 for non-crucial ECUs.
        FML1_CLR_WIR_STATUS_OF_DTC_MAPTBL(FltPath);
    }

    // Copy the readiness monitoring test results from NV memory to the
    // L1 status register
    FML1_RdyInit();

    // Function which updates the variable holds the readiness status
    FML1_HndlRdy();

    return;
}

/**
 *    @brief  Initialization of paramaters for Readiness monitoring task.
 *            Copy the readiness monitoring test results from NV memory
 *            to the L1 status register.
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FML1_RDYINIT]@{SDD_FML1_RDYINIT}
 */
static void FML1_RdyInit(void)
{
    uint8_t RdyByt_u8 = 0;
    u16_FaultPath_t FltPath = 0;
    uint16_t RdyBuffIndx_u16 = 0;
    uint8_t RdyBuffOffst_u8 = 0;

    // clear all readiness monitoring flags
    FML1_ClrRdyMonFlags();

    // There is no updates to be copied in to NV memory
    FML1_ClrRdyChanged();

    // Read the readiness monitoring test results from the NV memory
    FEE_Read(FM_RDYRESULTS, (uint8_t *)&FML1_FltMonRam_St.RdyMonMat_au8[0]);
    // FDL_Read(FEE_AddrConfig_aSt[FM_RDYRESULTS].StartAddress_u32, FEE_AddrConfig_aSt[FM_RDYRESULTS].Size_u16,(uint8_t*)&FML1_FltMonRam_St.RdyMonMat_au8[0]);
    // Read the TestFailedSinceLastClear test results from the NV memory
    FEE_Read(FM_TFSLC, (uint8_t *)&FML1_FltMonRam_St.TFSLC_Mat_au8[0]);
    //  FDL_Read(FEE_AddrConfig_aSt[FM_TFSLC].StartAddress_u32, FEE_AddrConfig_aSt[FM_TFSLC].Size_u16,(uint8_t*)&FML1_FltMonRam_St.TFSLC_Mat_au8[0]);
    //  Update the "Rdy" flag inside the L1 status registers for all the faults
    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        RdyBuffIndx_u16 = (uint16_t)(FltPath / 8);
        RdyBuffOffst_u8 = (uint8_t)(FltPath % 8);

        RdyByt_u8 = FML1_FltMonRam_St.RdyMonMat_au8[RdyBuffIndx_u16];

        if (0x01 & (RdyByt_u8 >> RdyBuffOffst_u8))
        {
            FML1_SetRdy(FltPath);
            FML1_CLR_TNCSLC_STATUS_OF_DTC_MAPTBL(FltPath);
        }
        else
        {
            FML1_ClrRdy(FltPath);
            FML1_SET_TNCSLC_STATUS_OF_DTC_MAPTBL(FltPath);
        }

        // Readiness group is updated from configuraion structure
        FML1_SetReadyGroup(FltPath, GET_FAULT_RDYGRP(FltPath));
    }

    return;
}

/**
 *    @brief  Clears the readiness completion variable. Also clears the
 *            readiness monitoring test results in NV memory
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FML1_CLRRDY]@{SDD_FML1_CLRRDY}
 */
void FML1_ClrRdyMonFlags(void)
{
    u16_FaultPath_t FltPath = 0;
    uint16_t count_u16 = 0;

    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        // Clear the Readiness bit in FM Level-1 data structure
        FML1_ClrRdy(FltPath);
        FML1_SET_TNCSLC_STATUS_OF_DTC_MAPTBL(FltPath);
    }

    // Clear the variable holding the readiness status
    FML1_CLRRdyCmpltFUELSys();
    FML1_CLRRdyCmpltEGRSys();
    FML1_CLRRdyCmpltCCOMPSys();

    // Clear the readiness matrix in the RAM
    for (count_u16 = 0; count_u16 < SIZE_OF_RDY_MON_MAT; count_u16++)
    {
        FML1_FltMonRam_St.RdyMonMat_au8[count_u16] = 0;
    }

    // Update the flag saying Readyness update to NV Memory
    FML1_SetRdyChanged();

    return;
}

/**
 *    @brief  Function which sets the readiness matrix stored in
 *            RAM, which in turn will be moved to NV RAM
 *
 *    @param  FaultPath_En - Application specific Fault Paths as configured
 *            by the application user in Fault manager.
 *
 *    @return None.
 *    @code tag - @[CODE_FML1_DERIVED_UPDT_RDYMAT]@{SDD_FML1_DERIVED_UPDT_RDYMAT}
 */
static void FML1_Update_RdyMatrix(u16_FaultPath_t FltPath)
{
    uint16_t RdyBuffIndx_u16 = 0;
    uint8_t RdyBuffOffst_u8 = 0;

    RdyBuffIndx_u16 = (uint16_t)(FltPath / 8);
    RdyBuffOffst_u8 = (uint8_t)(FltPath % 8);

    FML1_FltMonRam_St.RdyMonMat_au8[RdyBuffIndx_u16] = (uint8_t)(FML1_FltMonRam_St.RdyMonMat_au8[RdyBuffIndx_u16] | (0x01 << RdyBuffOffst_u8));

    return;
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  Fault status
 *             - FM_NO_ERR
 *             - FM_MAX_ERR
 *             - FM_MIN_ERR
 *             - FM_SIG_ERR
 *             - FM_NPL_ERR
 *             - FM_FER_ERR
 *
 *    @code tag - @[CODE_FML1_DERIVED_GET_FLTST]@{SDD_FML1_DERIVED_GET_FLTST}
 */
uint8_t FML1_GetFltSt(u16_FaultPath_t FltPath)
{
    return (uint8_t)FML1_GetErrType(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FML1_DERIVED_GET_ERRST]@{SDD_FML1_DERIVED_GET_ERRST}
 */
bool FML1_GetErrSt(u16_FaultPath_t FltPath)
{
    return (bool)FML1_GetErf(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FML1_DERIVED_GET_MAXERR_FLTST]@{SDD_FML1_DERIVED_GET_MAXERR_FLTST}
 */
bool FML1_GetMaxErrFltSt(u16_FaultPath_t FltPath)
{
    return (bool)FML1_GetMaxErr(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FML1_DERIVED_GETMINERR_FLTST]@{SDD_FML1_DERIVED_GETMINERR_FLTST}
 */
bool FML1_GetMinErrFltSt(u16_FaultPath_t FltPath)
{
    return (bool)FML1_GetMinErr(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FML1_DERIVED_GET_SIGERR_FLTST]@{SDD_FML1_DERIVED_GET_SIGERR_FLTST}
 */
bool FML1_GetSigErrFltSt(u16_FaultPath_t FltPath)
{
    return (bool)FML1_GetSigErr(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FML1_DERIVED_GET_NPLERR_FLTST]@{SDD_FML1_DERIVED_GET_NPLERR_FLTST}
 */
bool FML1_GetNplErrFltSt(u16_FaultPath_t FltPath)
{
    return (bool)FML1_GetNplErr(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FML1_DERIVED_GET_FERERR_FLTST]@{SDD_FML1_DERIVED_GET_FERERR_FLTST}
 */
bool FML1_GetFerErrFltSt(u16_FaultPath_t FltPath)
{
    return (bool)FML1_GetFerErr(FltPath);
}

/**
 *    @brief  read the DTCs of the current/present faults in L1
 *    @param  pointer to DTC list, no_of_DTC to be read,
 *    @return No_of_faults
 *    @code tag - @[CODE_FML1_DERIVED_RDPRESENT_DTC]@{SDD_FML1_DERIVED_RDPRESENT_DTC}
 */
uint16_t _FML1_ReadDTCsOfPresentFaults(uint32_t *DTC_pu32, uint16_t ReadLimit_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    u16_FaultPath_t FltPath_En = 0;
    int32_t FltType_s32 = 0;
    uint16_t NumOfFaults_u16 = 0;

    if (DTC_pu32 && ReadLimit_u16 > 0)
    {
        // searching in L1 memory
        for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
        {
            FltType_s32 = FML1_GetErrType(FltPath_En);
            if (FM_NO_ERR != FltType_s32)
            {
                // get the DTC from the DTC table
                DTC_pu32[NumOfFaults_u16] = GetDTCofFltPathFltType(FltPath_En, (uint8_t)FltType_s32, FM_ProtocolType_En);
                NumOfFaults_u16++;
                if (NumOfFaults_u16 >= ReadLimit_u16)
                {
                    return NumOfFaults_u16;
                }
            }
        }
    }
    return NumOfFaults_u16;
}
/**
 *    @brief  Returns the number of current/present faults in L1
 *    @param  -
 *    @return No_of_faults
 */
uint16_t FML1_GetNumberOfPresentFaults(void)
{
    u16_FaultPath_t FltPath_En = 0;
    uint16_t NumOfFaults_u16 = 0;

    // searching in L1 memory
    for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
    {
        if (FM_NO_ERR != FML1_GetErrType(FltPath_En))
        {
            NumOfFaults_u16++;
        }
    }
    return NumOfFaults_u16;
}

int8_t FML1_Get_DTC(uint32_t *DTC_pu32, uint16_t ReadLimit_u16, FM_ProtocolType_En_t FM_ProtocolType_En, FM_Fault_Type_En_t FM_Fault_Type_En, FM_Fault_Condition_En_t FM_Fault_Condition_En)
{
    u16_FaultPath_t FltPath_En = 0;
    uint32_t DTC_u32 = 0u;
    int32_t FltType_s32 = 0;
    uint16_t NumOfFaults_u16 = 0;
    uint8_t OccurCnt_u8 = 0u;
    bool OBD_Relevent_flag_b = FALSE;
    uint8_t index_u8 = 0U;

    if (DTC_pu32 && ReadLimit_u16 > 0)
    {
        // searching in L1 memory
        for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
        {
            switch (FM_Fault_Type_En)
            {
            case FM_OBD_E:
                OBD_Relevent_flag_b = IS_FAULT_OBD_RELEVANT(FltPath_En);
                //           	OBD_Relevent_flag_b = TRUE;
                break;

            case FM_NONOBD_E:
                OBD_Relevent_flag_b = !(IS_FAULT_OBD_RELEVANT(FltPath_En));
                //        	OBD_Relevent_flag_b = FALSE;
                break;

            case FM_ALL_E:
                OBD_Relevent_flag_b = TRUE;
                break;

            default:
                OBD_Relevent_flag_b = FALSE;
                break;
            }

            FltType_s32 = FML1_GetErrType(FltPath_En);

            if ((FM_NO_ERR != FltType_s32) && (OBD_Relevent_flag_b))
            {
                // get the DTC from the DTC table
                DTC_u32 = GetDTCofFltPathFltType(FltPath_En, (uint8_t)FltType_s32, FM_ProtocolType_En);
                for(index_u8 = 0u; index_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; index_u8++)
                {
                    if(FltPath_En == FML2_GetFaultPath(index_u8))
                    {
                        OccurCnt_u8 = FML2_GetOccurCnt(index_u8);
                        break;
                    }
                }
                DTC_pu32[NumOfFaults_u16] = (DTC_u32 & (0x01FFFFFF | ((uint32_t)OccurCnt_u8 << 25U)));
                NumOfFaults_u16++;
                if (NumOfFaults_u16 >= ReadLimit_u16)
                {
                    return NumOfFaults_u16;
                }
            }
        }
    }
    return NumOfFaults_u16;
}
