/*******************************************************************************
 *    FILENAME    : j1939_sched.c
 *
 *    @brief       : Contains J1939 sched. functions         
 *                  
 * 
 *
 *******************************************************************************
 * Revision history
 * 
 * Ver Author       Date       Description
 * 1   Sloki
 *******************************************************************************
*/

/*
 *******************************************************************************
 *    Includes
 *******************************************************************************
*/
#include "j1939.h"
#include "j1939_conf.h"
#include "j1939_73_dmx.h"
#include "j1939_81_nmac.h"
#include "J1939_sched.h"
//#include "candatarx_evcv.h"
/*
 *******************************************************************************
 *    Defines
 *******************************************************************************
*/

/*
 *******************************************************************************
 *    Variables
 *******************************************************************************
*/


/*
 *******************************************************************************
 *    Function definitions
 *******************************************************************************
*/
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
/**
 *  @brief        : initializes the scheduler variables
 *  @param        : none
 *  @return       : none   
*/
void J1939_Init(void)
{
    /* Initialize transport layer parameters */
    J1939_tp_init();  
    
    /* Initialize NMAC*/
    J1939_81_NMAC_Init();

    J1939_tp_RegDMRxClbk(J1939_DM1EMSMultiFrameMsgCallbck);
    return;
}

/**
*  @brief        : The task is called every 50ms from the task scheduler. 
*  @param        : none
*  @return       : none 
*                  
*/
void J1939_sched_50ms (void)
{
    /*Check Is Address Claim done?*/
    if(J1939_NMAC_DONE_E == J1939_81_NMAC_State_En)
    {
        J1939_DM1_schedTask ();
    }
    return;
}
#endif


