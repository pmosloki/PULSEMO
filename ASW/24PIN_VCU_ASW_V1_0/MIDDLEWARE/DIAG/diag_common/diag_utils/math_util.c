/*
 *-----------------------------------------------------------------------------
 *
 *  File Name       : math_util.c
 *
 *  @brief          : source file for math utility functions
 *-----------------------------------------------------------------------------
 *
 *  Description :
 *  The file contains the implementations of math functions that can be used by
 *  the application software.
.*
 *
 *-----------------------------------------------------------------------------
 *
 *  Revision History:
 *
 *  Version  Author   Date        Description
 *                  (DD/MM/YY)
 *
 *  1.0      Ambereen  11/09/08     Original
 *
 *-----------------------------------------------------------------------------
 */

/** include files **/
#include "math_util.h"


/**
 *  @brief     calculate additive check sum of the supplied array.  
 *  @param     length of the byte array and byte array.
 *  @return    additive checksum (32 bits)
 *
 */
uint32_t  CalcAdditiveChecksum (const uint32_t len_u32, const uint8_t* data_pu8)
{
    uint32_t sum_u32 = 0;
    uint32_t i = 0;
    for (i=0; i<len_u32; i++)
    {
        sum_u32 += data_pu8[i];
    }
    return sum_u32;
}

