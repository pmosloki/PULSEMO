/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : pal_gpio.c
|    Project        :  VCU_ASW
|    Description    : The file implements the peripheral driver for GPIO.
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
#include"pal_gpio_conf.h"
#include"pal_gpio.h"
//#include"Config_PORT.h"
#include"hal_gpio.h"
#include"Pin.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ReadDigitalInput
*   Description   : The function reads the digital input 
*   Parameters    : DInput_u8: Digital input number
*   Return Value  : The function returns the pin state 
*                   1 :- High level
*                   0 :- Low level
*  ---------------------------------------------------------------------------*/


/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ReadDigitalInput
*   Description   : The function reads the digital input 
*   Parameters    : DInput_u8: Digital input number
*   Return Value  : The function returns the pin state 
*                   1 :- High level
*                   0 :- Low level
*  ---------------------------------------------------------------------------*/
uint8_t ReadDigitalInput(const uint8_t DInput_u8)
{
    uint8_t LoopCnt_u8 = 0;

    for (LoopCnt_u8 = DI_START_E; LoopCnt_u8 < TOTAL_DI_E; LoopCnt_u8++)
    {
        if (DInput_u8 == DigitalInpConf_aSt[LoopCnt_u8].DigitalInp_u8)
        {
            return ReadPortPin(DigitalInpConf_aSt[LoopCnt_u8].Port_u8,DigitalInpConf_aSt[LoopCnt_u8].Pin_u8);
        }
    }

    return TURN_OFF;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : WriteDigitalOutput
*   Description   : The function reads the digital input 
*   Parameters    : DOutput_u8: Digital output number
*                   PinState_u8: 
*                   0:- Reset the pin
*                   1:- Set the pin
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void WriteDigitalOutput(const uint8_t DOutput_u8,const uint8_t PinState_u8)
{
    uint8_t LoopCnt_u8 = 0;

    for (LoopCnt_u8 = DO_START_E; LoopCnt_u8 < TOTAL_DO_E; LoopCnt_u8++)
    {
        if (DOutput_u8 == DigitalOutConf_aSt[LoopCnt_u8].DigitalOut_u8)
        {
            WritePortPin(DigitalOutConf_aSt[LoopCnt_u8].Port_u8,DigitalOutConf_aSt[LoopCnt_u8].Pin_u8,PinState_u8);
            break;
        }
    }
    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ToggleOutput
*   Description   : The function reads the digital input 
*   Parameters    : DOutput_u8: Digital output number
*                   PinState_u8: 
*                   0:- Reset the pin
*                   1:- Set the pin
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void ToggleOutput(const uint8_t DOutput_u8 )
{
    uint8_t LoopCnt_u8 = 0;

    for (LoopCnt_u8 = DO_START_E; LoopCnt_u8 < TOTAL_DO_E; LoopCnt_u8++)
    {
        if (DOutput_u8 == DigitalOutConf_aSt[LoopCnt_u8].DigitalOut_u8)
        {
            TogglePortPin(DigitalOutConf_aSt[LoopCnt_u8].Port_u8,DigitalOutConf_aSt[LoopCnt_u8].Pin_u8);
            break;
        }
    }
    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ReadDigitalOutput
*   Description   : The function reads the pin state of the digital output 
*   Parameters    : DOutput_u8 Digital output number
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint8_t ReadDigitalOutput(const uint8_t DOutput_u8)
{
    uint8_t LoopCnt_u8 = 0;

    for (LoopCnt_u8 = DO_START_E; LoopCnt_u8 < TOTAL_DO_E; LoopCnt_u8++)
    {
        if (DOutput_u8 == DigitalOutConf_aSt[LoopCnt_u8].DigitalOut_u8)
        {
            return ReadPortPin(DigitalOutConf_aSt[LoopCnt_u8].Port_u8,DigitalOutConf_aSt[LoopCnt_u8].Pin_u8);
        }
    }

    return TURN_OFF;
}
/*---------------------- End of File -----------------------------------------*/