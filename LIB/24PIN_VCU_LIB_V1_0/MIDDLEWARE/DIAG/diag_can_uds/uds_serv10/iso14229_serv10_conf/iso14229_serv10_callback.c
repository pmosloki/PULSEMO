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
#include "iso14229_serv10.h"
#include "iso14229_serv10_conf.h"
#include "uds_conf.h"
#include "nvm_data.h"
#include "nvm_conf.h"
#include "fee_adapt.h"
#include "uds_DID.h"
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
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint8_t DiagDefaultSessCbk(void)
{
   UDS_SessionTimeout();
    
    return 0;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint8_t DiagProgrammingSessCbk(void)
{
    uint8_t DiagSession_u8 = 0u;
    DidConf_st_t Did_St;
    BSW_NVM_Data_St.Software_Update_Mode_FOTA_Or_Tester = TESTER_PROGRAMMING_E;
    DiagSession_u8 = DIAG_SESS_TYPE_PROGRAMMING;
    Did_St.DID_u16 = DID_ACTIVE_DIAG_SESSION;
	Did_St.DidLen_u16 = ACTIVE_DIAG_SESION_SIZE;
    Did_St.MemoryType_u8 = EEPROM_STORAGE_E;
	(void)DidWrite(&Did_St, &DiagSession_u8);
    return 0;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint8_t DiagExtSessCbk(void)
{
    return 0;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint8_t DiagFotaSessCbk(void)
{
    BSW_NVM_Data_St.Software_Update_Mode_FOTA_Or_Tester = FOTA_PROGRAMMING_E;
     PblParam_St.ReprogramReqState_u8 = true;
    Nvm_Shutdown();
    UdsServ10RespPending_u8 = true;
    HardReset_b = true;
    return 0;
}
/*---------------------- End of File -----------------------------------------*/