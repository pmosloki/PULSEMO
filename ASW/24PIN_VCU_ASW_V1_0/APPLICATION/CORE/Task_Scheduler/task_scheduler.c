/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : task_scheduler.c
|    Project        :
|    Description    : The file schedule the task.
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
#include "task_scheduler.h"
#include "iso14229_serv10.h"
#include "iso14229_serv11.h"
#include "task_scheduler_conf.h"
#include "iso14229_serv31.h"



/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION
 ******************************************************************************/
uint16_t TaskTimerCounter[TOTAL_TASK] = {0};
static bool TaskSchdlrExit_b = false;
bool _5msTaskSchdlr_b = false;
uint16_t TScount_u16 = CLEAR;
uint32_t Counter_u32 = 0;
uint32_t Counter_5ms_u32 = 0;
uint8_t EcuStatus_u8;
bool InitTask_b = false;
uint8_t j;
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
static void TS_PowerOff(void);
void TasksScheduler_Exit(void);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : TaskSchedulerStart
 *   Description   : The function schedule the task to be executed.
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/

void TaskSchedulerStart(void)
{
    static uint8_t FM_Init_b = false;
    while (!TaskSchdlrExit_b)
    {
        while (_5msTaskSchdlr_b)
        {
            _5msTaskSchdlr_b = false;
            /* 5ms tasks */
            ReadInputSignal();
            WriteOutputSignals();           
            Nvm_Scheduler();
            Ea_Scheduler();
       
            

            // /* 50ms tasks */
            // if (0 == (GET_TIME_MS() % 50u))
            // {
            //     ReportDTCStatus();
            // }         

            if (false == FM_Init_b)
            {
                Diag_TS_FM_Proc_5ms();
                if (true == Is_Nvm_Idle())
                {
                    FM_Init_b = true;
                    CANinit();   
                    Diag_TS_FM_Init();
                    UDS_Serv_init();
                    
                }
            }
            else
            {
                Diag_TS_Proc_5ms();
            }

            if (HardReset_b == true)
            {
                Counter_5ms_u32++;
                if (Counter_5ms_u32 == 20)
                {
                    Counter_5ms_u32 = 0;
                    if (true == Is_Nvm_Idle())
                    {
                        TS_PowerOff();
                        TasksScheduler_Exit();
                        DI();
                    }
                    break;
                }
            }


             if (InitTask_b == false)
             {

                 for (j = 0; j < Total_Init_task_u8; j++)
                 {
                     if (InitTaskConf_St[j].TaskExecutedFlag_u8 == 0 && InitTaskConf_St[j].Task_Fptr)
                     {
                         // Execute the initialization task
                         InitTaskConf_St[j].Task_Fptr();

                         // Set the flag to prevent it from running again
                         InitTaskConf_St[j].TaskExecutedFlag_u8 = 1;
                     }
                 }
                 InitTask_b = true;
             }

            for (TScount_u16 = 0; TScount_u16 < Total_task_u8; TScount_u16++)
            {
                // Check if it's time to execute the task
                if (TaskConf_St[TScount_u16].TaskTimerCounter_u16 % TaskConf_St[TScount_u16].Periodicity_u16 == 0)
                {
                    if (TaskConf_St[TScount_u16].Task_Fptr)
                    {
                        // Execute the task function
                        TaskConf_St[TScount_u16].Task_Fptr();
                    }

                    // Reset the task's timer counter
                    TaskConf_St[TScount_u16].TaskTimerCounter_u16 = 0;
                }

                // Increment the task's timer counter for the next cycle
                TaskConf_St[TScount_u16].TaskTimerCounter_u16++;
            }


        }
    }
    return;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : TS_PowerOff
 *   Description   : This function stops the tasks and try to shutdown the ECU
 *					or put it into sleep.
 *   Parameters    : None
 *   Return Value  : None
 *******************************************************************************/
static void TS_PowerOff(void)
{
    WPROTR.PROTCMD0 = 0x000000A5;

    RESCTL.SWRESA = 0x1;

    RESCTL.SWRESA = ~0x01;

    RESCTL.SWRESA = 0x1;

    return;
}

void TasksScheduler_Exit(void)
{
    TaskSchdlrExit_b = true;
    return;
}

/*---------------------- End of File -----------------------------------------*/