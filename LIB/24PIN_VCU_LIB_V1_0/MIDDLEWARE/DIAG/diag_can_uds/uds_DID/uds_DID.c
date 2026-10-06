/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File	        : uds_DID.c
|    Project	    : MIL_PBL_CV
|    Description    : Service description for UDS service -
|					  READ DATA BY IDENTIFIER(0x22) and for
|	        		  Write Data By Identifier(0x2E).
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date     	     Name                      Company
| --------     ---------------------     ---------------------------------------
| 31/07/2024       Manikandan S            Sloki Software Technologies LLP.
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "uds_DID.h"
#include "diag_typedefs.h"
#include "diag_adapt.h"
#include "diag_sys_conf.h"
#if (TRUE == DIAG_CONF_FM_SUPPORTED)
#include "fmdtc_conf.h"
#include "fm.h"
#endif
#include "fee_adapt.h"
#include "nvm_data.h"
#include "nvm_conf.h"
#include "nvm_parameter.h"
/*******************************************************************************
 *  macros and #defines
 ******************************************************************************/
#define NVM_NOPE (0) /* No operation on NVM, termination/closing of \
						previous process 							  */
#define READ_NVM (1)
#define WRITE_NVM (2)

/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
uint8_t DataRecord[10] = {0};
/*******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/* -----------------------------------------------------------------------------
 *  FUNCTION DECLERATION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : ServiceTheDid
 *   Description   : The function service the DID request
 *   Parameters    : Data pointer,Did table index, access type
 *   Return Value  : true/false  pass/fail
 *******************************************************************************/
uint8_t DidRead(DidConf_st_t *ReadDidConf_pst, uint8_t *Buff_pu8)
{
	uint8_t Status_u8 = false;
	uint16_t LoopCnt_u16 = 0;

	for (LoopCnt_u16 = 0; LoopCnt_u16 < TOTAL_DID; LoopCnt_u16++)
	{
		if (DidNvmIf_aSt[LoopCnt_u16].DID_u16 == ReadDidConf_pst->DID_u16)
		{
			if(ReadDidConf_pst->MemoryType_u8 == EEPROM_STORAGE_E)
			{
				Nvm_Read(DidNvmIf_aSt[LoopCnt_u16].NvmParameter_u16, Buff_pu8);
			}
			else
			{
				MemCopy(Buff_pu8, (uint8_t *)(DidNvmIf_aSt[LoopCnt_u16].InternalFlashAddress_pu8), ReadDidConf_pst->DidLen_u16);
			}
			break;
		}
	}
	return Status_u8;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DECLERATION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : ServiceTheDid
 *   Description   : The function service the DID request
 *   Parameters    : Data pointer,Did table index, access type
 *   Return Value  : true/false  pass/fail
 *******************************************************************************/
uint8_t DidWrite(DidConf_st_t *WriteDidConf_pst, uint8_t *Buff_pu8)
{
	uint8_t Status_u8 = false;
	uint16_t LoopCnt_u16 = 0;

	for (LoopCnt_u16 = 0; LoopCnt_u16 < TOTAL_DID; LoopCnt_u16++)
	{
		if (DidNvmIf_aSt[LoopCnt_u16].DID_u16 == WriteDidConf_pst->DID_u16)
		{
			if(WriteDidConf_pst->MemoryType_u8 == EEPROM_STORAGE_E)
			{
				Nvm_Write(DidNvmIf_aSt[LoopCnt_u16].NvmParameter_u16, Buff_pu8);
				Status_u8 = true;
				break;
			}
		}
	}

	return Status_u8;
}


/*---------------------- End of File -----------------------------------------*/

