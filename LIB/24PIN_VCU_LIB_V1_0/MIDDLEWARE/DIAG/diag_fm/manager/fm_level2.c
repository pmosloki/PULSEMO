/***************************************************************************************************
 *    FILENAME    :
 *
 *    DESCRIPTION : Contains all declaration and interfaces used for Diagnostics on CAN (DCAN)
 *
 *    $Id         : $
 *
 ***************************************************************************************************
 * Revision history
 *
 * Ver Author       Date       Description
 * 1             16/06/2008
 ***************************************************************************************************
 */

/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
#include "fmdtc_conf.h"
#include "fm.h"
#include "fm_level1.h"
#include "fm_level2.h"
#include "fee_conf.h"
#include "diag_typedefs.h"

/*
 ***************************************************************************************************
 *    Defines
 ***************************************************************************************************
 */

/*
 ***************************************************************************************************
 *    Static functions
 ***************************************************************************************************
 */
static void IncErrFreqCtr(uint8_t);
// static uint16_t GetOperMinWthoutErr(uint8_t);
// static void   RstOperMinWthoutErr(uint8_t);
void FML2_FltHndl_proc(void);
static void L2StateMachine(u16_FaultPath_t, uint8_t);
static void Register_L2Err(uint8_t, u16_FaultPath_t);
static void FindLowestPriorityErr(int8_t *, uint8_t *);
// static void   FindHighestPriorityErr(int8_t*, uint8_t*);
// static void   FindLowestPriorityOBDErr(int8_t*, uint8_t*);

static int8_t Find_Free_Slot(void);
static int8_t Find_Deleting_Slot(void);
static int8_t Find_Potential_Slot(void);
static int8_t MemorizeToL2(u16_FaultPath_t);
int16_t Copy_FrzFrz_From_L1_to_L2(u16_FaultPath_t FltPath, int8_t L2Index_au8);
bool IncCntState_ab[FM_CONF_MAX_L2FAULT_ENTRIES] = {false};
/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
 */

// To hold the state of MIL test
/*
@@ SYMBOL     = MIL_Test_Mode_u8
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint8_t$
@@ CONVERSION = TABLE 0 "TEST ON" 1 "TEST FIRST OFF" 2 "TEST READINESS OK" 3 "TEST READINESS NOT OK" 4 "TEST SECOND OFF"  5 "TEST DONE"
@@ DESCRIPTION= "MIL lamp test status"
@@ END
*/
uint8_t MIL_Test_Mode_u8 = MIL_TEST_ON;

/*
@@ SYMBOL     = FM_CEL_MIL_b
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint8_t$
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "CEL / MIL state"
@@ END
*/
bool FM_CEL_MIL_b = FALSE;

/*
@@ SYMBOL     = FM_GPL_b
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint8_t$
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "GPL state"
@@ END
*/
bool FM_GPL_b = FALSE;

/*
    RAM Copy of the level-2 fault memory content
*/
FML2_FaultMem_St_t FML2_FaultMem_aSt[FM_CONF_MAX_L2FAULT_ENTRIES];

/*
    OpMin_WOE_CDC --> OPerating MINutes With Out Error in Current Driving Cycle

    Operating minutes with out errors of the fault incase of 2005/55/EC directive
    This variable helps the variable 'OperMin_WthoutErr_Total_u16' in  strcture
    'FML2_FaultMem_aSt'
*/
#if EUD_HDV_2005_55_EC
uint16_t OpMin_WOE_CDC_u16[FM_CONF_MAX_L2FAULT_ENTRIES];
#endif

/*
 **************************************************************************************************
 *    Function defintions
 **************************************************************************************************
 */

/**
 *    @brief  Initialization function for the status flag of occurrence count
 *
 *    @param  None
 *    @return None
 */
void FM_OCcnt_StatusInit(void)
{
    uint8_t FltIndx_u8 = 0;
    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        IncCntState_ab[FltIndx_u8] = true;
    }
}
 

/**
 *    @brief  Initialization function for Fault management level-2.
 *            1:Reads the level-2 fault memory from NV memory and keeps in RAM
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_INIT]@{SDD_DERIVED_FML2_INIT}
 */
void FML2_Init(void)
{

    uint8_t FltIndx_u8 = 0;

    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY1, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY1]);
    //       FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY1].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY1].Size_u16, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY1]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY2, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY2]);
    //       FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY2].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY2].Size_u16, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY2]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY3, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY3]);
    //   FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY3].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY3].Size_u16, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY3]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY4, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY4]);
    // FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY4].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY4].Size_u16, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY4]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY5, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY5]);
    // FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY5].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY5].Size_u16, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY5]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY6, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY6]);
    //   FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY6].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY6].Size_u16, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY6]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY7, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY7]);
    //  FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY7].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY7].Size_u16,(uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY7]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY8, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY8]);
    // FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY8].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY8].Size_u16,(uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY8]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY9, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY9]);
    // FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY9].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY9].Size_u16,(uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY9]);
    // Read the Level 2 fault memory from the NV memory
    FEE_Read(FM_L2_ENTRY10, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY10]);
    //   FDL_Read(FEE_AddrConfig_aSt[FM_L2_ENTRY10].StartAddress_u32, FEE_AddrConfig_aSt[FM_L2_ENTRY10].Size_u16,(uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY10]);

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
#if EUD_HDV_2005_55_EC
        OpMin_WOE_CDC_u16[FltIndx_u8] = 0;
#endif
    }

    return;
}

/**
 *    @brief  Poweroff function for FM level2 operation.
 *            1:The function ensures updating of NV memory with level-2 faults
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FML2_PWROFF]@{SDD_FML2_PWROFF}
 */
uint8_t FML2 = 0, FML1 = 0, DrivingCounter = 0;
void FML2_PwrOff(void)
{
    uint8_t FltIndx_u8 = 0;
    u16_FaultPath_t FltPath = 0;
    uint8_t Err_type = 0u;
    uint8_t Err_type1 = 0u;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        switch (FML2_GetPrsntFaultState(FltIndx_u8))
        {
        //----------------//
        // FM_POTENTIAL   //
        //----------------//
        case FM_POTENTIAL:
        {
            if (TRUE == FM_GET_DryCylInd())
            {

                FltPath = FML2_GetFaultPath(FltIndx_u8);

                if (FML2_GetErrType(FltIndx_u8) & FML1_GetErrType(FltPath))
                {
                    if (FML1_GetErrType(FltPath) & FML1_GetAllContPrsnt(FltPath))
                    {
                        if (FML2_GetDrvCyCtr(FltIndx_u8) > 0)
                        {
                            /* decrement the driving cycle */
                            FML2_DecDrvCyCtr(FltIndx_u8);
                        }

                        if (0 == FML2_GetDrvCyCtr(FltIndx_u8))
                        {
                            // set the present & previous state
                            FML2_SetPrsntFaultState(FltIndx_u8, FM_CONFIRMED);
                            FML2_SetPrevFaultState(FltIndx_u8, FM_POTENTIAL);
                            FML1_SET_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
                            if (FML2_GetOccurCnt(FltIndx_u8) < 126U)
                            {
                                FML2_IncOccurCnt(FltIndx_u8);
                                IncCntState_ab[FltIndx_u8] = true;
                            }
                            // Check for DTC clear flags for L2 and update the first Fault since last DTC clear
                            if (TRUE == FM_FirstFailedFltsInfo_St.DTCClearedIndicator[DTCCLR_IND_FOR_CNFRM_FLT])
                            {
                                // Clear the corresponding flag
                                FM_FirstFailedFltsInfo_St.DTCClearedIndicator[DTCCLR_IND_FOR_CNFRM_FLT] = FALSE;
                                Err_type = FML2_GetErrType(FltIndx_u8);
                                if (((Err_type) & 0x01) == 0x01)
                                {
                                    Err_type1 = 4;
                                }
                                else if (((Err_type >> 1) & 0x01) == 0x01)
                                {
                                    Err_type1 = 3;
                                }
                                else if (((Err_type >> 2) & 0x01) == 0x01)
                                {
                                    Err_type1 = 2;
                                }
                                else if (((Err_type >> 3) & 0x01) == 0x01)
                                {
                                    Err_type1 = 1;
                                }
                                else if (((Err_type >> 4) & 0x01) == 0x01)
                                {
                                    Err_type1 = 0;
                                }
                                // get the DTC from the DTC table
                                FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32 = GetDTCofFltPathFltType(FltPath, Err_type1, FM_PROTO_UDS_E);
                                //                  FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32[1] = GetDTCofFltPathFltType (FltPath, FML2_GetErrType(FltIndx_u8),FM_PROTO_OBD_E);
                                //                  FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32[2] = GetDTCofFltPathFltType (FltPath, FML2_GetErrType(FltIndx_u8),FM_PROTO_J1939_E);
                            }

                            // driving cycle count
                            if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);

#if EUD_HDV_2005_55_EC
                                // reset operating hours
                                RstOperMinWthoutErr(FltIndx_u8);
#endif
                            }
                            else
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
                            }
                        }
                    }
                    else
                    {
                        // reset the driving cycle counter
                        if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                        {
                            FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_POTENTIALCNTS);
                        }
                        else
                        {
                            FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_POTENTIALCNTS(FltPath));
                        }
                    }
                }
                else
                {
                    // updating the state
                    FML2_SetPrsntFaultState(FltIndx_u8, FM_DELETING);
                    FML2_SetPrevFaultState(FltIndx_u8, FM_POTENTIAL);

                    // resetting the driving cycle count
                    FML2_RstDrvCyCtr(FltIndx_u8);

                    if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                    {
                        // warm up cycle initialisation
                        FML2_SetWUPCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_DELETECNTS);

// reset the operating hours with out errors(OPWE)
#if EUD_HDV_2005_55_EC
                        RstOperMinWthoutErr(FltIndx_u8);
#endif
                    }
                    else
                    {
                        // warm up cycle initialisation
                        FML2_SetWUPCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_DELETECNTS(FltPath));
                    }
                }

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }
            break;
        }

        //----------------//
        // FM_CONFIRMED   //
        //----------------//
        case FM_CONFIRMED:
        {
            //            	DrivingCounter = FM_GET_DryCylInd();
            if (TRUE == FM_GET_DryCylInd())
            {
                // reading the fault path
                FltPath = FML2_GetFaultPath(FltIndx_u8);

                // error still exists in level 1?
                FML2 = FML2_GetErrType(FltIndx_u8);
                FML1 = FML1_GetErrType(FltPath);
                if (FML2_GetErrType(FltIndx_u8) & FML1_GetErrType(FltPath))
                {
                    // driving cycle count
                    if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                    {
                        FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);

#if EUD_HDV_2005_55_EC
                        // reset operating hours
                        RstOperMinWthoutErr(FltIndx_u8);
#endif
                    }
                    else
                    {
                        FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
                    }
                }
                else
                {
                    if (FML2_GetDrvCyCtr(FltIndx_u8) > 0)
                    {
                        /* decrement the driving cycle */
                        FML2_DecDrvCyCtr(FltIndx_u8);
                    }

                    if (0 == FML2_GetDrvCyCtr(FltIndx_u8))
                    {
                        // set the present & previous state
                        FML2_SetPrsntFaultState(FltIndx_u8, FM_DELETING);
                        FML2_SetPrevFaultState(FltIndx_u8, FM_CONFIRMED);
                        /* As the Confirm state should be there for previously active fault
                        we are not supposed to clear it till the deleting count completes */
                        // FML1_CLR_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);//Todo: Lavanya
                        // FML1_CLR_WIR_STATUS_OF_DTC_MAPTBL(FltPath); //Todo: Lavanya
                        // resetting the driving cycle count
                        FML2_RstDrvCyCtr(FltIndx_u8);

                        if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                        {
                            // warm up cycle initialisation
                            FML2_SetWUPCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_DELETECNTS);

// reset the operating hours with out errors(OPWE)
#if EUD_HDV_2005_55_EC
                            RstOperMinWthoutErr(FltIndx_u8);
#endif
                        }
                        else
                        {
                            // warm up cycle initialisation
                            FML2_SetWUPCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_DELETECNTS(FltPath));
                        }
                    }
                }

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }

            break;
        }

        //----------------//
        // FM_DELETING    //
        //----------------//
        case FM_DELETING:
        {
            if (TRUE == FM_GET_WUPCylInd())
            {
                // reading the fault path
                FltPath = FML2_GetFaultPath(FltIndx_u8);

                // error still exists in level 1?
                if (FML2_GetErrType(FltIndx_u8) & FML1_GetErrType(FltPath))
                {
                    if (FM_POTENTIAL == FML2_GetPrevFaultState(FltIndx_u8))
                    {
                        // set the present & previous state
                        FML2_SetPrsntFaultState(FltIndx_u8, FM_POTENTIAL);
                        FML2_SetPrevFaultState(FltIndx_u8, FM_DELETING);
                        IncCntState_ab[FltIndx_u8] = false;
                        // warm up cycle count
                        FML2_RstWUPCyCtr(FltIndx_u8);

                        // driving cycle count
                        if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                        {
                            if (FM_CONF_NOF_OBD_POTENTIALCNTS)
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, (FM_CONF_NOF_OBD_POTENTIALCNTS - 1));
                            }
                            else
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, (FM_CONF_NOF_OBD_POTENTIALCNTS));
                            }
                        }
                        else
                        {
                            if (FM_CONF_NOF_NONOBD_POTENTIALCNTS(FltPath))
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, (FM_CONF_NOF_NONOBD_POTENTIALCNTS(FltPath) - 1));
                            }
                            else
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_POTENTIALCNTS(FltPath));
                            }
                        }

                        // if the counter value become zero, then move the
                        // state from potential to confirmed
                        if (0 == FML2_GetDrvCyCtr(FltIndx_u8))
                        {
                            // set the present & previous state
                            FML2_SetPrsntFaultState(FltIndx_u8, FM_CONFIRMED);
                            FML2_SetPrevFaultState(FltIndx_u8, FM_POTENTIAL);
                            FML1_SET_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
                            if (FML2_GetOccurCnt(FltIndx_u8) < 126U)
                            {
                                FML2_IncOccurCnt(FltIndx_u8);
                                IncCntState_ab[FltIndx_u8] = true;
                            }

                            // driving cycle count
                            if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);

#if EUD_HDV_2005_55_EC
                                // reset operating hours
                                RstOperMinWthoutErr(FltIndx_u8);
#endif
                            }
                            else
                            {
                                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
                            }
                        }
                    }
                    else if (FM_CONFIRMED == FML2_GetPrevFaultState(FltIndx_u8))
                    {
                        // set the present & previous state
                        FML2_SetPrsntFaultState(FltIndx_u8, FM_CONFIRMED);
                        FML2_SetPrevFaultState(FltIndx_u8, FM_DELETING);
                        FML1_SET_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
                        if (FML2_GetOccurCnt(FltIndx_u8) < 126U)
                        {
                            FML2_IncOccurCnt(FltIndx_u8);
                            IncCntState_ab[FltIndx_u8] = true;
                        }
                        // warm up cycle count
                        FML2_RstWUPCyCtr(FltIndx_u8);

                        // driving cycle count
                        if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                        {
                            FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);

#if EUD_HDV_2005_55_EC
                            // reset operating hours
                            RstOperMinWthoutErr(FltIndx_u8);
#endif
                        }
                        else
                        {
                            FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
                        }
                    }
                    else
                    {
                        // do nothing
                        // Control should not come here because deleting state is
                        // reached either from potential or confirmed
                    }
                }
                else
                {
                    // decrement the driving cycle
                    FML2_DecWUPCyCtr(FltIndx_u8);

                    if (0 == FML2_GetWUPCyCtr(FltIndx_u8))
                    {
                        // updating the fault state
                        FML2_SetPrsntFaultState(FltIndx_u8, FM_NOENTRY);
                        FML2_SetPrevFaultState(FltIndx_u8, FM_NOENTRY);
                        IncCntState_ab[FltIndx_u8] = false;
                        FML2_SetOccurCnt(FltIndx_u8, 0u);
                        // Clear the confirmed fault
                        /* The fault state should be active for previously active fault we are
                        clearing after the fault completes the deleting counts */
                        FML1_CLR_CDTC_STATUS_OF_DTC_MAPTBL(FltPath); // Todo: Lavanya
                        FML1_CLR_WIR_STATUS_OF_DTC_MAPTBL(FltPath);  // Todo: Lavanya
                        // reset warm up & Driving cycle counters
                        FML2_RstWUPCyCtr(FltIndx_u8);
                        FML2_RstDrvCyCtr(FltIndx_u8);

                        // decrement L2 entries in common counters
                        FM_DecNofEntries_L2();

#if EUD_HDV_2005_55_EC
                        // reset operating hours
                        RstOperMinWthoutErr(FltIndx_u8);
#endif
                    }
                }

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }
            break;
        }

        //----------------//
        //    DEFAULT     //
        //----------------//
        default:
        {
            // do nothing
            break;
        }
        }
    }

    // Update the status of the MIL
    FML2_MonWarnLamp_proc();

    // update the latest changes in to NV memory
    FML2_UpdtNVMem_proc();

    return;
}

/**
 *    @brief  Task to handle the level-2 fault history state machine.
 *            1:The task is scheduled by task scheduler every 100ms.
 *            2:Changing of state based on driving cycle is done at the end of
 *              each driving cycle
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FML2_FLTHNDL_PROC]@{SDD_FML2_FLTHNDL_PROC}
 */
uint8_t Fp_u8 = 0;
void FML2_FltHndl_proc(void)
{
    u16_FaultPath_t FltPath = 0;
    uint8_t FltType_u8 = 0;
    int8_t FltIndx_s8 = -1;
    uint8_t i = 0;

    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        Fp_u8 = FltPath;
        // Fault with priority greater than 0 are only eligible for getting
        // registered in L2 memory
        if (FML1_GetPriority(FltPath) > FM_PRIO_0_E)
        {
            // Any updates since last time?
            if (FML1_GetUpt_St(FltPath))
            {
                // error or no error?
                if (FML1_GetErf(FltPath))
                {
                    // which error type was reported?
                    FltType_u8 = (uint8_t)FML1_GetErrType(FltPath);

                    // copies the data from L1 to L2 if not previously
                    // registered
                    FltIndx_s8 = FML2_CopyL1_to_L2(FltPath, FltType_u8);

                    // if ((0 <= FltIndx_s8) && (FML1_GetUpt_St(FltPath)))
                    if (0 <= FltIndx_s8) // Bugfix (24th Jun'19) - for DTC lifecycle (CONFIRM the fault in same driving cycle of occurrence.
                    {
                        L2StateMachine(FltPath, (uint8_t)FltIndx_s8);
                    }

                    // Clear the updation status
                    FML1_ClrUpt_St(FltPath);
                }
                else
                {
                    // find the index where it is stored in L2 memory
                    // and do the needfull accordingly
                    for (i = 0; i < FM_CONF_MAX_L2FAULT_ENTRIES; i++)
                    {
                        if ((FM_NOENTRY != FML2_GetPrsntFaultState(i)) && (FltPath == FML2_GetFaultPath(i)))
                        {
                            L2StateMachine(FltPath, i);
                        }
                    }
                }
            }
        }
    }

    return;
}

/**
 *    @brief  Function helps in state transitions
 *            1:The function will be called from "FML2_FltHndl_proc"
 *    @param  Fault Path
 *    @param  Fault Index
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_ST_MCH]@{SDD_DERIVED_FML2_ST_MCH}
 */
static void L2StateMachine(u16_FaultPath_t FltPath, uint8_t FltIndx_u8)
{
    switch (FML2_GetPrsntFaultState(FltIndx_u8))
    {
    case FM_POTENTIAL:
    {
        if (0 == FML2_GetDrvCyCtr(FltIndx_u8))
        {
            // set the present & previous state
            FML2_SetPrsntFaultState(FltIndx_u8, FM_CONFIRMED);
            FML2_SetPrevFaultState(FltIndx_u8, FM_POTENTIAL);
            FML1_SET_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
            if (FML2_GetOccurCnt(FltIndx_u8) < 126U)
            {
                FML2_IncOccurCnt(FltIndx_u8);
                IncCntState_ab[FltIndx_u8] = true;
            }
#if (CAPTURE_FRZFRAME_LATEST_CONFIRMED)
            if (FML1_FrzFrmUpdated_b)
            {
                FML1_FrzFrmUpdated_b = FALSE;
                Copy_FrzFrz_From_L1_to_L2(FltPath, FltIndx_u8);
            }
#endif

            // driving cycle count
            if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
            {
                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);

#if EUD_HDV_2005_55_EC
                // reset operating hours
                RstOperMinWthoutErr(FltIndx_u8);
#endif
            }
            else
            {
                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
            }
        }

        if (FML1_GetErf(FltPath))
        {
            // increment frequency counter
            IncErrFreqCtr(FltIndx_u8);
            // Data got updated in L2 memory
            FML2_SetUpt_St(FltIndx_u8);
        }
        else
        {
            FML1_CLR_PDTC_STATUS_OF_DTC_MAPTBL(FltPath);
            FML2_SetPrsntFaultState(FltIndx_u8, FM_DELETING);
            FML2_SetPrevFaultState(FltIndx_u8, FM_POTENTIAL);
            FML1_ClrAllContPrsnt(FltPath);
            FML2_SetUpt_St(FltIndx_u8);
        }
        // Clear the updation status
        FML1_ClrUpt_St(FltPath);

        break;
    }
    case FM_CONFIRMED:
    {
        if (FML1_GetErf(FltPath))
        {
            if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
            {
// reset the operating hours with out errors(OPWE)
#if EUD_HDV_2005_55_EC
                RstOperMinWthoutErr(FltIndx_u8);
#endif

                // reset the driving cycle counter
                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);
            }
            else
            {
                // reset the driving cycle counter
                FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
            }

            // increment frequency counter
            IncErrFreqCtr(FltIndx_u8);

            // Data got updated in L2 memory
            FML2_SetUpt_St(FltIndx_u8);

            if (TRUE == FM_FirstFailedFltsInfo_St.DTCClearedIndicator[DTCCLR_IND_FOR_CNFRM_FLT])
            {
                // Clear the corresponding flag
                FM_FirstFailedFltsInfo_St.DTCClearedIndicator[DTCCLR_IND_FOR_CNFRM_FLT] = FALSE;

                // get the DTC from the DTC table
                FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32 = GetDTCofFltPathFltType(FltPath, FML2_GetErrType(FltIndx_u8), FM_PROTO_UDS_E);
                //                  FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32[1] = GetDTCofFltPathFltType (FltPath, FML2_GetErrType(FltIndx_u8),FM_PROTO_OBD_E);
                //                  FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32[2] = GetDTCofFltPathFltType (FltPath, FML2_GetErrType(FltIndx_u8),FM_PROTO_J1939_E);
            }

#if (CAPTURE_FRZFRAME_LATEST_CONFIRMED)
            if (FML1_FrzFrmUpdated_b)
            {
                FML1_FrzFrmUpdated_b = FALSE;
                Copy_FrzFrz_From_L1_to_L2(FltPath, FltIndx_u8);
            }
#endif
        }
        else
        {
            if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
            {
#if EUD_HDV_2005_55_EC
                if (FM_CONF_OBD_HEALING_MIN <= GetOperMinWthoutErr(FltIndx_u8))
                {
                    // updating the fault state
                    FML2_SetPrsntFaultState(FltIndx_u8, FM_DELETING);
                    FML2_SetPrevFaultState(FltIndx_u8, FM_CONFIRMED);

                    FML1_CLR_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);

                    // warm up cycle initialisation
                    FML2_SetWUPCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_DELETECNTS);
                    FML2_RstDrvCyCtr(FltIndx_u8);

                    // reset operating hours
                    RstOperMinWthoutErr(FltIndx_u8);

                    // Data got updated in L2 memory
                    FML2_SetUpt_St(FltIndx_u8);
                }
#endif
            }
        }

        // Clear the updation status
        FML1_ClrUpt_St(FltPath);

        break;
    }
    case FM_DELETING:
    {
        if (FML1_GetErf(FltPath))
        {
            if (FM_POTENTIAL == FML2_GetPrevFaultState(FltIndx_u8))
            {
                // set the present & previous state
                FML2_SetPrsntFaultState(FltIndx_u8, FM_POTENTIAL);
                FML2_SetPrevFaultState(FltIndx_u8, FM_DELETING);
                IncCntState_ab[FltIndx_u8] = false;
                // warm up cycle count
                FML2_RstWUPCyCtr(FltIndx_u8);

                // driving cycle count
                if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                {
                    FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_POTENTIALCNTS);
                }
                else
                {
                    FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_POTENTIALCNTS(FltPath));
                }
                IncErrFreqCtr(FltIndx_u8);

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }
            else if (FM_CONFIRMED == FML2_GetPrevFaultState(FltIndx_u8))
            {
                // set the present & previous state
                FML2_SetPrsntFaultState(FltIndx_u8, FM_CONFIRMED);
                FML2_SetPrevFaultState(FltIndx_u8, FM_DELETING);
                FML1_SET_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
                if (FML2_GetOccurCnt(FltIndx_u8) < 126U)
                {
                    FML2_IncOccurCnt(FltIndx_u8);
                    IncCntState_ab[FltIndx_u8] = true;
                }
                // warm up cycle count
                FML2_RstWUPCyCtr(FltIndx_u8);

                // driving cycle count
                if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
                {
                    FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_HEALINGCNTS);

#if EUD_HDV_2005_55_EC
                    // reset operating hours
                    RstOperMinWthoutErr(FltIndx_u8);
#endif
                }
                else
                {
                    FML2_SetDrvCyCtr(FltIndx_u8, FM_CONF_NOF_NONOBD_HEALINGCNTS(FltPath));
                }
                IncErrFreqCtr(FltIndx_u8);

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }
            else
            {
                // do nothing
                // Control should not come here because deleting state is
                // reached either from potential or confirmed
            }
        }
        else
        {
            if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
            {
#if EUD_HDV_2005_55_EC
                if (FM_CONF_OBD_DELETING_MIN <= GetOperMinWthoutErr(FltIndx_u8))
                {
                    // updating the fault state
                    FML2_SetPrsntFaultState(FltIndx_u8, FM_NOENTRY);
                    FML2_SetPrevFaultState(FltIndx_u8, FM_NOENTRY);

                    // reset warm up & Driving cycle counters
                    FML2_RstWUPCyCtr(FltIndx_u8);
                    FML2_RstDrvCyCtr(FltIndx_u8);

                    // reset operating hours
                    RstOperMinWthoutErr(FltIndx_u8);

                    // decrement L2 entries in common counters
                    FM_DecNofEntries_L2();

                    // Data got updated in L2 memory
                    FML2_SetUpt_St(FltIndx_u8);
                }
#endif
            }
            else
            {
                /* for NON-OBD case */
                // updating the fault state
                FML2_SetPrsntFaultState(FltIndx_u8, FM_NOENTRY);
                FML2_SetPrevFaultState(FltIndx_u8, FM_NOENTRY);
                FML2_SetOccurCnt(FltIndx_u8, 0u);
                IncCntState_ab[FltIndx_u8] = false;
                FML1_CLR_PDTC_STATUS_OF_DTC_MAPTBL(FltPath);
                FML1_CLR_CDTC_STATUS_OF_DTC_MAPTBL(FltPath);
                FML1_CLR_WIR_STATUS_OF_DTC_MAPTBL(FltPath);
                // reset warm up & Driving cycle counters
                FML2_RstWUPCyCtr(FltIndx_u8);
                FML2_RstDrvCyCtr(FltIndx_u8);

                // decrement L2 entries in common counters
                FM_DecNofEntries_L2();

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }
        }

        // Clear the updation status
        FML1_ClrUpt_St(FltPath);

        break;
    }
    default:
    {
        // do nothing
        break;
    }
    }

    return;
}

/**
 *    @brief  Task to change the state of faults based on EUD_HDV_2005_55_EC
 *            1:This task will be called from the fault manager scheduler
 *               every 1 min
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_EUD_HDV2005]@{SDD_DERIVED_FML2_EUD_HDV2005}
 */
#if EUD_HDV_2005_55_EC
void FML2_FltHndl_EUD_HDV_2005_55_EC_proc(void)
{
    uint8_t FltIndx_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        switch (FML2_GetPrsntFaultState(FltIndx_u8))
        {
        case FM_POTENTIAL:
        {
            // do nothing
            break;
        }
        case FM_CONFIRMED:
        {
            if (FM_CONF_OBD_HEALING_MIN <= GetOperMinWthoutErr(FltIndx_u8))
            {
                // updating the fault state
                FML2_SetPrsntFaultState(FltIndx_u8, FM_DELETING);
                FML2_SetPrevFaultState(FltIndx_u8, FM_CONFIRMED);

                // warm up cycle initialisation
                FML2_SetWUPCyCtr(FltIndx_u8, FM_CONF_NOF_OBD_DELETECNTS);
                FML2_RstDrvCyCtr(FltIndx_u8);

                // reset operating hours
                RstOperMinWthoutErr(FltIndx_u8);

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }

            break;
        }
        case FM_DELETING:
        {
            if (FM_CONF_OBD_DELETING_MIN <= GetOperMinWthoutErr(FltIndx_u8))
            {
                // updating the fault state
                FML2_SetPrsntFaultState(FltIndx_u8, FM_NOENTRY);
                FML2_SetPrevFaultState(FltIndx_u8, FM_NOENTRY);

                // reset warm up & Driving cycle counters
                FML2_RstWUPCyCtr(FltIndx_u8);
                FML2_RstDrvCyCtr(FltIndx_u8);

                // reset operating hours
                RstOperMinWthoutErr(FltIndx_u8);

                // decrement L2 entries in common counters
                FM_DecNofEntries_L2();

                // Data got updated in L2 memory
                FML2_SetUpt_St(FltIndx_u8);
            }

            break;
        }
        default:
        {
            // do nothing
            break;
        }
        }
    }

    return;
}
#endif

/**
 *    @brief  The task keeps track of the faults in level-1 for registering it
 *            into level-2.
 *            1:The task is invoked by the "FML2_FltHndl_proc"
 *            2:Checks whether a fault has sufficient priority level (priority
 *              level greater than zero) to get registered into level-2 fault
 *              memory.
 *            3:Checks whether a same fault is already present in level-2 fault
 *              memory or not.Makes an entry only if the fault does not exist
 *              in the memory
 *            4:Performs the priority handling operation when there is no space
 *              in the level-2 fault memory and a new fault has occurred with
 *              higher priority. Then the task will delete the low priority task
 *              from the level-2 fault memory and stores the relatively higher
 *              priority fault.
 *            5:Fault already exist, task increments the frequency counter
 *              associated with this fault.
 *
 *    @param  Fault path
 *    @param  Fault type
 *    @return Index where the fault is stored in L2
 *    @return -1 - not registered
 *    @code tag - @[CODE_FML2_COPYL1_TO_L2]@{SDD_FML2_COPYL1_TO_L2}
 */
int8_t FML2_CopyL1_to_L2(u16_FaultPath_t FltPath, uint8_t FltType_u8)
{
    int8_t FltIndx_s8 = 0;

    // if index is valid(positive number) the fault was memorized
    FltIndx_s8 = FML2_GetFltIndxOfPath((u16_FaultPath_t)FltPath, FltType_u8);

    // fault was not memorized
    if (-1 == FltIndx_s8)
    {
        // Memorize the fault into L2 memory
        FltIndx_s8 = MemorizeToL2((u16_FaultPath_t)FltPath);

        // Clear the updation status
        FML1_ClrUpt_St(FltPath);
    }

    return FltIndx_s8;
}

/**
 *    @brief  Task to monitor the state of the Mal-function Indicator Lamp (MIL)
 *            and Glow Plug Lamp (GPL). The function modifies the status of MIL
 *            and GPL depending on the status of the level-2 faults.
 *
 *            1:The task is scheduled by Fault manager scheduler every 50ms.
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FML2_MONWARNLAMP_PROC]@{SDD_FML2_MONWARNLAMP_PROC}
 */
void FML2_MonWarnLamp_proc(void)
{
    uint8_t FltIndx_u8 = 0;
    u16_FaultPath_t FltPath = 0;
    bool MIL_on_b = FALSE;
    bool GPL_on_b = FALSE;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if (FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8))
        {
            // get fault path
            FltPath = FML2_GetFaultPath(FltIndx_u8);

            if (IS_FAULT_OBD_RELEVANT(FltPath))
            {
                // MIL on
                MIL_on_b = TRUE;
            }
            else if (IS_FAULT_GPL_INDICATABLE(FltPath))
            {
                // GPL on
                GPL_on_b = TRUE;
            }
            else
            {
                // do nothing
            }
        }
    }

    // controlling the warining lamp
    if (TRUE == MIL_on_b)
    {
        FML2_TurnOnMI();
    }
    else
    {
        FML2_TurnOffMI();
    }

    if (TRUE == GPL_on_b)
    {
        FML2_TurnOnGPL();
    }
    else
    {
        FML2_TurnOffGPL();
    }

    return;
}

/**
 *    @brief  Function to know the status of Mal Indication Lamp.
 *
 *    @param  None.
 *    @return TRUE: if MIL is ON, FALSE: otherwise.
 *    @code tag - @[CODE_FML2_STOFMIL]@{SDD_FML2_STOFMIL}
 */
bool FML2_StatusOf_MI(void)
{
    return FM_CEL_MIL_b;
}

/**
 *    @brief  Function to turn ON the MIL.
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FML2_MILON]@{SDD_FML2_MILON}
 */
void FML2_TurnOnMI(void)
{
    FM_CEL_MIL_b = (bool)TRUE;
    return;
}

/**
 *    @brief  Function to turn OFF the MIL.
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FML2_MILOFF]@{SDD_FML2_MILOFF}
 */
void FML2_TurnOffMI(void)
{
    FM_CEL_MIL_b = (bool)FALSE;
    return;
}

/**
 *    @brief  Function to know the status of Glow Plug Lamp.
 *
 *    @param  None.
 *    @return TRUE: if GPL is ON, FALSE: otherwise.
 *    @code tag - @[CODE_DERIVED_FML2_STOFGPL]@{SDD_DERIVED_FML2_STOFGPL}
 */
bool FML2_StatusOf_GPL(void)
{
    return FM_GPL_b;
}

/**
 *    @brief  Function to turn ON the GPL.
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_DERIVED_FML2_GPLON]@{SDD_DERIVED_FML2_GPLON}
 */
void FML2_TurnOnGPL(void)
{
    FM_GPL_b = (bool)TRUE;
    return;
}

/**
 *    @brief  Function to turn OFF the GPL.
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_DERIVED_FML2_GPLOFF]@{SDD_DERIVED_FML2_GPLOFF}
 */
void FML2_TurnOffGPL(void)
{
    FM_GPL_b = (bool)FALSE;
    return;
}

/**
 *    @brief  Tasks to check for any changes in the level-2 fault memory.If yes,
 *            update the same to the NV memory.
 *            1:The task is scheduled by Fault manager scheduler every 1000ms.
 *            2:The task ensures the latest values in the volatile (RAM) copy of
 *              the level-2 fault memory to be updated to corresponding NV
 *              version of the fault memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FML2_UPDATENVMEM_PROC]@{SDD_FML2_UPDATENVMEM_PROC}
 */

void FML2_UpdtNVMem_proc(void)
{
    uint8_t FltIndx_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if (TRUE == FML2_GetUpt_St(FltIndx_u8))
        {
            // clear the updation status
            FML2_ClrUpt_St(FltIndx_u8);

            switch (FltIndx_u8)
            {
            case FM_L2_ERR_ENTRY1:
            {
                // Write in to NV memory

                // Fdl_Erase(2, 1);
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY1].start_u8 = 0xA0;
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY1].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY1, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY1]);
                break;
            }
            case FM_L2_ERR_ENTRY2:
            {
                // Write in to NV memory

                // Fdl_Erase(3, 1);
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY2].start_u8 = 0xA1;
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY2].end_u8 = 0xA1;
                FEE_Write(FM_L2_ENTRY2, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY2]);
                break;
            }
            case FM_L2_ERR_ENTRY3:
            {
                // Write in to NV memory

                // Fdl_Erase(4, 1);
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY3].start_u8 = 0xA2;
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY3].end_u8 = 0xA2;
                FEE_Write(FM_L2_ENTRY3, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY3]);
                break;
            }
            case FM_L2_ERR_ENTRY4:
            {
                // Write in to NV memory

                // Fdl_Erase(5, 1);
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY4].start_u8 = 0xA3;
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY4].end_u8 = 0xA3;
                FEE_Write(FM_L2_ENTRY4, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY4]);
                break;
            }
            case FM_L2_ERR_ENTRY5:
            {
                // Write in to NV memory
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY5].start_u8 = 0xAA;
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY5].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY5, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY5]);
                break;
            }
            case FM_L2_ERR_ENTRY6:
            {
                // Write in to NV memory
                //  FML2_FaultMem_aSt[FM_L2_ERR_ENTRY6].start_u8 = 0xAA;
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY6].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY6, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY6]);
                break;
            }
            case FM_L2_ERR_ENTRY7:
            {
                // Write in to NV memory
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY7].start_u8 = 0xAA;
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY7].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY7, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY7]);
                break;
            }
            case FM_L2_ERR_ENTRY8:
            {
                // Write in to NV memory
                //    FML2_FaultMem_aSt[FM_L2_ERR_ENTRY8].start_u8 = 0xAA;
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY8].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY8, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY8]);
                break;
            }
            case FM_L2_ERR_ENTRY9:
            {
                // Write in to NV memory
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY9].start_u8 = 0xAA;
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY9].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY9, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY9]);
                break;
            }
            case FM_L2_ERR_ENTRY10:
            {
                // Write in to NV memory
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY10].start_u8 = 0xAA;
                // FML2_FaultMem_aSt[FM_L2_ERR_ENTRY10].end_u8 = 0xA0;
                FEE_Write(FM_L2_ENTRY10, (uint8_t *)&FML2_FaultMem_aSt[FM_L2_ERR_ENTRY10]);
                break;
            }
            default:
            {
                break;
            }
            }
        }
    }

    return;
}

/**
 *    @brief  Function to read the NONOBD specific Freeze frame from level-2
 *            fault mamory.
 *
 *    @param  DTC
 *    @param  Address of the buffer where the freeze frames datas are copied.
 *
 *    @return Number of DTC freeze frames successfully copied
 *    @code tag - @[CODE_FML2_RDFRZFRM_CMPLNOBD]@{SDD_FML2_RDFRZFRM_CMPLNOBD}
 */
uint8_t FML2_ReadCmpltNONOBDFrzFrm(uint16_t DTC_u16, uint16_t *DataBuff_pu16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    uint8_t FrzFrmCpyd_u8 = 0;
    uint8_t Indx_u8 = 0;
    u16_FaultPath_t FltPath = 0;
    uint8_t FltType_u8 = 0;
    int8_t FltIndx_u8 = 0;
    uint16_t i = 0;
    uint16_t j = 0;

    for (i = 0; i < NUM_OF_FAULTPATHS_E; i++)
    {
        for (j = 0; j < MAX_NUM_ERROR_TYPES - 1; j++)
        {

            if (DTC_u16 == GET_DTC_VALUE_DTC_MAPTBL(i, j))
            {
                // reading the fault path and fault type
                FltPath = i;
                FltType_u8 = (uint8_t)(FM_GET_FLTTYPE(j));

                FltIndx_u8 = 0;
                for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
                {
                    if ((FM_NOENTRY != FML2_GetPrsntFaultState(FltIndx_u8)))
                    //&& (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
                    {
                        // fault path and type match?
                        if ((FltPath == FML2_GetFaultPath(FltIndx_u8)) && (FltType_u8 == FML2_GetErrType(FltIndx_u8)))
                        {
                            // copying the data
                            FM_ReadFrzFrm_ByFltIndex(FltIndx_u8, (uint8_t *)&DataBuff_pu16[Indx_u8], FM_ProtocolType_En);

                            // incrementing the entries
                            FrzFrmCpyd_u8++;

                            // break to main loop
                            break;
                        }
                    }
                }
            }
        }
    }
    return FrzFrmCpyd_u8;
}

/**
 *    @brief  This function helps 'FML2_CopyL1_to_L2' for registring the faults
 *            in L2 RAM
 *            1:Performs the priority handling operation when there is no space
 *              in the level-2 fault memory and a new fault has occurred with
 *              higher priority. Then the task will delete the low priority task
 *              from the level-2 fault memory and stores the relatively higher
 *              priority fault.
 *            2:If more than one fault exits with the same priority, then delete
 *              the fault which is old
 *
 *    @param  FltPath_En - Fault path
 *    @return Index if success
 *    @return -1 if failure
 *    @code tag - @[CODE_DERIVED_FML2_MEMORIZE]@{SDD_DERIVED_FML2_MEMORIZE}
 */
static int8_t MemorizeToL2(u16_FaultPath_t FltPath_En)
{
    int8_t FltIndx_s8 = -1;
    uint8_t Lowest_Priority_u8 = 0;

    // Is there memory left to register this fault
    // Empty slots are available for registration, find the index
    FltIndx_s8 = Find_Free_Slot();

    if ((0 <= FltIndx_s8) && (FltIndx_s8 < FM_CONF_MAX_L2FAULT_ENTRIES))
    {
        // register data
        Register_L2Err((uint8_t)FltIndx_s8, FltPath_En);
    }
    else
    {
        // No empty slots
        // So check whether any low priority fault exit or not, if yes delete
        // that entry and add up the new one
        // If more than one fault exits with the same priority, then delete
        // the fault which is old

        FindLowestPriorityErr(&FltIndx_s8, &Lowest_Priority_u8);

        if ((0 <= FltIndx_s8) && (FML1_GetPriority(FltPath_En) > Lowest_Priority_u8))
        {
            // register data
            Register_L2Err((uint8_t)FltIndx_s8, FltPath_En);
        }
        else
        {
            FltIndx_s8 = Find_Deleting_Slot();

            if ((0 <= FltIndx_s8) && (FltIndx_s8 < FM_CONF_MAX_L2FAULT_ENTRIES))
            {
                // register data
                Register_L2Err((uint8_t)FltIndx_s8, FltPath_En);
            }
            else
            {
                FltIndx_s8 = Find_Potential_Slot();

                if ((0 <= FltIndx_s8) && (FltIndx_s8 < FM_CONF_MAX_L2FAULT_ENTRIES))
                {
                    // register data
                    Register_L2Err((uint8_t)FltIndx_s8, FltPath_En);
                }
                else
                {
                    FltIndx_s8 = -1;
                }
            }
        }
    }

    return FltIndx_s8;
}

/**
 *    @brief  This function helps in finding free slots for new registration of
 *             errors in L2 memory
 *
 *    @param  None
 *    @return -1 -> Failure
 *    @return Index
 *    @code tag - @[CODE_DERIVED_FML2_FREE_SLOT]@{SDD_DERIVED_FML2_FREE_SLOT}
 */
static int8_t Find_Free_Slot(void)
{
    int8_t i = 0;

    while (i < FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        if (FM_NOENTRY == FML2_GetPrsntFaultState(i))
        {
            return i;
        }
        i++;
    }

    return -1;
}

/**
 *    @brief  This function helps in finding free slots for new registration of
 *             errors in L2 memory
 *
 *    @param  None
 *    @return -1 -> Failure
 *    @return Index
 *    @code tag - @[CODE_DERIVED_FML2_FREE_SLOT]@{SDD_DERIVED_FML2_FREE_SLOT}
 */
static int8_t Find_Potential_Slot(void)
{
    int8_t i = 0;

    while (i < FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        bool potential_b = (FM_POTENTIAL == FML2_GetPrsntFaultState(i));
        bool isnotobdrelevant_b = (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(i)));
        bool storedActive_b = (bool)FML1_GetErf(FML2_GetFaultPath(i));

        if ((potential_b) && (!storedActive_b) && (isnotobdrelevant_b))
        {
            return i;
        }
        i++;
    }

    return -1;
}

/**
 *    @brief  This function helps in finding free slots for new registration of
 *             errors in L2 memory
 *
 *    @param  None
 *    @return -1 -> Failure
 *    @return Index
 *    @code tag - @[CODE_DERIVED_FML2_FREE_SLOT]@{SDD_DERIVED_FML2_FREE_SLOT}
 */
static int8_t Find_Deleting_Slot(void)
{
    int8_t i = 0;

    while (i < FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        bool deleting_b = (FM_DELETING == FML2_GetPrsntFaultState(i));
        bool isnotobdrelevant_b = (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(i)));
        bool storedActive_b = (bool)FML1_GetErf(FML2_GetFaultPath(i));

        if ((deleting_b) && (!storedActive_b) && (isnotobdrelevant_b))
        {
            return i;
        }
        i++;
    }
    return -1;
}

/**
*    @brief  This function helps in finding lowest priority error logged.
*            If more than one error has same priority, find the oldest logged
*            error in that
*
*    @param  Pointer which will be updated with Index
*            (-1 if no entries are valid)
*    @param  Pointer which wil
 l be updated with priority
*    @return None
*    @code tag - @[CODE_DERIVED_FML2_LOW_PRI_ERR]@{SDD_DERIVED_FML2_LOW_PRI_ERR}
*/

static void FindLowestPriorityErr(int8_t *index_ps8, uint8_t *priority_pu8)
{
    uint8_t i = 0;
    uint8_t low_pri_u8 = NUM_OF_FM_PRIORITIES_E;
    uint32_t low_handle_u32 = uint32_t_MAX;
    int8_t low_index_s8 = -1;

    while (i < FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        if (FM_NOENTRY != FML2_GetPrsntFaultState(i))
        {
            if ((FML2_GetPriority(i) < low_pri_u8) || ((FML2_GetPriority(i) == low_pri_u8) && (FML2_GetErrorHandle(i) < low_handle_u32)))
            {
                low_pri_u8 = (uint8_t)FML2_GetPriority(i);
                low_handle_u32 = FML2_GetErrorHandle(i);
                low_index_s8 = (int8_t)i;
            }
        }

        i++;
    }

    *index_ps8 = low_index_s8;
    *priority_pu8 = low_pri_u8;

    return;
}

/**
 *    @brief  This function helps in finding highest priority error logged.
 *            If more than one error has same priority, find the oldest logged
 *            error in that
 *
 *    @param  Pointer which will be updated with Index
 *            (-1 if no entries are valid)
 *    @param  Pointer which will be updated with priority
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_HIG_PRI_ERR]@{SDD_DERIVED_FML2_HIG_PRI_ERR}
 */
#ifdef TODO_FM_PRIO
static void FindHighestPriorityErr(int8_t *index_as8, uint8_t *priority_pu8)
{
    uint8_t i = 0;
    uint8_t high_pri_u8 = FM_PRIO_0_E;
    uint32_t high_handle_u32 = uint32_t_MAX;
    int8_t high_index_s8 = -1;

    while (i < FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        if (FM_NOENTRY != FML2_GetPrsntFaultState(i))
        {
            if ((FML2_GetPriority(i) > high_pri_u8) || ((FML2_GetPriority(i) == high_pri_u8) && (FML2_GetErrorHandle(i) < high_handle_u32)))
            {
                high_pri_u8 = (uint8_t)FML2_GetPriority(i);
                high_handle_u32 = FML2_GetErrorHandle(i);
                high_index_s8 = (int8_t)i;
            }
        }

        i++;
    }

    *index_as8 = high_index_s8;
    *priority_pu8 = high_pri_u8;

    return;
}
#endif
/**
 *    @brief  Function copies data from L1 memory to L2 memory. This function
 *            helps main copying function "FML2_CopyL1_to_L2"
 *
 *    @param  Index_au8   - Index in L2 memory(destination of copy operation)
 *    @param  FltPath_au8 - Index in L1 memory(source of copy operation)
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_REG_L2ERR]@{SDD_DERIVED_FML2_REG_L2ERR}
 */
static void Register_L2Err(uint8_t L2Index_au8, u16_FaultPath_t FltPath)
{
    // uint8_t i = 0 ;    // loop count

    // set the fault path
    FML2_SetFaultPath(L2Index_au8, (FM_FaultPath_En_t)FltPath);

    // Fault type
    FML2_ClrErrType(L2Index_au8);
    FML2_SetErrType(L2Index_au8, FML1_GetErrType(FltPath));

    // priority
    FML2_SetPriority(L2Index_au8, FML1_GetPriority(FltPath));

    // MIL indicatable
    if (IS_FAULT_OBD_RELEVANT(FltPath))
    {
        FML2_SetMIL_Indicatable(L2Index_au8);
    }
    else
    {
        FML2_ClrMIL_Indicatable(L2Index_au8);
    }

    // GPL indicatable
    if (IS_FAULT_GPL_INDICATABLE(FltPath))
    {
        FML2_SetGPL_Indicatable(L2Index_au8);
    }
    else
    {
        FML2_ClrGPL_Indicatable(L2Index_au8);
    }

    Copy_FrzFrz_From_L1_to_L2(FltPath, L2Index_au8);

    // frequency count
    FML2_SetFreqCtr(L2Index_au8, 1);
    FML2_FaultMem_aSt[L2Index_au8].OperationCycle_u16 = 1;

    // operating hours
    if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
    {
#if EUD_HDV_2005_55_EC
        RstOperMinWthoutErr(L2Index_au8);
#endif
    }

    // warm up cycle count
    FML2_RstWUPCyCtr(L2Index_au8);

    // driving cycle count
    if (TRUE == IS_FAULT_OBD_RELEVANT(FltPath))
    {
        FML2_SetDrvCyCtr(L2Index_au8, FM_CONF_NOF_OBD_POTENTIALCNTS);
        FML1_SET_PDTC_STATUS_OF_DTC_MAPTBL(FltPath);
    }
    else
    {
        FML2_SetDrvCyCtr(L2Index_au8, FM_CONF_NOF_NONOBD_POTENTIALCNTS(FltPath));

        // additionally the Pending fault status is set for non-OBD faults.
        // refer to ISO14229-1;2013, Section D.6 (figure D.10)
        // Pending fault to be set for faults occuring in current or in previous driving cycle.
        // Here we are setting for the "current" driving cycle.
        FML1_SET_PDTC_STATUS_OF_DTC_MAPTBL(FltPath);
    }

    // updating the fault state
    FML2_SetPrsntFaultState(L2Index_au8, FM_POTENTIAL);
    FML2_SetPrevFaultState(L2Index_au8, FM_NOENTRY);

    // error handle number
    FML2_SetErrorHandle(L2Index_au8, FM_Get_TotalErrorHandle());
    FM_Inc_Error_handle();

    // increment total entries in L2
    FM_IncNofEntries_L2();

    // Data got updated in L2 memory
    FML2_SetUpt_St(L2Index_au8);

    return;
}

/**
 *    @brief  Get the fault index for a specified fault path.
 *
 *    @param  FltPath_En - Application specific Fault Path value.
 *    @param  FltType_u8 - Fault type e.g. MAX, MIN. SIG, NPL and FER.
 *
 *    @return Fault memory Index.
 *    @code tag - @[CODE_DERIVED_FML2_GETINDXOFPATH]@{SDD_DERIVED_FML2_GETINDXOFPATH}
 */
// uint8_t temp1= 0;
// uint8_t temp2= 0;
// uint8_t temp3= 0;
int8_t FML2_GetFltIndxOfPath(u16_FaultPath_t FltPath_aEn, uint8_t FltType_au8)
{
    int8_t i = 0; // loop count

    for (i = 0; i < FM_CONF_MAX_L2FAULT_ENTRIES; i++)
    {
        //		temp1 = FML2_GetPrsntFaultState(i);
        //		temp2 = FML2_GetFaultPath(i);
        //		temp3 = FML2_GetErrType(i);
        if ((FM_NOENTRY != FML2_GetPrsntFaultState(i)) && (FltPath_aEn == FML2_GetFaultPath(i)) && (FltType_au8 & FML2_GetErrType(i)))
        {
            return (i);
        }
    }
    // if control comes here, it means match was not found
    // so return an failure
    return -1;
}

/**
 *    @brief  Task to test the Mal Function Lamp Test.
 *            1:The task is scheduled by Fault manager scheduler every 50ms.
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FML2_TESTMIL_PROC]@{SDD_FML2_TESTMIL_PROC}
 */
void FML2_TestMIL_proc(void)
{
    static uint32_t MIL_Test_TimeMs_u32 = 0;                       // variable helps in state transitions based on time
    static uint8_t Pattern_repeat_u8 = MIL_NOT_RDY_PATTERN_REPEAT; // variable which helps in creating the pattern when readyness is not completed

    if ((0 == RPM_N_u16) && (MIL_TEST_DONE != MIL_Test_Mode_u8))
    {
        switch (MIL_Test_Mode_u8)
        {
        //---------------//
        // ----ON------- //
        //---------------//
        case MIL_TEST_ON:
        {
            // initialising the variable for the first time
            if (!MIL_Test_TimeMs_u32)
            {
                MIL_Test_TimeMs_u32 = FM_GET_TIME_SINCE_KEY_ON_MS();
            }

            if (MIL_TEST_ON_TIME_MS <= (FM_GET_TIME_SINCE_KEY_ON_MS() - MIL_Test_TimeMs_u32))
            {
                // update the local counter
                MIL_Test_TimeMs_u32 = FM_GET_TIME_SINCE_KEY_ON_MS();

                // change the state
                MIL_Test_Mode_u8 = MIL_TEST_OFF1;
            }
            else
            {
                FML2_TurnOnMI();
            }

            break;
        }
        //---------------//
        // ----OFF1----- //
        //---------------//
        case MIL_TEST_OFF1:
        {
            if (MIL_TEST_OFF1_TIME_MS <= (FM_GET_TIME_SINCE_KEY_ON_MS() - MIL_Test_TimeMs_u32))
            {
                // update the local counter
                MIL_Test_TimeMs_u32 = FM_GET_TIME_SINCE_KEY_ON_MS();

                if (FML1_IsRdyComplete())
                {
                    // change the state
                    MIL_Test_Mode_u8 = MIL_TEST_RDY;
                }
                else
                {
                    // change the state
                    MIL_Test_Mode_u8 = MIL_TEST_NOT_RDY;
                }
            }
            else
            {
                FML2_TurnOffMI();
            }

            break;
        }
        //---------------//
        // ----RDY------ //
        //---------------//
        case MIL_TEST_RDY:
        {
            if (MIL_TEST_RDY_TIME_MS <= (FM_GET_TIME_SINCE_KEY_ON_MS() - MIL_Test_TimeMs_u32))
            {
                // update the local counter
                MIL_Test_TimeMs_u32 = FM_GET_TIME_SINCE_KEY_ON_MS();

                // change the state
                MIL_Test_Mode_u8 = MIL_TEST_OFF2;
            }
            else
            {
                FML2_TurnOnMI();
            }

            break;
        }
        //---------------//
        // ---NOT RDY--- //
        //---------------//
        case MIL_TEST_NOT_RDY:
        {
            if (MIL_TEST_NOTRDY_TIME_MS <= (FM_GET_TIME_SINCE_KEY_ON_MS() - MIL_Test_TimeMs_u32))
            {
                // update the local counter
                MIL_Test_TimeMs_u32 = FM_GET_TIME_SINCE_KEY_ON_MS();

                Pattern_repeat_u8--;
            }

            if (!Pattern_repeat_u8)
            {
                // update the local counter
                MIL_Test_TimeMs_u32 = FM_GET_TIME_SINCE_KEY_ON_MS();

                // change the state
                MIL_Test_Mode_u8 = MIL_TEST_OFF2;
            }
            else if (0 == (Pattern_repeat_u8 % 2))
            {
                FML2_TurnOnMI();
            }
            else
            {
                FML2_TurnOffMI();
            }

            break;
        }
        //---------------//
        // ----OFF2----- //
        //---------------//
        case MIL_TEST_OFF2:
        {
            if (MIL_TEST_OFF2_TIME_MS <= (FM_GET_TIME_SINCE_KEY_ON_MS() - MIL_Test_TimeMs_u32))
            {
                // change the state
                MIL_Test_Mode_u8 = MIL_TEST_DONE;
            }
            else
            {
                FML2_TurnOffMI();
            }

            break;
        }
        //---------------//
        // ----DONE----- //
        //---------------//
        case MIL_TEST_DONE:
        {
            break;
        }
        //---------------//
        // ---DEFAULT--- //
        //---------------//
        default:
        {
            break;
        }
        }
    }
    else
    {
        if (MIL_TEST_DONE != MIL_Test_Mode_u8)
        {
            FML2_TurnOffMI();
        }

        MIL_Test_Mode_u8 = MIL_TEST_DONE;
    }

    return;
}

#if EUD_HDV_2005_55_EC
/**
 *    @brief  Task to update the total operating hours with out errors
 *
 *    @param  fault Index.
 *    @return Minutes since error occured last time
 *    @code tag - @[CODE_DERIVED_FML2_GETOPERMINWOERR]@{SDD_DERIVED_FML2_GETOPERMINWOERR}
 */
static uint16_t GetOperMinWthoutErr(uint8_t FltIndx_u8)
{
    FML2_UptOPMWE(FltIndx_u8);
    return FML2_GetOPMWE(FltIndx_u8);
}

/**
 *    @brief  Task to reset the operating minutes with out errors
 *
 *    @param  fault Index.
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_RSTOPERMINWOERR]@{SDD_DERIVED_FML2_RSTOPERMINWOERR}
 */
static void RstOperMinWthoutErr(uint8_t FltIndx_u8)
{
    FML2_RstOPMWE(FltIndx_u8);

    return;
}
#endif

/**
 *    @brief  Increments the freequency counter of the error in L2 memory
 *
 *    @param  fault Index.
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_INC_ERRFREQ_CTR]@{SDD_DERIVED_FML2_INC_ERRFREQ_CTR}
 */
static void IncErrFreqCtr(uint8_t FltIndx_u8)
{
    if (MAX_L2ERR_FREQ_CNT > FML2_GetFreqCtr(FltIndx_u8))
    {
        FML2_IncFreqCtr(FltIndx_u8);
    }

    return;
}

/**
 *    @brief  Function to clear the specified fault entry.
 *
 *    @param  Index to the level-2 fault memory array.
 *    @return None
 *    @code tag - @[CODE_FML2_CLRFLTENTRY]@{SDD_FML2_CLRFLTENTRY}
 */
void ClrFltEntry(uint8_t FltIndx_u8)
{
    if (FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        // updating the fault state
        FML2_SetPrsntFaultState(FltIndx_u8, FM_NOENTRY);
        FML2_SetPrevFaultState(FltIndx_u8, FM_NOENTRY);
        IncCntState_ab[FltIndx_u8] = false;
        // reset warm up & Driving cycle counters
        FML2_RstWUPCyCtr(FltIndx_u8);
        FML2_RstDrvCyCtr(FltIndx_u8);
        /* Clear the Occurrence count value */
        FML2_SetOccurCnt(FltIndx_u8, 0u);

        // decrement L2 entries in common counters
        FM_DecNofEntries_L2();

        if (IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8)))
        {
#if EUD_HDV_2005_55_EC
            // reset operating hours
            RstOperMinWthoutErr(FltIndx_u8);
#endif
        }

        // updating the status
        FML2_SetUpt_St(FltIndx_u8);
    }

    return;
}
/**
 *    @brief  Function to clear the all NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLRALL_NOBDFLTS]@{SDD_DERIVED_FML2_CLRALL_NOBDFLTS}
 */
void FML2_ClrAllNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_NOENTRY != FML2_GetPrsntFaultState(FltIndx_u8)) &&
            (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            ClrFltEntry(FltIndx_u8);
        }
    }

    return;
}

/**
 *    @brief  Function to clear the pending NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_PNDNOBDFLTS]@{SDD_DERIVED_FML2_CLR_PNDNOBDFLTS}
 */
void FML2_ClrPndngNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;
    u16_FaultPath_t FltPath = 0;

    // L1 memory
    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        if (FALSE == IS_FAULT_OBD_RELEVANT(FltPath))
        {
            // Initialising all the error in the L1 data structure with false
            FML1_ClrStReg(FltPath);

            // Reset the debounce count to zero
            FML1_ResetDebCnt(FltPath);

            // Set all the flags of the fault in the continuously monitored
            FML1_RSTContPrsnt((u16_FaultPath_t)FltPath);
        }
    }

    // L2 memory
    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_POTENTIAL == FML2_GetPrsntFaultState(FltIndx_u8)) ||
            ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) && (FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) == FML2_GetDrvCyCtr(FltIndx_u8))) ||
            ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) && ((FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) - 1) == FML2_GetDrvCyCtr(FltIndx_u8))))
        {
            if (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8)))
            {
                ClrFltEntry(FltIndx_u8);
            }
        }
    }

    return;
}

/**
 *    @brief  Function to clear the confirmed NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_CONFNOBD_FLTS]@{SDD_DERIVED_FML2_CLR_CONFNOBD_FLTS}
 */
void FML2_ClrCnfrmNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) && (FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) == FML2_GetDrvCyCtr(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            ClrFltEntry(FltIndx_u8);
        }
    }

    return;
}

/**
 *    @brief  Function to clear the deleting NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_DLTG_NOBDFLTS]@{SDD_DERIVED_FML2_CLR_DLTG_NOBDFLTS}
 */
void FML2_ClrDeletingNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_DELETING == FML2_GetPrsntFaultState(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            ClrFltEntry(FltIndx_u8);
        }
    }

    return;
}

/**
 *    @brief  Function to clear all faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_ALLFLTS]@{SDD_DERIVED_FML2_CLR_ALLFLTS}
 */
void FML2_ClrAllFaults(void)
{
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // FML2_ClrAllOBDFaults();
#endif
    FML2_ClrAllNONOBDFaults();

    return;
}

/**
 *    @brief  Function to clear all pending faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_ALLPNDFLTS]@{SDD_DERIVED_FML2_CLR_ALLPNDFLTS}
 */
void FML2_ClrAllPndngFaults(void)
{
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    //  FML2_ClrPndngOBDFaults();
#endif
    FML2_ClrPndngNONOBDFaults();

    return;
}

/**
 *    @brief  Function to clear all confirmed faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_ALLCONF_FLTS]@{SDD_DERIVED_FML2_CLR_ALLCONF_FLTS}
 */
void FML2_ClrAllCnfrmFaults(void)
{
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // FML2_ClrCnfrmOBDFaults();
#endif
    FML2_ClrCnfrmNONOBDFaults();

    return;
}

/**
 *    @brief  Function to clear all deleting faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FML2_CLR_ALLDLTGFLTS]@{SDD_DERIVED_FML2_CLR_ALLDLTGFLTS}
 */
void FML2_ClrAllDeletingFaults(void)
{
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // FML2_ClrDeletingOBDFaults();
#endif
    FML2_ClrDeletingNONOBDFaults();

    return;
}
/**
 *    @brief  : Function to find Number of the all NONOBD related faults in L2 memory
 *    @param  : None.
 *    @return : No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOF_ALLNOBDFLTS]@{SDD_DERIVED_FML2_NOF_ALLNOBDFLTS}
 */
uint8_t FML2_NofAllNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;
    uint8_t Nof_flt_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_NOENTRY != FML2_GetPrsntFaultState(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            Nof_flt_u8++;
        }
    }

    return Nof_flt_u8;
}

/**
 *    @brief  Function to find Number of the pending NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOFPNDG_NOBD_FLT]@{SDD_DERIVED_FML2_NOFPNDG_NOBD_FLT}
 */
u16_FaultPath_t FML2_NofPndngNONOBDFaults(void)
{
    u16_FaultPath_t FltPath = 0;
    uint8_t FltIndx_u8 = 0;
    u16_FaultPath_t Nof_flt_u8 = 0;

    // searchign in L1 memory
    for (FltPath = FM_FAULT_START_E; FltPath < NUM_OF_FAULTPATHS_E; FltPath++)
    {
        if (FALSE == IS_FAULT_OBD_RELEVANT(FltPath))
        {
            // re initialisation
            FltIndx_u8 = 0;

            // searching in L2 memory
            for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
            {
                if (FM_NOENTRY != FML2_GetPrsntFaultState(FltIndx_u8))
                {
                    if (FML2_GetFaultPath(FltIndx_u8) == FltPath)
                    {
                        if (FML2_GetErrType(FltIndx_u8) != FML1_GetErrType(FltPath))
                        {
                            Nof_flt_u8++;
                        }
                    }
                }
            }

            if (NO_ERROR != FML1_GetErrType(FltPath))
            {
                Nof_flt_u8++;
            }
        }
    }

    return Nof_flt_u8;
}

/**
 *    @brief  Function to find Number of the confirmed NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOFCNFRM_NOBD_FLT]@{SDD_DERIVED_FML2_NOFCNFRM_NOBD_FLT}
 */
uint8_t FML2_NofCnfrmNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;
    uint8_t Nof_flt_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) && (FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) == FML2_GetDrvCyCtr(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            Nof_flt_u8++;
        }
    }

    return Nof_flt_u8;
}

/**
 *    @brief  Function to find Number of the deleting NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOF_DLTGNOBDFLTS]@{SDD_DERIVED_FML2_NOF_DLTGNOBDFLTS}
 */
uint8_t FML2_NofDeletingNONOBDFaults(void)
{
    uint8_t FltIndx_u8 = 0;
    uint8_t Nof_flt_u8 = 0;

    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_DELETING == FML2_GetPrsntFaultState(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            Nof_flt_u8++;
        }
    }

    return Nof_flt_u8;
}

/**
 *    @brief  Function to find Number of all faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOF_ALL_FLT]@{SDD_DERIVED_FML2_NOF_ALL_FLT}
 */
uint8_t FML2_NofAllFaults(void)
{
    uint8_t Faults_u8 = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // Faults_u8 = FML2_NofAllOBDFaults();
#endif
    Faults_u8 += FML2_NofAllNONOBDFaults();
    return Faults_u8;
}

/**
 *    @brief  Function to find Number of all pending faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOF_ALLPNDFLTS]@{SDD_DERIVED_FML2_NOF_ALLPNDFLTS}
 */
u16_FaultPath_t FML2_NofAllPndngFaults(void)
{
    u16_FaultPath_t Faults_u16 = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // Faults_u16 = FML2_NofPndngOBDFaults();
#endif
    Faults_u16 += FML2_NofPndngNONOBDFaults();
    return Faults_u16;
}

/**
 *    @brief  Function to find Number of all confirmed faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOF_ALLCONF_FLTS]@{SDD_DERIVED_FML2_NOF_ALLCONF_FLTS}
 */
uint8_t FML2_NofAllCnfrmFaults(void)
{
    u16_FaultPath_t Faults_u16 = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    //  Faults_u16 = FML2_NofCnfrmOBDFaults();
#endif
    Faults_u16 += FML2_NofCnfrmNONOBDFaults();
    return Faults_u16;
}

/**
 *    @brief  Function to find Number of all deleting faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_DERIVED_FML2_NOF_ALLDLTG_FLTS]@{SDD_DERIVED_FML2_NOF_ALLDLTG_FLTS}
 */
uint8_t FML2_NofAllDeletingFaults(void)
{
    uint8_t Faults_u8 = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    //  Faults_u16 = FML2_NofDeletingOBDFaults();
#endif
    Faults_u8 += FML2_NofDeletingNONOBDFaults();
    return Faults_u8;
}
/**
 *    @brief  Function fills the parameter pointer with DTC codes of confirmed
 *            NON OBD faults
 *
 *    @param  Pointer to update the DTC codes
 *
 *    @return Number of DTC codes copied
 *    @code tag - @[CODE_DERIVED_FML2_GET_ALLCONFNOBD_DTCS]@{SDD_DERIVED_FML2_GET_ALLCONFNOBD_DTCS}
 */
int8_t _FML2_GetAllCnfrmNONOBD_DTCs(uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    uint8_t FltIndx_u8 = 0;
    int8_t Nof_DTCs = 0;
    uint32_t DTC_Value = 0;

    // searching in L2 memory
    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) &&
            (!IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            // re intialising
            DTC_Value = 0;

            // fetch the DTC value from the DTC table
            DTC_Value = GetDTCofFltPathFltType(FML2_GetFaultPath(FltIndx_u8), (uint8_t)FML2_GetErrType(FltIndx_u8), FM_ProtocolType_En);

            if (DTC_Value)
            {
                // copy the value to pointer passed
                DTCVal_pu32[Nof_DTCs] = DTC_Value;

                // incrementing the number of DTC's copied
                Nof_DTCs++;
            }
        }
    }

    return Nof_DTCs;
}

/**
 *    @brief  Function fills the parameter pointer with DTC codes of confirmed
 *            OBD and NON OBD faults
 *
 *    @param  Pointer to update the DTC codes
 *
 *    @return Number of DTC codes copied
 *    @code tag - @[CODE_DERIVED_FML2_GET_ALLCONF_DTCS]@{SDD_DERIVED_FML2_GET_ALLCONF_DTCS}
 */
int8_t _FML2_GetAllCnfrmDTCs(uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    int8_t Nof_DTCs = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    /* uint16_t num_DTC_req_u16 = FM_CONF_MAX_L2FAULT_ENTRIES;
      // invoking for OBD DTCs
      Nof_DTCs = _FML2_GetAllCnfrmOBD_DTCs(DTCVal_pu32, num_DTC_req_u16, FM_ProtocolType_En); */
#endif
    // invoking for NON-OBD DTCs
    Nof_DTCs += _FML2_GetAllCnfrmNONOBD_DTCs((DTCVal_pu32 + Nof_DTCs), FM_ProtocolType_En);

    return Nof_DTCs;
}
/**
 *    @brief  Function fills the parameter pointer with DTC code of
 *            Nth(EntryNum_u8) entry NON OBD fault
 *
 *    @param  Entry Number in L2 memory
 *    @param  Pointer to update the DTC codes
 *
 *    @return -1 - No OBD DTC exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_DERIVED_FML2_GET_CONFNOBD_DTC]@{SDD_DERIVED_FML2_GET_CONFNOBD_DTC}
 */
int8_t FML2_GetCnfrmNONOBD_DTC(uint8_t EntryNum_u8, uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    int8_t Ret_Status = -1;
    uint8_t Index_u8 = 0;
    uint8_t FltIndx_u8 = 0;
    uint16_t DTC_Value = 0;

    if (EntryNum_u8 > 0 && EntryNum_u8 <= FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        // searching in L2 memory
        for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
        {
            if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) && (FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) == FML2_GetDrvCyCtr(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
            {
                Index_u8++;

                if (EntryNum_u8 == Index_u8)
                {
                    // fetch the DTC value from the DTC table
                    DTC_Value = GetDTCofFltPathFltType(FML2_GetFaultPath(FltIndx_u8), (uint8_t)FML2_GetErrType(FltIndx_u8), FM_ProtocolType_En);

                    if (DTC_Value)
                    {
                        // copy the value to pointer passed
                        *DTCVal_pu32 = DTC_Value;

                        // updating success
                        Ret_Status = 0;
                    }

                    // breaking from loop
                    break;
                }
            }
        }
    }

    return Ret_Status;
}

/**
 *    @brief  Function fills the parameter pointer with DTC code of
 *            Nth(EntryNum_u8) entry of fault(inclusive of OBD and NON-OBD)
 *
 *    @param  Entry Number in L2 memory
 *    @param  Pointer to update the DTC codes
 *
 *    @return -1 - No OBD DTC exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_DERIVED_FML2_GET_CONFDTC]@{SDD_DERIVED_FML2_GET_CONFDTC}
 */
int8_t FML2_GetCnfrmDTC(uint8_t EntryNum_u8, uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    int8_t Ret_Status = -1;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    //  Ret_Status = FML2_GetCnfrmOBD_DTC(EntryNum_u8, DTCVal_pu32, FM_ProtocolType_En);
#endif
    if (-1 == Ret_Status)
    {
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
        /* To do : For this project OBD faults are not required. So commented this for J1939. */
        // subtracting the numberof OBD faults
        //  EntryNum_u8 = (uint8_t)(EntryNum_u8 - FML2_NofCnfrmOBDFaults());
#endif
        Ret_Status = FML2_GetCnfrmNONOBD_DTC(EntryNum_u8, DTCVal_pu32, FM_ProtocolType_En);
    }
    return Ret_Status;
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of NON OBD related entries in L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return Number of NON OBD related faults in L2 memory
 *    @code tag - @[CODE_DERIVED_FML2_GET_ALLCONFNOBD_FLTPTH]@{SDD_DERIVED_FML2_GET_ALLCONFNOBD_FLTPTH}
 */
int8_t FML2_GetAllCnfrmNONOBD_FltPaths_FltTypes(uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
    int8_t Nof_Flt_s8 = 0;
    uint8_t FltIndx_u8 = 0;

    // searching in L2 memory
    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) && (FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) == FML2_GetDrvCyCtr(FltIndx_u8)) && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            // copying the fault path and fault type
            *FltPath_pu16 = FML2_GetFaultPath(FltIndx_u8);
            *FltType_pu8 = (uint8_t)FML2_GetErrType(FltIndx_u8);

            // incrementing the pointed location
            FltPath_pu16++;
            FltType_pu8++;

            // incrementing the counter
            Nof_Flt_s8++;
        }
    }

    return Nof_Flt_s8;
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of all (OBD + NON-OBD) entries in L2 memory.
 *            Function will copy OBD faukts first followed by NOn-OBD faults
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return Number of OBD + NON-OBD related faults in L2 memory
 *    @code tag - @[CODE_DERIVED_FML2_GET_ALLCONF_FLTPTH]@{SDD_DERIVED_FML2_GET_ALLCONF_FLTPTH}
 */
int8_t FML2_GetAllCnfrm_FltPaths_FltTypes(uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
    int8_t Nof_Flt_s8 = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    //   Nof_Flt_s8 = FML2_GetAllCnfrmOBD_FltPaths_FltTypes(FltPath_pu16, FltType_pu8);
#endif
    Nof_Flt_s8 += FML2_GetAllCnfrmNONOBD_FltPaths_FltTypes((FltPath_pu16 + Nof_Flt_s8), (FltType_pu8 + Nof_Flt_s8));

    return Nof_Flt_s8;
}
/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of particular(EntryNum_u8) NON-OBD entry in L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return -1 - No NON OBD fault exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_DERIVED_FML2_GET_CONF_NOBD_FLTPTH]@{SDD_DERIVED_FML2_GET_CONF_NOBD_FLTPTH}
 */
int8_t FML2_GetCnfrmNONOBD_FltPath_FltType(uint8_t EntryNum_u8, uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
    int8_t Ret_Status = -1;
    uint8_t Index_u8 = 0;
    uint8_t FltIndx_u8 = 0;

    if (EntryNum_u8 > 0 && EntryNum_u8 <= FM_CONF_MAX_L2FAULT_ENTRIES)
    {
        // searching in L2 memory
        for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
        {
            if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) &&
                (FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8)) == FML2_GetDrvCyCtr(FltIndx_u8)) &&
                (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
            {
                Index_u8++;

                if (EntryNum_u8 == Index_u8)
                {
                    // Updating fault path and type
                    *FltPath_pu16 = FML2_GetFaultPath(FltIndx_u8);
                    *FltType_pu8 = (uint8_t)FML2_GetErrType(FltIndx_u8);

                    // updating success
                    Ret_Status = 0;

                    // breaking from loop
                    break;
                }
            }
        }
    }

    return Ret_Status;
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of particular(EntryNum_u8) OBD or NON-OBD  entry in
 *            L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return -1 - No OBD or NON-OBD fault exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_DERIVED_FML2_GET_CONFFLTPTH]@{SDD_DERIVED_FML2_GET_CONFFLTPTH}
 */
int8_t FML2_GetCnfrm_FltPath_FltType(uint8_t EntryNum_u8, uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
    int8_t Ret_Status = -1;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // Ret_Status = FML2_GetCnfrmOBD_FltPath_FltType(EntryNum_u8, FltPath_pu16, FltType_pu8);
#endif
    if (-1 == Ret_Status)
    {
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
        /* To do : For this project OBD faults are not required. So commented this for J1939. */
        // subtracting the numberof OBD faults
        //  EntryNum_u8 = (uint8_t)(EntryNum_u8 - FML2_NofCnfrmOBDFaults());
#endif
        Ret_Status = FML2_GetCnfrmNONOBD_FltPath_FltType(EntryNum_u8, FltPath_pu16, FltType_pu8);
    }

    return Ret_Status;
}

/**
 *    @brief  function to delete an entry of L2 based on DTC value.
 *    algorithm:
 *    1. read DTC from requested command
 *    2. extract fault path & fault type from DTC value
 *    3. Search whether the fault path & fault type is present in L2 memory. get the fault index in L2.
 *    4. if present in L2, delete the entry using fault index
 *    7. If not present even in L2, return -1. (this case does not arise)
 *
 *    @ param: Requested DTC value
 *
 *    @return  -1 = failure condition
 *             0 = success
 */
int16_t FML2_ClrL2Err_ByDTC(uint16_t DTC_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    u16_FaultPath_t FaultPath_En = 0;
    int16_t RetVal_s16 = -1;
    uint8_t FltType_u8 = 0;
    int8_t FltIndex_s8 = -1;

    RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u16, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

    if (RetVal_s16 >= 0)
    {
        /* DTC found in L2 memory */
        FltIndex_s8 = FML2_GetFltIndxOfPath(FaultPath_En, FltType_u8);
        ClrFltEntry((uint8_t)FltIndex_s8);
        return 0;
    }
    /* no DTC found */
    return -1;
}

/**
 *    @brief  Function returns the list of all (OBD & non OBD) 'Deleting' stated DTCs.
 *
 *    @param  result buffer with dtc list.
 *
 *    @return Number of deleting faults in L2 memory.
 *
 */
int8_t _FML2_GetAllDeletingDTCs(uint16_t *DTCVal_pu16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    int8_t Nof_DTCs = 0;
#if ((TRUE == FM_OBD_SUPPORTED) || (TRUE == FM_J1939_SUPPORTED))
    /* To do : For this project OBD faults are not required. So commented this for J1939. */
    // uint16_t num_DTC_req_u16 = FM_CONF_MAX_L2FAULT_ENTRIES;
    // invoking for OBD DTCs
    // Nof_DTCs = _FML2_GetAllDeletingOBD_DTCs(DTCVal_pu16, num_DTC_req_u16, FM_ProtocolType_En);
#endif
    // invoking for NON-OBD DTCs
    Nof_DTCs += _FML2_GetAllDeletingNONOBD_DTCs((DTCVal_pu16 + Nof_DTCs), FM_ProtocolType_En);

    return Nof_DTCs;
}
/**
 *    @brief  Function returns the list of non OBD specific 'Deleting' stated DTCs.
 *
 *    @param  result buffer with dtc list.
 *
 *    @return Number of deleting faults in L2 memory.
 *
 */
int8_t _FML2_GetAllDeletingNONOBD_DTCs(uint16_t *DTCVal_pu16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    uint8_t FltIndx_u8 = 0;
    int8_t Nof_DTCs = 0;
    uint16_t DTC_Value = 0;

    // searching in L2 memory
    for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
    {
        if ((FM_DELETING == FML2_GetPrsntFaultState(FltIndx_u8))
            // && (FM_CONF_NOF_NONOBD_DELETECNTS == FML2_GetDrvCyCtr(FltIndx_u8))
            && (FALSE == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))))
        {
            // re intialising
            DTC_Value = 0;

            // fetch the DTC value from the DTC table
            DTC_Value = GetDTCofFltPathFltType(FML2_GetFaultPath(FltIndx_u8), (uint8_t)FML2_GetErrType(FltIndx_u8), FM_ProtocolType_En);

            if (DTC_Value)
            {
                // copy the value to pointer passed
                *(DTCVal_pu16 + Nof_DTCs) = DTC_Value;

                // incrementing the number of DTC's copied
                Nof_DTCs++;
            }
        }
    }

    return Nof_DTCs;
}

/**
 *    @brief  Function to return whether given DTC is present in the FM-L2 memory.
 *
 *    @param  DTC (32 bit).
 *
 *    @return 	-1: Not present.
 *	        >=0 : index of the fault
 *
 */
int16_t FML2_GetFaultIndexByDTC(uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
    u16_FaultPath_t FaultPath_En = 0;
    int16_t Ret_s16 = -1;
    uint8_t FltType_u8 = 0;
    int8_t FltIndx_u8 = -1;

    Ret_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

    if (0 == Ret_s16)
    {
        for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
        {
            if (FML2_GetErrType(FltIndx_u8))
            {
                return FltIndx_u8;
            }
        }
    }
    return Ret_s16;
}

/**
 *    @brief  Function to update the freeze frame information from L1 to L2.
 *
 *    @param  Fault path and index
 *
 *    @return 	-1: Not present.
 *	        >=0 : index of the fault
 *
 */
int16_t Copy_FrzFrz_From_L1_to_L2(u16_FaultPath_t FltPath, int8_t L2Index_au8)
{
    uint32_t i = 0;

#if (TRUE == FM_UDS_SUPPORTED)
    for (i = 0; i < FMFF_CONF_UDS_FRZFRM_TOTALBYTES; i++)
    {
        FML2_FaultMem_aSt[L2Index_au8].L2FrzFrm_UDS_au8[i] = FaultPath_aSt[FltPath].L1FrzFrm_UDS_au8[i];
    }
#endif

#if (TRUE == FM_OBD_SUPPORTED)
    for (i = 0; i < FMFF_CONF_OBD_FRZFRM_TOTALBYTES; i++)
    {
        FML2_FaultMem_aSt[L2Index_au8].L2FrzFrm_OBD_au8[i] = FaultPath_aSt[FltPath].L1FrzFrm_OBD_au8[i];
    }

#endif

#if (TRUE == FM_J1939_SUPPORTED)
    /* To do : For this project OBD faults are not required. So commented this freeze frame info for J1939. */
    // for (i = 0; i < FMFF_CONF_J1939_FRZFRM_TOTALBYTES; i++)
    // {
    //     FML2_FaultMem_aSt[L2Index_au8].L2FrzFrm_J1939_au8[i] = FaultPath_aSt[FltPath].L1FrzFrm_J1939_au8[i];
    // }

#endif

    FML2_SetUpt_St(L2Index_au8);
    return 0;
}

int8_t FML2_Get_DTCs(uint32_t *DTC_pu32, uint16_t numdtc_req_u16, FM_ProtocolType_En_t FM_ProtocolType_En, FM_Fault_Type_En_t FM_Fault_Type_En, FM_Fault_Condition_En_t FM_Fault_Condition_En)
{
    uint8_t FltIndx_u8 = 0;
    int8_t Nof_DTCs = 0;
    uint16_t DTC_Value = 0;
    bool OBD_Relevent_flag_b = TRUE;
    bool flag1_b = FALSE;
    uint8_t count = 1;
    uint8_t i = 0;
    FML2_FaultState_En_t FML2_FaultState_En = FM_NOENTRY;
    if (FM_Fault_Type_En == FM_ALL_E)
    {
        count = 2;
        FM_Fault_Type_En = FM_OBD_E;
    }
    if (FM_Fault_Condition_En == FM_CONFIRMED_E)
    {
        FML2_FaultState_En = FM_CONFIRMED;
    }
    else if (FM_Fault_Condition_En == FM_DELETING_E)
    {
        FML2_FaultState_En = FM_DELETING;
    }
    for (i = 0; i < count; i++)
    {
        // searching in L2 memory
        for (FltIndx_u8 = 0; ((FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES) && numdtc_req_u16); FltIndx_u8++)
        {
            flag1_b = FALSE;
            OBD_Relevent_flag_b = TRUE;
            if (FM_OBD_E == FM_Fault_Type_En)
            {
                OBD_Relevent_flag_b = IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8));
                if (FM_Fault_Condition_En != FM_DELETING_E)
                {
                    if (FM_CONF_NOF_OBD_HEALINGCNTS == FML2_GetDrvCyCtr(FltIndx_u8))
                    {
                        flag1_b = TRUE;
                    }
                    else
                    {
                        flag1_b = FALSE;
                    }
                }
                else
                {
                    flag1_b = TRUE;
                }
            }
            else if (FM_NONOBD_E == FM_Fault_Type_En)
            {
                OBD_Relevent_flag_b = !(IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8)));
                // if(FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8) == FML2_GetDrvCyCtr(FltIndx_u8) )
                // {
                flag1_b = TRUE;
                // }
            }
            else
            {
                flag1_b = TRUE;
                OBD_Relevent_flag_b = TRUE;
            }
            if ((FML2_FaultState_En == FML2_GetPrsntFaultState(FltIndx_u8)) // do compare with the FM_Fault_Condition_En_t with  FML2_FaultState_En_t
                && (TRUE == flag1_b) && (TRUE == OBD_Relevent_flag_b))
            {
                // re intialising
                DTC_Value = 0;

                // fetch the DTC value from the DTC table
                DTC_Value = GetDTCofFltPathFltType(FML2_GetFaultPath(FltIndx_u8), (uint8_t)FML2_GetErrType(FltIndx_u8), FM_ProtocolType_En);
                DTC_Value = (DTC_Value & (0x01FFFFFF | ((uint32_t)FML2_GetOccurCnt(FltIndx_u8) << 25U)));
                if (DTC_Value)
                {
                    // copy the value to pointer passed
                    *(DTC_pu32 + Nof_DTCs) = DTC_Value;

                    // incrementing the number of DTC's copied
                    Nof_DTCs++;
                    numdtc_req_u16--;
                }
            }
        }
        FM_Fault_Type_En = FM_NONOBD_E;
    }

    return Nof_DTCs;
}
