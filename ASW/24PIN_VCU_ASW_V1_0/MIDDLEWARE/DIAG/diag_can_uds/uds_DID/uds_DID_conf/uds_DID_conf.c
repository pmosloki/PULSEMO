/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File         : uds_DID_conf.c
|    Project      : MIL_PBL_CV
|    Description  : Service description for UDS DID service configurations
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

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"uds_DID_conf.h"
#include"fee_conf.h"
#include"nvm_parameter.h"
#include"nvm_conf.h"
#include "uds_DID.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/
uint8_t Sample1_au8[1]= {2};
uint8_t Sample2_au8[1] = {3};
uint8_t Sample3_au8[2] = {8, 9};
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/
const DidConf_st_t DidConf_ast[TOTAL_DID] =
{	
	{ 
		/* DID */
		DID_REPAIR_SHOP_CODE,
		/* DID size */
		REPAIR_SHOP_CODE_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_WRITE_E,
		/* Session to write */
		UDS_SESS_EQU_PROG | UDS_SESS_EQU_EXTENDED,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		SECURITY_EQU_LEVEL_1 | SECURITY_EQU_LEVEL_2,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_PROGRAMMING_DATE,
		/* DID size */
		PROGRAMMING_DATE_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_WRITE_E,
		/* Session to write */
		UDS_SESS_EQU_PROG | UDS_SESS_EQU_EXTENDED,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		SECURITY_EQU_LEVEL_1 | SECURITY_EQU_LEVEL_2,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_REPROGRAMMING_SEQUENCE,
		/* DID size */
		REPROGRAMMING_SEQ_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_EQU_EXTENDED,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		SECURITY_EQU_LEVEL_1,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_ACTIVE_DIAG_SESSION,
		/* DID size */
		ACTIVE_DIAG_SESION_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_WRITE_E,
		/* Session to write */
		UDS_SESS_ALL,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},	
	{ 
		/* DID */
		DID_VIN_NUM,
		/* DID size */
		VIN_NUMBER_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_WRITE_E,
		/* Session to write */
		UDS_SESS_EQU_PROG | UDS_SESS_EQU_EXTENDED,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		SECURITY_EQU_LEVEL_1 | SECURITY_EQU_LEVEL_2,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_BAUDRATE_DATA_CODE,    
		/* DID size */
		BAUDRATE_DATA_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_WRITE_E,
		/* Session to write */
		UDS_SESS_EQU_EXTENDED,          
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		SECURITY_EQU_LEVEL_1,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{
		/* DID */
		DID_APPLICATION_SOFTWARE_VERSION,
		/* DID size */
		APPLICATION_SOFTWARE_VERSION_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		(uint8_t*)(ASW_Release_Ver_u8),
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_EQU_PROG,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{
		/* DID */
		DID_CUSTOMER_NAME,
		/* DID size */
		CUSTOMER_NAME_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		(uint8_t*)(CustomerName_u8),
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_EQU_PROG,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{
		/* DID */
		DID_INTERNAL_ASW_VERSION,
		/* DID size */
		APPLICATION_SOFTWARE_VERSION_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		(uint8_t*)(Internal_ASW_Release_Ver_u8),
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_EQU_PROG,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_ECU_MANUFACTURING_DATE,
		/* DID size */
		ECU_MANUFACTURING_DATE_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_SUPPLIER_ECU_SERIAL_NUMBER,
		/* DID size */
		SUPPLIER_ECU_SERIAL_NUMBER_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_ECU_HARDWARE_NUMBER,
		/* DID size */
		ECU_HARDWARE_NUMBER_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_SUPPLIER_ECU_PART_NUMBER,
		/* DID size */
		SUPPLIER_ECU_PART_NUMBER,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		DID_BSW_IDENTIFICATION_NUMBER,
		/* DID size */
		BSW_IDENTIFICATION_NUMBER_SIZE,
		/* DID storage location */
		FLASH_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		READ_ONLY_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_ALL,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		NULL
	},
	{ 
		/* DID */
		GLOBAL_FF_DID_1,
		/* DID size */
		SAMPLE_DID_DATA_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		FREEZE_FRAME_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_EQU_DEFAULT | UDS_SESS_EQU_EXTENDED,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		{(uint8_t *)(&Sample1_au8),  1, NULL}
	},
	{ 
		/* DID */
		GLOBAL_FF_DID_2,
		/* DID size */
		SAMPLE_DID_DATA_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		FREEZE_FRAME_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_EQU_DEFAULT | UDS_SESS_EQU_EXTENDED,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		{(uint8_t *)(&Sample2_au8),  1, NULL}
	},
	{ 
		/* DID */
		SAMPLE_DID_3,
		/* DID size */
		SAMPLE_DID_DATA_SIZE,
		/* DID storage location */
		EEPROM_STORAGE_E,
		/* pointer to RAM/ROM Copy */
		NULL,
		/* Access mode */
		FREEZE_FRAME_E,
		/* Session to write */
		UDS_SESS_NON,
		/* Session to Read */
		UDS_SESS_EQU_DEFAULT | UDS_SESS_EQU_EXTENDED,
		/* Security to write */
		NO_SECURITY_EQU,
		/* Security to read*/
		NO_SECURITY_EQU,
		/* function pointer*/
		NULL,
		/* Freeze Frame struct*/
		{(uint8_t *)(&Sample3_au8),  2, NULL}
	},
};


const DidNvmIf_St_t DidNvmIf_aSt[TOTAL_DID] = 
{
    { NULL, 											DID_REPAIR_SHOP_CODE	    			,(uint16_t)NVM_BLK_ID_1_E, 	(uint16_t)REPAIR_SHOP_CODE_E		  },
    { NULL, 											DID_PROGRAMMING_DATE	    			,(uint16_t)NVM_BLK_ID_1_E, 	(uint16_t)PROGRAMMING_DATE_E		  },
    { NULL, 											DID_REPROGRAMMING_SEQUENCE				,(uint16_t)NVM_BLK_ID_1_E, 	(uint16_t)ASW_REPROGRAM_SEQ_E		  },
    { NULL, 											DID_ACTIVE_DIAG_SESSION					,(uint16_t)NVM_BLK_ID_1_E, 	(uint16_t)ACTIVE_DIAG_SESSION_E		  },
	{ NULL, 											DID_VIN_NUM								,(uint16_t)NVM_BLK_ID_2_E, 	(uint16_t)VIN_NUMBER_E      		  },
	{ NULL, 											DID_BAUDRATE_DATA_CODE					,(uint16_t)NVM_BLK_ID_2_E, 	(uint16_t)BAUDRATE_DATA_E			  },
	{ ((uint8_t*)ECU_ASW_NUMBER_ADDRESS), 				DID_APPLICATION_SOFTWARE_VERSION	    ,DUMMY_BLOCK_E , 				DUMMY_PARAMETER_E				  },
	{ ((uint8_t*)ECU_CUSTOMER_NAME_ADDRESS), 			DID_CUSTOMER_NAME	   					,DUMMY_BLOCK_E , 				DUMMY_PARAMETER_E				  },
	{ ((uint8_t*)ECU_INTERNAL_ASW_NUMBER_ADDRESS), 		DID_INTERNAL_ASW_VERSION        	    ,DUMMY_BLOCK_E , 				DUMMY_PARAMETER_E				  },
	{ ((uint8_t*)FLASH_DID_START_ADDRESS), 				DID_ECU_MANUFACTURING_DATE			    ,DUMMY_BLOCK_E , 	            DUMMY_PARAMETER_E				  },
    { ((uint8_t*)SUPPLIER_ECU_SERIAL_NUMBER_ADDRESS), 	DID_SUPPLIER_ECU_SERIAL_NUMBER		    ,DUMMY_BLOCK_E , 	            DUMMY_PARAMETER_E  				  },
    { ((uint8_t*)ECU_HARDWARE_NUMBER_ADDRESS),			DID_ECU_HARDWARE_NUMBER	 			    ,DUMMY_BLOCK_E , 	            DUMMY_PARAMETER_E				  },
    { ((uint8_t*)SUPPLIER_ECU_PART_NUMBER_ADDRESS), 	DID_SUPPLIER_ECU_PART_NUMBER			,DUMMY_BLOCK_E , 	            DUMMY_PARAMETER_E				  },
    { ((uint8_t*)BSW_IDENTIFICATION_NUMBER_ADDRESS), 	DID_BSW_IDENTIFICATION_NUMBER		    ,DUMMY_BLOCK_E , 	            DUMMY_PARAMETER_E				  },
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