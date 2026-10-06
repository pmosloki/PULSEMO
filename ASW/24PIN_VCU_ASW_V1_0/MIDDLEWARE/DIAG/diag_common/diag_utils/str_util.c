/******************************************************************************
 *    FILENAME    : str_util.c
 *    DESCRIPTION : source file for string utility functions.
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
*/ 


 
/** include files **/
#include "str_util.h"
#include <stdbool.h>
#include "diag_sys_conf.h"
#include "diag_typedefs.h"
#include <stdlib.h>

/**************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */


/**
*
* @brief       : This function implements string concat function.
* @param       : Pointer to the destination address.
* @param       : Pointer to the source address.
* @param       : maximal destination buffer length
* @return      : actual string length
*/
uint16_t STR_Cat (uint8_t *dest_pv, const void *src_pv, uint16_t maxBufferLen_u16)
{
    uint8_t* dest_pu8 = (uint8_t*)dest_pv;
    uint8_t* src_pu8  = (uint8_t*)src_pv;
    uint16_t cntr_u16 = 0;
    
    //search the terminating "0" in the string, 256= response buffer length
    while (cntr_u16 < maxBufferLen_u16 - 1) 
    {
        if (*dest_pu8 == 0x0)
        {
            break;   
        }
        else 
        {
            dest_pu8++;  
            cntr_u16++;
        }                                     
    }       
  
    while (cntr_u16 < maxBufferLen_u16 - 1)
    {
        if (*src_pu8 == 0x0)
        {
            break;   
        }
        else 
        {
            *dest_pu8 = *src_pu8;
            src_pu8++;
            dest_pu8++;
            cntr_u16++;
        }                                     
    }
    
    //"0" - terminator
    *dest_pu8 = 0x0;  

    //return counter elements beginning with "0", add "1" 
    cntr_u16++;        
        
    return cntr_u16;
}
/**
*
* @brief        : This function implements memcpy function.
* @param void*  : Pointer to the destination address.
* @param uint32_t : Length.
* @param void*  : Pointer to the source address.
* @return       : void
*/
void CopyData (void *dest_pv, uint32_t len_u32, const void *src_pv)
{
    uint8_t* dest_pu8 = NULL;
    uint8_t* src_pu8  = NULL;
    dest_pu8 = (uint8_t*)dest_pv;
    src_pu8  = (uint8_t*)src_pv;
//    uint8_t i = 0;
    
    while (len_u32 > 0)
    {
//    	i = *src_pu8;
        *dest_pu8 = *src_pu8;
        src_pu8++;
        dest_pu8++;
        len_u32--;
    }
    return;
}

uint8_t CompareBuff(uint8_t const *Buff1_pu8,uint8_t const *Buff2_pu8,const uint32_t Len_u32)
{
    uint8_t CompState_u8 = true;
    uint32_t LoopCnt_u32 = 0;

    for(LoopCnt_u32 = 0;LoopCnt_u32<Len_u32;LoopCnt_u32++)
    {
        if(Buff1_pu8[LoopCnt_u32] != Buff2_pu8[LoopCnt_u32])
        {
            CompState_u8 = false;
            break;
        }
    }

    return CompState_u8;
}
//#if 1
void ReversData (uint32_t len_u32, const void *src_pv)
{
    uint8_t* src_pu8  = NULL;
    uint8_t i = 0;
    uint8_t k = 0;
    src_pu8  = (uint8_t*)src_pv;
    while (k < len_u32)
    {
    	i = *(src_pu8+len_u32-1);
        *(src_pu8+len_u32-1) = *(src_pu8+k);
        *(src_pu8+k) = i;
        len_u32--;
        k++;
    }
    return;
}
/* Function to reverse arr[] from start to end*/
void rvereseArray(uint8_t arr[], uint8_t start, uint8_t end)
{
	uint8_t temp;
    while (start < end)
    {
        temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;
        start++;
        end--;
    }
}

/**
*
* @brief        : This function implements memset function.
* @param uint8_t* : Pointer to the memory.
* @param uint8_t  : value to be set
* @param uint32_t : Length.
* @return void
*/
void SetMem (uint8_t *dest_pu8, uint8_t value_u8, uint32_t len_u32)
{

    while (len_u32 > 0)
    {
        *dest_pu8 = value_u8;
        dest_pu8++;
        len_u32--;
    }
    return;
}

/**
*
* @brief         : This function implements memcmp function.
* @param uint8_t*  : First buffer to compare.
* @param uint32_t  : number of bytes to compare
* @param uint8_t*  : Second buffer to compare
* @return bool : returns true if values are same else false.
*/
bool CmpData (const uint8_t *dest_pu8, uint32_t len_u32, const uint8_t *src_pu8)
{
    while (len_u32 > 0)
    {
        if (*dest_pu8 != *src_pu8)
        {
            return FALSE;
        }
        src_pu8++;
        dest_pu8++;
        len_u32--;
    }
    return TRUE;
}

/**
*
* @brief              : This function converts a given decimal string to an unsigned integer.
*                       The conversion stops at the first invalid character but returns the
*                       result converted so far.
*
* @param src_pv       : Pointer to decimal string
* @param value_p32    : Pointer for result 
*
* @return TRUE        : conversion was okay
*         FALSE       : invalid characters in source string
*/
bool STR_toUINT (const void *src_pv, uint32_t* value_p32)
{
    const uint8_t* src_pu8  = (const uint8_t*) src_pv;
    uint32_t result_u32 = 0;
    bool  ret_b = TRUE;    
    
    while (*src_pu8 != 0)
    {
        if ((*src_pu8 < '0') || (*src_pu8 > '9'))
        {
            // invalid character in string --> return error
            ret_b = FALSE;
            break;
        }
        
        result_u32 = result_u32 * 10 + *src_pu8 - '0';
        src_pu8++;
    }
    
    *value_p32 = result_u32;
    
    return ret_b;
}

/**

* @brief              : This function counts the number of characters in a given string.
* @param src_pv       : Pointer to string
* @return             : Number of characters in string
*/
uint32_t STR_len (const void *src_pv)
{
    const uint8_t* src_pu8  = (const uint8_t*) src_pv;
    uint32_t len_u32 = 0;
    
    while (src_pu8[len_u32] != 0)
    {
        len_u32++;
    }
    
    
    return len_u32;
}

/**
*
* @brief            : This function counts the number of characters in a given string.
*
* @param src_pv     : Pointer to string
*
* @return           : Number of characters in string
*/
void ReverseString (uint8_t * str_pu8, uint16_t len_u16)
{
    if ( len_u16 > 1 && len_u16 < 256 )
    {
        uint8_t src, dest, s_idx, d_idx;
        s_idx = 0;
        d_idx = (uint8_t) (len_u16-1);
        while( s_idx < d_idx ) 
        {
          src = *(str_pu8+s_idx );
          dest = *(str_pu8+d_idx );
          *(str_pu8+s_idx ) = src;
          *(str_pu8+d_idx ) = dest;
          s_idx++;
          d_idx--;
          
        }
    }
}

/* *****************************************************************************
 End of File
 */



