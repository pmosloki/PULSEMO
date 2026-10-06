/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv10_callback.c
|    Project      	: MIL_PBL_CV
|    Description    : callback function description for UDS service - Session Control
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

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "iso14229_serv31.h"
#include "uds_conf.h"
#include "iso14229_serv31_conf.h"
#include "nvm_data.h"
#include "nvm_conf.h"
//#include "eeprom.h"
#include "nvm_parameter.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CheckValidSoftware
*   Description   : This function will Erase flash memory from defined start
                    address to end address.
*   Parameters    : buffPu8 - ASW block to br erase.
*   Return Value  : int8_t status.
*******************************************************************************/
uint8_t CheckProgDependency(uint8_t *DataPtr_pu8, uint8_t RoutineType_u8, uint8_t *RIDdiscrepter)
{
    RIDConf_St_t *RIDdiscrepter_st = (RIDConf_St_t *)RIDdiscrepter;

    /*if all conditions are satisfied*/
    RIDdiscrepter_st->Result_au8[0] = 0x00;
    RIDdiscrepter_st->RoutineState_u8 = ROUTINE_IDLE_E;

    return 1;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ReprogramPreCondition
*   Description   : The function checks the reprogrammimg precondition
*   Parameters    : buffPu8 - ASW block to br erase.
*   Return Value  : int8_t status.
*******************************************************************************/
uint8_t ReprogramPreCondition(uint8_t *DataPtr_pu8, uint8_t RoutineType_u8, uint8_t *RIDdiscrepter)
{
    RIDConf_St_t *RIDdiscrepter_st = (RIDConf_St_t *)RIDdiscrepter;

	if (true) /*check for any preconditions */
	{
		// PblParam_St.ReprogramReqState_u8 = true;
		// UpdateNVMblock(PBL_NVM_BLOCK);
		/*if all conditions are satisfied*/
        RIDdiscrepter_st->Result_au8[0] = 0x00;
        RIDdiscrepter_st->RoutineState_u8 = ROUTINE_IDLE_E;
	}
	else
	{
		/*if conditions not satisfied*/
		RIDdiscrepter_st->Result_au8[0] = 0x01;
        RIDdiscrepter_st->RoutineState_u8 = ROUTINE_IDLE_E;
	}
	return 1;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UpdateServiceResetInfo
*   Description   : This function will Erase flash memory from defined start
                    address to end address.
*   Parameters    : buffPu8 - ASW block to br erase.
*   Return Value  : int8_t status.
*******************************************************************************/
// uint8_t UpdateServiceResetInfo(uint8_t *DataPtr_pu8, uint8_t RoutineType_u8, uint8_t *RIDdiscrepter)
// {
//     RIDConf_St_t *RIDdiscrepter_st = (RIDConf_St_t *)RIDdiscrepter;
//     UpdateResetServRmd();
//     uint32_t OdoValueKm_u32 = ClusterConfigData_St.DistInKmAtServ_u32;
// 	uint32_t HourMeterValue_u32 = ClusterConfigData_St.HrsAtServ_u32;
//     uint8_t ServKmNHrs_u8[6] = {0};
//     ServKmNHrs_u8[0] = (uint8_t)((OdoValueKm_u32 >> 16) & 0xff);
// 	ServKmNHrs_u8[1] = (uint8_t)((OdoValueKm_u32 >> 8) & 0xff);
// 	ServKmNHrs_u8[2] = (uint8_t)((OdoValueKm_u32) & 0xff);
// 	ServKmNHrs_u8[3] = (uint8_t)((HourMeterValue_u32 >> 16) & 0xff);
// 	ServKmNHrs_u8[4] = (uint8_t)((HourMeterValue_u32 >> 8) & 0xff);
// 	ServKmNHrs_u8[5] = (uint8_t)(HourMeterValue_u32 & 0xff);
//     Nvm_Write(KM_HR_LAST_SERVICE_E,(uint8_t*)&ServKmNHrs_u8[0]);

//     /*if all conditions are satisfied*/
//     RIDdiscrepter_st->Result_au8[0] = 0x00;
//     RIDdiscrepter_st->RoutineState_u8 = ROUTINE_IDLE_E;

//     return 1;
// }

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CheckValidSoftware
*   Description   : This function will Erase flash memory from defined start
                    address to end address.
*   Parameters    : buffPu8 - ASW block to br erase.
*   Return Value  : int8_t status.
*******************************************************************************/
uint8_t WriteDefaultConfigData(uint8_t *DataPtr_pu8, uint8_t RoutineType_u8, uint8_t *RIDdiscrepter)
{
    uint32_t Loop_Cnt_u32 = 0;
    NvmBlk_St_t *Blk_pSt = NULL;
    uint16_t Crc_Cal_u16 = 0;
    RIDConf_St_t *RIDdiscrepter_st = (RIDConf_St_t *)RIDdiscrepter;

    if(ROUTINE_IDLE_E == RIDdiscrepter_st->RoutineState_u8)
    {
        RIDdiscrepter_st->RoutineState_u8 = ROUTINE_INPROGRESS_E;
        //ResetEventBasedData();
        // Reset_NVM_b = true;
        for (Loop_Cnt_u32 = NVM_BLK_START_E; Loop_Cnt_u32 < TOTAL_NVM_BLK_E; Loop_Cnt_u32++)
        {
            Blk_pSt = (NvmBlk_St_t *)&NvmBlk_aSt[Loop_Cnt_u32];
            if (NVM_IDLE_E == NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En)
            {
                MemCopy((Blk_pSt->RAMCopy_pu8), Blk_pSt->RomCopy_pu8, Blk_pSt->Length_u32);
                Crc_Cal_u16 = crc16(&(Blk_pSt->RAMCopy_pu8[BLOCK_START_OFFSET]), (Blk_pSt->Length_u32 - NVM_BLOCK_HEADER_SIZE), INIT_CRC);
                Blk_pSt->RAMCopy_pu8[NVM_CRC_POS_1] = (uint8_t)(Crc_Cal_u16 >> 8) & 0xFFU;
                Blk_pSt->RAMCopy_pu8[NVM_CRC_POS_2] = (uint8_t)Crc_Cal_u16 & 0xFFU;
                NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_UPLOAD_E;
            }
        }
    }
    else if(ROUTINE_INPROGRESS_E == RIDdiscrepter_st->RoutineState_u8)
    {
        if(true == Is_Nvm_Idle())
        {
            RIDdiscrepter_st->RoutineState_u8 = ROUTINE_COMPLETED_E;
            RIDdiscrepter_st->Result_au8[0] = 0x00;
            // OdoReset();
            // HourReset();
            //ResetServRmd();
        }
        else
        {

        }

    }
    else
    {

    }
    return 1;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : EnterBootMode
*   Description   : This function will Erase flash memory from defined start
                    address to end address.
*   Parameters    : buffPu8 - ASW block to br erase.
*   Return Value  : int8_t status.
*******************************************************************************/
uint8_t EnterBootMode(uint8_t *DataPtr_pu8, uint8_t RoutineType_u8, uint8_t *RIDdiscrepter)
{
    RIDConf_St_t *RIDdiscrepter_st = (RIDConf_St_t *)RIDdiscrepter;
    if (ROUTINE_IDLE_E == RIDdiscrepter_st->RoutineState_u8)
    {
        RIDdiscrepter_st->RoutineState_u8 = ROUTINE_INPROGRESS_E;
        PblParam_St.ReprogramReqState_u8 = true;
        UpdateNVMblock(PBL_NVM_BLOCK);
        HardReset_b = true;
        Nvm_Shutdown();
    }
    else
    {
        RIDdiscrepter_st->RoutineState_u8 = ROUTINE_INPROGRESS_E;
    }
    return 1;
}
/*---------------------- End of File -----------------------------------------*/