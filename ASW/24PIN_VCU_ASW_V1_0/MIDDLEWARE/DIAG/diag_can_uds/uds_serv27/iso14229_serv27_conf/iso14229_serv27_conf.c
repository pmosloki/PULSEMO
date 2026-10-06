/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv27_conf.c
|    Project      	: MIL_PBL_CV
|    Description    : configuration description for UDS service - Security
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
#include "iso14229_serv27_conf.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/
#define TOTAL_DELAY_TIMER       1
#define MAX_ATTEMPT       3
/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/
static uint32_t DelayTimer1_au32[TOTAL_DELAY_TIMER] = 
{
    10000u
};
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/

/* LDRA_EXCLUDE_START 27 D */
/*LDRA_INSPECTED 1 X */
/* LDRA_EXCLUDE_START 104 S */
/* LDRA_EXCLUDE_START 331 S */
const UdsSecurityAccess_St_t UdsSecurityAccess_aSt[TOTAL_SECURITY_LVL] = 
{
    {DelayTimer1_au32,(UDS_SESS_EQU_PROG|UDS_SESS_EQU_EXTENDED), SECURITY_EQU_LEVEL_1, SECURITY_LEVEL_1,(uint8_t) SEED_LEN,(uint8_t) KEY_LEN,(uint8_t)true,TOTAL_DELAY_TIMER,MAX_ATTEMPT},
  
#if(TRUE == FOTA_ENABLED)
    {DelayTimer1_au32,(UDS_SESS_EQU_EXTENDED|UDS_SESS_EQU_FOTA), SECURITY_EQU_LEVEL_2, SECURITY_LEVEL_2,(uint8_t) SEED_LEN,(uint8_t) KEY_LEN,(uint8_t)true,TOTAL_DELAY_TIMER,MAX_ATTEMPT}
#elif(FALSE == FOTA_ENABLED)
    {DelayTimer1_au32,(UDS_SESS_EQU_EXTENDED), SECURITY_EQU_LEVEL_2, SECURITY_LEVEL_2,(uint8_t) SEED_LEN,(uint8_t) KEY_LEN,(uint8_t)true,TOTAL_DELAY_TIMER,MAX_ATTEMPT}
#endif
};
/* LDRA_EXCLUDE_END 27 D */
/* LDRA_EXCLUDE_END 104 S */
/* LDRA_EXCLUDE_END 331 S */

 const RandomNumGenerator_Fptr_t RandomNumGenerator_Fptr = GenerateSeedCallback;
 const KeyGenerator_Fptr_t KeyGenerator_Fptr = GenerateKeyCallback;
 /* LDRA_EXCLUDE_START 27 D */
 const UdsServInitCallback_Fptr_t UdsServ27InitCallback_Fptr = Uds27ServInitCallback;
 
/* LDRA_EXCLUDE_END 27 D */
/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/


/*---------------------- End of File -----------------------------------------*/