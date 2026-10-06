/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File         : uds_conf.c
|    Project      : MIL_PBL_CV
|    Description  : Service description for UDS service configurations
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S        Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

 #ifndef UDS_CONF_C
 #define UDS_CONF_C

/*******************************************************************************
 *  Includes
 ******************************************************************************/

#include "uds_conf.h"
#include "diag_appl_test.h"
#include "diag_adapt.h"
#include "diag_sys_conf.h"
#include "fee_adapt.h"
#include "fee_conf.h"
#include "crc_util.h"
#include "uds_DTC.h"

#pragma section const "APP_VER"
const uint8_t ASW_Release_Ver_u8[0x20] = "ASW_SDK_V1";
const uint8_t CustomerName_u8[0x20] = "PULSEMO";
const uint8_t Internal_ASW_Release_Ver_u8[0x20] = "48PIN_ASW_SDK_V3";
#pragma section

#if (TRUE == UDS_SERVICE10_ENABLE)
  #include "iso14229_serv10.h"
#endif
#if (TRUE == UDS_SERVICE11_ENABLE)
  #include "iso14229_serv11.h"
#endif
#if (TRUE == UDS_SERVICE14_ENABLE)
  #include "iso14229_serv14.h"
#endif
#if (TRUE == UDS_SERVICE19_ENABLE)
  #include "iso14229_serv19.h"
#endif
#if (TRUE == UDS_SERVICE22_ENABLE)
  #include "iso14229_serv22.h"
#endif
#if (TRUE == UDS_SERVICE27_ENABLE)
  #include "iso14229_serv27.h"
#endif
#if (TRUE == UDS_SERVICE28_ENABLE)
  #include "iso14229_serv28.h"
#endif
#if (TRUE == UDS_SERVICE2E_ENABLE)
  #include "iso14229_serv2E.h"
#endif
#if (TRUE == UDS_SERVICE31_ENABLE)
  #include "iso14229_serv31.h"
#endif
#if (TRUE == UDS_SERVICE34_ENABLE)
  #include "iso14229_serv34.h"
#endif
#if (TRUE == UDS_SERVICE36_ENABLE)
  #include "iso14229_serv36.h"
#endif
#if (TRUE == UDS_SERVICE37_ENABLE)
  #include "iso14229_serv37.h"
#endif
#if (TRUE == UDS_SERVICE3D_ENABLE)
  #include "iso14229_serv3D.h"
#endif
#if (TRUE == UDS_SERVICE3E_ENABLE)
  #include "iso14229_serv3E.h"
#endif
#if (TRUE == UDS_SERVICE83_ENABLE)
  #include "iso14229_serv83.h"
#endif
#if (TRUE == UDS_SERVICE85_ENABLE)
  #include "iso14229_serv85.h"
#endif
#if (TRUE == UDS_SERVICE87_ENABLE)
  #include "iso14229_serv87.h"
#endif
#if ((TRUE == UDS_SERVICE22_ENABLE) || (TRUE == UDS_SERVICE2E_ENABLE) || (TRUE == UDS_SERVICE2F_ENABLE))
  #include "uds_DID.h"
#endif

/*******************************************************************************
 *  macros
 ******************************************************************************/


/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
BSW_NVM_Data_St_t BSW_NVM_Data_St;
BSW_NVM_Update_St_t BSW_NVM_Update_St;
PblParam_St_t PblParam_St;
SblPgmData_St_t SblPgmData_St = {0,0};

bool SoftReset_b = false;
bool JumpToSBL_b = false;
volatile bool HardReset_b = false;
bool Prg_stratd_flag = FALSE;
bool JumpToApplication_b = false;
uint32_t SblJumpCnt_u32 = 0;
uint32_t ApplJumpCnt_u32 = 0;
uint32_t EcuResetCnt_u32 = 0;
uint32_t PblDelayWaitCnt_u32 = 0;
uint8_t ReProgmReq_u8 = false;
uint8_t PgmDelayEn_u8 = false;


/******Constant service distributor table for ISO 14229 services***************/ 

const ServiceDistUDS_St_t UDSServDist_apt[UDS_TOTAL_SERVICE_E] =
{
/*  SID     Initialization function   service implementation   */
#if (TRUE == UDS_SERVICE10_ENABLE)
    {SID_SESSIONCONTROL,        UDS_Serv10Init,  &iso14229_serv10, (uint8_t)(UDS_SESS_ALL),true},
#endif

#if (TRUE == UDS_SERVICE11_ENABLE)
    {SID_ECURESET,              Serv11Init,  &iso14229_serv11, (uint8_t)(UDS_SESS_ALL/*UDS_SESS_EQU_EXTENDED |UDS_SESS_EQU_DEFAULT| UDS_SESS_EQU_PROG*/),false},
#endif
    
#if (TRUE == UDS_SERVICE14_ENABLE)
    {SID_CDTC,                  NULL,  &iso14229_serv14, (uint8_t)(UDS_SESS_ALL),false},
#endif
    
#if (TRUE == UDS_SERVICE19_ENABLE)
    {SID_RDTC,                  NULL,  &iso14229_serv19, (uint8_t)(UDS_SESS_ALL),false},
#endif    

#if (TRUE == UDS_SERVICE22_ENABLE)
    {SID_RDBDID,                UdsServ22Init,  &iso14229_serv22, (uint8_t)(UDS_SESS_ALL /*| UDS_SESS_EQU_DEFAULT|UDS_SESS_EQU_PROG*/),false},
#endif

#if (TRUE == UDS_SERVICE23_ENABLE)
    {SID_RMBA,                  NULL,  &iso14229_serv23, (uint8_t)(UDS_SESS_EQU_EXTENDED)},
#endif

#if (TRUE == UDS_SERVICE27_ENABLE)
    {SID_SA,                    NULL,  &iso14229_serv27, (uint8_t)(UDS_SESS_EQU_EXTENDED | UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA),false},
#endif

#if (TRUE == UDS_SERVICE28_ENABLE)
    {SID_COMMUNICATIONCONTROL,  NULL,  &iso14229_serv28, (uint8_t)(UDS_SESS_EQU_DEFAULT /*| UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA*/),true},
#endif

#if (TRUE == UDS_SERVICE2E_ENABLE)
    {SID_WDBDID,                NULL,  &iso14229_serv2E, (uint8_t)(UDS_SESS_EQU_EXTENDED | UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA),false},
#endif

#if (TRUE == UDS_SERVICE2F_ENABLE)
    {SID_IOCBDID,               NULL,  &iso14229_serv2F, (uint8_t)(UDS_SESS_EQU_EXTENDED)},
#endif

#if (TRUE == UDS_SERVICE31_ENABLE)    
    {SID_ROUTINE_CONTROL,       NULL,  &iso14229_serv31, (uint8_t)(UDS_SESS_EQU_EXTENDED | UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA ),false},
#endif

#if (TRUE == UDS_SERVICE34_ENABLE)  
    {SID_REQUEST_DOWNLOAD,      NULL,  &iso14229_serv34, (uint8_t)(UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA )},
#endif

#if (TRUE == UDS_SERVICE36_ENABLE)
    {SID_TRANSFER_DATA,         NULL,  &iso14229_serv36, (uint8_t)(UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA)},
#endif  

#if (TRUE == UDS_SERVICE37_ENABLE)
    {SID_TRANSFER_DATA_EXIST,   NULL,  &iso14229_serv37, (uint8_t)(UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA)},
#endif  

#if (TRUE == UDS_SERVICE3E_ENABLE)
    {SID_TESTERPRESENT,         NULL,  &iso14229_serv3E,  (uint8_t)(UDS_SESS_ALL),false},
#endif

#if (TRUE == UDS_SERVICE3D_ENABLE)
    {SID_WMBA,                  NULL,  &iso14229_serv3D, (uint8_t)(UDS_SESS_EQU_EXTENDED)},
#endif

#if (TRUE == UDS_SERVICE83_ENABLE)
    {SID_ATP,                   NULL,  &iso14229_serv83, (uint8_t)(UDS_SESS_EQU_EXTENDED)},
#endif

#if (TRUE == UDS_SERVICE85_ENABLE)
    {SID_CONTROLDTC,            NULL,  &iso14229_serv85, (uint8_t)(UDS_SESS_EQU_DEFAULT /*| UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA*/),true},
#endif

#if (TRUE == UDS_SERVICE87_ENABLE)
    {SID_LINK_CONTROL,          NULL,  &iso14229_serv87, (uint8_t)(UDS_SESS_EQU_EXTENDED | UDS_SESS_EQU_DEFAULT)},
#endif
};

/******************************************************************************/
/**********************Bootloader StateMechine*********************/
static FlashState_En_t Flash_State_En = SERVICE_IDLE_E;

/******************************************************************************/
/**********************structure of arrays  for memory blocks******************/

// MemAccFptr_St_t MemAccFptr_aSt[TOTAL_BLOCK_E] = 
// {
  
//   {NULL,Flash_Write,PFlash_Read,Graphics_Flash_Erase,Graphics_Flash_Write,Graphics_Flash_Read,NULL },
// };

// MemBlockConf_St_t MemBlockConf_aSt[TOTAL_BLOCK_E] =
// {
//   {	ASW_BLOCK_E, 0xFED98000, ( 0xFEDFFFFF - 0xFED98000), ERASABLE_E,DATA_BLOCK_INVALID_E,BACKUP_SBL_START_ADDR, (BACKUP_SBL_END_ADDR - BACKUP_SBL_START_ADDR), RAM_MEMORY_E, EXTERNAL_EEPROM_1_STORAGE_E,&MemAccFptr_aSt[ASW_BLOCK_E]}, 
// };
const SubFunctionSer19_St_t Service19_SubFunctionsaSt[SERV19_NUMOF_SUB_FUNCTIONS] =
	{
		{SERV19_NUMBER_OF_DTC_BY_STATUS_MASK_E, &GetNumOfFaultsByMask, THREE},
		{SERV19_DTC_BY_STATUS_MASK_E, &GetDTCsByStatusMask, THREE},
		{SERV19_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER_E, &GetDTCSnapshotRecordByDTCNumber, SIX},
		{SERV19_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER_E, &GetExtendedDataRecordByDTCNumber, SIX},
		{SERV19_SUPPORTED_DTC_E, &SupportedDTCs, TWO},
};
/*0x14 - Clear DTC Information*/
const uint32_t GroupOfDTCIDs_au32[NUMBER_OF_GROUP_DTC_IDs] =
	{
		0xFFFFFF};


/******************************************************************************/
/**********************structure of arrays for DIDs****************************/

#if ((TRUE == UDS_SERVICE2E_ENABLE) || (TRUE == UDS_SERVICE22_ENABLE) || (TRUE == UDS_SERVICE2F_ENABLE))



// uint8_t NVM0BlkHdrCrc_au8[NVM_HEADER_SIZE+NVM_CRC_SIZE];





#endif


/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : iso14229_securitycheck
*   Description   : The function checks the security status
*   Parameters    : uint8_t Lvl_u8 
*   Return Value  : bool
*******************************************************************************/ 
uint8_t iso14229_securitycheck(uint16_t Lvl_u16)
{
  bool ServerUnlocked_b = FALSE;
#if (TRUE == UDS_SERVICE27_ENABLE)
  if(NO_SECURITY_EQU == Lvl_u16)
  {
    ServerUnlocked_b =TRUE;
  }
  else if ((iso14229_serv27_securitycheck() & Lvl_u16) > 0)
  {
    ServerUnlocked_b =TRUE;
  }
  else
  {
    ServerUnlocked_b = FALSE;
  }

#else
  return ServerUnlocked_b = FALSE;
  #endif

  return ServerUnlocked_b;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UdsDiagSessionCheck
*   Description   : The function varify the session
*   Parameters    : session_u8 : Session to be varified.
*   Return Value  : true(pass)/false(fail)
*******************************************************************************/ 
bool UdsDiagSessionCheck(uint8_t session_u8)
{
  return DiagSessionCheck((uint16_t)session_u8);
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UDS_SessionTimeout
*   Description   : The function Reset the session
*   Parameters    : None 
*   Return Value  : None
*******************************************************************************/ 
void UDS_SessionTimeout(void)
{
 
  #if (TRUE == UDS_SERVICE10_ENABLE)
  UdsServ10Timeout();
  #endif
  #if (TRUE == UDS_SERVICE27_ENABLE)
  iso14229_serv27_timeout();
  #endif
	#if (TRUE == UDS_SERVICE87_ENABLE)
  iso14229_serv87_timeout();
  #endif
  UDS_UpdateBootloader_SM(SERVICE_IDLE_E);

  if(true == PblParam_St.SBLstate_u8)
  {
    SblPgmData_St.SblErasenum_u16 = 0;
    SblPgmData_St.SblVarifiednum_u16 = 0;
    PblParam_St.SBLstate_u8 = false;
    //ClearRAM();
   UpdateNVMblock(PBL_NVM_BLOCK);
  }


  return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UDS_GetBootloader_SM
*   Description   : The function provide the flashing state
*   Parameters    : None 
*   Return Value  : FlashState_En_t : flashing state
*******************************************************************************/ 
FlashState_En_t UDS_GetBootloader_SM(void)
{
  return Flash_State_En;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UDS_UpdateBootloader_SM
*   Description   : The function updates the flashing state
*   Parameters    : None 
*   Return Value  : None
*******************************************************************************/ 
void UDS_UpdateBootloader_SM(FlashState_En_t Set_FlashState_En)
{
   Flash_State_En = Set_FlashState_En;
   return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UdsAppProc
*   Description   : The function used to check the delay timers
*   Parameters    : None 
*   Return Value  : None
*******************************************************************************/ 
void UdsAppProc(void)
{
  DelayTimerCheck();
  return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ResetSoftwareJumpCnt
*   Description   : The function resets the software jump count
*   Parameters    : None 
*   Return Value  : None
*******************************************************************************/ 
void ResetSoftwareJumpCnt(void)
{
  SblJumpCnt_u32 = 0;
  ApplJumpCnt_u32 = 0;
  EcuResetCnt_u32 = 0;
  return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : ReadPblParameter
 *   Description   : The function updates the PBL flags which are stored in NVM
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
void ReadPblParameter(void)
{
  Nvm_Read(DIAG_SECURITY_STATE_E, (uint8_t*)&PblParam_St.DiagSecurityState_u8);
  Nvm_Read(REPROGRAM_REQ_STATE_E, (uint8_t*)&PblParam_St.ReprogramReqState_u8);
  Nvm_Read(ECU_RESET_E, (uint8_t*)&PblParam_St.EcuReset_u8);
  Nvm_Read(SBL_STATE_E, (uint8_t*)&PblParam_St.SBLstate_u8);
  Nvm_Read(NEW_SBL_CRC_E, (uint8_t*)&PblParam_St.New_SBL_CRC_u32);
  Nvm_Read(NEW_SBL_SIZE_E, (uint8_t*)&PblParam_St.New_SBL_Size_u32);
  Nvm_Read(SBL_UPDATE_FLAG_E, (uint8_t*)&PblParam_St.SBL_Update_Flag_u8);
  Nvm_Read(RANNDUM_NUMBER_E, (uint8_t*)&PblParam_St.RandomNum_u32);
  
	PblParam_St.DiagSecurityState_u8 = 0x00;
	PblParam_St.SBLstate_u8 = 0x00;
  PblParam_St.ReprogramReqState_u8 = 0x00;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : UpdateNVMblock
 *   Description   : The function updates the pbl parameter to NVM block
 *   Parameters    : BlockID_u8 - Which block need to write
 *   Return Value  : None
 *******************************************************************************/
void UpdateNVMblock(uint8_t BlockID_u8)
{

	switch(BlockID_u8)
	{
		case PBL_NVM_BLOCK:
		{
      Nvm_Write(DIAG_SECURITY_STATE_E, (uint8_t*)&PblParam_St.DiagSecurityState_u8);
      Nvm_Write(REPROGRAM_REQ_STATE_E, (uint8_t*)&PblParam_St.ReprogramReqState_u8);
      Nvm_Write(ECU_RESET_E, (uint8_t*)&PblParam_St.EcuReset_u8);
      Nvm_Write(SBL_STATE_E, (uint8_t*)&PblParam_St.SBLstate_u8);
      Nvm_Write(NEW_SBL_CRC_E, (uint8_t*)&PblParam_St.New_SBL_CRC_u32);
      Nvm_Write(NEW_SBL_SIZE_E, (uint8_t*)&PblParam_St.New_SBL_Size_u32);
      Nvm_Write(SBL_UPDATE_FLAG_E, (uint8_t*)&PblParam_St.SBL_Update_Flag_u8);
      Nvm_Write(RANNDUM_NUMBER_E, (uint8_t*)&PblParam_St.RandomNum_u32);
			break;
		}
     case BSW_NVM_BLOCK:
		{
			break;
		}
		default:
		{
			break;
		}
	}
	return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DECLERATION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : UpdateDataBlockStatus
*   Description   : The function update the pbl block
*   Parameters    : None
*   Return Value  : None
*******************************************************************************/
void UpdateDataBlockStatus(void)
{
   
    return;
}


void UDS_Security_Lock (void)
{
  #if (TRUE == UDS_SERVICE27_ENABLE)
  iso14229_serv27_timeout();
  #endif
}

#endif/*UDS_CONF_C*/

/*------------------------------ End of File ---------------------------------*/
