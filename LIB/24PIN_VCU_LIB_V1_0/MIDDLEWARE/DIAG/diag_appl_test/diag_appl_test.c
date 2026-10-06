/******************************************************************************
 *    FILENAME    : diag_appl_test.c
 *    DESCRIPTION : Application test interface file for DIAG Stacks.
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki       10/01/2019		   Initial version
 ******************************************************************************
*/ 


#include "diag_appl_test.h"
//#include "eeprom.h"
#include "diag_adapt.h" 
#include "can_if.h"
#include "can_sched_conf.h"
#include "diag_sys_conf.h"
#include "cil_can_conf.h"
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
	#include "fm.h"
	#include "fm_level2.h"
#endif
#if(TRUE == DIAG_CONF_UDS_SUPPORTED)
	#include "uds_conf.h"
#endif
//#include "FLM_User.h"
//#include "Task_Scheduler.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */



/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

//static int16_t ECT_Deg_s16 = 0;    //variable which holds the Engine starting coolent temperature    

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
#if (TRUE == DIAG_TEST_J1939_DEMO)

void FM_GetDemo_DTCs(uint32_t * prevActiveFaults_aUni, uint8_t NumOfPrevActiveFaults_u8)
{
	uint8_t DTC_Index = 0;
	for(DTC_Index = 0; DTC_Index< NumOfPrevActiveFaults_u8; DTC_Index++)
	{
		*(prevActiveFaults_aUni + DTC_Index) = J1939_DTC_Sample_Conf_aUni[DTC_Index].DTC_u32;
	}
}

const J1939_DTC_Sample_Uni_t J1939_DTC_Sample_Conf_aUni[10] =
{
	{0x7FE01,	 0,	  0 ,	 0x7F},
	{0x7FE02,	 3,	  0 ,	 0x7F},
	{0x7FE03,	 3,	  0 ,	 0x7F},
	{0x7FE04,	 0,	  0 ,	 0x7F},
	{0x7FE05,	 0,	  0 ,	 0x7F},
	{0x7FE06,	 3,	  0 ,	 0x7F},
	{0x7FE07,	 0,	  0 ,	 0x7F},
	{0x7FE08,	 3,	  0 ,	 0x7F},
	{0x7FE09,	 0,	  0 ,	 0x7F},
	{0x7FE0A,	 3,	  0 ,	 0x7F}

};

#endif

#if(TRUE == DIAG_TEST_DEMO_CODE)

//extern void TS_StopScheduler (void);
//uint8_t    diag_fltpath_u16 = 0xFF;
  
uint8_t test_fault_input1_au8[1] = {0}; 
uint8_t test_fault_input2_au8[1] = {0};
uint8_t test_fault_input3_au8[1] = {0};
uint8_t test_fault_input4_au8[1] = {0};  
uint8_t test_fault_input5_au8[1] = {0};  
uint8_t test_fault_input6_au8[1] = {0};
uint8_t test_fault_input7_au8[1] = {0};
uint8_t test_fault_input8_au8[1] = {0};
uint8_t test_fault_input9_au8[1] = {0};
uint8_t test_fault_input10_au8[1] = {0};

uint8_t Rv_test_fault_input1_au8[1] = {0};
uint8_t Rv_test_fault_input2_au8[1] = {0}; 
uint8_t Rv_test_fault_input3_au8[1] = {0}; 
uint8_t Rv_test_fault_input4_au8[1] = {0}; 
uint8_t Rv_test_fault_input5_au8[1] = {0}; 
uint8_t Rv_test_fault_input6_au8[1] = {0}; 
uint8_t Rv_test_fault_input7_au8[1] = {0}; 
uint8_t Rv_test_fault_input8_au8[1] = {0}; 
uint8_t Rv_test_fault_input9_au8[1] = {0}; 
uint8_t Rv_test_fault_input10_au8[1] = {0}; 

FM_Test_St_t FM_Test_St[10] = 
{
    {TEST_FAULT_DID_1 , test_fault_input1_au8},
    {TEST_FAULT_DID_2 , test_fault_input2_au8},
    {TEST_FAULT_DID_3 , test_fault_input3_au8},
    {TEST_FAULT_DID_4 , test_fault_input4_au8},
    {TEST_FAULT_DID_5 , test_fault_input5_au8},
    {TEST_FAULT_DID_6 , test_fault_input6_au8},
    {TEST_FAULT_DID_7 , test_fault_input7_au8},
    {TEST_FAULT_DID_8 , test_fault_input8_au8},
    {TEST_FAULT_DID_9 , test_fault_input9_au8},
    {TEST_FAULT_DID_10, test_fault_input10_au8},
};



void diag_appl_test_fm_proc_50ms(void)
{
    uint8_t i = 0;
  
    for(i=0;i<10;i++)
    {
        if (0 == *FM_Test_St[i].FM_test_input_pu8)
        {
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
            FM_ReportFault(FM_FP_TEST1_E, FM_NO_ERR);
#endif
        }        
        else if (1 == *FM_Test_St[i].FM_test_input_pu8)
        {
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
           FM_ReportFault(FM_FP_TEST1_E, FM_MAX_ERR);
#endif
        }
        *FM_Test_St[i].FM_test_input_pu8 = 0xFF;
    }
}

#endif

#if(TRUE == DIAG_TEST_FM_EEPROM_DEMO)

CanSchedMsg_St_t CanTest_St [1] = {0};

void diag_appl_test_fm_write(uint32_t Addr, uint32_t data)
{
    uint32_t i = 0;
    uint8_t shift_u8 = 24;
    uint32_t mask_u32 = 0xFF000000;

    CAN_MessageFrame_St_t CAN_MessageFrame_St;
    CAN_MessageFrame_St.ID_u32 = 0x512;
    CAN_MessageFrame_St.DataLength_u8 = 8U;
    CAN_MessageFrame_St.MessageType_u8 = STD_E;

    CAN_MessageFrame_St.DataBytes_au8[0] = 0xFF;//write commande
    for (i=1; i<5; i++)
    {
    	CAN_MessageFrame_St.DataBytes_au8[i] = ((Addr & mask_u32) >> shift_u8);
        shift_u8 -= 8;
        mask_u32 = mask_u32 >> 8;
    }
//    CIL_CAN_Tx_DynamicMsg(CIL_EEPROM_FAULT_FUNC_TX_E, CAN_MessageFrame_St);
    CIL_CAN_Tx_AckMsg(CIL_EEPROM_FAULT_FUNC_TX_E, CAN_MessageFrame_St);

    //command to write with address

    shift_u8 = 24;
    mask_u32 = 0xFF000000;
    for (i=0; i<4; i++)
    {
    	CAN_MessageFrame_St.DataBytes_au8[i] = ((data & mask_u32) >> shift_u8);
        shift_u8 -= 8;
        mask_u32 = mask_u32 >> 8;
    }

//    CIL_CAN_Tx_DynamicMsg(CIL_EEPROM_FAULT_FUNC_TX_E, CAN_MessageFrame_St);
    CIL_CAN_Tx_AckMsg(CIL_EEPROM_FAULT_FUNC_TX_E, CAN_MessageFrame_St);
    //cammand to write the data
//    HAL_CAN_SendCANMsg((HAL_CAN_BufferId_En_t)TX_BUFFER_ID, &CAN_Message_St);
}
uint32_t diag_appl_test_fm_read(uint32_t Addr)
{
    uint32_t Data_u32 = 0;
    uint32_t i = 0;
    uint8_t shift_u8 = 24;
    uint32_t mask_u32 = 0xFF000000;

    CAN_MessageFrame_St_t CAN_MessageFrame_St;
    CAN_MessageFrame_St.ID_u32 = 0x512;
    CAN_MessageFrame_St.DataLength_u8 = 8U;
    CAN_MessageFrame_St.MessageType_u8 = STD_E;

    CAN_MessageFrame_St.DataBytes_au8[0] = 0x00;  //command to read
    for (i=1; i<5; i++)
    {
    	CAN_MessageFrame_St.DataBytes_au8[i] = ((Addr & mask_u32) >> shift_u8);
    	CAN_MessageFrame_St.DataBytes_au8[i + 4] = 0xFF;
        shift_u8 -= 8;
        mask_u32 = mask_u32 >> 8;
    }
    CAN_MessageFrame_St.DataBytes_au8[5] = 0x00;
    CAN_MessageFrame_St.DataBytes_au8[6] = 0x00;
    CAN_MessageFrame_St.DataBytes_au8[7] = 0x00;
    shift_u8 = 24;
    mask_u32 = 0xFF000000;
 // sent a command to read the data
    CIL_CAN_Tx_AckMsg(CIL_EEPROM_FAULT_FUNC_TX_E, CAN_MessageFrame_St);
//    CIL_CAN_Tx_DynamicMsg(CIL_EEPROM_FAULT_FUNC_TX_E, CAN_MessageFrame_St);


    //wait for recieving the data
    while(CanTest_St[0].rxFlag == false);
    for (i=1; i<5; i++)
    {
        Data_u32 |= (uint32_t)((CanTest_St->msg.DataBytes_au8[i] << shift_u8) & mask_u32);
        shift_u8 -= 8;
        mask_u32 = mask_u32 >> 8;
    }
    CanTest_St[0].rxFlag = false;
    return Data_u32;


}
#endif
#if(TRUE == DIAG_TEST_FM_DEMO && TRUE == DIAG_CONF_FM_SUPPORTED)

void diag_fm_tx_fault1 (void)
{

    CAN_MessageFrame_St_t Can_Applidata_St;//CAN_MessageFrame_St_t
    Can_Applidata_St.DataLength_u8 = 0x08;
    Can_Applidata_St.DataBytes_au8[0] = (FML2_GetPrsntFaultState(0)<<4 | FML2_GetFaultPath(0));
    Can_Applidata_St.DataBytes_au8[1] = (FML2_GetPrsntFaultState(1)<<4 | FML2_GetFaultPath(1));
    Can_Applidata_St.DataBytes_au8[2] = (FML2_GetPrsntFaultState(2)<<4 | FML2_GetFaultPath(2));
    Can_Applidata_St.DataBytes_au8[3] = (FML2_GetPrsntFaultState(3)<<4 | FML2_GetFaultPath(3));
    Can_Applidata_St.DataBytes_au8[4] = (FML2_GetPrsntFaultState(4)<<4 | FML2_GetFaultPath(4));
    Can_Applidata_St.DataBytes_au8[5] = (FML2_GetPrsntFaultState(5)<<4 | FML2_GetFaultPath(5));
    Can_Applidata_St.DataBytes_au8[6] = (FML2_GetPrsntFaultState(6)<<4 | FML2_GetFaultPath(6));
    Can_Applidata_St.DataBytes_au8[7] = (FML2_GetPrsntFaultState(7)<<4 | FML2_GetFaultPath(7));
    Can_Applidata_St.MessageType_u8 = STD_E;
    Can_Applidata_St.ID_u32 = CIL_CAN_GetID(CIL_FM_TEST1_TX_E);
    CIL_CAN_Tx_DynamicMsg(CIL_FM_TEST1_TX_E, Can_Applidata_St);
}
void diag_fm_tx_fault2 (void)
{

    CAN_MessageFrame_St_t Can_Applidata_St;//CAN_MessageFrame_St_t
    Can_Applidata_St.DataLength_u8 = 0x08;
    Can_Applidata_St.DataBytes_au8[0] = (FML2_GetPrsntFaultState(8)<<4 | FML2_GetFaultPath(8));
    Can_Applidata_St.DataBytes_au8[1] = (FML2_GetPrsntFaultState(9)<<4 | FML2_GetFaultPath(9));
    Can_Applidata_St.DataBytes_au8[2] = 0xFFU;
    Can_Applidata_St.DataBytes_au8[3] = 0xFFU;
    Can_Applidata_St.DataBytes_au8[4] = 0xFFU;
    Can_Applidata_St.DataBytes_au8[5] = 0xFFU;
    Can_Applidata_St.DataBytes_au8[6] = 0xFFU;
    Can_Applidata_St.DataBytes_au8[7] = 0xFFU;
    Can_Applidata_St.MessageType_u8 = STD_E;
    Can_Applidata_St.ID_u32 = CIL_CAN_GetID(CIL_FM_TEST2_TX_E);
    CIL_CAN_Tx_DynamicMsg(CIL_FM_TEST2_TX_E, Can_Applidata_St);
}

void diag_appl_test_fm_drv_Cycle(uint16_t CIL_SigName_En, CAN_MessageFrame_St_t* Can_Applidata_St)
{
	if(0 == Can_Applidata_St->DataBytes_au8[0])
	{
		FM_SET_WUPCylInd();
		FM_SET_DryCylInd();
		FM_PowerOff();
		// FM_Init();
//		FM_SET_WUPCylInd();
//		FM_SET_DryCylInd();
//		TS_StopScheduler();
	}
	else if (0xFF == Can_Applidata_St->DataBytes_au8[0])
	{
		FM_SET_WUPCylInd();
		FM_SET_DryCylInd();
		FM_ReportFault(Can_Applidata_St->DataBytes_au8[1], Can_Applidata_St->DataBytes_au8[2]);
	}
	else if (0x55 == Can_Applidata_St->DataBytes_au8[0])
	{
//		FM_SET_WUPCylInd();
//		FM_SET_DryCylInd();
        FM_ReportFault(Can_Applidata_St->DataBytes_au8[1], FM_NO_ERR);
	}

}

#endif

void EEpromTest(void)
{
    // uint8_t VolConfRegister_au8[8] = {0x07,0x00,0x00};
    // VolatileConfig(0x01,VolConfRegister_au8,0x01);
   
    // VolConfRegister_au8[0] = 0XFD;
    // VolatileConfig(0x03,VolConfRegister_au8,0x01);   

}

