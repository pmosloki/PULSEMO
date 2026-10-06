/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : nvm_conf.c
|    Project        : EMBDES_GSHIFTER
|    Description    : The file implements the board initialization.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 17/07/2022       Jeevan Jestin N             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"nvm_conf.h"

/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/
/*LDRA_EXCLUDE_START 397 S 
    <justification start> 
    ROM values are default to 0x00
    <justification end>
*/
const uint8_t Rom_Pbl_Param_parameter_au8[PBL_PARAM_NVM_BLOCK_SIZE] = {0};
const uint8_t Rom_Bootloader_DID_parameter_au8[BOOTLOADER_PARAM_NVM_BLOCK_SIZE] = {0};
const uint8_t Rom_ASW_DID_parameter_au8[ASW_DID_BLOCK_SIZE] = {0u};
const uint8_t Rom_FM_ConfigData_au8[FM_CONFIG_NVM_BLOCK_SIZE] = {0u};
const uint8_t Rom_FM_Entry_au8[FM_ENTRY_BLK_SIZE] = {0u};
/*LDRA_EXCLUDE_END 397 S */

/* RAM copy of the NVM block */
uint8_t Pbl_Param_parameter_au8[PBL_PARAM_NVM_BLOCK_SIZE];
uint8_t Bootloader_DID_parameter_au8[BOOTLOADER_PARAM_NVM_BLOCK_SIZE];
uint8_t ASW_DID_parameter_au8[ASW_DID_BLOCK_SIZE];
uint8_t FM_ConfigData_au8[FM_CONFIG_NVM_BLOCK_SIZE];
uint8_t FM_Entry_au8[FM_ENTRY_BLK_SIZE];
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/
Blk_Descriptor_St_t Pbl_Param_parameter_aSt[TOTAL_PBL_PARAM_PARAMETER] = 
{
    {DIAG_SECURITY_STATE_E          ,DIAG_SECURITY_STATE_OFFSET         ,DIAG_SECURITY_STATE_SIZE   }, 
    {REPROGRAM_REQ_STATE_E          ,REPROGRAM_REQ_STATE_OFFSET         ,REPROGRAM_REQ_STATE_SIZE   }, 
    {ECU_RESET_E                    ,ECU_RESET_OFFSET                   ,ECU_RESET_SIZE   }, 
    {SBL_STATE_E                    ,SBL_STATE_OFFSET                   ,SBL_STATE_SIZE   }, 
    {NEW_SBL_CRC_E                  ,NEW_SBL_SIZE_OFFSET                ,NEW_SBL_SIZE_SIZE   }, 
    {NEW_SBL_SIZE_E                 ,NEW_SBL_CRC_OFFSET                 ,NEW_SBL_CRC_SIZE   }, 
    {SBL_UPDATE_FLAG_E              ,SBL_UPDATE_FLAG_OFFSET             ,SBL_UPDATE_FLAG_SIZE   }, 
    {RANNDUM_NUMBER_E               ,RANNDUM_NUMBER_OFFSET              ,RANNDUM_NUMBER_SIZE   }, 
};

Blk_Descriptor_St_t Bootloader_DID_parameter_aSt[TOTAL_BOOTLOADER_DID_BLK_PARAMETER] = 
{

    {REPAIR_SHOP_CODE_E             ,REPAIR_SHOP_CODE_OFFSET            ,REPAIR_SHOP_CODE_SIZE    }, 
    {PROGRAMMING_DATE_E             ,PROGRAMMING_DATE_OFFSET            ,PROGRAMMING_DATE_SIZE    }, 
    {ASW_REPROGRAM_SEQ_E            ,REPROGRAMMING_SEQ_OFFSET           ,REPROGRAMMING_SEQ_SIZE   }, 
    {ACTIVE_DIAG_SESSION_E          ,ACTIVE_DIAG_SESION_OFFSET          ,ACTIVE_DIAG_SESION_SIZE  }, 
};

Blk_Descriptor_St_t ASW_DID_BlkData_aSt[TOTAL_ASW_DID_PARAMETER] = 
{
    {VIN_NUMBER_E                   ,VIN_NUMBER_OFFSET                    ,VIN_NUMBER_SIZE          },
    {BAUDRATE_DATA_E                ,BAUDRATE_DATA_OFFSET                 ,BAUDRATE_DATA_SIZE       },  
};

Blk_Descriptor_St_t FM_ConfigData_aSt[TOTAL_FM_CONFIG_BLK_PARAMETER] = 
{
    {FEE_FM_COMMON_DATA_E, FM_COMMON_DATA_OFFSET, FM_COMMON_DATA_SIZE},
    {FEE_FM_RDYRESULTS_E , FM_RDYRESULTS_OFFSET,  FM_RDYRESULTS_SIZE},
    {FEE_FM_TFSLC_E      , FM_TFSLC_OFFSET,       FM_TFSLC_SIZE},
};

Blk_Descriptor_St_t FM_Entry_aSt[TOTAL_FM_ENTRY_BLK_PARAMETER] =
{
    {FEE_FM_L2_ENTRY1_E , FEE_FM_L2_ENTRY1_OFFSET  ,FEE_FM_L2_ENTRY1_SIZE},
    {FEE_FM_L2_ENTRY2_E , FEE_FM_L2_ENTRY2_OFFSET  ,FEE_FM_L2_ENTRY2_SIZE},
    {FEE_FM_L2_ENTRY3_E , FEE_FM_L2_ENTRY3_OFFSET  ,FEE_FM_L2_ENTRY3_SIZE},
    {FEE_FM_L2_ENTRY4_E , FEE_FM_L2_ENTRY4_OFFSET  ,FEE_FM_L2_ENTRY4_SIZE},
    {FEE_FM_L2_ENTRY5_E , FEE_FM_L2_ENTRY5_OFFSET  ,FEE_FM_L2_ENTRY5_SIZE},
    {FEE_FM_L2_ENTRY6_E , FEE_FM_L2_ENTRY6_OFFSET  ,FEE_FM_L2_ENTRY6_SIZE},
    {FEE_FM_L2_ENTRY7_E , FEE_FM_L2_ENTRY7_OFFSET  ,FEE_FM_L2_ENTRY7_SIZE},
    {FEE_FM_L2_ENTRY8_E , FEE_FM_L2_ENTRY8_OFFSET  ,FEE_FM_L2_ENTRY8_SIZE},
    {FEE_FM_L2_ENTRY9_E , FEE_FM_L2_ENTRY9_OFFSET  ,FEE_FM_L2_ENTRY9_SIZE},
    {FEE_FM_L2_ENTRY10_E, FEE_FM_L2_ENTRY10_OFFSET ,FEE_FM_L2_ENTRY10_SIZE},
};

const NvmBlk_St_t NvmBlk_aSt[TOTAL_NVM_BLK_E] = 
{
    /*Block ID */        /* Start Address */                                        /* Block Length */                           /*RAM Copy pointer*/                    /*ROM Copy Pointer*/         /* Block Descriptor*/                     /*Total Parameter */              /* Direct NVM write*/
    {NVM_BLK_ID_0_E , PBL_PARAM_BLK_START_ADDR,                                 sizeof(Pbl_Param_parameter_au8)            , Pbl_Param_parameter_au8               ,Rom_Pbl_Param_parameter_au8      , Pbl_Param_parameter_aSt               ,TOTAL_PBL_PARAM_PARAMETER              ,false},
    {NVM_BLK_ID_1_E , BOOTLOADER_DID_BLK_START_ADDR,                            sizeof(Bootloader_DID_parameter_au8)       , Bootloader_DID_parameter_au8          ,Rom_Bootloader_DID_parameter_au8 , Bootloader_DID_parameter_aSt          ,TOTAL_BOOTLOADER_DID_BLK_PARAMETER     ,false},
    {NVM_BLK_ID_2_E , ASW_DID_BLK_START_ADDR,                                   sizeof(ASW_DID_parameter_au8)              , ASW_DID_parameter_au8                 ,Rom_ASW_DID_parameter_au8        , ASW_DID_BlkData_aSt                   ,TOTAL_ASW_DID_PARAMETER                ,false},
    {NVM_BLK_ID_3_E , FM_CONFIG_BLK_START_ADDR,                                 sizeof(FM_ConfigData_au8)                  , FM_ConfigData_au8                     ,Rom_FM_ConfigData_au8            , FM_ConfigData_aSt                     ,TOTAL_FM_CONFIG_BLK_PARAMETER          ,false},
    {NVM_BLK_ID_4_E , FM_ENTRY_BLK_START_ADDR,                                  sizeof(FM_Entry_au8)                       , FM_Entry_au8                          ,Rom_FM_Entry_au8                 , FM_Entry_aSt                          ,TOTAL_FM_ENTRY_BLK_PARAMETER           ,false},
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

