/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : DataBank.h
|    Project        :  OSM_VCU_ASW 
|    Description    : 
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
#ifndef DATABANK_H
#define DATABANK_H
/*******************************************************************************
 *  HEARDER FILE INCLUDES
 ******************************************************************************/
#include"app_typedef.h"

/*******************************************************************************
 *  MACRO DEFNITION
 ******************************************************************************/


/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/
typedef enum
{
	GPINUPUT_START_E = 0,
	B6_GPINUPUT1_E = GPINUPUT_START_E,
	GPINUPUT_TOTAL_E
} GpInputSignal_En_t;


typedef enum
{
	ADC_START_E = 0,
	C7_ADC1_E = ADC_START_E,
	C4_ADC2_E,
	B4_ADC3_E,
	C1_ADC4_E,
	C5_ADC5_E,
	B5_ADC6_E,
	ADC_TOTAL_E
} AdcInput_En_t;

typedef enum
{
	GPOUTPUT_START_E = 0,
	A7_GPOUTPUT1_HS1_E = GPOUTPUT_START_E,/* A7 */
	B7_GPOUTPUT2_HS2_E,/* B7 */
	A8_GPOUTPUT3_HS3_E,/* A8*/	
	B8_GPOUTPUT4_HS4_E,/* B8 */
	A4_GPOUTPUT5_LS1_E, /* A4 */
	A5_GPOUTPUT6_LS2_E,/* A5 */
	A6_GPOUTPUT7_LS3_E,/* A6 */
	GPOUTPUT_TOTAL_E,
} GpOutputSignal_En_t;




/*******************************************************************************
 *  EXTERN GLOBAL VARIABLES
 ******************************************************************************/
// extern EcuOutputs_St_t VcuOutputs_St;
// extern EcuOutputs_St_t EcuOutputs_St;
// extern UserInputSignal_St_t UserInputSignal_St;
// extern SensorAnalogInput_St_t SensorAnalogInput_St;
//extern uint8_t safeMode_u8;
/*******************************************************************************
 *  EXTERN FUNCTION
 ******************************************************************************/

extern uint8_t Get_GPInput_State(GpInputSignal_En_t GPI_Channel_Num);
extern uint16_t Get_ADC_Result(AdcInput_En_t ADC_Channel_Num);
extern void Set_GPO_State(GpOutputSignal_En_t GPOutput_Channel_Num, bool PinState_b);


#endif

/*---------------------- End of File -----------------------------------------*/