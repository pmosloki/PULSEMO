#include "AppTest.h"
#include "databank.h"
#include "pal_can_if.h"
#include "nvm_data.h"
#include "eeprom.h"
#include "nvm_userinterface.h"



bool GPIO_PIN_HIGH_b;
bool GPIO_PIN_LOW_b;

uint8_t DataBuff_0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t DataBuff_1[8] = {1, 1, 1, 1, 1, 1, 1, 1};
uint8_t DataBuff_2[8] = {2, 2, 2, 2, 2, 2, 2, 2};
uint8_t GpioInpdata[8];
uint32_t i;
uint16_t AdcDataBuff[8];
uint8_t Adcresult[8]; 
uint8_t Adcresult1[8];   
uint8_t Switch_var = 0;
uint8_t Switch_var2 = 0;


void Call_Back_CAN0_Tx_Test(uint8_t CanNum_u8, uint8_t Channel_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
{
  CanMsgTransmit(CAN_0, DataBuff_0, 8, 0, 0, CanId_u32 + 1);
  return;
}

void Call_Back_CAN1_RX_Test(uint8_t CanNum_u8, uint8_t Channel_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
{
  Set_GPO_State(A7_GPOUTPUT1_HS1_E, DataBuff_pu8[0]);
  Set_GPO_State(B7_GPOUTPUT2_HS2_E, DataBuff_pu8[1]);
  Set_GPO_State(A8_GPOUTPUT3_HS3_E, DataBuff_pu8[2]);
  Set_GPO_State(B8_GPOUTPUT4_HS4_E, DataBuff_pu8[3]);
  /* LOW SIDE OUTPUTS*/
  Set_GPO_State(A4_GPOUTPUT5_LS1_E, DataBuff_pu8[4]);
  Set_GPO_State(A5_GPOUTPUT6_LS2_E, DataBuff_pu8[5]);
  Set_GPO_State(A6_GPOUTPUT7_LS3_E, DataBuff_pu8[6]);
  return;
}

void Call_Back_CAN2_Tx_Test(uint8_t CanNum_u8, uint8_t Channel_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
{
  CanMsgTransmit(CAN_2, DataBuff_0, 8, 0, 0, CanId_u32 + 1);

  return;
}

void Test_GpioInput(void)
{
  GpioInpdata[0] = Get_GPInput_State(B6_GPINUPUT1_E); // Read gpio input
  CanMsgTransmit(CAN_1, &GpioInpdata[0], 8, 0, 0, 0x300);
  return;
}

void Test_HS_Gpio_Output(void)
{
  GPIO_PIN_HIGH_b = !GPIO_PIN_HIGH_b;

  // Set_GPO_State(A2_GPOUTPUT1_HS1_E, GPIO_PIN_HIGH_b);
  // Set_GPO_State(A3_GPOUTPUT2_HS2_E, GPIO_PIN_HIGH_b);
  // Set_GPO_State(B4_GPOUTPUT3_HS3_E, GPIO_PIN_LOW_b);
  // Set_GPO_State(C4_GPOUTPUT4_HS4_E, GPIO_PIN_LOW_b);
  // return;
}

void Test_LS_Gpio_Output(void)
{

  // Set_GPO_State(A1_GPOUTPUT5_LS1_E, SetLsOutData[0]); // Set gpio input
  // Set_GPO_State(A1_GPOUTPUT5_LS1_E, 1);
  // // Set_GPO_State(B1_GPOUTPUT6_LS2_E, SetLsOutData[1]);
  // Set_GPO_State(B1_GPOUTPUT6_LS2_E, 1);
  // // Set_GPO_State(C1_GPOUTPUT7_LS3_E, SetLsOutData[2]);
  // Set_GPO_State(C1_GPOUTPUT7_LS3_E, 1);
  // // Set_GPO_State(D1_GPOUTPUT8_LS4_E, SetLsOutData[3]);
  // Set_GPO_State(D1_GPOUTPUT8_LS4_E, 1);
}
void Test_GpioOutput(void)
{
  switch (Switch_var2)
  {
  case 0:
  {
    Test_LS_Gpio_Output();
    Switch_var2 = 1;
    break;
  }
  case 1:
  {
    Test_HS_Gpio_Output();
    Switch_var2 = 0;
    break;
  }
  default:
    break;
  }
  return;
}
void Test_Adc(void)
{
  switch (Switch_var)
  {
  case 0:
  {
    AdcDataBuff[0] = Get_ADC_Result(C7_ADC1_E);
    Adcresult[0] = AdcDataBuff[0];
    Adcresult[1] = (AdcDataBuff[0] >> 8);

    AdcDataBuff[1] = Get_ADC_Result(C4_ADC2_E);
    Adcresult[2] = AdcDataBuff[1];
    Adcresult[3] = (AdcDataBuff[1] >> 8);

    AdcDataBuff[2] = Get_ADC_Result(B4_ADC3_E);
    Adcresult[4] = AdcDataBuff[2];
    Adcresult[5] = (AdcDataBuff[2] >> 8);

    AdcDataBuff[3] = Get_ADC_Result(C1_ADC4_E);
    Adcresult[6] = AdcDataBuff[3];
    Adcresult[7] = (AdcDataBuff[3] >> 8);

    
    CanMsgTransmit(CAN_1, &Adcresult[0], 8, 0, 0, 0x201);
    Switch_var = 1;
    break;
  }
  case 1:
  {
    AdcDataBuff[4] = Get_ADC_Result(C5_ADC5_E);
    Adcresult1[0] = AdcDataBuff[4];
    Adcresult1[1] = (AdcDataBuff[4] >> 8);

    AdcDataBuff[5] = Get_ADC_Result(B5_ADC6_E);
    Adcresult1[2] = AdcDataBuff[5];
    Adcresult1[3] = (AdcDataBuff[5] >> 8);

    CanMsgTransmit(CAN_1, &Adcresult1[0], 8, 0, 0, 0x202);
    Switch_var = 0;
    break;
  }
  default:
    break;
  }

  return;
}



void CAN_functionality()
{
  // void CAN1_Tx_Test1(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
  // {
  DataBuff_1[0] = 1;
  DataBuff_1[1] = 1;
  DataBuff_1[2] = 1;
  DataBuff_1[3] = 1;
  DataBuff_1[4] = 1;
  DataBuff_1[5] = 1;
  DataBuff_1[6] = 1;
  DataBuff_1[7] = 1;
  CanMsgTransmit(CAN_1, DataBuff_1, 8, 0, 0, 0x181);
  // return;
  // }

  // void CAN0_Tx_Test0(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
  // {

  DataBuff_0[0] = 2;
  DataBuff_0[1] = 2;
  DataBuff_0[2] = 2;
  DataBuff_0[3] = 2;
  DataBuff_0[4] = 2;
  DataBuff_0[5] = 2;
  DataBuff_0[6] = 2;
  DataBuff_0[7] = 2;
  CanMsgTransmit(CAN_0, DataBuff_0, 8, 0, 0, 0x180);
  // return;
  // }

  // void CAN2_Tx_Test2(uint8_t CanNum_u8, uint8_t *DataBuff_pu8, uint8_t Dlc_u8, uint8_t CanMode_u8, uint8_t Ide_u8, uint32_t CanId_u32)
  // {
  DataBuff_2[0] = 3;
  DataBuff_2[0] = 3;
  DataBuff_2[1] = 3;
  DataBuff_2[2] = 3;
  DataBuff_2[3] = 3;
  DataBuff_2[4] = 3;
  DataBuff_2[5] = 3;
  DataBuff_2[6] = 3;
  DataBuff_2[7] = 3;
  CanMsgTransmit(CAN_2, DataBuff_2, 8, 0, 0, 0x182);
  //   return;
  // }
}

void fun1()
{
   CanMsgTransmit(CAN_2, DataBuff_2, 8, 0, 0, 0x183);
}
void fun2()
{
   CanMsgTransmit(CAN_0, DataBuff_2, 8, 0, 0, 0x184);
}
void fun3()
{
  CanMsgTransmit(CAN_1, DataBuff_2, 8, 0, 0, 0x184);
  
}
void fun4()
{
   CanMsgTransmit(CAN_0, DataBuff_2, 8, 0, 0, 0x184);
}


