/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : hal_gpio.c
|    Project        :  VCU_ASW
|    Description    : The file implements the HAL for the GPIO.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
|
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"hal_gpio.h"
#include"r_cg_macrodriver.h"

/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/
#define PIN_READ_MASK   (0x01u)
#define PIN_HIGH        (0x01u) 
#define PIN_LOW         (0x00u)  
/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/

HalPort_St_t HalPort_St[TOTAL_PORT_E] = 
{
    {&PORT.PPR0,&PORT.P0,PORT_0_E},
    {&PORT.PPR8,&PORT.P8,PORT_8_E},
     {&PORT.PPR9,&PORT.P9,PORT_9_E},
    {&PORT.PPR10,&PORT.P10,PORT_10_E},
    {&PORT.PPR11,&PORT.P11,PORT_11_E},
    {&PORT.APPR0,&PORT.AP0,APORT_0_E},
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
*   Function Name : ReadPortPin
*   Description   : The function read the port pin state
*   Parameters    : PortNum_u8 :- Port number
*                   Pin_u8 :- Pin number of the port
*   Return Value  : PinState_u8 : State of the port pin
                    0 :- Low level.
                    1 :- High level.
*  ---------------------------------------------------------------------------*/
uint8_t ReadPortPin(uint8_t PortNum_u8,uint8_t Pin_u8)
{
    uint8_t LoopCnt_u8 = 0;
    uint8_t PinState_u8 = 0;
    for(LoopCnt_u8 = PORT_0_E;LoopCnt_u8<TOTAL_PORT_E;LoopCnt_u8++)
    {
        if((NULL != HalPort_St[LoopCnt_u8].PortPinRead_pu16)&& 
            (PortNum_u8 == HalPort_St[LoopCnt_u8].Port_u8))
        {
            PinState_u8 = (*(HalPort_St[LoopCnt_u8].PortPinRead_pu16)>>Pin_u8) & PIN_READ_MASK;
            break;
        }
    }
    return PinState_u8;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : WritePortPin
*   Description   : The function write the port pin
*   Parameters    : PortNum_u8 :- Port number
*                   Pin_u8 :- Pin number of the port
*                   State_u8 :- State of pin to be write.
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void WritePortPin(uint8_t PortNum_u8, uint8_t Pin_u8, uint8_t State_u8)
{
    uint8_t LoopCnt_u8 = 0;
    if (PIN_HIGH < State_u8)
    {
        return;
    }

    for (LoopCnt_u8 = PORT_0_E; LoopCnt_u8 < TOTAL_PORT_E; LoopCnt_u8++)
    {
        if((NULL != HalPort_St[LoopCnt_u8].PortPinWrite_pu16)&& 
            (PortNum_u8 == HalPort_St[LoopCnt_u8].Port_u8))
        {
            if (PIN_HIGH == State_u8)
            {
                *HalPort_St[LoopCnt_u8].PortPinWrite_pu16 = (PIN_HIGH << Pin_u8) | (*HalPort_St[LoopCnt_u8].PortPinWrite_pu16);
            }
            else
            {
                *HalPort_St[LoopCnt_u8].PortPinWrite_pu16 = (~(PIN_HIGH << Pin_u8)) & (*HalPort_St[LoopCnt_u8].PortPinWrite_pu16);
            }
            break;
        }
    }
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : WritePortPin
*   Description   : The function toggle the port pin
*   Parameters    : PortNum_u8 :- Port number
*                   Pin_u8 :- Pin number of the port
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void TogglePortPin(uint8_t PortNum_u8, uint8_t Pin_u8)
{
    uint8_t LoopCnt_u8 = 0;
    for (LoopCnt_u8 = PORT_0_E; LoopCnt_u8 < TOTAL_PORT_E; LoopCnt_u8++)
    {
        if((NULL != HalPort_St[LoopCnt_u8].PortPinWrite_pu16)&& 
            (PortNum_u8 == HalPort_St[LoopCnt_u8].Port_u8))
        {
            *HalPort_St[LoopCnt_u8].PortPinWrite_pu16 = (PIN_HIGH << Pin_u8) ^ (*HalPort_St[LoopCnt_u8].PortPinWrite_pu16);
        }
    }
}
/*---------------------- End of File -----------------------------------------*/