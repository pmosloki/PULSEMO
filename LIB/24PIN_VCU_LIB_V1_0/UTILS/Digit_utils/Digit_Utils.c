/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : Digit_Utils.c
|    Project        : VCU_ASW 
|    Description    : The file implememts general digits operation.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------

|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"Digit_Utils.h"
#include "math.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/
#define BASE_VALUE_2    2
#define BASE_VALUE_10   10
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
*   Function Name : GetEquMaskValue
*   Description   : The function provides Mask value of BitSize_u8 
*   Parameters    : BitSize_u8 :- Number of Bit to be masked.
*   Return Value  : EquMaskValue_u32 :- masked value of size BitSize_u8.
*  ---------------------------------------------------------------------------*/
uint32_t GetEquMaskValue(uint8_t BitSize_u8)
{
	uint8_t MaskIndex_u8 = 0;
	uint32_t EquMaskValue_u32 = 0;
	for(MaskIndex_u8 = 0; MaskIndex_u8 < BitSize_u8 ; MaskIndex_u8++)
	{
		EquMaskValue_u32 = EquMaskValue_u32 + Power_Of_2(MaskIndex_u8);
	}

	return EquMaskValue_u32;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : Power_Of_2
*   Description   : The function provides the power of 2 based on the exponent value.
*   Parameters    : Exponent_u16 :- exponent value for the base.
*   Return Value  : value of 2 power exponent.
*  ---------------------------------------------------------------------------*/
uint32_t Power_Of_2(uint16_t Pow2Exponent_u16)
{
	uint32_t	Power2ofNum_u32 = 0;
	uint16_t	Pow2index_u16 = 0;
	
	Power2ofNum_u32 = 1;
	
	for(Pow2index_u16 = 1; Pow2index_u16 <= Pow2Exponent_u16; Pow2index_u16++)
	{
		Power2ofNum_u32 *= BASE_VALUE_2;
	}
	return Power2ofNum_u32;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : Power_Of_2
*   Description   : The function provides the power of 2 based on the exponent value.
*   Parameters    : Exponent_u16 :- exponent value for the base.
*   Return Value  : value of 2 power exponent.
*  ---------------------------------------------------------------------------*/
uint32_t Power_of_10(uint16_t Pow10Exponent_u16)
{
    uint32_t 	  Power10ofNum_u32 = 0;
	uint16_t	  Pow10Index_u16 = 0;
	
	Power10ofNum_u32 = 1;
	
	for(Pow10Index_u16 = 1; Pow10Index_u16 <= Power10ofNum_u32; Pow10Index_u16++)
	{
		Power10ofNum_u32 *= BASE_VALUE_10;
	}
	return Power10ofNum_u32;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
* Function Name: LowPassFilter
* Description  : The function implements the low pass filter.
* Arguments    : Input_u16 :- 
                 *Output_pu16 :-
* Return Value : uint16_t *Output_pu16
*  ---------------------------------------------------------------------------*/
uint16_t LowPassFilter(uint16_t Input_u16, uint16_t *Output_pu16, int16_t PosConstant_s16, int16_t NegConstant_s16)
{
	uint16_t DiffVar_u16 	= 0;
	uint16_t OffsetVal_u16  = 0;
	
	// If the current measeured value is not equal to the input
	if(Input_u16 != *Output_pu16)
	{
		// Compute the filter output by using the formula for the first order low pass filter
        	// y[i] = y[i-1] + alpha(x[i] - y[i-1])
		if(Input_u16 > *Output_pu16)
		{
			DiffVar_u16 = (uint16_t)(Input_u16 - *Output_pu16);
			
			if(PosConstant_s16 > 0)
			{
				OffsetVal_u16 = (uint16_t)(((uint32_t)DiffVar_u16 * (uint32_t)PosConstant_s16) >> 15);
									// [x]=[x]*[15]/[15]
			}
			else
			{
				;
			}
		}
		else
		{
			DiffVar_u16 = (uint16_t)(*Output_pu16 - Input_u16);
			
			if(NegConstant_s16 > 0)
			{
				OffsetVal_u16 = (uint16_t)(((uint32_t)DiffVar_u16 * (uint32_t)NegConstant_s16) >> 15);
									// [x]=[x]*[15]/[15]
			}
			else
			{
				;
			}
		}
		
		// If the step value is 0 then set it to 1
		if(OffsetVal_u16 == 0)
		{
			OffsetVal_u16 = 1;
		}
		else
		{
			;
		}
		
		if(Input_u16 > *Output_pu16)
		{
			// Input curve is moving up, hence step up the result.
			*Output_pu16 += OffsetVal_u16;
		}
		else
		{
			// Input curve is moving down, hence step down the result.
			*Output_pu16 -= OffsetVal_u16;
		}
	}
	else
	{
		;
	}
	
	return (*Output_pu16);
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
* Function Name: Differenceof_2_num
* Description  : The function provides difference of 2 numbers.
* Arguments    : Digit1_u32 :- Digit1
                 Digit2_u32 :- Digit2
* Return Value : Difference_u32 :- difference of two number.
*  ---------------------------------------------------------------------------*/
uint32_t Differenceof_2_num(uint32_t Digit1_u32,uint32_t Digit2_u32)
{
	uint32_t Difference_u32;
	if(Digit1_u32 > Digit2_u32)
	{
		Difference_u32 = Digit1_u32 - Digit2_u32;
	}
	else
	{
		Difference_u32 = Digit2_u32 - Digit1_u32;

	}
	return Difference_u32;
}

void Delay_ms(uint32_t delay)
{
    uint16_t i;
    for(i = 0; i < delay;i++);
}



float roundDifference(float difference)
{
    // Check if the absolute difference is greater than or equal to 1
    if (fabs(difference) >= 1.0f)
    {
        return roundf(difference); // Round to the nearest integer
    }
    else if (fabs(difference) >= 0.1f)
    {
        return roundf(difference * 10.0f) / 10.0f; // Round to the nearest 0.1
    }
	else
    {
        return 0.1f; // Return 0.1 for values smaller than 0.1
    }
}
/*---------------------- End of File -----------------------------------------*/