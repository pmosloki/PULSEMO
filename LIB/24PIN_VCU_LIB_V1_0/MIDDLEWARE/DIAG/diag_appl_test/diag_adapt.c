/******************************************************************************
 *    FILENAME    : diag_adapt.c
 *    DESCRIPTION : Application adapter interface file for DIAG Stacks.
 ******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki       10/01/2019		   Initial version
 ******************************************************************************
*/ 

/* ************************************************************************** */
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "diag_typedefs.h"
#include "diag_adapt.h" 
#include "diag_sys_conf.h"
#if(TRUE == DIAG_CONF_UDS_SUPPORTED)
	#include "uds_conf.h"
#endif
#if(DIAG_CONF_INTERRUPTS == TRUE)

#endif
#include "can_if.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

uint16_t Dist_Drvn = 0;
uint8_t COMMUNICATION_Status_au8			 	[1 ];

void Diag_Disable_Interrupts()
{
#if(DIAG_CONF_INTERRUPTS == TRUE)
	DISABLE_INTERRUPTS();
#endif
}

void Diag_Restore_Interrupts()
{
#if(DIAG_CONF_INTERRUPTS == TRUE)
	ENABLE_INTERRUPTS();
#endif
}

uint8_t CNTRL_MODULE_Config_Type_au8		 	[1 ];
uint8_t COMMUNICATION_Status_au8			 	[1 ];

uint8_t MILStatus_au8		 	                [4 ] = {1,2,3,4};
uint8_t ENGINE_Speed_au8		        	 	[2 ] = {5,6};
uint8_t VEHICLE_Speed_au8					 	[1 ] = {7};
uint8_t ENGINE_Coolant_Temp_au8			     	[1 ] = {8};
uint8_t Throttle_Position_au8                  	[1 ] = {9};
