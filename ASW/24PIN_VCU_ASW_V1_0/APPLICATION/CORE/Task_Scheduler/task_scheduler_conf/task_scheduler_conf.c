/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : task_scheduler_conf.c
|    Project        :  
|    Description    : The file implements .
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "task_scheduler_conf.h"
#include "com_tasksched.h"
#include "data_acquire.h"
#include "databank.h"
#include "nvm_data.h"
#include "ext_eeprom.h"
//#include "app_init.h"
#include "data_acquire.h"
#include "AppTest.h"

/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION
 ******************************************************************************/
const uint8_t Total_task_u8=TOTAL_TASK;
const uint8_t Total_Init_task_u8 = TOTAL_INIT_TASK;
TaskConf_St_t TaskConf_St[TOTAL_TASK] =
{	
    // {Nvm_Scheduler,          TS_PERIODICTY(10),  INIT_VALUE},
    // {Ea_Scheduler,           TS_PERIODICTY(10),  INIT_VALUE},
    // {ReportDTCStatus        ,TS_PERIODICTY(50),  INIT_VALUE},
    {Test_Adc               ,TS_PERIODICTY(50),  INIT_VALUE},
    {Test_GpioInput         ,TS_PERIODICTY(50),  INIT_VALUE},
    {Test_GpioOutput        ,TS_PERIODICTY(50),  INIT_VALUE},
    {CAN_functionality        ,TS_PERIODICTY(50),  INIT_VALUE},
    {fun3,TS_PERIODICTY(50),  INIT_VALUE},
    {fun4,TS_PERIODICTY(50),  INIT_VALUE},
    
};

 InitTaskConf_St_t InitTaskConf_St[TOTAL_INIT_TASK] =
 {
     {Test_GpioInput, 0},  // Initialization task for GPIO input, flag = 0
 };

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : NONE
 *   Description   : NONE
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/

/*---------------------- End of File -----------------------------------------*/