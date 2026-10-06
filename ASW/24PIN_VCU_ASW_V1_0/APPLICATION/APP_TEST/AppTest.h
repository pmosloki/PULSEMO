#ifndef APPTEST
#define APPTEST

#include"pal_can_conf.h"
// #include"rscan_driver.h"
// #include"rscan_typedefs.h"
// #include"rscan_reg.h"
extern void CAN_functionality();
extern void Call_Back_CAN0_Tx_Test(uint8_t CanNum_u8,uint8_t Channel_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32);
extern void Call_Back_CAN1_RX_Test(uint8_t CanNum_u8,uint8_t Channel_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32);
extern void Call_Back_CAN2_Tx_Test(uint8_t CanNum_u8,uint8_t Channel_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32);
extern void Test_GpioOutput(void);
extern void Test_GpioInput(void);
extern void Test_Adc(void);
extern void Test_LS_Gpio_Output(void);
extern void Test_HS_Gpio_Output(void);
extern void Test_LS_Gpio_Output(void);
extern void fun3();
extern void fun4();
extern void CAN1_Tx_Test1(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32);
extern void CAN0_Tx_Test0(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32);
extern void CAN2_Tx_Test2(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32);
#endif /* APPTEST */
