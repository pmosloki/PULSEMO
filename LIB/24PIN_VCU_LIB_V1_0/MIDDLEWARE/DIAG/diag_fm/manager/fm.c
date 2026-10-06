/***************************************************************************************************
 *    FILENAME    : fm.c
 *
 *    DESCRIPTION : File contains the common functions related to Fault manager
 *                  level 1 and Level 2. It monitors and updates the common
 *                  data used for Fault manager
 *
 *    $Id         : $
 *
 ***************************************************************************************************
 * Revision history
 *
 * Ver Author       Date       Description
 *
 * 1   Sloki             25/09/2008
 ***************************************************************************************************
 */

/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
#include "fmdtc_conf.h"
#include "uds_DTC.h"
#include "fm.h"
#include "fm_level1.h"
#include "fm_level2.h"
#include "fee_conf.h"
#include "app_typedef.h"
#include "diag_typedefs.h"
#include "diag_sys_conf.h"
#include "diag_adapt.h"
#include <stdbool.h>
#include <stdlib.h>
// #include "fm_stm.h"
/*
 ***************************************************************************************************
 *    Defines
 ***************************************************************************************************
 */
#define NVM_NOPE (U1)0
#define READ_NVM (U1)1
#define WRITE_NVM (U1)2
/*
 **************************************************************************************************
 *    Variables
 **************************************************************************************************
 */
bool FM_DTC_SettingsOff_b = false;
// FM_ProtocolType_En_t FM_ProtocolTypeSet_En = FM_NO_PROTO_E;

// const u16_FaultPath_t u2_FM_VOLT_EN_TBL[NUM_OF_FAULTPATHS_E] =
// 	{
// 		off,
// 		on,
// 		on,
// //		off,

// };
#ifdef NVM_BACKUPRAM
static U1 u1_dtcbitmask_step = NVM_NOPE;
static uint16_t DTC_loop_index;
#endif
static uint64_t u64_DTCBitMask = 0x0000000000000000;
// static uint8_t	    u1_DTC_NVM_step;

// Raw ADC value for freeze frames.
/*
@@ SYMBOL     = ADCRawCntMAP_u16
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint16_t$
@@ CONVERSION = LINEAR $RADIX_4$ "cnts"
@@ DESCRIPTION= "MAP raw counts for FM"
@@ END
*/
uint16_t ADCRawCntMAP_u16;
/*
@@ SYMBOL     = ADCRawCntACT_u16
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint16_t$
@@ CONVERSION = LINEAR $RADIX_4$ "cnts"
@@ DESCRIPTION= "ACT raw counts for FM"
@@ END
*/
uint16_t ADCRawCntACT_u16;

// RAM copy of the common data
/*
@@ INSTANCE  = FML2_CommonNVData_St
@@ STRUCTURE = FML2_CommonData_St_t
@@ END
*/
FML2_CommonData_St_t FML2_CommonNVData_St;

// Driving cycle Indicator - The flag is set 11 seconds after the engine is ON
/*
@@ SYMBOL     = FM_DrvCyInd_b
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint8_t$
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "driving cycle indicator"
@@ END
*/
bool FM_DrvCyInd_b;

// WUP cycle Indicator - The flag is set when engine coolant temperature
// has increased by 275 degK and the minimum temperature of
// 343 degK (70 deg Celcius).
/*
@@ SYMBOL     = FM_WUPCyInd_b
@@ A2L_TYPE   = PARAMETER
@@ DATA_TYPE  = $uint8_t$
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "WUP cycle indicator"
@@ END
*/
bool FM_WUPCyInd_b;

// Operating hours for the current driving cycle
#if EUD_HDV_2005_55_EC
uint16_t OperMin_CrntDryCyl_u16;
#endif

static void FM_Inc_DistDrv_MIL_ON_Km(uint16_t);
static void FM_Inc_DistDrv_DTC_Clr_Km(uint16_t);
static void FM_IncTime_MIL_ON_Min(uint16_t);
static void FM_IncTime_DTC_Clr_Min(uint16_t);
static void FM_IncWupCyl_DTC_Clr(void);
static void FM_IncWupCyl_MIL_Off(void);
// static void vd_Write_DTCBitMask_BkupRAM(void);
// static void vd_Read_DTCBitMask_BkupRAM(void);
static void FM_IncDrvCycleCntForFaultyIndex(void);
static void FM_TestFailedSinceLastClear_Proc(void);

/*
 **************************************************************************************************
 *    Functions
 **************************************************************************************************
 */
/**
 *    @brief  Task to update the status flag - Test Failed Since Last Clear.
 *			 The task continuously checks the fault status, if present then it updates flag.
 *
 *
 *    @param  None
 *    @return None
 */

static void FM_TestFailedSinceLastClear_Proc(void)
{
	uint8_t FltIndx_u8 = 0;
	u16_FaultPath_t FltPath = 0;

	for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
	{
		if (FM_NOENTRY != FML2_GetPrsntFaultState(FltIndx_u8))
		{
			FltPath =(u16_FaultPath_t)(FML2_GetFaultPath(FltIndx_u8));
			FML1_SET_TFSLC_STATUS_OF_DTC_MAPTBL(FltPath);
		}
	}
	return;
}

void FM_HandlePendingFault_proc()
{
	uint8_t FltIndx_u8 = 0;
	for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
	{
		if ((FM_POTENTIAL == FML2_GetPrsntFaultState(FltIndx_u8)) ||
			((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) &&
			 (((FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8))-FML2_GetDrvCyCtr(FltIndx_u8)) < FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8))) ||
			 (0u == FM_CONF_NOF_NONOBD_HEALINGCNTS(FML2_GetFaultPath(FltIndx_u8))))))
		{
			FML1_SET_PDTC_STATUS_OF_DTC_MAPTBL(FML2_GetFaultPath(FltIndx_u8));
		}

		if ((FM_CONFIRMED == FML2_GetPrsntFaultState(FltIndx_u8)) || ((FM_DELETING == FML2_GetPrsntFaultState(FltIndx_u8)) && 
		((FM_CONF_NOF_NONOBD_DELETECNTS(FML2_GetFaultPath(FltIndx_u8)) - FML2_GetWUPCyCtr(FltIndx_u8)) < FM_CONF_NOF_NONOBD_DELETECNTS(FML2_GetFaultPath(FltIndx_u8)))))
		{
			FML1_SET_CDTC_STATUS_OF_DTC_MAPTBL(FML2_GetFaultPath(FltIndx_u8));
			if((false == IncCntState_ab[FltIndx_u8]) && (FML2_GetOccurCnt(FltIndx_u8) < 126U))
			{
				FML2_IncOccurCnt(FltIndx_u8);
                IncCntState_ab[FltIndx_u8] = true;
			}
			else
			{
				/* Do Nothing */
			}
		}
	}
}

/**
 *    @brief  Schedules the Fault manager related tasks.
 *
 *            1)This task is invoked by main task scheduler every 50 millisecond
 *            2)Before invoking this function task scheduler shall call the
 *              initilisation routine'FM_Init'
 *            3)Invoking the shutdown operation'FM_PowerOff'is again the
 *              responsibility of Task scheduler
 *
 *    @param  None
 *    @return None
 *    Added in System_scheduler.c
 */
void FM_Sched_proc(void)
{
	static uint32_t EntryCnt_u32 = 0;

	//----------------------------------//
	//----All 50 Milli second Task------//
	//----------------------------------//
	FM_MonDrvCy_proc();
	FM_WUPCy_proc();
	FML2_TestMIL_proc();

	if (MIL_TEST_DONE == MIL_Test_Mode_u8)
	{
		FML2_MonWarnLamp_proc();
	}

	//----------------------------------//
	//----All 100 Milli second Task-----//
	//----------------------------------//
	if (0 == (EntryCnt_u32 % FM_100MS_SCHEDULER_CNT))
	{
		FML2_FltHndl_proc();
	}

	//----------------------------------//
	//----All 1000 Milli second Task----//
	//----------------------------------//
	if (0 == (EntryCnt_u32 % FM_1000MS_SCHEDULER_CNT))
	{
		FML1_UpdtNVMem_proc();
		FML2_UpdtNVMem_proc();

		FM_TestFailedSinceLastClear_Proc();
		FM_HandlePendingFault_proc();
	}

	//----------------------------------//
	//-------All 1 Minute Task----------//
	//----------------------------------//
	if (0 == (EntryCnt_u32 % FM_1MIN_SCHEDULER_CNT))
	{
		FM_HndlCommonCntrs_proc();
		FM_UpdtCommonData_proc();

#if EUD_HDV_2005_55_EC
		FML2_FltHndl_EUD_HDV_2005_55_EC_proc();
#endif
	}

	//----------------------------------//
	//-------Counter increment----------//
	//----------------------------------//
	EntryCnt_u32++;

	return;
}

/**
 *    @brief  Task to monitor the driving cycle. The task will update the global
 *            flag "FM_DrvCyInd_b".
 *            This is scheduled every 50 millisecond
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_MONDRVCY]@{SDD_FM_MONDRVCY}
 */
void FM_MonDrvCy_proc(void)
{
	if (FALSE == FM_GET_DryCylInd())
	{
		if (DRIVING_CYCLE_TRIGGER_SEC <= FM_GET_TIME_SINCE_ENGINE_ON_SEC())
		{
			FM_SET_DryCylInd();
			// FM_IncDrvCycleCntForFaultyIndex();
		}
	}

	return;
}

/**
 *    @brief  Function to increment the operation cycle only once per driving cycle for faulty entries.
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_MONDRVCY]@{SDD_FM_MONDRVCY}
 */
void FM_IncDrvCycleCntForFaultyIndex(void)
{
	// static bool	 DrvCycTriggered_b = FALSE;

	// if (FALSE == DrvCycTriggered_b)
	// {
	// DrvCycTriggered_b = FM_GET_DryCylInd();

	if (FM_GET_DryCylInd())
	{
		int8_t i = 0; // loop count

		for (i = 0; i < FM_CONF_MAX_L2FAULT_ENTRIES; i++)
		{
			if (FM_NOENTRY != FML2_GetPrsntFaultState(i))
			{
				if (FML2_FaultMem_aSt[i].OperationCycle_u16 < 0xFFFF)
				{
					FML2_FaultMem_aSt[i].OperationCycle_u16++;
				}
			}
			else
			{
				FML2_FaultMem_aSt[i].OperationCycle_u16 = 0;
			}
		}
	}
	// }

	return;
}

/**
 *    @brief  Task to monitor the warm up cycle. The task will update the global
 *            flag "FM_WUPCyInd_b".
 *            This is scheduled every 50 millisecond
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_MONWUPCY]@{SDD_FM_MONWUPCY}
 */
void FM_WUPCy_proc(void)
{
	static bool Alrdy_Entrd_b = FALSE; // variable which holds the entry information

	// already entered?
	if (TRUE == Alrdy_Entrd_b)
	{
		FM_SET_WUPCylInd();
	}
	else
	{
		Alrdy_Entrd_b = TRUE;
	}

	return;
}

/**
 *    @brief  Task to handle the common objects like duration counter and
 *            driven distance.
 *            1:The task is scheduled by Fault manager scheduler every one min
 *            2:Distance driven since DTCs are cleared  -(16bit counter)
 *            3:Time Duration since DTCs are cleared -(16bit counter)
 *            4:Number of Warm-up cycles since DTCs are cleared -(8bit counter)
 *
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_HNDLCOMMONCNTRS_PROC]@{SDD_FM_HNDLCOMMONCNTRS_PROC}
 */
void FM_HndlCommonCntrs_proc(void)
{
	static uint16_t MILOnDist_Km_u16 = 0;	  // distance driven since MIL on
	static uint16_t DtcClrDist_Km_u16 = 0;	  // distance driven DTC is cleared
	static uint16_t MILOnTime_Min_u16 = 0;	  // MIL on time
	static uint16_t DtcClrTime_Min_u16 = 0;	  // Time since DTC is cleared
	static bool WUPCyl_Cntr_UpdSt1_b = FALSE; // warm up cycle counter
	static bool WUPCyl_Cntr_UpdSt2_b = FALSE; // warm up cycle counter

	//-------------------------------------------//
	//----Distance driven in Meters with MIL ON-----//
	//-------------------------------------------//
	if (TRUE == FML2_StatusOf_MI())
	{
		if (!MILOnDist_Km_u16)
		{
			MILOnDist_Km_u16 = GET_DIST_SINCE_ENGINE_ON();
		}
		else
		{
			// updating the timer variable
			FM_Inc_DistDrv_MIL_ON_Km((uint16_t)(GET_DIST_SINCE_ENGINE_ON() - MILOnDist_Km_u16));
			FM_Set_CmnCntr_UptSt();

			MILOnDist_Km_u16 = GET_DIST_SINCE_ENGINE_ON();
		}
	}
	else
	{
		MILOnDist_Km_u16 = 0;
	}

	//-------------------------------------------//
	//--Distance driven in Meters since DTC Clear---//
	//-------------------------------------------//
	if (!DtcClrDist_Km_u16)
	{
		DtcClrDist_Km_u16 = GET_DIST_SINCE_ENGINE_ON();
	}
	else
	{
		// updating the timer variable
		FM_Inc_DistDrv_DTC_Clr_Km((uint16_t)(GET_DIST_SINCE_ENGINE_ON() - DtcClrDist_Km_u16));
		FM_Set_CmnCntr_UptSt();

		DtcClrDist_Km_u16 = GET_DIST_SINCE_ENGINE_ON();
	}

	//-------------------------------------------//
	//---Operating Time in minutes with MIL on---//
	//-------------------------------------------//
	if (TRUE == FML2_StatusOf_MI())
	{
		if (!MILOnTime_Min_u16)
		{
			MILOnTime_Min_u16 = (uint16_t)GET_TIME_SINCE_ENGINE_ON_MIN();
		}
		else
		{
			// updating the timer variable
			FM_IncTime_MIL_ON_Min((uint16_t)(GET_TIME_SINCE_ENGINE_ON_MIN() - MILOnTime_Min_u16));
			FM_Set_CmnCntr_UptSt();

			MILOnTime_Min_u16 = (uint16_t)GET_TIME_SINCE_ENGINE_ON_MIN();
		}
	}
	else
	{
		// Reset MIL Timer if MIL was not active for 40 driving cycles
		if (FM_GET_WUPCYL_MIL_OFF() >= FM_CONF_NOF_OBD_DELETECNTS)
		{
			FM_RST_TIME_MIL_ON_MIN();
			FM_RST_WUPCYL_MIL_OFF();
			FM_Set_CmnCntr_UptSt();
		}

		MILOnTime_Min_u16 = 0;
	}

	//-------------------------------------------//
	//-Operating Time in minutes since DTC Clear-//
	//-------------------------------------------//
	if (!DtcClrTime_Min_u16)
	{
		DtcClrTime_Min_u16 = (uint16_t)GET_TIME_SINCE_ENGINE_ON_MIN();
	}
	else
	{
		// updating the timer variable
		FM_IncTime_DTC_Clr_Min((uint16_t)(GET_TIME_SINCE_ENGINE_ON_MIN() - DtcClrTime_Min_u16));
		FM_Set_CmnCntr_UptSt();

		DtcClrTime_Min_u16 = (uint16_t)GET_TIME_SINCE_ENGINE_ON_MIN();
	}

	//-------------------------------------------//
	//--Number of Warmup cycles since DTC Clear--//
	//-------------------------------------------//
	if (FALSE == WUPCyl_Cntr_UpdSt1_b)
	{
		if (TRUE == FM_GET_WUPCylInd())
		{
			FM_IncWupCyl_DTC_Clr();
			FM_Set_CmnCntr_UptSt();
			WUPCyl_Cntr_UpdSt1_b = TRUE;
		}
	}

	//-------------------------------------------//
	//-Number of Warmup cycles since MIL not active-//
	//-------------------------------------------//
	if (FALSE == WUPCyl_Cntr_UpdSt2_b)
	{
		if (TRUE == FM_StatusOf_MI())
		{
			FM_RST_WUPCYL_MIL_OFF();
			FM_Set_CmnCntr_UptSt();
		}

		if ((TRUE == FM_GET_WUPCylInd()) && (FALSE == FM_StatusOf_MI()))
		{
			FM_IncWupCyl_MIL_Off();
			FM_Set_CmnCntr_UptSt();
			WUPCyl_Cntr_UpdSt2_b = TRUE;
		}
	}

	return;
}

/**
 *    @brief  Function to clear the common data in fault memory, such as
 *            duration counter, distance driven and MIL status.
 *            The function resets the common data values to zero and same is
 *            updated to NV memory
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_DERIVED_CLR_COMMON_DATA]@{SDD_FM_DERIVED_CLR_COMMON_DATA}
 */
void FM_ClrCommonData(void)
{
	// resetting all the common data
	FM_RST_TotalErrorHandle();
	FM_RST_DIST_DRV_MIL_ON_KM();
	FM_RST_DIST_DRV_DTC_CLR_KM();
	FM_RST_TIME_MIL_ON_MIN();
	FM_RST_TIME_DTC_CLR_MIN();
	FM_RST_WUPCYL_DTC_CLR();
	FM_RST_Entries_In_L2();
	FM_RST_WUPCYL_MIL_OFF();
	// setting the change flag
	FM_Set_CmnCntr_UptSt();

	return;
}

/**
 *    @brief  The function ensures that common counters are stored on to NV
 *            This is scheduled every 1 min
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_DERIVED_UPDT_COMMON_DATA_PROC]@{SDD_FM_DERIVED_UPDT_COMMON_DATA_PROC}
 */
void FM_UpdtCommonData_proc(void)
{
	if (TRUE == FM_Get_CmnCntr_UptSt())
	{
		// clear the updated flag
		FM_Clr_CmnCntr_UptSt();

		// write common data to NV memory
		// FDL_write(12,1);
		FEE_Write(FM_COMMON_DATA, (uint8_t *)&FML2_CommonNVData_St); //Block 12
//		   FDL_write(FEE_AddrConfig_aSt[FM_COMMON_DATA].StartAddress_u32, FEE_AddrConfig_aSt[FM_COMMON_DATA].Size_u16, (uint8_t*)&FML2_CommonNVData_St);
		// FDL_write(0x000010,10,(uint8_t *)&FML2_CommonNVData_St);
	}

	return;
}

/**
 *    @brief  Function which executes Initialisation functionalities FM in seequence
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_INIT]@{SDD_FM_INIT}
 */
void FM_Init(void)
{
	// copy the common data from Flash to RAM
	FEE_Read(FM_COMMON_DATA, (uint8_t *)&FML2_CommonNVData_St);
	//FDL_Read(FEE_AddrConfig_aSt[FM_COMMON_DATA].StartAddress_u32, FEE_AddrConfig_aSt[FM_COMMON_DATA].Size_u16, (uint8_t *)&FML2_CommonNVData_St);
	

	// initialising driving & warm up cycle variables
	FM_CLR_DryCylInd();
	FM_CLR_WUPCylInd();

	// L1 memory initialisation
	FML1_Init();

	// L2 memory initialisation
	FML2_Init();

	// Occurrence count status flag initialization
	FM_OCcnt_StatusInit();

	return;
}

void FM_NVM_Entry_Write(void)
{
	/* For Write purpose with compare */
#ifdef NVM_BACKUPRAM
	DTC_loop_index = 0;
	u1_DTC_NVM_step = READ_NVM;
	NvmmgrSetReq(u2_REQ_DTCCONFIG_PARAM, ON);
#endif
}

/**
 *    @brief  Function which executes power off functionalities FM in seequence
 *
 *    @param  None.
 *    @return None.
 *    @code tag - @[CODE_FM_PWROFF]@{SDD_FM_PWROFF}
 */
void FM_PowerOff(void)
{
	FM_IncDrvCycleCntForFaultyIndex();

	// update common data
	FM_UpdtCommonData_proc();

	// power off function of L1
	FML1_Rdy_pwroff();

	// power off function of L2
	FML2_PwrOff();

	FM_NVM_Entry_Write();

	return;
}

/**
 *    @brief  Function increments the total number of errors logged in L2 memory.
 *            This variable will be cleared when tester request for clearing all
 *            NONOBD specific faults or if the number of current entries in L2
 *            becomes zero
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INC_ERR_HANDLE]@{SDD_DERIVED_FM_INC_ERR_HANDLE}
 */
void FM_Inc_Error_handle(void)
{
	if (uint32_t_MAX > FM_Get_TotalErrorHandle())
	{
		FM_INC_TotalErrorHandle();
	}

	return;
}

/**
 *    @brief  Function decrements the total number of errors logged in L2 memory.
 *            This variable will be cleared when tester request for clearing all
 *            NONOBD specific faults or if the number of current entries in L2
 *            becomes zero
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_DEC_ERR_HANDLE]@{SDD_DERIVED_FM_DEC_ERR_HANDLE}
 */
void FM_Dec_Error_handle(void)
{
	if (0 < FM_Get_TotalErrorHandle())
	{
		FM_DEC_TotalErrorHandle();
	}

	return;
}

/**
 *    @brief  Function increments the number of entries currently in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INC_NOFENTRIES_L2]@{SDD_DERIVED_FM_INC_NOFENTRIES_L2}
 */
void FM_IncNofEntries_L2(void)
{
	if (FM_CONF_MAX_L2FAULT_ENTRIES > FM_Get_Entries_In_L2())
	{
		FM_INC_Entries_In_L2();
	}

	return;
}

/**
 *    @brief  Function decrements the number of entries currently in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_DEC_NOFENTRIES_L2]@{SDD_DERIVED_FM_DEC_NOFENTRIES_L2}
 */
void FM_DecNofEntries_L2(void)
{
	if (0 < FM_Get_Entries_In_L2())
	{
		FM_DEC_Entries_In_L2();

		if (0 == FM_Get_Entries_In_L2())
		{
			FM_RST_TotalErrorHandle();
		}
	}

	return;
}

/**
 *    @brief  Function Increments Distance Driven with MIL on in Km
 *
 *    @param  Increment in distance since last update
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INC_DISTDRV_MILON_KM]@{SDD_DERIVED_FM_INC_DISTDRV_MILON_KM}
 */
static void FM_Inc_DistDrv_MIL_ON_Km(uint16_t DrvDist_u16)
{
	if ((MAX_DIST_DRVN_MIL_ON_KM - FM_GET_DIST_DRV_MIL_ON_KM()) >= DrvDist_u16)
	{
		FM_UPT_DIST_DRV_MIL_ON_KM((uint16_t)(FM_GET_DIST_DRV_MIL_ON_KM() + DrvDist_u16));
	}
	else
	{
		FM_UPT_DIST_DRV_MIL_ON_KM((uint16_t)MAX_DIST_DRVN_MIL_ON_KM);
	}

	return;
}

/**
 *    @brief  Function Increments Distance Driven since DTC clear in Km
 *
 *    @param  Increment in distance since last update
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INC_DISTDRV_DTCCLR_KM]@{SDD_DERIVED_FM_INC_DISTDRV_DTCCLR_KM}
 */

static void FM_Inc_DistDrv_DTC_Clr_Km(uint16_t DrvDist_u16)
{
	if ((MAX_DIST_DRVN_DTC_CLR_KM - FM_GET_DIST_DRV_DTC_CLR_KM()) >= DrvDist_u16)
	{
		FM_UPT_DIST_DRV_DTC_CLR_KM((uint16_t)(FM_GET_DIST_DRV_DTC_CLR_KM() + DrvDist_u16));
	}
	else
	{
		FM_UPT_DIST_DRV_DTC_CLR_KM((uint16_t)MAX_DIST_DRVN_DTC_CLR_KM);
	}

	return;
}

/**
 *    @brief  Function Increments Time with MIL on in Minutes
 *
 *    @param  Increment in time since last update
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INCTIME_MILON_MIN]@{SDD_DERIVED_FM_INCTIME_MILON_MIN}
 */
static void FM_IncTime_MIL_ON_Min(uint16_t Time_u16)
{
	if ((MAX_TIME_MIL_ON_MIN - FM_GET_TIME_MIL_ON_MIN()) >= Time_u16)
	{
		FM_UPT_TIME_MIL_ON_MIN((uint16_t)(FM_GET_TIME_MIL_ON_MIN() + Time_u16));
	}
	else
	{
		FM_UPT_TIME_MIL_ON_MIN((uint16_t)MAX_TIME_MIL_ON_MIN);
	}

	return;
}

/**
 *    @brief  Function Increments Time since DTC clear in Minutes
 *
 *    @param  Increment in time since last update
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INCTIME_DTCCLR_MIN]@{SDD_DERIVED_FM_INCTIME_DTCCLR_MIN}
 */
static void FM_IncTime_DTC_Clr_Min(uint16_t Time_u16)
{
	if ((MAX_TIME_DTC_CLR_MIN - FM_GET_TIME_DTC_CLR_MIN()) >= Time_u16)
	{
		FM_UPT_TIME_DTC_CLR_MIN((uint16_t)(FM_GET_TIME_DTC_CLR_MIN() + Time_u16));
	}
	else
	{
		FM_UPT_TIME_DTC_CLR_MIN((uint16_t)MAX_TIME_DTC_CLR_MIN);
	}

	return;
}

/**
 *    @brief  Function Increments the warmup cycle count since DTC clear
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_INCWUPCYL_DTCCLR]@{SDD_DERIVED_FM_INCWUPCYL_DTCCLR}
 */
static void FM_IncWupCyl_DTC_Clr(void)
{
	if (MAX_COUNT_WARMUP_CYL > FM_GET_WUPCYL_DTC_CLR())
	{
		FM_UPT_WUPCYL_DTC_CLR((uint8_t)(FM_GET_WUPCYL_DTC_CLR() + 1));
	}
	else
	{
		FM_UPT_WUPCYL_DTC_CLR((uint16_t)MAX_COUNT_WARMUP_CYL);
	}

	return;
}

/**
 *    @brief  Function Increments the warmup cycle count if MIL is not active
 *
 *    @param  None
 *    @return None
 */
static void FM_IncWupCyl_MIL_Off(void)
{
	if (MAX_COUNT_WARMUP_CYL > FM_GET_WUPCYL_MIL_OFF())
	{
		FM_UPT_WUPCYL_MIL_OFF((uint8_t)(FM_GET_WUPCYL_MIL_OFF() + 1));
	}
	else
	{

		FM_UPT_WUPCYL_MIL_OFF((uint16_t)MAX_COUNT_WARMUP_CYL);
	}

	return;
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
 *    @code tag - @[CODE_FM_REPORT_FLT]@{SDD_FM_REPORT_FLT}
 */
void FM_ReportFault(FM_FaultPath_En_t FltPath, uint8_t ErrorType_u8)
{
	// if ((((u64_DTCBitMask >> FltPath) & 0x01)) == TRUE)
	// {
	// 	ErrorType_u8 = FM_NO_ERR;
	// }
	// else
	// {
	// 	;
	// }
	// if ((u2_FM_VOLT_EN_TBL[FltPath] | u1_GetFmEnableSts())
	//		&& (FALSE == FM_DTC_SettingsOff_b))
	if (FALSE == FM_DTC_SettingsOff_b)
	{
		FML1_ReportFault(FltPath, ErrorType_u8);
	}
	return;
}

/**
 *    @brief   Returns the status of DTC mask
 *
 *    @param   FaultPath_En - Application specific Fault Paths as configured
 *             by the application user in Fault manager.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FM_RDY_CMPLT]@{SDD_FM_RDY_CMPLT}
 */
bool FM_DTC_Mask_Sts(u16_FaultPath_t FltPath)
{
	if ((((u64_DTCBitMask >> FltPath) & 0x01)) == TRUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
 *    @brief   Returns the status of readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FM_RDY_CMPLT]@{SDD_FM_RDY_CMPLT}
 */
bool FM_IsRdyComplete(void)
{
	return FML1_IsRdyComplete();
}

/**
 *    @brief   Returns the status of Fuel system readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FM_FUEL_RDY_CMPLT]@{SDD_FM_FUEL_RDY_CMPLT}
 */
bool FM_IsFUELSysRdyComplete(void)
{
	return FML1_IsFUELSysRdyComplete();
}

/**
 *    @brief   Returns the status of EGR system readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FM_EGR_RDY_CMPLT]@{SDD_FM_EGR_RDY_CMPLT}
 */
bool FM_IsEGRSysRdyComplete(void)
{
	return FML1_IsEGRSysRdyComplete();
}

/**
 *    @brief   Returns the status of CCOMP system readiness
 *
 *    @param   None.
 *    @return  TRUE or FALSE
 *    @code tag - @[CODE_FM_CCOPM_RDY_CMPLT]@{SDD_FM_CCOPM_RDY_CMPLT}
 */
bool FM_IsCCOPMSysRdyComplete(void)
{
	return FML1_IsCCOPMSysRdyComplete();
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
 *    @code tag - @[CODE_FM_GET_ERR]@{SDD_FM_GET_ERR}
 */
uint8_t FM_GetErr(u16_FaultPath_t FltPath)
{
	return FML1_GetFltSt(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_DERIVED_FM_GET_ERR_ST]@{SDD_DERIVED_FM_GET_ERR_ST}
 */
bool FM_GetErrSt(u16_FaultPath_t FltPath)
{
	return FML1_GetErrSt(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FM_GET_MAX_ERR]@{SDD_FM_GET_MAX_ERR}
 */
bool FM_GetMaxErr(u16_FaultPath_t FltPath)
{
	return FML1_GetMaxErrFltSt(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FM_GET_MIN_ERR]@{SDD_FM_GET_MIN_ERR}
 */
bool FM_GetMinErr(u16_FaultPath_t FltPath)
{
	return FML1_GetMinErrFltSt(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FM_GET_SIG_ERR]@{SDD_FM_GET_SIG_ERR}
 */
bool FM_GetSigErr(u16_FaultPath_t FltPath)
{
	return FML1_GetSigErrFltSt(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FM_GET_NPL_ERR]@{SDD_FM_GET_NPL_ERR}
 */
bool FM_GetNplErr(u16_FaultPath_t FltPath)
{
	return FML1_GetNplErrFltSt(FltPath);
}

/**
 *    @brief   Return the status of the Fault type
 *
 *    @param   Fault Type
 *    @return  TRUE
 *    @return  FALSE
 *    @code tag - @[CODE_FM_GET_FER_ERR]@{SDD_FM_GET_FER_ERR}
 */
bool FM_GetFerErr(u16_FaultPath_t FltPath)
{
	return FML1_GetFerErrFltSt(FltPath);
}

/**
 *    @brief  Function returns the DTC value of a fault path.
 *            Refers to the DTC mapping table and returns the corresponding
 *            DTC value
 *
 *    @param  Fault Path.
 *    @param  Fault type.
 *
 *    @return 16 bit P-code (power train code)
 *    @return 0 -- if failure
 *    @code tag - @[CODE_DERIVED_FM_GET_DTC_PTH]@{SDD_DERIVED_FM_GET_DTC_PTH}
 */
uint16_t FM_GetDTCofPath(u16_FaultPath_t FaultPath, uint8_t FltType_u8, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return GetDTCofFltPathFltType(FaultPath, FltType_u8, FM_ProtocolType_En);
}
/**
 *    @brief  Function to read the NONOBD specific Freeze frame from level-2
 *            fault mamory.
 *
 *    @param  DTC
 *    @param  Address of the buffer where the freeze frames datas are copied.
 *
 *    @return Number of DTC freeze frames successfully copied
 *    @code tag - @[CODE_FM_READ_CMPLT_NONOBD_FZRFRM]@{SDD_FM_READ_CMPLT_NONOBD_FZRFRM}
 */
uint8_t FM_ReadCmpltNONOBDFrzFrm(uint16_t DTC_u16, uint16_t *DataBuff_pu16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return FML2_ReadCmpltNONOBDFrzFrm(DTC_u16, DataBuff_pu16, FM_ProtocolType_En);
}
/**
 *    @brief  Function to clear the all NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_ALL_NOBD_FLTS]@{SDD_FM_DERIVED_CLR_ALL_NOBD_FLTS}
 */
void FM_ClrAllNONOBDFaults(void)
{
	FML2_ClrAllNONOBDFaults();
	return;
}

/**
 *    @brief  Function to clear the pending NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_PNDNG_NOBD_FLTS]@{SDD_FM_DERIVED_CLR_PNDNG_NOBD_FLTS}
 */
void FM_ClrPndngNONOBDFaults(void)
{
	FML2_ClrPndngNONOBDFaults();
	return;
}

/**
 *    @brief  Function to clear the confirmed NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_CNFRM_NOBD_FLTS]@{SDD_FM_DERIVED_CLR_CNFRM_NOBD_FLTS}
 */
void FM_ClrCnfrmNONOBDFaults(void)
{
	FML2_ClrCnfrmNONOBDFaults();
	return;
}

/**
 *    @brief  Function to clear the deleting NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_DLT_NOBD_FLTS]@{SDD_FM_DERIVED_CLR_DLT_NOBD_FLTS}
 */
void FM_ClrDeletingNONOBDFaults(void)
{
	FML2_ClrDeletingNONOBDFaults();
	return;
}

/**
 *    @brief  Function to clear all faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_ALL_FLTS]@{SDD_FM_DERIVED_CLR_ALL_FLTS}
 */
void FM_ClrAllFaults(void)
{
	FML2_ClrAllFaults();
	return;
}

/**
 *    @brief  Function to clear all pending faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_ALL_PNDNG_FLTS]@{SDD_FM_DERIVED_CLR_ALL_PNDNG_FLTS}
 */
void FM_ClrAllPndngFaults(void)
{
	FML2_ClrAllPndngFaults();
	return;
}

/**
 *    @brief  Function to clear all confirmed faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_ALL_CNFRM_FLTS]@{SDD_FM_DERIVED_CLR_ALL_CNFRM_FLTS}
 */
void FM_ClrAllCnfrmFaults(void)
{
	FML2_ClrAllCnfrmFaults();
	return;
}

/**
 *    @brief  Function to clear all deleting faults in L2 memory
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_FM_DERIVED_CLR_ALL_DLT_FLTS]@{SDD_FM_DERIVED_CLR_ALL_DLT_FLTS}
 */
void FM_ClrAllDeletingFaults(void)
{
	FML2_ClrAllDeletingFaults();
	return;
}

/**
 *    @brief : Function to find number of the all NONOBD related faults in L2 memory
 *    @param : none.
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_ALL_NOBD_FLTS]@{SDD_FM_DERIVED_NOF_ALL_NOBD_FLTS}
 */
uint8_t FM_NofAllNONOBDFaults(void)
{
	return FML2_NofAllNONOBDFaults();
}

/**
 *    @brief  Function to find number of the pending NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_PNDNG_NOBD_FLTS]@{SDD_FM_DERIVED_NOF_PNDNG_NOBD_FLTS}
 */
u16_FaultPath_t FM_NofPndngNONOBDFaults(void)
{
	return (u16_FaultPath_t)(FML2_NofPndngNONOBDFaults());
}

/**
 *    @brief  Function to find number of the confirmed NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_CNFRM_NOBD_FLTS]@{SDD_FM_DERIVED_NOF_CNFRM_NOBD_FLTS}
 */
uint8_t FM_NofCnfrmNONOBDFaults(void)
{
	return FML2_NofCnfrmNONOBDFaults();
}

/**
 *    @brief  Function to find number of the deleting NONOBD related faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_PREV_ACTV_NOBD_FLTS]@{SDD_FM_DERIVED_NOF_PREV_ACTV_NOBD_FLTS}
 */
uint8_t FM_NofPrevActvNONOBDFaults(void)
{
	return FML2_NofDeletingNONOBDFaults();
}

/**
 *    @brief  Function to find number of all faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_ALL_FLTS]@{SDD_FM_DERIVED_NOF_ALL_FLTS}
 */
uint8_t FM_NofAllFaults(void)
{
	return FML2_NofAllFaults();
}

/**
 *    @brief  Function to find number of all pending faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_ALL_PNDNG_FLTS]@{SDD_FM_DERIVED_NOF_ALL_PNDNG_FLTS}
 */
u16_FaultPath_t FM_NofAllPndngFaults(void)
{
	return FML2_NofAllPndngFaults();
}

/**
 *    @brief  Function to find number of all confirmed faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_ALL_CNFRM_FLTS]@{SDD_FM_DERIVED_NOF_ALL_CNFRM_FLTS}
 */
uint8_t FM_NofAllCnfrmFaults(void)
{
	return (uint8_t)FML2_NofAllCnfrmFaults();
}

/**
 *    @brief  Function to find number of all deleting faults in L2 memory
 *
 *    @param  None
 *    @return No of faults
 *    @code tag - @[CODE_FM_DERIVED_NOF_ALL_PREV_ACTV_FLTS]@{SDD_FM_DERIVED_NOF_ALL_PREV_ACTV_FLTS}
 */
uint8_t FM_NofAllPrevActvFaults(void)
{
	return FML2_NofAllDeletingFaults();
}

/**
 *    @brief  Function fills the parameter pointer with DTC codes of confirmed
 *            NON OBD faults
 *
 *    @param  Pointer to update the DTC codes
 *
 *    @return Number of DTC codes copied
 *    @code tag - @[CODE_FM_DERIVED_GET_ALL_CNFRMNOBD_DTC]@{SDD_FM_DERIVED_GET_ALL_CNFRMNOBD_DTC}
 */
int8_t FM_GetAllCnfrmNONOBD_DTCs(uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return _FML2_GetAllCnfrmNONOBD_DTCs(DTCVal_pu32, FM_ProtocolType_En);
}
/**
 *    @brief  Function fills the parameter pointer with DTC codes of confirmed
 *            OBD and NON OBD faults
 *
 *    @param  Pointer to update the DTC codes
 *
 *    @return Number of DTC codes copied
 *    @code tag - @[CODE_FM_DERIVED_GET_ALL_CNFRM_DTC]@{SDD_FM_DERIVED_GET_ALL_CNFRM_DTC}
 */
int8_t FM_GetAllCnfrmDTCs(uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return _FML2_GetAllCnfrmDTCs(DTCVal_pu32, FM_ProtocolType_En);
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
 *    @code tag - @[CODE_FM_DERIVED_GET_CNFRMNOBD_DTC]@{SDD_FM_DERIVED_GET_CNFRMNOBD_DTC}
 */
int8_t FM_GetCnfrmNONOBD_DTC(uint8_t EntryNum_u8, uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return FML2_GetCnfrmNONOBD_DTC(EntryNum_u8, DTCVal_pu32, FM_ProtocolType_En);
}

/**
 *    @brief  Function fills the parameter pointer with DTC code of
 *            Nth(EntryNum_u8) entry of OBD or NON-OBD fault
 *
 *    @param  Entry Number in L2 memory
 *    @param  Pointer to update the DTC codes
 *
 *    @return -1 - No OBD DTC exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_FM_DERIVED_GET_CNFRM_DTC]@{SDD_FM_DERIVED_GET_CNFRM_DTC}
 */
int8_t FM_GetCnfrmDTC(uint8_t EntryNum_u8, uint32_t *DTCVal_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return FML2_GetCnfrmDTC(EntryNum_u8, DTCVal_pu32, FM_ProtocolType_En);
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of NON OBD related entries in L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return Number of NON OBD related faults in L2 memory
 *    @code tag - @[CODE_FM_DERIVED_GET_ALL_CNFRMNOBD_FLTPTH]@{SDD_FM_DERIVED_GET_ALL_CNFRMNOBD_FLTPTH}
 */
int8_t FM_GetAllCnfrmNONOBD_FltPaths_FltTypes(uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
	return FML2_GetAllCnfrmNONOBD_FltPaths_FltTypes(FltPath_pu16, FltType_pu8);
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of all (OBD + NON-OBD) entries in L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return Number of OBD + NON-OBD related faults in L2 memory
 *    @code tag - @[CODE_FM_DERIVED_GET_ALL_CNFRM_FLTPTH]@{SDD_FM_DERIVED_GET_ALL_CNFRM_FLTPTH}
 */
int8_t FM_GetAllCnfrm_FltPaths_FltTypes(uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
	return FML2_GetAllCnfrm_FltPaths_FltTypes(FltPath_pu16, FltType_pu8);
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of particular OBD entry in L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return -1 - No NON OBD fault exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_FM_DERIVED_GET_CNFRMNOBD_FLTPTH]@{SDD_FM_DERIVED_GET_CNFRMNOBD_FLTPTH}
 */
int8_t FM_GetCnfrmNONOBD_FltPath_FltType(uint8_t EntryNum_u8, uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
	return FML2_GetCnfrmNONOBD_FltPath_FltType(EntryNum_u8, FltPath_pu16, FltType_pu8);
}

/**
 *    @brief  Function fills the parameter pointer with fault type and
 *            fault paths of particular OBD or NON-OBD entry in L2 memory
 *
 *    @param  pointer to update the FltPaths
 *    @param  Pointer to update the FltTypes
 *
 *    @return -1 - No OBD or NON-OBD fault exists with specified entry number
 *    @return  0 - Success
 *    @code tag - @[CODE_FM_DERIVED_GET_CNFRM_FLTPTH]@{SDD_FM_DERIVED_GET_CNFRM_FLTPTH}
 */
int8_t FM_GetCnfrm_FltPath_FltType(uint8_t EntryNum_u8, uint16_t *FltPath_pu16, uint8_t *FltType_pu8)
{
	return FML2_GetCnfrm_FltPath_FltType(EntryNum_u8, FltPath_pu16, FltType_pu8);
}

/**
 *    @brief  Function to find the list of readiness components that are supported.
 *    @param  None
 *    @return Bit mask indicating the supported readiness groups
 *    @code tag - @[CODE_DERIVED_FM_GETSUPP_RDYGRP]@{SDD_DERIVED_FM_GETSUPP_RDYGRP}
 */
uint8_t FM_GetSuppRdyGroup(void)
{
	// This function searches through the entire fm coniguration table and gets the supported
	// readiness groups
	u16_FaultPath_t FltPath = 0;
	bool Flag_b = TRUE;
	uint8_t RdyGrp_st_u8 = 0;

	for (FltPath = FM_FAULT_START_E; (FltPath < NUM_OF_FAULTPATHS_E && Flag_b); FltPath++)
	{
		// parse through the configuration file and set the corresponding bits
		switch (GET_FAULT_RDYGRP(FltPath))
		{
		// Check if any of the fault paths support fuel system readiness groups
		case RDY_FUEL_GRP:

			// Check if its OBD relavent
			if (IS_FAULT_OBD_RELEVANT(FltPath))
			{
				SETBIT(RdyGrp_st_u8, FM_RDY_FUEL_GRP_BP);
			}
			break;

		// Check if any of the fault paths support EGR readiness groups
		case RDY_EGR_GRP:

			// Check if its OBD relavent
			if (IS_FAULT_OBD_RELEVANT(FltPath))
			{
				SETBIT(RdyGrp_st_u8, FM_RDY_EGR_GRP_BP);
			}
			break;

		// Check if any of the fault paths belongs to comprehensive components readiness groups
		case RDY_CCOMP_GRP:
			// Check if its OBD relavent
			if (IS_FAULT_OBD_RELEVANT(FltPath))
			{
				SETBIT(RdyGrp_st_u8, FM_RDY_CCOMP_GRP_BP);
			}
			break;

		default:
			break;
		}
		// Flag to break out of the for loop if we find atleast 1 fault in each group
		if ((RdyGrp_st_u8 & FM_RDYGRP_MASK) == FM_RDYGRP_MASK)
		{
			Flag_b = 0;
		}
	}

	return RdyGrp_st_u8;
}

/**
 *    @brief  Wrapper function to know the status of Mal Indication Lamp.
 *
 *    @param  None.
 *    @return TRUE: if MIL is ON, FALSE: otherwise.
 *
 *    @code tag - @[CODE_DERIVED_FM_STATUSOFMI]@{SDD_DERIVED_FM_STATUSOFMI}
 */
bool FM_StatusOf_MI(void)
{
	return FML2_StatusOf_MI();
}

/**
 *    @brief  Wrapper function to turn off MI
 *
 *    @param  None
 *    @return None
 *    @code tag - @[CODE_DERIVED_FM_TURNOFFMI]@{SDD_DERIVED_FM_TURNOFFMI}
 */
void FM_TurnOffMI(void)
{
	FML2_TurnOffMI();
	return;
}

/**
 *    @brief : Clears the readiness completion variable. Also clears the
 *             readiness monitoring test results in NV memory
 *    @param : none.
 *    @return :none.
 *    @code tag - @[CODE_DERIVED_FM_CLRRDY_MONFLGS]@{SDD_DERIVED_FM_CLRRDY_MONFLGS}
 */
void FM_ClrRdyMonFlags(void)
{
	FML1_ClrRdyMonFlags();
	return;
}
/**
 *    @brief  Function to get time since key ON in milli second.
 *    @param  None
 *    @return time in milli seconds
 *    @code tag - @[CODE_DERIVED_FM_GETTIME_SINCEKEYON_MS]@{SDD_DERIVED_FM_GETTIME_SINCEKEYON_MS}
 */
uint32_t FM_GET_TIME_SINCE_KEY_ON_MS(void)
{
	//	uint64_t get_cpu_time_u64 = 0;
	uint32_t time_since_key_ON_ms_u32 = 0;

	/* Capture CPU time */
	time_since_key_ON_ms_u32 = (uint32_t)GET_TIME_MS();

	return time_since_key_ON_ms_u32;
}

/**
 *    @brief  Function to get time since key ON in second.
 *    @param  None
 *    @return time in seconds
 *    @code tag - @[CODE_DERIVED_FM_GETTIME_SINCE_ENG_ON_SEC]@{SDD_DERIVED_FM_GETTIME_SINCE_ENG_ON_SEC}
 */
uint32_t FM_GET_TIME_SINCE_ENGINE_ON_SEC(void)
{
	// UINT64 get_cpu_time_u64 = 0;
	uint32_t time_since_key_ON_sec_u32 = 0;

	/* Capture CPU time */
	time_since_key_ON_sec_u32 = (uint32_t)(GET_TIME_MS());

	time_since_key_ON_sec_u32 = (uint32_t)(time_since_key_ON_sec_u32 / 1000.0f);

	return time_since_key_ON_sec_u32;
}
/**
 *    @brief  read the DTCs of the current/present faults in L1
 *    @param  pointer to DTC list, max num_of_DTC to be read,
 *    @return No_of_faults
 *    @code tag - @[CODE_FML1_DERIVED_RDPRESENT_DTC]@{SDD_FML1_DERIVED_RDPRESENT_DTC}
 */
uint16_t FM_ReadDTCsOfPresentFaults(uint32_t *DTC_pu32, uint16_t ReadLimit_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	//	return _FML1_ReadDTCsOfPresentFaults(DTC_pu32, ReadLimit_u16, FM_ProtocolType_En);
	//	return _FML1_ReadDTCsOfPresentFaults(DTC_pu32, ReadLimit_u16, FM_ProtocolType_En);
	return FM_Get_DTCs(DTC_pu32, ReadLimit_u16, FM_ProtocolType_En, FM_ALL_E, FM_PRESENT_E); // todo Sandeep K Y
}

/**
 *    @brief  read the number of current/present faults in L1
 *    @param
 *    @return No_of_faults
 */
uint16_t FM_GetNumberOfPresentFaults(void)
{
	return FML1_GetNumberOfPresentFaults();
}

/**
 *    @brief  module level function to read the fault path and fault type from DTC value.
 *    @param  None
 *    @return 0: suucess. -1: failure
 *    @code tag - @[CODE_DERIVED_FML2_GET_FREQCNT_OBDFRZFRM]@{SDD_DERIVED_FML2_GET_FREQCNT_OBDFRZFRM}
 */
int16_t GetFltPathFltType_ByDTC(uint32_t DTC_u32, uint16_t *FltPath_pu16, uint8_t *FltType_pu8, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	uint16_t i = 0;
	uint16_t j = 0;
	bool found_b = 0;

	*FltPath_pu16 = 0xFFFF;
	*FltType_pu8 = 0;

	for (i = 0; i < NUM_OF_FAULTPATHS_E; i++)
	{
		for (j = 0; (j < (MAX_NUM_ERROR_TYPES - 1)); j++)
		{
			if (DTC_u32 == GET_DTC_VALUE_DTC_MAPTBL(i, j))
			{
				*FltPath_pu16 = i;
				*FltType_pu8 |= (uint8_t)FM_GET_FLTTYPE(j);
				found_b = TRUE;
			}
		}
		if (found_b)
		{
			return 0; // found
		}
	}		   // Todo Sandeep K Y add a campability for OBD and J1939
	return -1; // not found
}

/**
 *    @brief  Function returns the DTC value of a fault path.
 *            Refers to the DTC mapping table and returns the corresponding
 *            DTC value
 *
 *    @param  Fault Path.
 *    @param  Fault type.
 *
 *    @return 16 bit P-code (power train code)
 *    @return 0 -- if failure
 *    @code tag - @[CODE_DERIVED_FML2_GetDTCofFltPathFltType]@{SDD_DERIVED_FML2_GetDTCofFltPathFltType}
 */
uint32_t GetDTCofFltPathFltType(u16_FaultPath_t FaultPath, uint8_t FltType_u8, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	uint32_t DTC_val_u32 = 0;
	int16_t Col_s16 = 0;

	if ((FaultPath < NUM_OF_FAULTPATHS_E) &&
		(FltType_u8 <= FM_FER_ERR))
	{
		Col_s16 = (int16_t)(GET_COLINDEX_BY_FLTTYPE(FltType_u8));
		if (Col_s16 >= 0)
		{
            switch(FM_ProtocolType_En)
            {
            	case FM_PROTO_UDS_E:   
					#if (TRUE == FM_UDS_SUPPORTED)
            		if(FltType_u8 < 8)
            		{
            			DTC_val_u32 = DTCMappingTable_UDS_aSt[FaultPath][FltType_u8];
            		}
            		else if(FltType_u8 == 8)
            		{
            			FltType_u8 =0;
            			DTC_val_u32 = DTCMappingTable_UDS_aSt[FaultPath][FltType_u8];
            		}
            		else if(FltType_u8 == 16)
            		{
            		    FltType_u8 = 1;
            		    DTC_val_u32 = DTCMappingTable_UDS_aSt[FaultPath][FltType_u8];
            		}
            		else if(FltType_u8 == 32)
            		{
            			FltType_u8 = 2;
            			DTC_val_u32 = DTCMappingTable_UDS_aSt[FaultPath][FltType_u8];
            		}
            		else if(FltType_u8 == 64)
            		{
            		     FltType_u8 = 3;
            		     DTC_val_u32 = DTCMappingTable_UDS_aSt[FaultPath][FltType_u8];
            		}
            		else if(FltType_u8 == 128)
            		{
            		     FltType_u8 = 4;
            		     DTC_val_u32 = DTCMappingTable_UDS_aSt[FaultPath][FltType_u8];
            		}
                    #endif
                    break;
                            
                case FM_PROTO_OBD_E:
                    #if (TRUE == FM_OBD_SUPPORTED)
                    DTC_val_u32 =  DTCMappingTable_OBD_aSt[FaultPath][Col_s16];
                    #endif
                    break;

				case FM_PROTO_J1939_E:
				#if (TRUE == FM_J1939_SUPPORTED)
				if(J1939_DTC_Conf_aUni[FaultPath].DTC_St.SPN_u19 != 0x7FFFF)
				{
					DTC_val_u32 = J1939_DTC_Conf_aUni[FaultPath].DTC_u32;
				}
				#endif
				break;

			default:
				break;
			}
		}
	}
	return DTC_val_u32;
}

/**
 *    @brief  function reads the freeze frames from fault memory (using flt-index of L2).
 *    algorithm:
 *    1. input available: L2 fault index.
 *    2. first try to get frzfrm from L2. if not available then look into L1.
 *    3. get L2 flt index from fly path & type.
 *    4. if present in L2, read the freeze frame from L2 memory
 *    5. if not present in L2, send error.
 *
 *    @ param: Requested DTC value
 *    @ param: pointer to buffer to hold freeze frame value.
 *
 *    @return -1    = for failure condition
 *             > -1 = number of freezeframe parameters
 */
int16_t FM_ReadFrzFrm_ByFltIndex(int8_t FltIndex_s8, uint8_t *destBuff_pu8, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	int16_t i = 0;
	int16_t j = 0;
	int16_t totalValidBytes_s8 = -1;

	if (FltIndex_s8 >= 0)
	{
		/* DTC found in L2 memory */
		uint16_t FaultPath_En = FML2_GetFaultPath(FltIndex_s8);

#if (TRUE == FM_UDS_SUPPORTED)
		if (FM_PROTO_UDS_E == FM_ProtocolType_En)
		{
			for (i = 0; i < FMFF_CONF_UDS_FRZFRM_TOTALBYTES; i++)
			{
				destBuff_pu8[i] = FML2_FaultMem_aSt[FltIndex_s8].L2FrzFrm_UDS_au8[i]; // FML2_GET_FRZFRM_VALUE(FltIndex_s8, i);
			}
			totalValidBytes_s8 = FMFF_CONF_UDS_FRZFRM_GLBSS_BYTES;

			for (j = 0; j < FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].NumOfEntries_u16; j++)
			{
				totalValidBytes_s8 += Get_FreezeFrame_ParamSize(FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].FM_FrzFrm_LocalEntries[j]);
			}
		}
#endif

#if (TRUE == FM_OBD_SUPPORTED)
		if (FM_PROTO_OBD_E == FM_ProtocolType_En)
		{
			for (i = 0; i < FMFF_CONF_OBD_FRZFRM_TOTALBYTES; i++)
			{
				destBuff_pu8[i] = FML2_FaultMem_aSt[FltIndex_s8].L2FrzFrm_OBD_au8[i]; // FML2_GET_FRZFRM_VALUE(FltIndex_s8, i);
			}
			totalValidBytes_s8 = FMFF_CONF_OBD_FRZFRM_GLBSS_BYTES;
			for (j = 0; j < FM_FrzFrmMasterConfig_OBD_aSt[FaultPath_En].NumOfEntries_u16; j++)
			{
				totalValidBytes_s8 += FM_FrzFrmMasterConfig_OBD_aSt[FaultPath_En].FrzFrm_Data_pSt[j].ParamSize_u16;
			}
		}
#endif

#if (TRUE == FM_J1939_SUPPORTED)
/* To do : For this project OBD faults are not required. So commented this for J1939. */ 
		// if (FM_PROTO_J1939_E == FM_ProtocolType_En)
		// {
		// 	for (i = 0; i < FMFF_CONF_J1939_FRZFRM_TOTALBYTES; i++)
		// 	{
		// 		destBuff_pu8[i] = FML2_FaultMem_aSt[FltIndex_s8].L2FrzFrm_J1939_au8[i]; // FML2_GET_FRZFRM_VALUE(FltIndex_s8, i);
		// 	}
		// 	totalValidBytes_s8 = FMFF_CONF_J1939_FRZFRM_GLBSS_BYTES;
		// 	for (j = 0; j < FM_FrzFrmMasterConfig_J1939_aSt[FaultPath_En].NumOfEntries_u16; j++)
		// 	{
		// 		totalValidBytes_s8 += FM_FrzFrmMasterConfig_J1939_aSt[FaultPath_En].FrzFrm_Data_pSt[j].ParamSize_u16;
		// 	}
		// }
#endif
	}
	else
	{
		/* The DTC not available in L2 memory */
		totalValidBytes_s8 = -1;
	}

	/* no DTC found */
	return totalValidBytes_s8;
}

/**
 *    @brief  function reads the freeze frames from fault memory.
 *    algorithm:
 *    1. input available: fault path, fault type.
 *    2. first try to get frzfrm from L2. if not available then look into L1.
 *    3. get L2 flt index from fly path & type.
 *    4. if present in L2, read the freeze frame from L2 memory
 *    5. if not present in L2, read it from L1 memory.
 *    7. If not present even in L1, return -1.
 *
 *    @ param: Requested DTC value
 *    @ param: pointer to buffer to hold freeze frame value.
 *
 *    @return -1    = for failure condition
 *             > -1 = number of freezeframe parameters
 */
int16_t FM_ReadFrzFrm_ByFltPathFltType(u16_FaultPath_t FaultPath_En, uint8_t FltType_u8, uint8_t *destBuff_pu8, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	int16_t i = 0;
	int16_t j = 0;
	int8_t totalValidBytes_s8 = -1;
	int8_t FltIndex_s8 = -1;

	FltIndex_s8 = FML2_GetFltIndxOfPath(FaultPath_En, FltType_u8);

	if (FltIndex_s8 >= 0)
	{
		/* DTC found in L2 memory */
		FM_ReadFrzFrm_ByFltIndex(FltIndex_s8, destBuff_pu8, FM_ProtocolType_En);
	}
	else
	{
		/* The DTC not available in L2 memory */
		totalValidBytes_s8 = -1;
	}

#if FRZFRM_FROM_FML1
	if (totalValidBytes_s8 < 0)
	{
		/* DTC not found in L2 memory,
		   search in L1 memory */
		if (FM_GetErr(FaultPath_En) == FltType_u8)
		{
#if (TRUE == FM_UDS_SUPPORTED)
			if (FM_PROTO_UDS_E == FM_ProtocolType_En)
			{
				for (i = 0; i < FMFF_CONF_UDS_FRZFRM_TOTALBYTES; i++)
				{
					destBuff_pu8[i] = FaultPath_aSt[FaultPath_En].L1FrzFrm_UDS_au8[i]; // FML2_GET_FRZFRM_VALUE(FltIndex_s8, i);
				}
				totalValidBytes_s8 = FMFF_CONF_UDS_FRZFRM_GLBSS_BYTES;
				for (j = 0; j < FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].NumOfEntries_u16; j++)
				{
					totalValidBytes_s8 += Get_FreezeFrame_ParamSize(FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].FM_FrzFrm_LocalEntries[j]);
				}
			}
#endif

#if (TRUE == FM_OBD_SUPPORTED)
			if (FM_PROTO_OBD_E == FM_ProtocolType_En)
			{
				for (i = 0; i < FMFF_CONF_OBD_FRZFRM_TOTALBYTES; i++)
				{
					destBuff_pu8[i] = FaultPath_aSt[FaultPath_En].L1FrzFrm_OBD_au8[i]; // FML2_GET_FRZFRM_VALUE(FltIndex_s8, i);
				}
				totalValidBytes_s8 = FMFF_CONF_OBD_FRZFRM_GLBSS_BYTES;
				for (j = 0; j < FM_FrzFrmMasterConfig_OBD_aSt[FaultPath_En].NumOfEntries_u16; j++)
				{
					totalValidBytes_s8 += FM_FrzFrmMasterConfig_OBD_aSt[FaultPath_En].FrzFrm_Data_pSt[j].ParamSize_u16;
				}
			}
#endif

#if (TRUE == FM_J1939_SUPPORTED)
/* To do : For this project OBD faults are not required. So commented this for J1939. */ 
			// if (FM_PROTO_J1939_E == FM_ProtocolType_En)
			// {
			// 	for (i = 0; i < FMFF_CONF_J1939_FRZFRM_TOTALBYTES; i++)
			// 	{
			// 		destBuff_pu8[i] = FaultPath_aSt[FaultPath_En].L1FrzFrm_J1939_au8[i]; // FML2_GET_FRZFRM_VALUE(FltIndex_s8, i);
			// 	}
			// 	totalValidBytes_s8 = FMFF_CONF_J1939_FRZFRM_GLBSS_BYTES;
			// 	for (j = 0; j < FM_FrzFrmMasterConfig_J1939_aSt[FaultPath_En].NumOfEntries_u16; j++)
			// 	{
			// 		totalValidBytes_s8 += FM_FrzFrmMasterConfig_J1939_aSt[FaultPath_En].FrzFrm_Data_pSt[j].ParamSize_u16;
			// 	}
			// }
#endif
		}
		else
		{
			/* The DTC not available in L1 memory */
			totalValidBytes_s8 = -1;
		}
	}
#endif

	/* no DTC found */
	return totalValidBytes_s8;
}

/**
 *    @brief  function reads the freeze frames from fault memory.
 *    algorithm:
 *    1. read DTC from requested command
 *    2. extract fault path & fault type from DTC value
 *    3. Search whether the fault path & fault type is present in L2 memory
 *    4. if present in L2, read the freeze frame from L2 memory
 *    7. If not present even in L2, return -1.
 *
 *    @ param: Requested DTC value
 *    @ param: pointer to buffer to hold freeze frame value.
 *
 *    @return -1    = for failure condition
 *             > -1 = number of freezeframe parameters
 */
int16_t FM_ReadFrzFrm_ByDTC(uint16_t DTC_u16, uint16_t *FrzFrmBuff_pu16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	u16_FaultPath_t FaultPath_En = 0;
	//	int16_t             i = 0;
	//	int16_t             j = 0;
	int16_t RetVal_s16 = -1;
	int16_t totalValidBytes_s8 = -1;
	uint8_t FltType_u8 = 0;
	//	int8_t              FltIndex_s8 = -1;

	RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u16, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

	if (RetVal_s16 >= 0)
	{
		totalValidBytes_s8 = FM_ReadFrzFrm_ByFltPathFltType(FaultPath_En, FltType_u8, (uint8_t *)FrzFrmBuff_pu16, FM_ProtocolType_En);
	}
	/* no DTC found */
	return totalValidBytes_s8;
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
 *    @return  FALSE = failure condition
 *             TRUE  = success
 */
int16_t FM_ClrL2Err_ByDTC(uint16_t DTC_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return FML2_ClrL2Err_ByDTC(DTC_u16, FM_ProtocolType_En);
}

/**
 *    @brief  Function returns the Fault state of the L2 error memory.
 *
 *    @param  FaultState_au8 Status Array which contains the L2 Fault state.
 *
 *    @return Number of L2Errors
 *
 */
uint8_t FM_GetStateL2FaultMatrix(uint8_t *FaultState_au8)
{
	//	int16_t Col_s16 = 0;
	uint8_t i;

	uint8_t NUM_OfL2Faults = FM_NofAllFaults();

	for (i = 0; ((i < FM_CONF_MAX_L2FAULT_ENTRIES) && (i < NUM_OfL2Faults)); i++)
	{
		FaultState_au8[i] = (uint8_t)FML2_GetPrsntFaultState(i);
	}

	return NUM_OfL2Faults;
}

/**
 *    @brief  Function returns the Fault path and state of the L2 error memory.
 *
 *    @param  FaultState_au8 Status Array which contains the L2 Fault state.
 *
 *    @return Number of L2Errors
 *
 */
uint8_t FM_GetPathStateL2FaultMatrix(uint8_t *FaultPath_au8, uint8_t *FaultState_au8)
{
	//	int16_t Col_s16 = 0;
	uint8_t i;
	uint8_t NUM_OfL2Faults = FM_NofAllFaults();

	for (i = 0; ((i < FM_CONF_MAX_L2FAULT_ENTRIES) && (i < NUM_OfL2Faults)); i++)
	{
		FaultPath_au8[i] = (uint8_t)FML2_GetFaultPath(i);
		FaultState_au8[i] = (uint8_t)FML2_GetPrsntFaultState(i);
	}

	return NUM_OfL2Faults;
}

/**
 *    @brief  Function returns the list of all (OBD & non OBD) 'Deleting' stated DTCs.
 *
 *    @param  result buffer with dtc list.
 *
 *    @return Number of deleting faults in L2 memory.
 *
 */
int8_t _FM_GetAllDeletingDTCs(uint16_t *DTCVal_pu16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	return _FML2_GetAllDeletingDTCs(DTCVal_pu16, FM_ProtocolType_En);
}

/**
 *    @brief  Function returns the list of all (OBD & non OBD) pending ('potential') stated DTCs from L2 for specified protocol.
 *
 *    @param  result buffer with dtc list.
 *
 *    @return Number of potential faults in L2 memory.
 *
 */
// int8_t FML2_GetAllProtocolPndng_DTCs(uint32_t* DTCVal_pu32, uint16_t numdtc_req_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
//{
//         int8_t retval_s8;
//         //set the protocol to read the DTC of that protocol
////        FM_ProtocolTypeSet_En = FM_ProtocolType_En;
//  	retval_s8 =  _FML2_GetAllPndng_DTCs(DTCVal_pu32, numdtc_req_u16, FM_ProtocolType_En);
////        FM_ProtocolTypeSet_En = FM_NO_PROTO_E;
//        return retval_s8;
//}

/**
 *    @brief  Function returns the list of all (OBD & non OBD) pending ('potential') stated DTCs from L2.
 *
 *    @param  result buffer with dtc list.
 *
 *    @return Number of potential faults in L2 memory.
 *
 */
int8_t _FML2_GetAllPndng_DTCs(uint32_t *DTCVal_pu32, uint16_t numdtc_req_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	uint8_t FltIndx_u8 = 0;
	int8_t Nof_DTCs = 0;
	uint32_t DTC_Value = 0;
	// searching in L2 memory
	for (FltIndx_u8 = 0; ((FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES) && numdtc_req_u16); FltIndx_u8++)
	{
		if (FM_POTENTIAL == FML2_GetPrsntFaultState(FltIndx_u8))
		{
			// re intialising
			DTC_Value = 0;

			// fetch the DTC value from the DTC table
			DTC_Value = GetDTCofFltPathFltType(FML2_GetFaultPath(FltIndx_u8), (uint8_t)FML2_GetErrType(FltIndx_u8), FM_ProtocolType_En);

			if (DTC_Value)
			{
				// copy the value to pointer passed
				*(DTCVal_pu32 + Nof_DTCs) = DTC_Value;

				// incrementing the number of DTC's copied
				Nof_DTCs++;
				numdtc_req_u16--;
			}
		}
	}
	return Nof_DTCs;
}

/**
 *    @brief  Function returns the list of OBD specific 'Deleting' stated DTCs for specified protocol.
 *
 *    @param  result buffer with dtc list.
 *
 *    @return Number of deleting faults in L2 memory.
 *
 */
// int8_t FML2_GetOBDPndng_ProtocolDTCs(uint16_t* DTCVal_pu16, uint16_t numdtc_req_u16, FM_ProtocolType_En_t FM_ProtocolType_En)
//{
//         int8_t retval_s8;
//         //set the protocol to read the DTC of that protocol
////        FM_ProtocolTypeSet_En = FM_ProtocolType_En;
//  	retval_s8 =  _FML2_GetOBDPndng_DTCs(DTCVal_pu16, numdtc_req_u16, FM_ProtocolType_En);
////        FM_ProtocolTypeSet_En = FM_NO_PROTO_E;
//        return retval_s8;
//}

/**
 *    @brief  Function to clear all faults in L1 memory
 *
 *    @param  None
 *    @return None
 */
void FML1_ClrAllFaults(void)
{
	u16_FaultPath_t FltPath_En = 0;
	uint32_t DebounceCnt_u8 = 0;

	for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
	{
		for (DebounceCnt_u8 = 0; DebounceCnt_u8 <= 2; DebounceCnt_u8++)
		{
			if ((FM_NO_ERR != FML1_GetErrType(FltPath_En)))
			{
				FM_ReportFault(FltPath_En, FM_NO_ERR);
			}
			else
			{
				break;
			}
		}
	}

	return;
}

/**
 *    @brief  Function to get number of Active DTCs.
 *
 *    @param  DTCbuffer_au8  buffer to hold the DTCs.
 *    @return Number of DTCs.
 */
int8_t FM_GetTestActiveDTCs(uint32_t *DTCbuffer_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	/* variable declarations */
	int16_t NumofPrsntFault_u16 = 0u;
	uint16_t NumOfDTCs_u16 = FM_ReadDTCsOfPresentFaults((uint32_t *)DTCbuffer_pu32, NumofPrsntFault_u16, FM_ProtocolType_En);

	/*Get number of present faults*/
	NumofPrsntFault_u16 = FM_GetNumberOfPresentFaults();
	/* DTC information from FM */

	return NumOfDTCs_u16;
}

/**
 *    @brief  Function to get present fault status of requested DTC.
 *
 *    @param  DTC_u32  requested DTC.
 *    @return TRUE: Present Fault
 *            FALSE: Not Present Fault
 */
bool GetPresentFaultStatusByDTC(const uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	/* variable declarations */
	uint32_t PresentDTCs_au32[NUM_OF_FAULTPATHS_E];
	uint16_t NumofPrsntFault_u16 = 0u;
	int16_t i = 0;
	bool ret = FALSE;
	uint16_t NumOfDTCs_u16 = FM_ReadDTCsOfPresentFaults((uint32_t *)PresentDTCs_au32, NumofPrsntFault_u16, FM_ProtocolType_En);

	/*Get number of present faults*/
	NumofPrsntFault_u16 = FM_GetNumberOfPresentFaults();
	/* DTC information from FM */

	/*Check requested DTC is in the list*/
	for (i = 0; i < NumOfDTCs_u16; i++)
	{
		if (DTC_u32 == PresentDTCs_au32[i])
		{
			ret = TRUE;
		}
	}
	return ret;
}

/**
 *    @brief  Function to get potential fault status of requested DTC.
 *
 *    @param  DTC_u32  requested DTC.
 *    @return TRUE: potential Fault
 *            FALSE: Not potential Fault
 */
bool GetPotentialFaultStatusByDTC(const uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	/* variable declarations */
	uint32_t PendingDTCs_au32[NUM_OF_FAULTPATHS_E];
	uint16_t NumofPendingFault_u16 = 0u;
	int16_t i = 0;
	bool ret = FALSE;
	int8_t NumOfDTCs_u16 = _FML2_GetAllPndng_DTCs((uint32_t *)PendingDTCs_au32, NumofPendingFault_u16, FM_ProtocolType_En);

	NumofPendingFault_u16 = FM_NofAllPndngFaults();

	/*Check requested DTC is in the list*/
	for (i = 0; i < NumOfDTCs_u16; i++)
	{
		if (DTC_u32 == PendingDTCs_au32[i])
		{
			ret = TRUE;
		}
	}
	return ret;
}

/**
 *    @brief  Function to get confirmed fault status of requested DTC.
 *
 *    @param  DTC_u32  requested DTC.
 *    @return TRUE: confirmed Fault
 *            FALSE: Not confirmed Fault
 */
bool GetConfirmedFaultStatusByDTC(const uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	/* variable declarations */
	uint32_t confirmedDTCs_au32[FM_CONF_MAX_L2FAULT_ENTRIES];
	int16_t i = 0;
	bool ret = FALSE;
	uint16_t NumOfDTCs_u16 = 0;

	/* DTC information from FM */
	NumOfDTCs_u16 = FM_GetAllCnfrmDTCs((uint32_t *)confirmedDTCs_au32, FM_ProtocolType_En);

	/*Check requested DTC is in the list*/
	for (i = 0; i < NumOfDTCs_u16; i++)
	{
		if (DTC_u32 == confirmedDTCs_au32[i])
		{
			ret = TRUE;
		}
	}
	return ret;
}

/**
 *    @brief  Function to get status of requested DTC.
 *            Faultmanager maintains seperate 8-bit status field.
 *
 *              byte7	byte6	byte5	byte4	byte3	byte2	byte1	byte0
 *		Res      Res     Rdy    L2-D	 L2-H	 L2-C	 L2-P	 L1
 *
 *            L1: Active or Current faults
 *            L2-P: Potential or Pending faults
 *            L2-C: Confirmed faults
 *            L2-H: Healing
 *            L2-D: Deleting
 *            Rdy: Readiness.
 *
 *    @param  DTC_u32  requested DTC.
 *    @return Status.
 */
FM_DTCStatus_Un_t FM_GetStatusOfDTCByDTCs(uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	/* variable declarations */
	u16_FaultPath_t FaultPath_En = 0;
	int16_t RetVal_s16 = -1;
	uint8_t FltType_u8 = 0;
	//	int8_t        FltIndex_s8 = -1;

	/* variable declarations */
	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	FM_DTCStatus_Un.Status_u8 = 0;

	RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

	if (RetVal_s16 >= 0)
	// {
	// FltIndex_s8 = FML2_GetFltIndxOfPath (FaultPath_En, FltType_u8);
	// }

	// if (FltIndex_s8 >= 0)
	{
		/* Test failed - this shall be an indicator of the current status of the fault. */
		FM_DTCStatus_Un.status_bit.stTF_b0 = FML1_GET_TF_STATUS_OF_DTC_MAPTBL(FaultPath_En);
		FM_DTCStatus_Un.status_bit.stTFTOC_b1 = FML1_GET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
		FM_DTCStatus_Un.status_bit.stPDTC_b2 = FML1_GET_PDTC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
		FM_DTCStatus_Un.status_bit.stCDTC_b3 = FML1_GET_CDTC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
		FM_DTCStatus_Un.status_bit.stTNCSLC_b4 = FML1_GET_TNCSLC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
		FM_DTCStatus_Un.status_bit.stTFSLC_b5 = FM_GetTFSLC_ByFltPathFltType(FaultPath_En, FltType_u8);
		FM_DTCStatus_Un.status_bit.stTNCTOC_b6 = FML1_GET_TNCTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
		FM_DTCStatus_Un.status_bit.stWIR_b7 = FML1_GET_WIR_STATUS_OF_DTC_MAPTBL(FaultPath_En);
	}
	return FM_DTCStatus_Un;
}
//
// uint8_t FM_GetStatusOfDTCByDTCs_OLD(uint32_t DTC_u32)
//{
//    /* variable declarations */
//    u16_FaultPath_t  FaultPath_En =  0;
//    int16_t       RetVal_s16   = -1;
//    uint8_t        FltType_u8   =  0;
//    int8_t        FltIndex_s8  = -1;
//
//    /* variable declarations */
//    FM_DTCStatus_Un_t FM_DTCStatus_Un;
//
//    /*Check for present fault status*/
//    FM_DTCStatus_Un.status_bit.L1_b1 = GetPresentFaultStatusByDTC(DTC_u32);
//
//    /*Check for Pending/Potential faults*/
//    FM_DTCStatus_Un.status_bit.L2P_b2 = GetPotentialFaultStatusByDTC(DTC_u32);
//
//    /*Check confirm faults*/
//    FM_DTCStatus_Un.status_bit.L2C_b3 = GetConfirmedFaultStatusByDTC(DTC_u32);
//
//    /*Check healing status*/
//    RetVal_s16 = GetFltPathFltType_ByDTC (DTC_u32, &FaultPath_En, &FltType_u8);
//    if (RetVal_s16 >= 0)
//    {
//      FM_DTCStatus_Un.status_bit.L2H_b4 = 0;
//      if ((FML1_DEBHEAL_E == FML1_GetDebStatus(FaultPath_En)))
//      {
//        FM_DTCStatus_Un.status_bit.L2H_b4 = 1;
//      }
//    }
//    else
//    {
//      //Error status
//    }
//
//    /*Check deleting status*/
//    //RetVal_s16 = GetFltPathFltType_ByDTC (DTC_u32, &FaultPath_En, &FltType_u8);
//    FltIndex_s8 = FML2_GetFltIndxOfPath (FaultPath_En, FltType_u8);
//    FM_DTCStatus_Un.status_bit.L2D_b5 = 0;
//    if(FM_DELETING == FML2_GetPrsntFaultState(FltIndex_s8))
//    {
//      FM_DTCStatus_Un.status_bit.L2D_b5 = 1;
//    }
//
//    /*Check readiness status*/
//    //RetVal_s16 = GetFltPathFltType_ByDTC (DTC_u32, &FaultPath_En, &FltType_u8);
//    if (RetVal_s16 >= 0)
//    {
//      FM_DTCStatus_Un.status_bit.Rdy_b6 = 0;
//      if ((FML1_RDYCMPLT_E == FML1_GetDebStatus(FaultPath_En)))
//      {
//        FM_DTCStatus_Un.status_bit.Rdy_b6 = 1;
//      }
//    }
//    else
//    {
//      //Error status
//    }
//
//    if (RetVal_s16 >= 0)
//    {
//	FM_DTCStatus_Un.status_bit.TFTOS_b7 = FML1_GET_TFTOC_STATUS_OF_DTC_MAPTBL(FaultPath_En);
//    }
//    return FM_DTCStatus_Un.Status_u8;
//}

/**
 *    @brief  Function to get status of requested DTC.
 *            Faultmanager maintains seperate 8-bit status field.
 *
 *              byte7	byte6	byte5	byte4	byte3	byte2	byte1	byte0
 *		Res      Res     Rdy    L2-D	 L2-H	 L2-C	 L2-P	 L1
 *
 *            L1: Active or Current faults
 *            L2-P: Potential or Pending faults
 *            L2-C: Confirmed faults
 *            L2-H: Healing
 *
 *    @param  DTC_u32  requested DTC.
 *    @return Status.
 */
bool GetHistoryFaultStatusByDTC(const uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	/* variable declarations */
	u16_FaultPath_t FaultPath_En = 0;
	int16_t RetVal_s16 = -1;
	uint8_t FltType_u8 = 0;
	bool ret = FALSE;

	/*Check healing status*/
	RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);
	if (RetVal_s16 >= 0)
	{
		if (GetConfirmedFaultStatusByDTC(DTC_u32, FM_ProtocolType_En))
		{
			ret = TRUE;
		}
	}
	else
	{
		; // DO NOTHING
	}

	return ret;
}

/**
 *    @brief  Function to get all freezeframe recording DTCs.
 *
 *    @param  DTC_pu8  buffer to hold the DTCs.
 *    @return Number of DTCs.
 */
int8_t FM_GetAllSnapshotRecordDTCs(uint32_t *DTC_pu32)
{
	/* variable declarations */
	u16_FaultPath_t FltPath_En = 0;
	//	uint8_t FltType_u8 = 0;
	int8_t FltIndx_u8 = -1;
	int8_t Nof_DTCs = 0;
	uint32_t DTC_Value_32 = 0;

	for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
	{
		/**/
		/* if(0 != APPL_FAULTCONF_aSt[FltPath_En].FM_FrzFrm_Entries_u8
			&& NULL != APPL_FAULTCONF_aSt[FltPath_En].FM_FrzFrm_pSt)*/
		{
			// Fault index loop
			for (FltIndx_u8 = 0; ((FltIndx_u8 < MAX_NUM_ERROR_TYPES - 1)); FltIndx_u8++)
			{
				// re intialising
				DTC_Value_32 = 0;

				// fetch the DTC value from the DTC table
				DTC_Value_32 = DTCMappingTable_UDS_aSt[FltPath_En][FltIndx_u8];

				if (DTC_Value_32)
				{
					// copy the value to pointer passed
					*(DTC_pu32 + Nof_DTCs) = DTC_Value_32;

					// incrementing the number of DTC's copied
					Nof_DTCs++;
				}
			}
		}
	}

	return Nof_DTCs;
}

/**
 *    @brief  Function to get freezeframe record ID by DTCs.
 *
 *    @param  DTC_u32  requested DTC.
 *    @param  snapshot_records_ID_au8  id.
 *    @return Number of DTCs.
 */
uint8_t FM_GetAllSnapshotRecordIDByDTC(uint32_t DTC_u32, uint8_t *snapshot_records_ID_au8)
{
	*snapshot_records_ID_au8 = FREEZEFRAME_RECORD_ID;
	return NUM_OF_RECORDS;
}

/**
 *    @brief  Function to get number of freezeframe records (First ocurence and last occurence).
 *            We support First occurence hence records are 1.
 *
 *    @param  DTC_u32  requested DTC.
 *    @param  record_number_u8
 *    @return Number of DTCs.
 */
uint8_t FM_GetNumberOfSnapshotRecordIdentifiersByDTC(uint32_t DTC_u32, uint8_t record_number_u8)
{
	return NUM_OF_RECORD_IDENTIFIERS;
}

/**
 *    @brief  Function to get DataIdentifiers & total freeze frame size in bytes of freezeframes for particular DTC.
 *
 *    @param  DTC_u32  requested DTC.
 *    @param  record_number_u8
 *    @param  dataidentifiers_au16 - list of DIDs for a given DTC
 *    @param  number_of_records_ps8 - total number of bytes in the entire freeze frame structure for the gven DTC.
 *    @return Total count of DIDs. -1 if invalid inputs.
 */
// int16_t FM_GetDataIdentifiersByDTC(uint32_t DTC_u32, uint8_t record_number_u8, uint16_t* dataidentifiers_au16, int8_t* number_of_records_ps8)
// {
// uint32_t 	index_u32 = 0;
// uint32_t 	i = 0;
// u16_FaultPath_t FaultPath_En =  0;
// int16_t 	Response_Length_s16 = -1;
// int16_t 	Ret_s16 = 0;
// uint8_t 	FltType_u8   =  0;

// Ret_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8);
// if (-1 != Ret_s16)
// {
// Response_Length_s16 = (int16_t)(GLOBAL_SNAPSHOT_CNT + FM_FrzFrmConfig_aSt[FaultPath_En].NumOfEntries_u16);
// }
// *number_of_records_ps8 = 0;
// if (Response_Length_s16 > 0)
// {
// // Read DIDs of the Global snapshot table
// index_u32 = 0;
// for (i = 0; i < GLOBAL_SNAPSHOT_CNT; i++)
// {
// dataidentifiers_au16[i] = FM_GLOBAL_SNAPSHOT_aSt[index_u32].DID_u16;
// *number_of_records_ps8 += FM_GLOBAL_SNAPSHOT_aSt[index_u32].ParamSize_u8;
// index_u32++;
// }

// // Read DIDs of the Local snapshot table as per the given DTC.
// index_u32 = 0;
// for (; i < Response_Length_s16; i++)
// {
// dataidentifiers_au16[i] = FM_FrzFrmConfig_aSt[FaultPath_En].FrzFrm_ManufData_pSt[index_u32]->DID_u16;
// *number_of_records_ps8 += FM_FrzFrmConfig_aSt[FaultPath_En].FrzFrm_ManufData_pSt[index_u32]->ParamSize_u8;
// index_u32++;
// }
// }
// return Response_Length_s16;

// }

/**
 *    @brief  Function to get all freezeframe recording DTCs under records.
 *
 *    @param  record_number_u8  .
 *    @param  DTC_pu32  buffer to hold the DTCs.
 *    @return Number of DTCs.
 */
int8_t FM_GetDTCsOfSnapshotRecordByRecordNumber(uint8_t record_number_u8, uint32_t *DTC_pu32)
{
	int8_t NofDTCs_s8 = -1;
	if (1 == record_number_u8)
	{
		NofDTCs_s8 = FM_GetAllSnapshotRecordDTCs(DTC_pu32);
	}

	return NofDTCs_s8;
}

/**
 *    @brief  Function to get all stored recording DTCs under records.
 *
 *    @param  record_number_u8  .
 *    @param  DTC_pu32  buffer to hold the DTCs.
 *    @return Number of DTCs.
 */
int8_t FM_GetDTCsOfStoredRecordByRecordNumber(uint8_t record_number_u8, uint32_t *DTC_pu32)
{
	return -1;
}

/**
 *    @brief  Function to get all extended datas for requested DTC.
 *
 *    @param  DTC_u32  .
 *    @param  buffer_au8  buffer to hold the extended daata.
 *    @return size of buffer or NRC.
 */
int16_t FM_GetExtendedDataRecordByDTCNumber(uint32_t DTC_u32, uint8_t extended_record_number_u8, uint8_t *service_buffer_pu8, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	uint16_t Extended_Rescord_Value_u16 = 0u;
	u16_FaultPath_t FaultPath_En = 0;
	int8_t FltIndex_s8 = 0;
	uint8_t FltType_u8 = 0;
	int16_t Response_Length_s16 = 0;
	int16_t Ret_s16 = 0;
	//	uint16_t  LocalFreqCntr_u16 = 0;
	uint8_t DebounceCnt_u8 = 0;

	Ret_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);
	FltIndex_s8 = FML2_GetFltIndxOfPath(FaultPath_En, FltType_u8);

	if ((-1 == FltIndex_s8) || (-1 == Ret_s16))
	{
		Response_Length_s16 = -1;
	}
	else if (extended_record_number_u8 < 0xFF)
	{
		switch (extended_record_number_u8)
		{
		case 1:
		case 2:
			Extended_Rescord_Value_u16 = FML2_FaultMem_aSt[FltIndex_s8].OperationCycle_u16;
			service_buffer_pu8[5] = extended_record_number_u8;
			service_buffer_pu8[6] = (uint8_t)((Extended_Rescord_Value_u16 & 0xFF00) >> 8);
			service_buffer_pu8[7] = (uint8_t)(Extended_Rescord_Value_u16 & 0xFF);
			Response_Length_s16 = 8u;
			break;

			// case 3 :
			// Extended_Rescord_Value_u16 =  FM_CONF_NOF_NONOBD_DELETECNTS - FML2_GetWUPCyCtr(FltIndex_s8);
			// service_buffer_pu8[5] = extended_record_number_u8;
			// service_buffer_pu8[6] = (uint8_t)(Extended_Rescord_Value_u16 & 0x00FF);
			// Response_Length_s16 = 7;
			// break;

		case 5:

			if (FML2_GetPrsntFaultState(FltIndex_s8) == FM_POTENTIAL)
			{
				DebounceCnt_u8 = FM_CONF_NOF_NONOBD_POTENTIALCNTS(FaultPath_En);
			}
			else if (FML2_GetPrsntFaultState(FltIndex_s8) == FM_CONFIRMED)
			{
				DebounceCnt_u8 = FM_CONF_NOF_NONOBD_HEALINGCNTS(FaultPath_En);
			}
			else if (FML2_GetPrsntFaultState(FltIndex_s8) == FM_DELETING)
			{
				DebounceCnt_u8 = FM_CONF_NOF_NONOBD_DELETECNTS(FaultPath_En);
			}
			Extended_Rescord_Value_u16 = DebounceCnt_u8 - FML2_GetDrvCyCtr(FltIndex_s8);
			// Extended_Rescord_Value_u16 =  GET_DEBAging_CTR(FaultPath_En);
			service_buffer_pu8[5] = extended_record_number_u8;
			service_buffer_pu8[6] = (uint8_t)(Extended_Rescord_Value_u16 & 0x00FF);
			Response_Length_s16 = 7;
			break;

			// case 6 :
			// Extended_Rescord_Value_u16 =  0;// optional - OBD Only GET_DEBHEAL_CTR(FaultPath_En);
			// service_buffer_pu8[5] = extended_record_number_u8;
			// service_buffer_pu8[6] = (uint8_t)(Extended_Rescord_Value_u16 & 0x00FF);
			// Response_Length_s16 = 7;
			// break;

		case 7:
			Extended_Rescord_Value_u16 = 1;
			service_buffer_pu8[5] = extended_record_number_u8;
			service_buffer_pu8[6] = (uint8_t)(Extended_Rescord_Value_u16 & 0x00FF);
			Response_Length_s16 = 7;
			break;

		default:
			Response_Length_s16 = -1;
			break;
		}
	}
	else
	{
		service_buffer_pu8[5] = 0x02;
		Extended_Rescord_Value_u16 = FML2_FaultMem_aSt[FltIndex_s8].OperationCycle_u16;
		service_buffer_pu8[6] = (uint8_t)((Extended_Rescord_Value_u16 & 0xFF00) >> 8);
		service_buffer_pu8[7] = (uint8_t)(Extended_Rescord_Value_u16 & 0xFF);
		// service_buffer_pu8[8] = 0x03;
		// service_buffer_pu8[9] = FM_CONF_NOF_NONOBD_DELETECNTS - FML2_GetWUPCyCtr(FltIndex_s8);
		service_buffer_pu8[8] = 0x05;
		if (FML2_GetPrsntFaultState(FltIndex_s8) == FM_POTENTIAL)
		{
			DebounceCnt_u8 = FM_CONF_NOF_NONOBD_POTENTIALCNTS(FaultPath_En);
		}
		else if (FML2_GetPrsntFaultState(FltIndex_s8) == FM_CONFIRMED)
		{
			DebounceCnt_u8 = FM_CONF_NOF_NONOBD_HEALINGCNTS(FaultPath_En);
		}
		else if (FML2_GetPrsntFaultState(FltIndex_s8) == FM_DELETING)
		{
			DebounceCnt_u8 = FM_CONF_NOF_NONOBD_DELETECNTS(FaultPath_En);
		}
		service_buffer_pu8[9] = DebounceCnt_u8 - FML2_GetDrvCyCtr(FltIndex_s8);
		// service_buffer_pu8[12] = 0x06;
		// service_buffer_pu8[13] = 0;// Todo Optional OBD only, GET_DEBHEAL_CTR(FaultPath_En);
		service_buffer_pu8[10] = 0x07;
		service_buffer_pu8[11] = 1;
		Response_Length_s16 = 12u;
	}
	return Response_Length_s16;
}

/**
 *    @brief  Function to read the freeze frame.
 *
 *    @param  DTC_pu32  buffer to hold the DTCs.
 *    @param  record_number_u8
 *    @param  records_au8 array to hold the freeze data
 *    @return Number of data record.
 */
int16_t FM_GetDataRecordsByDTC(uint32_t DTC_u32, uint8_t record_number_u8, uint16_t *records_au16, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	u16_FaultPath_t FaultPath_En = 0;
	//	int16_t             i = 0;
	int16_t RetVal_s16 = -1;
	uint8_t FltType_u8 = 0;
	//	int8_t              FltIndex_s8 = -1;

	if (0x00 == record_number_u8)
	{
		RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

		if (RetVal_s16 >= 0)
		{
			RetVal_s16 = FM_ReadFrzFrm_ByFltPathFltType(FaultPath_En, FltType_u8, (uint8_t *)records_au16, FM_ProtocolType_En); // todo-hareesha; default proto=UDS
		}
	}
	return RetVal_s16;
}

/**
 *    @brief  Function to get priority of given DTC.
 *
 *    @param  DTC_u32  buffer to hold the DTCs.
 *    @return Priority.
 */
uint8_t FM_GetSeverityMaskByDTC(uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	int16_t RetVal_s16 = -1;
	u16_FaultPath_t FaultPath_En = 0;
	uint8_t FltType_u8 = 0;

	/*Get fault path required to get the priority of fault*/
	RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

	if (RetVal_s16 >= 0)
	{
		RetVal_s16 = GET_FAULT_PRIORITY(FaultPath_En);
	}

	return RetVal_s16;
}

/**
 *    @brief  Function to get unctionality ID for the particular DTC that has sent
 *
 *    @param  DTC_u32  buffer to hold the DTCs.
 *    @return Functional Unit or error.
 */
int16_t FM_GetFunctionalUnitOfDTCByDTC(uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	int16_t RetVal_s16 = -1;
	u16_FaultPath_t FaultPath_En = 0;
	uint8_t FltType_u8 = 0;

	/*Get fault path required to get the priority of fault*/
	RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

	if (RetVal_s16 >= 0)
	{
		RetVal_s16 = (int16_t)GET_FUNCTIONAL_UNIT(FaultPath_En);
	}

	return RetVal_s16;
}

/**
 *    @brief  Function to get number of all supported DTCs.
 *
 *    @param  DTCbuffer_pu32  buffer to hold the DTCs.
 *    @return Number of DTCs.
 */
uint8_t FM_GetAllSupportedDTCs(uint32_t *DTCbuffer_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	uint8_t FltIndx_u8 = 0;
	u16_FaultPath_t FltPath_En = 0;
	int8_t Nof_DTCs = 0;
	uint32_t DTC_Value_32 = 0;
	// Get all DTCs
	// Fault path loop
	for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
	{
		// Fault index loop
		for (FltIndx_u8 = 0; (FltIndx_u8 < MAX_NUM_ERROR_TYPES - 1) /*(MAX_NUM_ERROR_TYPES-TOTAL_NONSUPPORTING_FAULTS) - 1)*/; FltIndx_u8++)
		//			for(FltIndx_u8 = 0; (FltIndx_u8 < MAX_NUM_ERROR_TYPES); FltIndx_u8++)
		{
			// re intialising
			DTC_Value_32 = 0;
			// fetch the DTC value from the DTC table
			DTC_Value_32 = GetDTCofFltPathFltType(FltPath_En, FltIndx_u8, FM_ProtocolType_En);

			if (DTC_Value_32)
			{
				// copy the value to pointer passed
				*(DTCbuffer_pu32 + Nof_DTCs) = DTC_Value_32;

				// incrementing the number of DTC's copied
				Nof_DTCs++;
			}
		}
	}
	return Nof_DTCs;
}

/**
 *    @brief  Function to get DTC which was failed for the first time after clear diagnostic session.
 *
 *    @param  None.
 *    @return First failed DTC.
 */
uint32_t FM_GetFirstTestFailedDTC(void)
{
	return FM_FirstFailedFltsInfo_St.ActiveDTC_u32;
}

/**
 *    @brief  Function to get DTC which was confirmed for the first time after clear diagnostic session.
 *    @param  None.
 *    @return First failed DTC.
 */
uint32_t FM_GetFirstConfirmedDTC(void)
{
	return FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32;
}

/**
 *    @brief  Function to get mirror memory of FreezeFrame.
 *
 *    @param  DTCbuffer_au8  holds themirror memory DTC.
 *    @return Number of DTCs or error.
 */
int16_t FM_GetAllMirrorMemoryDTC(uint8_t *DTCbuffer_au8)
{
	return -1;
}

/**
 *    @brief  Function to get the status of the Mirror DTC that has passed
 *
 *    @param  DTC_u32  mirror memory DTC.
 *    @return Status or error.
 */
int16_t FM_GetStatusOfMirrorMemoryDTCByDTCs(uint32_t DTC_u32)
{
	return -1;
}

int32_t FM_MirrorMemoryExtendedDataByDTCNumber(uint32_t DTCnumber_u32, uint8_t *buffer)
{
	return -1;
}

/**
 *    @brief  Function to get the fault detection counter value of the particular DTC.
 *
 *    @param  DTC_u32   DTC.
 *    @return Detection count or error.
 */
int16_t FM_GetDTCFaultDetectionCounterByDTC(uint32_t DTC_u32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	int16_t RetVal_s16 = -1;
	u16_FaultPath_t FaultPath_En = 0;
	uint8_t FltType_u8 = 0;

	/*Get fault path required to get the priority of fault*/
	RetVal_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8, FM_ProtocolType_En);

	if (RetVal_s16 >= 0)
	{
		RetVal_s16 = FML1_GetActDebCnt(FaultPath_En);
	}

	return RetVal_s16;
}

/**
 *    @brief  Function to get the Permanent status DTCs.
 *
 *    @param  DTCbuffer_pu32   buffer.
 *    @return Number of DTCs copied into buffer.
 */
int8_t FM_GetDTCWithPermanentStatus(uint32_t *DTCbuffer_pu32, FM_ProtocolType_En_t FM_ProtocolType_En)
{
	int8_t NoFDTCCodeCopied_s8 = 0;
	NoFDTCCodeCopied_s8 = FM_GetAllCnfrmNONOBD_DTCs(DTCbuffer_pu32, FM_ProtocolType_En);
	return NoFDTCCodeCopied_s8;
}

/**
 *    @brief  Function to clear Diagnostic information fault.
 *
 *    @param  FM_FunctionalUnit_En   group of DTC.
 *    @return stastus.
 */
uint8_t Error_State = 0;
bool FM_ClearDiagnosticInformationForDTCGroup(FM_FunctionalUnit_En_t FM_FunctionalUnit_En)
{
	uint32_t DebounceCnt_u8 = 0u;
	uint8_t FltIndx_u8 = 0u;
	uint8_t Indx_u8 = 0u;
	int8_t NoErrThreshold_s8 = 0;
	uint8_t ErrThreshold_u8 = 0u;
	uint16_t DebounceCnt_u16 = 0u;
	u16_FaultPath_t FltPath_En = 0u;
	bool Status_b = TRUE;

	/*Clear malfunction lamp*/
	FM_TurnOffMI();

	/*Clear L2 faults*/
	for (FltIndx_u8 = 0; FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES; FltIndx_u8++)
	{
		//resetting the driving cycle count
		//FML2_RstDrvCyCtr(FltIndx_u8);
		FltPath_En = FML2_GetFaultPath(FltIndx_u8);
		if ((FM_NOENTRY != FML2_GetPrsntFaultState(FltIndx_u8)) &&
			((FM_FunctionalUnit_En == GET_FUNCTIONAL_UNIT(FltPath_En)) ||
				FM_ANY_E == FM_FunctionalUnit_En))
		{
			ClrFltEntry(FltIndx_u8);
		}
	}

	/* Clear the L1 memory */
	for (FltPath_En = FM_FAULT_START_E; FltPath_En < NUM_OF_FAULTPATHS_E; FltPath_En++)
	{
		/*Get the threshold for No error and error*/
		NoErrThreshold_s8 = GET_DEBHEAL_CTR(FltPath_En);
		ErrThreshold_u8 = GET_DEBDEFECT_CTR(FltPath_En);
		DebounceCnt_u16 = ErrThreshold_u8 - NoErrThreshold_s8;
		for (DebounceCnt_u8 = 0; DebounceCnt_u8 <= DebounceCnt_u16; DebounceCnt_u8++)
		{

			if ((FM_NO_ERR != FML1_GetErrType(FltPath_En)) &&
				((FM_FunctionalUnit_En == GET_FUNCTIONAL_UNIT(FltPath_En)) ||
					FM_ANY_E == FM_FunctionalUnit_En))
			{
				FML1_SetDebStatus(FltPath_En, FML1_DEBHEAL_E);
				FM_ReportFault(FltPath_En, FM_NO_ERR);
			}
			else
			{
				break;
			}
		}
		FML1_SetDebStatus(FltPath_En, FML1_IDLE_E);
		FML1_CLR_TF_STATUS_OF_DTC_MAPTBL(FltPath_En);
		FML1_CLR_TFTOC_STATUS_OF_DTC_MAPTBL(FltPath_En);
		FML1_ClrAllContPrsnt(FltPath_En);
		FML1_CLR_PDTC_STATUS_OF_DTC_MAPTBL(FltPath_En);
	    FML1_CLR_CDTC_STATUS_OF_DTC_MAPTBL(FltPath_En);
		FML1_CLR_WIR_STATUS_OF_DTC_MAPTBL(FltPath_En);
	    FML1_SetActDebCnt(FltPath_En, FML1_IDLE_E);
	    // updated the status
	    FML1_SetUpt_St(FltPath_En);
	}

	/*Clear readyness variable*/
	FM_ClrRdyMonFlags();

	/*Clear common counters*/
	FM_ClrCommonData();

	/*Clear the First failed DTC information*/
	FM_FirstFailedFltsInfo_St.ActiveDTC_u32 = 0;
	FM_FirstFailedFltsInfo_St.CnfrmedDTC_u32 = 0;

	/*Update the falgs*/
	for (Indx_u8 = 0u; Indx_u8 < TOTAL_DTCCLR_INDICATOR; Indx_u8++)
	{
		FM_FirstFailedFltsInfo_St.DTCClearedIndicator[Indx_u8] = TRUE;
	}

	FM_ClrTFSLC_All();

	return Status_b;
}

uint8_t FM_GetStatusAvailabiltyMask(void)
{
	return DTCSTATUS_AVAL_MASK;
}

uint8_t FM_GetSeverityAvailabiltyMask(void) //@added
{
	return DTCSEVERITY_AVAL_MASK;
}

/**
 *    @brief  Function to get the DTC extendended data record by record number.
 *
 *    @param  DTC_u32  emission related DTC.
 *    @return Status or error.
 */
int8_t FM_GetDTCExtDataRecordByRecordNumber(uint32_t DTC_u32)
{
	return -1;
}

/**
 *    @brief  Function to get the user defined memory DTC by status mask.
 *
 *    @param  DTC_u32  emission related DTC.
 *    @return Status or error.
 */
int8_t FM_GetUserDefMemoryDTCByStatusMask(uint32_t DTC_u32)
{
	return -1;
}

/**
 *    @brief  Function to get the user defined memory DTC by DTC snapshotrecord by DTC number.
 *
 *    @param  DTC_u32  emission related DTC.
 *    @return Status or error.
 */
int8_t FM_GetUserDefMemoryDTCSnapshotRecordByDTCNumber(uint32_t DTC_u32)
{
	return -1;
}

/**
 *    @brief  Function to get the user defined memory DTC Eextendend record by DTC number.0
 *
 *    @param  DTC_u32  emission related DTC.
 *    @return Status or error.
 */

int8_t FM_GetUserDefMemoryDTCExtDataRecordByDTCNumber(uint32_t DTC_u32)
{
	return -1;
}

/*
 *    @brief  Function to get the number of snapshot DIDs.
 *
 *    @param  DTC_u32  DTC number.
 *    @return number of snapshot DIDs.
 */

uint8_t u8_FM_GetNumOfSnapshotDIDs(u16_FaultPath_t FltPath_En, uint16_t idx)
{
	// uint16_t FrzFrmVal_u16 = 0;
	//	if(NULL != FML1_GetNonOBDFrzFrmCallBack(FltPath_En, idx))
	//     {
	//       FrzFrmVal_u16 = FML1_NonOBDFrzFrmCallBack (FltPath_En, idx);
	//     }
	//	else
	{
		return 0;
	}
}

uint8_t Rd_DTC_HistoryFlag(int8_t FltIndx_u8)
{
	uint8_t DTC_HistoryFlag;
	DTC_HistoryFlag = FML2_GetPrevFaultState(FltIndx_u8);
	// DTC_HistoryFlag = FML2_GetPrsntFaultState(FltIndx_u8);
	return DTC_HistoryFlag;
}

#ifdef NVM_BACKUPRAM
/*==============================================================================*/
/*																				*/
/*==============================================================================*/
/* [function]			- NVM write												*/
/* [interface]      	- void AcsDTC_param(void)								*/
/* [description]    	-  														*/
/* [argument]       	-   													*/
/* [return value] 		-   			                     					*/
/* [code]           	-  														*/
/* [update]         	-  				 										*/
/* [attention]      	-                                                    	*/
/*==============================================================================*/

void AcsDTC_param(void)
{
	static U2 u2_NVM_DTC_Data1;
	static U2 u2_NVM_DTC_Data2;
	static U2 u2_NVM_DTC_prev;
	U4 u4_BKUPRAM_DTC_Data;
	NVM_RET st_ret;

	switch (u1_DTC_NVM_step)
	{
	case NVM_NOPE:
		st_ret = st_NvmNonWrite();
		if (st_ret.u1_ret == OK)
		{
			DTC_loop_index = 0;
			NvmmgrSetReq(u2_REQ_DTCCONFIG_PARAM, OFF);
		}
		break;

	case READ_NVM: /*part of write process */
		st_ret = st_NvmNonWrite();
		if (st_ret.u1_ret == OK)
		{
			st_ret = st_NvmRead(u2_ADR_DTC_PARA + DTC_loop_index);
			if (st_ret.u1_ret == OK)
			{
				u2_NVM_DTC_prev = st_ret.u2_data;
				u1_DTC_NVM_step = WRITE_NVM;
			}
		}
		break;

	case WRITE_NVM:
		st_ret = st_NvmLetWrite();
		if (st_ret.u1_ret == OK)
		{
			st_ret.u1_ret = 0; // NG;
			if ((DTC_loop_index % 2) == 0)
			{
				u4_BKUPRAM_DTC_Data = (U4)Bkupram_u4_Read32(u4_DTCL2ENTRY_BKUP_RAM_ADD + (DTC_loop_index * 2));
				u2_NVM_DTC_Data1 = (U2)((u4_BKUPRAM_DTC_Data >> 16) & 0xFFFF);
				u2_NVM_DTC_Data2 = (U2)(u4_BKUPRAM_DTC_Data & 0xFFFF);
				if (u2_NVM_DTC_prev != u2_NVM_DTC_Data1)
				{
					st_ret = st_NvmWrite((U2)(u2_ADR_DTC_PARA + DTC_loop_index), (U2)u2_NVM_DTC_Data1);
				}
				else
				{
					st_ret.u1_ret = OK;
				}
			}
			else
			{
				if (u2_NVM_DTC_prev != u2_NVM_DTC_Data2)
				{
					st_ret = st_NvmWrite((U2)(u2_ADR_DTC_PARA + DTC_loop_index), (U2)u2_NVM_DTC_Data2);
				}
				else
				{
					st_ret.u1_ret = OK;
				}
			}
			if (st_ret.u1_ret == OK)
			{
				DTC_loop_index += 1;
				u1_DTC_NVM_step = READ_NVM;
			}
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
			if (DTC_loop_index * 2 >= u2_DTC_Entry_Size)
			{
				u1_DTC_NVM_step = NVM_NOPE;
			}
#endif
		}
		break;

	default:
		u1_DTC_NVM_step = NVM_NOPE;
		break;
	}
}
#endif
void FM_SetDTCBitMask(uint8_t u8_index, uint8_t u8_mask_enable)
{
	if (TRUE == u8_mask_enable)
	{
		u64_DTCBitMask |= (uint64_t)(0x01 << u8_index);
	}
	else
	{
		u64_DTCBitMask &= (uint64_t)(~((uint64_t)(0x01 << u8_index)));
	}
#ifdef NVM_BACKUPRAM
	u1_dtcbitmask_step = WRITE_NVM;
	NvmmgrSetReq(u2_REQ_NVM_DTCBITMASK, ON);
#endif
}

#ifdef NVM_BACKUPRAM
static void vd_Write_DTCBitMask_BkupRAM(void)
{
	//(void)Bkupram_en_Write32((u4_DTCBITMASK_BKUP_RAM_ADD), (U4)(u64_DTCBitMask & 0x00000000FFFFFFFF));
	//(void)Bkupram_en_Write32((u4_DTCBITMASK_BKUP_RAM_ADD + 4), (U4)((U6)(u64_DTCBitMask & 0xFFFFFFFF00000000) >> 32));
}
#endif

#ifdef NVM_BACKUPRAM
void AcsDTCBitMask(void)
{
	NVM_RET st_ret;
	static uint8_t buffer_idx = 0;

	switch (u1_dtcbitmask_step)
	{
	case NVM_NOPE:
		st_ret = st_NvmNonWrite();
		if (st_ret.u1_ret == OK)
		{
			buffer_idx = 0;
			NvmmgrSetReq(u2_REQ_NVM_DTCBITMASK, OFF);
		}
		break;
	case READ_NVM:
		if (buffer_idx < 4)
		{
			st_ret = st_NvmRead(u2_ADR_DTCBITMASK + buffer_idx);
			if (st_ret.u1_ret == OK)
			{
				u64_DTCBitMask |= (uint64_t)(st_ret.u2_data << (16 * buffer_idx));
				buffer_idx++;
			}
		}
		else
		{
			vd_Write_DTCBitMask_BkupRAM();
			u1_dtcbitmask_step = NVM_NOPE;
		}
		break;
	case WRITE_NVM:
		if (buffer_idx < 4)
		{
			st_ret = st_NvmLetWrite(); /* Rewriting permission			*/
			if (st_ret.u1_ret == OK)
			{
				st_ret = st_NvmWrite((u2_ADR_DTCBITMASK + buffer_idx), ((u64_DTCBitMask >> (16 * buffer_idx)) & 0xFFFF));
				if (st_ret.u1_ret == OK)
				{
					buffer_idx++;
				}
			}
		}
		else
		{
			vd_Write_DTCBitMask_BkupRAM();
			u1_dtcbitmask_step = NVM_NOPE;
		}
		break;
	default:
		u1_dtcbitmask_step = NVM_NOPE;
		break;
	}
}
#endif
#ifdef NVM_BACKUPRAM
// todo-hareesha
static void vd_Read_DTCBitMask_BkupRAM(void)
{
	u64_DTCBitMask = 0;
	u64_DTCBitMask |= (U4)(Bkupram_u4_Read32(u4_DTCBITMASK_BKUP_RAM_ADD));
	u64_DTCBitMask |= (U6)((U6)Bkupram_u4_Read32(u4_DTCBITMASK_BKUP_RAM_ADD + 4) << 32);
}
#endif

/**
 *    @brief  Function to SET "Test failed since last clear" flag for a given faultpath & type.
 *
 *    @param  faultpath & fault type
 *    @return TRUE: for valid faultpath value.
 *    @return FALSE: for invalid faultpath value (out of range).
 */
bool FM_SetTFSLC_ByFltPathFltType(u16_FaultPath_t FaultPath_En, uint8_t FltType_u8)
{
	if (FaultPath_En < NUM_OF_FAULTPATHS_E)
	{
		FML1_FltMonRam_St.TFSLC_Mat_au8[FaultPath_En] |= FltType_u8;
		return TRUE;
	}
	return FALSE;
}

/**
 *    @brief  Function to clear "Test failed since last clear" flag for a given faultpath & type.
 *
 *    @param  faultpath & fault type
 *    @return TRUE: for valid faultpath value.
 *    @return FALSE: for invalid faultpath value (out of range).
 */
bool FM_ClrTFSLC_ByFltPathFltType(u16_FaultPath_t FaultPath_En, uint8_t FltType_u8)
{
	if (FaultPath_En < NUM_OF_FAULTPATHS_E)
	{
		FML1_FltMonRam_St.TFSLC_Mat_au8[FaultPath_En] = 0;
		return TRUE;
	}
	return FALSE;
}

/**
 *    @brief  Function to clear "Test failed since last clear" flag for all fault paths.
 *
 *    @param  none
 *    @return TRUE: reset all the flags.
 */
bool FM_ClrTFSLC_All(void)
{
	uint16_t i = 0;
	for (i = 0; i < NUM_OF_FAULTPATHS_E; i++)
	{
		FML1_FltMonRam_St.TFSLC_Mat_au8[i] = 0;
	}
	return TRUE;
}

/**
 *    @brief  Function to get the Test Failed Since last clear flag for a given fault path & type.
 *
 *    @param  fault path & fault type
 *    @return TRUE: for valid faultpath value.
 *			 FALSE: for invalid faultpath value.
 */
bool FM_GetTFSLC_ByFltPathFltType(u16_FaultPath_t FaultPath_En, uint8_t FltType_u8)
{
	if (FaultPath_En < NUM_OF_FAULTPATHS_E)
	{
		if (FML1_FltMonRam_St.TFSLC_Mat_au8[FaultPath_En] & FltType_u8)
		{
			return TRUE;
		}
	}
	return FALSE;
}

int8_t FM_Get_DTCs(uint32_t *DTC_pu32, uint16_t ReadLimit_u16, FM_ProtocolType_En_t FM_ProtocolType_En, FM_Fault_Type_En_t FM_Fault_Type_En, FM_Fault_Condition_En_t FM_Fault_Condition_En)
{
	if (FM_PRESENT_E == FM_Fault_Condition_En)
	{
		return FML1_Get_DTC(DTC_pu32, ReadLimit_u16, FM_ProtocolType_En, FM_Fault_Type_En, FM_Fault_Condition_En);
	}
	else if ((FM_CONFIRMED_E == FM_Fault_Condition_En) || (FM_DELETING_E == FM_Fault_Condition_En))
	{
		return FML2_Get_DTCs(DTC_pu32, ReadLimit_u16, FM_ProtocolType_En, FM_Fault_Type_En, FM_Fault_Condition_En);
	}
	else if (FM_PENDING_E == FM_Fault_Condition_En)
	{
		return FM_FM_Get_DTCs(DTC_pu32, ReadLimit_u16, FM_ProtocolType_En, FM_Fault_Type_En, FM_Fault_Condition_En);
	}
	else
	{
		/* code */
	}
	return -1;
}

int8_t FM_FM_Get_DTCs(uint32_t *DTC_pu32, uint16_t numdtc_req_u16, FM_ProtocolType_En_t FM_ProtocolType_En, FM_Fault_Type_En_t FM_Fault_Type_En, FM_Fault_Condition_En_t FM_Fault_Condition_En)
{
	uint8_t FltIndx_u8 = 0;
	int8_t Nof_DTCs = 0;
	uint16_t DTC_Value = 0;
	bool OBD_Relevent_flag_b = FALSE;

	// searching in L2 memory
	for (FltIndx_u8 = 0; ((FltIndx_u8 < FM_CONF_MAX_L2FAULT_ENTRIES) && numdtc_req_u16); FltIndx_u8++)
	{
		switch (FM_Fault_Type_En)
		{
		case FM_OBD_E:
			OBD_Relevent_flag_b = IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8));
			break;

		case FM_NONOBD_E:
			OBD_Relevent_flag_b = !(IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8)));
			break;

		case FM_ALL_E:
			OBD_Relevent_flag_b = TRUE;
			break;

		default:
			OBD_Relevent_flag_b = FALSE;
			break;
		}
		if ((FM_POTENTIAL == FML2_GetPrsntFaultState(FltIndx_u8)) && (OBD_Relevent_flag_b /* == IS_FAULT_OBD_RELEVANT(FML2_GetFaultPath(FltIndx_u8))*/))
		{
			// re intialising
			DTC_Value = 0;

			/* fetch the DTC value from the DTC table */ 
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
	return Nof_DTCs;
}
