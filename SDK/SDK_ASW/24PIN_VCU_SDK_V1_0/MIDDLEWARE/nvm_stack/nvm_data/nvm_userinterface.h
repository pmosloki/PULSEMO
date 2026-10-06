/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : nvm_userinterface.h
|    Project        : 
|    Description    : 
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                      Company
| --------     ---------------------     ---------------------------------------
| 20/04/2021     Name           Sloki Software Technologies LLP.
|-------------------------------------------------------------------------------
|******************************************************************************/
#ifndef NVM_USERINTERFACE_H
#define NVM_USERINTERFACE_H
/*******************************************************************************
 *  HEARDER FILE INCLUDES
 ******************************************************************************/
#include "sys_typedefs.h"
#include "app_typedef.h"
#include "nvm_parameter.h"
#include "nvm_conf.h"
#include "r_fdl_types.h"
/*******************************************************************************
 *  MACRO DEFNITION
 ******************************************************************************/


/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/
typedef enum NVM_STATUS_T
{
    NVM_STATUS_OK                    = 0,        /**< Operation terminated successfully */
    NVM_STATUS_BUSY                  = 1,        /**< Operation is still ongoing */
    NVM_STATUS_SUSPENDED             = 2,        /**< Flash operation is suspended */
    NVM_STATUS_ERR_CONFIGURATION     = 3,        /**< The FDL configuration (descriptor) was wrong */
    NVM_STATUS_ERR_PARAMETER         = 4,        /**< A parameter of the FDL function call was wrong */
    NVM_STATUS_ERR_PROTECTION        = 5,        /**< Operation blocked due to wrong parameters */
    NVM_STATUS_ERR_REJECTED          = 6,        /**< Flow error, e.g. another operation is still busy */
    NVM_STATUS_ERR_WRITE             = 7,        /**< Flash write error */
    NVM_STATUS_ERR_ERASE             = 8,        /**< Flash erase error */
    NVM_STATUS_ERR_BLANKCHECK        = 9,        /**< Flash blank check error */
    NVM_STATUS_ERR_COMMAND           = 10,       /**< Unknown command */
    NVM_STATUS_ERR_ECC_SED           = 11,       /**< Single bit error detected by ECC */
    NVM_STATUS_ERR_ECC_DED           = 12,       /**< Double bit error detected by ECC */
    NVM_STATUS_ERR_INTERNAL          = 13,       /**< Library internal error */
    NVM_STATUS_CANCELLED             = 14        /**< Flash operation is cancelled */
}nvm_status_En_t;

/*******************************************************************************
 *  EXTERN GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *  EXTERN FUNCTION
 ******************************************************************************/
extern nvm_status_En_t NvmWriteBlock(uint32_t block_id_u32, uint8_t *buffer_pu8, uint32_t size_u32);
extern uint8_t NvmReadBlock(uint32_t block_id_u32, uint8_t *buffer_pu8, uint32_t size_u32);
#endif
/*---------------------- End of File -----------------------------------------*/

