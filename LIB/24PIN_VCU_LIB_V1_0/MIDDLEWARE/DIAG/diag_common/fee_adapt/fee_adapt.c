/******************************************************************************
 *    FILENAME    : fee_adapt.c
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki       10/01/2019		   Initial version
 ******************************************************************************
*/ 

#include "fee_adapt.h"
#include "fee_conf.h"
#include "diag_adapt.h"
#include "diag_appl_test.h"
#include "diag_sys_conf.h"
#if(TRUE == DIAG_CONF_EEPROM)
	#include "eeprom.h"//include the eeprom related files
#endif

#if(TRUE == DIAG_CONF_FM_SUPPORTED)
	U2 u2_DTC_Entry_Size = 0;
#endif

//void WriteFlash_Adapt(uint32_t flashAddr,  uint32_t sourceData)
//{
//
//}

//bool WRITE_STATUS_Adapt()
//{
//    return FALSE;
//}

#if(TRUE == DIAG_CONF_FM_SUPPORTED)
/**
*    @brief  Read data from Flash to buffer
*
*    @param  Message name
*    @param  Pointer to buffer
*    @return None
*/
void FEE_Read(uint8_t EntryName_u8, uint8_t* buf_pu8)
{
	// U4 u4_dWordData;
	U2 u2_loop_index;
	
	for(u2_loop_index = 0; u2_loop_index < TOTAL_FM_SIGNAL_E; u2_loop_index++)
	{
		//u4_dWordData = Bkupram_u4_Read32(FEE_AddrConfig_aSt[EntryName_u8].StartAddress_u32 + u2_loop_index);
#if(TRUE == DIAG_TEST_FM_EEPROM_DEMO)
        u4_dWordData = diag_appl_test_fm_read(FEE_AddrConfig_aSt[EntryName_u8].StartAddress_u32 + u2_loop_index);

#endif
#if(TRUE == DIAG_CONF_EEPROM)
        u4_dWordData = FM_EEE_Read(EntryName_u8,u2_loop_index);
#endif	
		if(EntryName_u8 == FEE_FM_Config_aSt[u2_loop_index].Entry_Name_u16)
		{
			Nvm_Read(FEE_FM_Config_aSt[u2_loop_index].FEE_Nvm_Param_u16,buf_pu8);
		}
	}
}

/**
*    @brief  Write data from Flash to buffer
*
*    @param  Message name
*    @param  Pointer to buffer
*    @return None
*/
void FEE_Write(uint8_t EntryName_u8, uint8_t* buf_pu8)
{
	U2 u2_loop_index;
	// U4 u4_dWordData;
	
	for(u2_loop_index = 0; u2_loop_index < TOTAL_FM_SIGNAL_E; u2_loop_index++)
	{
		if(EntryName_u8 == FEE_FM_Config_aSt[u2_loop_index].Entry_Name_u16)
		{
			 Nvm_Write(FEE_FM_Config_aSt[u2_loop_index].FEE_Nvm_Param_u16,buf_pu8);
		}
		
		//(void)Bkupram_en_Write32(FEE_AddrConfig_aSt[EntryName_u8].StartAddress_u32 + u2_loop_index, u4_dWordData);
#if(TRUE == DIAG_TEST_FM_EEPROM_DEMO)
  		diag_appl_test_fm_write(FEE_AddrConfig_aSt[EntryName_u8].StartAddress_u32 + u2_loop_index, u4_dWordData);
#endif
#if(TRUE == DIAG_CONF_EEPROM)
  		FM_EEE_Write(EntryName_u8, u4_dWordData,u2_loop_index);
#endif
	}
	/* FM_NVM_Entry_Write(); */
}
#endif
#if(TRUE == DIAG_BOOT_LOADER_SUPPORTED)
	// bool Flash_Pattern_Write(uint32_t address,uint32_t data)
	// {
	// 	PFlash_Pattern_Write(address, data);
	// }
#endif

/* End of File */
