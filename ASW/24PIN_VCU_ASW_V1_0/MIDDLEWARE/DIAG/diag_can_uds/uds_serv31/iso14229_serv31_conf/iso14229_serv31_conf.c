/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv31_conf.c
|    Project      	: MIL_PBL_CV
|    Description    : configuration description for UDS service - Routine Control
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
#include "iso14229_serv31_conf.h"
// #include "uds_routine.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/
RIDConf_St_t RIDConf_aSt[TOTAL_RID] =
{
  {CheckProgDependency,   RID_CHECK_PROGRAMMING_DEPENDENCY,     RID_START_CHECK_PROG_DEPENDENCY_LEN,  RID_STOP_CHECK_PROG_DEPENDENCY_LEN, RID_RESULT_CHECK_PROG_DEPENDENCY_LEN, RID_CHECK_PROG_DEPENDENCY_LEN_RESP_LEN,   ROUTINE_IDLE_E, UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA,    NO_SECURITY_EQU, ZERO,  RID_START_CHECK_PROG_DEPENDENCY_RESP_OPTBYTE_LEN,RID_STOP_CHECK_PROG_DEPENDENCY_RESP_OPTBYTE_LEN,RID_RESULT_CHECK_PROG_DEPENDENCY_RESP_OPTBYTE_LEN,false,false,RC_PASS_E},
  {ReprogramPreCondition, RID_CHECK_REPROGRAMMING_PRECONDITION, RID_START_CHECK_REPROG_PRECOND_LEN,   RID_STOP_CHECK_REPROG_PRECOND_LEN,  RID_RESULT_CHECK_REPROG_PRECOND_LEN,  RID_CHECK_REPROG_PRECOND_LEN_RESP_LEN,    ROUTINE_IDLE_E, UDS_SESS_EQU_EXTENDED,                    NO_SECURITY_EQU, ZERO,  RID_START_CHECK_REPROG_PRECOND_RESP_OPTBYTE_LEN, RID_STOP_CHECK_REPROG_PRECOND_RESP_OPTBYTE_LEN, RID_RESULT_CHECK_REPROG_PRECOND_RESP_OPTBYTE_LEN, false,false,RC_PASS_E},
  {WriteDefaultConfigData,   RID_DEFAULT_CONFIG_DATA,   RID_START_DEFAULT_CONFIG_DATA_LEN, RID_STOP_DEFAULT_CONFIG_DATA_LEN,                 RID_RESULT_DEFAULT_CONFIG_DATA_LEN,             RID_DEFAULT_CONFIG_DATA_LEN_RESP_LEN,                 ROUTINE_IDLE_E   ,UDS_SESS_EQU_EXTENDED| UDS_SESS_EQU_PROG |UDS_SESS_EQU_FOTA   ,SECURITY_EQU_LEVEL_1|SECURITY_EQU_LEVEL_2, ZERO,RID_START_DEFAULT_CONFIG_DATA_RESP_OPTBYTE_LEN,RID_STOP_DEFAULT_CONFIG_DATA_RESP_OPTBYTE_LEN,RID_RESULT_DEFAULT_CONFIG_DATA_RESP_OPTBYTE_LEN,true,false,RC_PASS_E},
  {EnterBootMode,   RID_ENTER_BOOTMODE_DATA,   RID_START_ENTER_BOOTMODE_LEN, RID_STOP_ENTER_BOOTMODE_LEN,  RID_RESULT_ENTER_BOOTMODE_LEN, RID_ENTER_BOOTMODE_LEN_RESP_LEN, ROUTINE_IDLE_E   , UDS_SESS_EQU_PROG |UDS_SESS_EQU_FOTA   ,SECURITY_EQU_LEVEL_1|SECURITY_EQU_LEVEL_2, {ZERO},RID_START_ENTER_BOOTMODE_RESP_OPTBYTE_LEN,RID_STOP_ENTER_BOOTMODE_RESP_OPTBYTE_LEN,RID_RESULT_ENTER_BOOTMODE_RESP_OPTBYTE_LEN,true,false,RC_PASS_E},
  // {UpdateServiceResetInfo, RID_SERVICE_RMD_RESET,   RID_START_SERVICE_RMD_LEN,  RID_STOP_SERVICE_RMD_LEN, RID_RESULT_SERVICE_RMD_LEN, RID_SERVICE_RMD_LEN_RESP_LEN   ,ROUTINE_IDLE_E   ,UDS_SESS_EQU_EXTENDED|UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA    ,SECURITY_EQU_LEVEL_1|SECURITY_EQU_LEVEL_2, ZERO,RID_START_SERVICE_RMD_RESP_OPTBYTE_LEN,RID_STOP_SERVICE_RMD_RESP_OPTBYTE_LEN,RID_RESULT_SERVICE_RMD_RESP_OPTBYTE_LEN,false,false,RC_PASS_E},
//   {EraseRoutine,                RID_ERASE,                          RID_START_ERASE_LEN,      RID_STOP_ERASE_LEN,                   RID_RESULT_ERASE_LEN,               RID_ERASE_LEN_RESP_LEN,               ROUTINE_IDLE_E   ,UDS_SESS_EQU_PROG  | UDS_SESS_EQU_FOTA  ,SECURITY_EQU_LEVEL_1|SECURITY_EQU_LEVEL_2, ZERO,RID_START_ERASE_LEN_RESP_OPTBYTE_LEN,RID_STOP_ERASE_LEN_RESP_OPTBYTE_LEN,RID_RESULT_ERASE_LEN_RESP_OPTBYTE_LEN,false,false,RC_PASS_E},
//   {CalculateCheckSum,   RID_CHECKSUM,                       RID_START_CHEKSUM_LEN, RID_STOP_CHEKSUM_LEN,                 RID_RESULT_CHEKSUM_LEN,             RID_CHEKSUM_RESP_LEN,                 ROUTINE_IDLE_E   ,UDS_SESS_EQU_PROG | UDS_SESS_EQU_FOTA   ,SECURITY_EQU_LEVEL_1|SECURITY_EQU_LEVEL_2, ZERO,RID_START_CHECKSUM_RESP_OPTBYTE_LEN,RID_STOP_CHECKSUM_RESP_OPTBYTE_LEN,RID_RESULT_CHECKSUM_RESP_OPTBYTE_LEN,false,false,RC_PASS_E},
// {ExternalRollback,   RID_EXTERNAL_ROLLBACK,   RID_START_EXTERNAL_ROLLBACK_LEN, RID_STOP_EXTERNAL_ROLLBACK_LEN,                 RID_RESULT_EXTERNAL_ROLLBACK_LEN,             RID_EXTERNAL_ROLLBACK_LEN_RESP_LEN,                 ROUTINE_IDLE_E   ,UDS_SESS_EQU_PROG  | UDS_SESS_EQU_FOTA  ,SECURITY_EQU_LEVEL_1|SECURITY_EQU_LEVEL_2, ZERO,RID_START_EXTERNAL_ROLLBACK_OPTBYTE_LEN,RID_STOP_EXTERNAL_ROLLBACK_OPTBYTE_LEN,RID_RESULT_EXTERNAL_ROLLBACK_OPTBYTE_LEN,false,false,RC_PASS_E},
 
};
/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/

/*---------------------- End of File -----------------------------------------*/