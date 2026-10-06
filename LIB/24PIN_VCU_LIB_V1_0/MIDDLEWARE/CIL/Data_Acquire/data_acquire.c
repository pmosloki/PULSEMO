/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : data_acquire.h
|    Project        : VCU ASW
|    Description    : The file acquires the data.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                      Company
| --------     ---------------------     ---------------------------------------
| 
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"data_acquire.h"
#include"pal_gpio.h"
#include"databank.h"
#include"Digit_Utils.h"
#include "rscan_reg.h"
#include "fm_conf.h"
#include"pal_can_if.h"
#include"pal_adc_conf.h"
#include"pal_adc.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/
#define ADC_THRESHOLD                   (1000)
#define DEBOUNCE_COUNT                  (PIN_READ_TIME * 10) // 50 millisec
#define ADC_NOISE_THRESHOLD             (100)

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/
uint8_t CAN_BusOff_Error_u8;
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/
DInpSignal_St_t DInpSignal_aSt[TOTAL_DI_E];
AdcInpSignal_St_t AdcInpSignal_aSt[TOTAL_ANI_E];
DOutputSignal_St_t DOutputSig_aSt[TOTAL_DO_E];

//static Adc_Filter_St_t Adc_Filter_aSt[TOTAL_ADC_INPUT] =
//{
//    /* [ADC_PtSensor1HighTemp           ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.PtSensor1HighTemp_u16     },
//    /* [ADC_PtSensor1HighPressure       ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.PtSensor1HighPress_u16    },
//    /* [ADC_IncarSensor                 ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.InCarSensor_u16           },
//    /* [ADC_AmbientSensor               ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.Ambisensor_u16            },
//    /* [ADC_PtSensor1LowTemp            ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.PtSensor1LowTemp_u16      },
//    /* [ADC_PtSensor1LowPress           ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.PtSensor1LowPress_u16     },
//    /* [ADC_ExtraAdcPin                 ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.ExtraAdcPin               },
//    /* [ADC_PtSensor2HighTemp           ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.PtSensor2HighTemp_u16     },
//    /* [ADC_PtSensor2HighPress          ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.Ptsensor2HighPress_u16    },
//    /* [ADC_PtSensor2LowTemp            ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.PtSensor2LowTemp_u16      },
//    /* [ADC_PtSensor2LowPress           ] = */ {FILTER_FACTOR_1    ,&SensorAnalogInput_St.Ptsensor2LowPress_u16     },
    
//};
/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
static void SwitchDebounce(DInpSignal_St_t*const VcuSwitchInp_pSt);
static void AdcDebounce(AdcInpSignal_St_t*const AdcInp_pSt);

float ReturnFloat(void);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : DataAquireInit
*   Description   : The function initialize the Analog and Digital input structure. 
*   Parameters    : None
*   Return Value  : None
*  -----------------------------------------------------------------------------*/
void DataAquireInit(void)
{
    uint8_t InpSig_u8 = 0;
    
    for(InpSig_u8 = DI_1_E; InpSig_u8 < TOTAL_DI_E; InpSig_u8++)
    {
        /* Initialize the digital Input structure */
        DInpSignal_aSt[InpSig_u8].ReadValue_u8 = GPIO_PIN_RESET_E;
        DInpSignal_aSt[InpSig_u8].InpState_En = GPIO_INP_OFF_E;
        DInpSignal_aSt[InpSig_u8].SignalState_u8 = TURN_OFF;
        DInpSignal_aSt[InpSig_u8].LongPress_b = false;
        DInpSignal_aSt[InpSig_u8].DebounceCount_u16 = RESET;
    }

   for(InpSig_u8 = ANI_START_E; InpSig_u8 < TOTAL_ANI_E; InpSig_u8++)
   {
       /* Initialize the analog input structure */
       AdcInpSignal_aSt[InpSig_u8].ActualValue_u16 = RESET;
       AdcInpSignal_aSt[InpSig_u8].DebounceCount_u16 = RESET;
       AdcInpSignal_aSt[InpSig_u8].PresentAdc_u16 = RESET;
       AdcInpSignal_aSt[InpSig_u8].PrevAdc_u16 = RESET;
       AdcInpSignal_aSt[InpSig_u8].StartDebounce_b = false;
   }

    
    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : ReadInputSignal
*   Description   : The function reads the VCU input signal. 
*   Parameters    : None
*   Return Value  : None
*  -----------------------------------------------------------------------------*/
void ReadInputSignal(void)
{
    uint8_t InpSig_u8 = 0;

    /* Read and update the digital input signals */
    for(InpSig_u8 = DI_1_E; InpSig_u8 < TOTAL_DI_E; InpSig_u8++)
    {
        DInpSignal_aSt[InpSig_u8].ReadValue_u8 = ReadDigitalInput(InpSig_u8);
        SwitchDebounce(&DInpSignal_aSt[InpSig_u8]);    
    }

    /* Read and update the analog input signals */
   ReadAdcInput();
   for(InpSig_u8 = ANI_START_E; InpSig_u8 < TOTAL_ANI_E; InpSig_u8++)
   {
       AdcInpSignal_aSt[InpSig_u8].PresentAdc_u16 = GetAdcResult(InpSig_u8);
        AdcDebounce(&AdcInpSignal_aSt[InpSig_u8]);
        //debounce function
   }

    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : SwitchDebounce
*   Description   : The function update switch type input
*   Parameters    : VcuSwitchInp_pSt:- Gpio input structure
*   Return Value  : None
*  -----------------------------------------------------------------------------*/
void SwitchDebounce(DInpSignal_St_t*const VcuSwitchInp_pSt)
{
    if(GPIO_PIN_SET_E == VcuSwitchInp_pSt->ReadValue_u8)
    {
        /* GPIO input is Turned On*/
        if(GPIO_INP_OFF_E == VcuSwitchInp_pSt->InpState_En)
        {
            /* Hold the GPIO state and capture the time */
            VcuSwitchInp_pSt->InpState_En = GPIO_INP_ON_HOLD_E;
            VcuSwitchInp_pSt->DebounceCount_u16 = CLEAR;
        }
        else
        {
            ;
        }

        if((GPIO_INP_ON_HOLD_E == VcuSwitchInp_pSt->InpState_En) && 
            (DEBOUNCE_COUNT <= VcuSwitchInp_pSt->DebounceCount_u16) )
        {
            /* If GPIO state is ON more than Debounce count time, Change the state to ON*/
            VcuSwitchInp_pSt->InpState_En = GPIO_INP_ON_E;
            VcuSwitchInp_pSt->SignalState_u8 = TURN_ON;
            VcuSwitchInp_pSt->DebounceCount_u16 = CLEAR;
        }
        else if((GPIO_INP_ON_HOLD_E == VcuSwitchInp_pSt->InpState_En) &&
            ( DEBOUNCE_COUNT > VcuSwitchInp_pSt->DebounceCount_u16))
        {
            /* If GPIO state is ON, but it is less than Debounce count time, then update the Hold time*/
           VcuSwitchInp_pSt->DebounceCount_u16 += PIN_READ_TIME;
        }
        else if(GPIO_INP_OFF_HOLD_E == VcuSwitchInp_pSt->InpState_En)
        {
            /* The GPIO state is reseted within the debounce time*/
            VcuSwitchInp_pSt->InpState_En = GPIO_INP_ON_E;
            VcuSwitchInp_pSt->DebounceCount_u16 = CLEAR;
        }
        else
        {
            ;
        }
    }
    else if(GPIO_PIN_RESET_E == VcuSwitchInp_pSt->ReadValue_u8)
    {
        /* GPIO input is Turned On*/
        if(GPIO_INP_ON_E == VcuSwitchInp_pSt->InpState_En)
        {
            /* Hold the GPIO state and capture the time */
            VcuSwitchInp_pSt->InpState_En = GPIO_INP_OFF_HOLD_E;
             VcuSwitchInp_pSt->DebounceCount_u16 = CLEAR;
        }
        else
        {
            ;
        }

        if((GPIO_INP_OFF_HOLD_E == VcuSwitchInp_pSt->InpState_En) && 
            (DEBOUNCE_COUNT <= VcuSwitchInp_pSt->DebounceCount_u16) )
        {
            VcuSwitchInp_pSt->InpState_En = GPIO_INP_OFF_E;
            VcuSwitchInp_pSt->SignalState_u8 = TURN_OFF;
            VcuSwitchInp_pSt->DebounceCount_u16 = CLEAR;
        }
        else if((GPIO_INP_OFF_HOLD_E == VcuSwitchInp_pSt->InpState_En) &&
            ( DEBOUNCE_COUNT > VcuSwitchInp_pSt->DebounceCount_u16))
        {
            VcuSwitchInp_pSt->DebounceCount_u16 += PIN_READ_TIME;
        }
        else if(GPIO_INP_ON_HOLD_E == VcuSwitchInp_pSt->InpState_En)
        {
            /* The GPIO state is reseted within the debounce time*/
            VcuSwitchInp_pSt->InpState_En = GPIO_INP_OFF_E;
            VcuSwitchInp_pSt->DebounceCount_u16 = CLEAR;
        }
        else
        {
            ;
        }
    }
    else
    {

    }
    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : AdcDebounce
*   Description   : The function debounce the ADC value
*   Parameters    : AdcInp_pSt:- ADC input structure
*   Return Value  : None
*  -----------------------------------------------------------------------------*/
void AdcDebounce(AdcInpSignal_St_t*const AdcInp_pSt)
{
    if((Differenceof_2_num(AdcInp_pSt->PresentAdc_u16,AdcInp_pSt->PrevAdc_u16)) > ADC_NOISE_THRESHOLD)
    {
        /* Change in the ADC value more than the ADC_NOISE_THRESHOLD
        Restart the debounce */
        AdcInp_pSt->StartDebounce_b = true;
        AdcInp_pSt->DebounceCount_u16 = RESET;
        AdcInp_pSt->PrevAdc_u16 = AdcInp_pSt->PresentAdc_u16;
    }

    if(true == AdcInp_pSt->StartDebounce_b)
    {
        if(AdcInp_pSt->DebounceCount_u16 < DEBOUNCE_COUNT)
        {
            /* Increment the debounce */
            AdcInp_pSt->DebounceCount_u16 += PIN_READ_TIME;
        }
        else
        {
            /* ADC value is the actual value(not a noise) */
            AdcInp_pSt->StartDebounce_b = false;
            AdcInp_pSt->ActualValue_u16 =  AdcInp_pSt->PresentAdc_u16;
            AdcInp_pSt->PrevAdc_u16 = AdcInp_pSt->PresentAdc_u16;
        }  
    }

    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : WriteOutputSignals
*   Description   : The function writes the vcu output signals
*   Parameters    : AdcInp_pSt:- ADC input structure
*   Return Value  : None
*  -----------------------------------------------------------------------------*/
void WriteOutputSignals(void)
{
    uint8_t OutSig_u8 = 0;
    for(OutSig_u8 = DO_1_E; OutSig_u8 < TOTAL_DO_E; OutSig_u8++)
    {
        if(DOutputSig_aSt[OutSig_u8].PresentState_u8 != DOutputSig_aSt[OutSig_u8].PreviousState_u8)
        {
            WriteDigitalOutput(OutSig_u8,DOutputSig_aSt[OutSig_u8].PresentState_u8);
            DOutputSig_aSt[OutSig_u8].PreviousState_u8 = DOutputSig_aSt[OutSig_u8].PresentState_u8;
        }
    }
    return;
}


/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint16_t Read_Adc_u16;
void Update_Adc_Value(void)
{
    // uint8_t AdcInput_u8;
    // for (AdcInput_u8 = 0u; AdcInput_u8 < TOTAL_ADC_INPUT; AdcInput_u8++)
    // {
    //     Read_Adc_u16 = GetAdcResult(AdcInput_u8);
    //     *Adc_Filter_aSt[AdcInput_u8].Data_pu16 = lowPassFilter16bit(Read_Adc_u16, *Adc_Filter_aSt[AdcInput_u8].Data_pu16,
    //                                                    Adc_Filter_aSt[AdcInput_u8].Factor_u16);
    // }
    return;
}
// /* -----------------------------------------------------------------------------
// *  FUNCTION DESCRIPTION
// *  -----------------------------------------------------------------------------
// *   Function Name : NONE
// *   Description   : NONE
// *   Parameters    : None
// *   Return Value  : None
// *  ---------------------------------------------------------------------------*/
// void UpdateCanBusState(void)
// {
//     if (CAN_BUS_OFF_STATUS == get_canbus_status(0u))
//     {
//         CAN_BusOff_Error_u8 = FM_SIG_ERR;
//     }
//     else if (CAN_PASSIVE_ERROR_STATUS == get_canbus_status(0u))
//     {
//         CAN_BusOff_Error_u8 = FM_SIG_ERR;
//     }
//     else
//     {
//         CAN_BusOff_Error_u8 = FM_NO_ERR;
//     }
//     return;
// }
/*---------------------- End of File -----------------------------------------*/