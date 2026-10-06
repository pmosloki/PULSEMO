/******************************************************************************
 *    FILENAME    : crc_util.c
 *    DESCRIPTION : This file has the structure for generator polynomial and the CRC32/CRC8 calculation
 *                  function for specified memory blocks .
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
*/ 


/**
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
 
#include <stdint.h>
#include "crc_util.h" 
/**************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
 

/* This table has 256 divisor or generator polynomial which is used to calculate the CRC*/
/* Table is generated for the polynomial x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 0x04C11DB7*/ 


/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : crc16.
*   Description   : This function will calculate the crc.
*   Parameters    : data - on which crc has to be calcukated.
*	                length - size of the data.
*   Return Value  :UDS_Serv_resptype_En_t ( Type of response).
*******************************************************************************/
// uint16_t crc16(const uint8_t *data, uint32_t length) 
// {
//     uint16_t crc = INIT_CRC;
//     uint16_t i;
//     uint8_t j;
//     uint32_t K;
//       uint16_t crc_entry;
//     static uint16_t crc_table[256] = { 0 };
//     // Generate the CRC lookup table
//     if (!crc_table[1]) {
//         for ( i = 0; i < 256; i++) {
//             crc_entry = i << 8;
//             for (j = 0; j < 8; j++) {
//                 crc_entry = (crc_entry & 0x8000) ? (crc_entry << 1) ^ POLY : crc_entry << 1;
//             }
//             crc_table[i] = crc_entry;
//         }
//     }
    
//     for (K = 0; K < length; K++) {
//         crc = (crc << 8) ^ crc_table[(crc >> 8) ^ data[K]];
//     }
//     return crc;
// }
/* *****************************************************************************
 End of File
 */



