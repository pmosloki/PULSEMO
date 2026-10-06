/****************************************************************************************
*    FILENAME    : j1939_71.c
*
*    DESCRIPTION : File contains J1939-71 related functionalities. They are periodic in nature. 
*
*    $Id         : $
*
*****************************************************************************************
* Revision history
*
* Ver Author         Date          Description
* 1   Sloki
* 2   Sloki         5/28/2018     Fix for single frame DMx response.
*****************************************************************************************
*/

/*
******************************************************************************
*    Includes
******************************************************************************
*/
#include "can_sched_conf.h" 
#include "j1939.h"   
#include "j1939_conf.h"                                            
#include "math_util.h"
#include "cil_can_conf.h"
#include "diag_sys_conf.h"

/************************************/
/*         module variables         */
/************************************/



/************************************/
/*         Global variables         */
/************************************/


/*****************************************/
/*         module functions prototype    */
/*****************************************/


/*****************************************/
/*         Global functions definitions  */
/*****************************************/
/**
* @brief	: J1939 TEST1 Transmit message. The frame is sent every 50ms rate.
* @param	: none,
* @return	: none.
*/
void J1939_71_TX_TEST1 (void)
{
    uint16_t MAP_COM_u16 = 0;  // Test variable
    
    CAN_MessageFrame_St_t Can_Applidata_St;//CAN_MessageFrame_St_t
    Can_Applidata_St.DataLength_u8 = 0x08;     
    
        
    Can_Applidata_St.DataBytes_au8[0] = SIGNAL_NA;
      
    /* Byte 1,2: The desired absolute intake manifold pressure of the engine. */
    /* SPN: 1692 */
    /* factor: 0.1 kPa/bit , offset: 0 kPa/bit   */
    /* 0..6425.5 kPa, 255 = Signal not available */
    Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(MAP_COM_u16 & 0xFF);
    Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((MAP_COM_u16 >> 8) & 0xFF);
    Can_Applidata_St.DataBytes_au8[3] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[4] = SIGNAL_NA; 
    Can_Applidata_St.DataBytes_au8[5] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[6] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[7] = SIGNAL_NA;
    Can_Applidata_St.MessageType_u8 = EXT_E;
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
   Can_Applidata_St.ID_u32 = CIL_CAN_GetID(CIL_J1939_71_TEST1_TX_E) + J1939_NMAC_ECU_ClaimedAddr_u8;
   CIL_CAN_Tx_DynamicMsg(CIL_J1939_71_TEST1_TX_E, Can_Applidata_St);
#endif

}

/**
* @brief	: J1939 TEST2 Transmit message. The frame is sent every 100 ms rate.
* @param	: none,
* @return	: none.
*/
void J1939_71_TX_TEST2 (void)
{
    uint16_t MAP_COM_u16 = 0;  // Test variable
   
    CAN_MessageFrame_St_t Can_Applidata_St;//CAN_MessageFrame_St_t
    Can_Applidata_St.DataLength_u8 = 0x08;     
    
        
    Can_Applidata_St.DataBytes_au8[0] = SIGNAL_NA;
      
    /* Byte 1,2: The desired absolute intake manifold pressure of the engine. */
    /* SPN: 1692*/
    /* factor: 0.1 kPa/bit , offset: 0 kPa/bit   */
    /* 0..6425.5 kPa, 255 = Signal not available */
    Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(MAP_COM_u16 & 0xFF);
    Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((MAP_COM_u16 >> 8) & 0xFF);
    Can_Applidata_St.DataBytes_au8[3] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[4] = SIGNAL_NA; 
    Can_Applidata_St.DataBytes_au8[5] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[6] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[7] = SIGNAL_NA;
    Can_Applidata_St.MessageType_u8 = EXT_E;
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
//    Can_Applidata_St.ID_u32 = CIL_CAN_GetID(CIL_J1939_71_TEST2_TX_E) + J1939_NMAC_ECU_ClaimedAddr_u8;
//    CIL_CAN_Tx_DynamicMsg(CIL_J1939_71_TEST2_TX_E, Can_Applidata_St);
#endif

}

/**
* @brief	: J1939 TEST1 Transmit message. The frame is sent every 500ms rate.
* @param	: none,
* @return	: none.
*/
void J1939_71_TX_TEST3 (void)
{
    uint16_t MAP_COM_u16 = 0;  // Test variable
    
    CAN_MessageFrame_St_t Can_Applidata_St;//CAN_MessageFrame_St_t
    Can_Applidata_St.DataLength_u8 = 0x08;     
    
        
    Can_Applidata_St.DataBytes_au8[0] = SIGNAL_NA;
      
    /* Byte 1,2: The desired absolute intake manifold pressure of the engine. */
    /* SPN: 1692*/
    /* factor: 0.1 kPa/bit , offset: 0 kPa/bit   */
    /* 0..6425.5 kPa, 255 = Signal not available */
    Can_Applidata_St.DataBytes_au8[1] = (uint8_t)(MAP_COM_u16 & 0xFF);
    Can_Applidata_St.DataBytes_au8[2] = (uint8_t)((MAP_COM_u16 >> 8) & 0xFF);
    Can_Applidata_St.DataBytes_au8[3] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[4] = SIGNAL_NA; 
    Can_Applidata_St.DataBytes_au8[5] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[6] = SIGNAL_NA;
    Can_Applidata_St.DataBytes_au8[7] = SIGNAL_NA;
    Can_Applidata_St.MessageType_u8 = EXT_E;
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
//    Can_Applidata_St.ID_u32 = CIL_CAN_GetID(CIL_J1939_71_TEST3_TX_E) + J1939_NMAC_ECU_ClaimedAddr_u8;
//    CIL_CAN_Tx_DynamicMsg(CIL_J1939_71_TEST3_TX_E, Can_Applidata_St);
#endif

}

