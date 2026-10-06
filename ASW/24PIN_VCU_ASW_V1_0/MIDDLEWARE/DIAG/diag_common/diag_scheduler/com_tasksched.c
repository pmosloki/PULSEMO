/******************************************************************************
 *    FILENAME    : com_ts.c
 *    DESCRIPTION : File contains the common declarations related to CIL layers.
 ******************************************************************************
 * Revision history
 *  
 * Ver   Author       Date               Description
 * 1     Sushil      27/10/2018		     Initial version
 ******************************************************************************
*/ 

/* Section: Included Files */
#include "com_tasksched.h"
#include "can_sched.h"
#include "diag_typedefs.h"
#include "diag_sys_conf.h"
#if(TRUE == DIAG_CONF_CANTP_SUPPORTED)
	#include "i15765.h"
#endif
#if (TRUE == DIAG_CONF_UDS_SUPPORTED)
	#include "uds_session.h"
#endif
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
	#include "fm.h"
#endif
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
	#include "j1939.h"
#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)
	#include "obd_session.h"
#endif
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

static bool FmInitSuccess_b = false;

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */


/**
 *  @function name : void Diag_TS_Init(void)
 *  @brief         : Function to call for initialisation of network layer,application layer and service distributor.
 *  @param         : None
 *  @return        : None
 *
 */
void Diag_TS_Init(void)
{
    CanSchedulerState_En = TX_ON_RX_ON_E;
    CIL_CAN_Init();
    CanSched_Init();

#if (TRUE == DIAG_CONF_CANTP_SUPPORTED)
    // Initialize network layer 
    DCAN_NL_Init();
     // Initialize application layer 
    DCAN_TP_Init();
#endif
#if (TRUE == DIAG_CONF_UDS_SUPPORTED)
    //Serv Distributor initialisation
    ServDistUDS_init();
#endif
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
    // FM_Init();
#endif
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
    J1939_Init();
#endif
#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)

    ServDistOBD_init();
#endif
}

/**
 *  @function name : void Diag_TS_FM_Init(void)
 *  @brief         : Function to call for initialisation of network layer,application layer and service distributor.
 *  @param         : None
 *  @return        : None
 *
 */
void Diag_TS_FM_Init(void)
{

#if(TRUE == DIAG_CONF_FM_SUPPORTED)
    FM_Init();
    FmInitSuccess_b = true;
#endif

}


/**
 *  FUNCTION NAME : Diag_TS_Proc_5ms
 *  @brief        : DCAN application cyclic process.This process must be called cyclically
 *                  normally every 10ms
 *  @param        : none
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_CO_PROC] @{SDD_I15765_DCAN_CO_PROC}
 *
 */
void Diag_TS_Proc_5ms (void)
{
    /* DCAN application process to call DCAN do
    changes such as change baud rate,exec function etc */
	uint32_t Time_MS_u32 = GET_TIME_MS();
	/* Schedule the CAN CIL Functions */
    CanSched_5ms();
    if(0 == (Time_MS_u32 % 5))
    {
#if(TRUE == DIAG_CONF_CANTP_SUPPORTED)
    /* ISO15765 Network layer process*/
    DCAN_NL_Proc_5ms();
    /* ISO15765 application layer process */
    DCAN_TP_Proc_5ms();
#endif
#if (TRUE == DIAG_CONF_UDS_SUPPORTED)
    /* ISO14229 Session Layer */
    uds_session_proc_5ms();
#endif
    }
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
    if(true  == FmInitSuccess_b)
    {
        if( 0 == (Time_MS_u32%50) )
        {
            FM_Sched_proc();
        }

    }
   
#endif
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)

    if( 0 == (Time_MS_u32%50) )
    {
        J1939_81_nmac_50ms();
        J1939_sched_50ms();
    }

#endif

#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)

    obd_session_proc_5ms();

#endif

    return;
}

/**
 *  FUNCTION NAME : Diag_TS_Proc_5ms
 *  @brief        : DCAN application cyclic process.This process must be called cyclically
 *                  normally every 10ms
 *  @param        : none
 *  @return       : none
 *
 *  @code tag     : @[CODE_I15765_DCAN_CO_PROC] @{SDD_I15765_DCAN_CO_PROC}
 *
 */
void Diag_TS_FM_Proc_5ms (void)
{
    /* DCAN application process to call DCAN do
    changes such as change baud rate,exec function etc */
	uint32_t Time_MS_u32 = GET_TIME_MS();
	/* Schedule the CAN CIL Functions */
    CanSched_5ms();
    if(0 == (Time_MS_u32 % 5))
    {
#if(TRUE == DIAG_CONF_CANTP_SUPPORTED)
    /* ISO15765 Network layer process*/
    DCAN_NL_Proc_5ms();
    /* ISO15765 application layer process */
    DCAN_TP_Proc_5ms();
#endif
#if (TRUE == DIAG_CONF_UDS_SUPPORTED)
    /* ISO14229 Session Layer */
    uds_session_proc_5ms();
#endif
    }
#if(TRUE == DIAG_CONF_FM_SUPPORTED)
    if(true  == FmInitSuccess_b)
    {
        if( 0 == (Time_MS_u32%50) )
        {
            FM_Sched_proc();
        }

    }
   
#endif
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)

    if( 0 == (Time_MS_u32%50) )
    {
        J1939_81_nmac_50ms();
        J1939_sched_50ms();
    }

#endif

#if(TRUE == DIAG_CONF_OBD2_SUPPORTED)

    obd_session_proc_5ms();

#endif

    return;
}

/* *****************************************************************************
 End of File
 */
