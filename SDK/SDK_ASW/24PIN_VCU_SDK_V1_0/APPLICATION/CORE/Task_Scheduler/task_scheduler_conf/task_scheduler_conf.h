/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : task_scheduler_conf.h
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

#ifndef TASK_SCHEDULER_CONF
#define TASK_SCHEDULER_CONF
/*******************************************************************************
 *  HEARDER FILE INCLUDES
 ******************************************************************************/
#include "app_typedef.h"
/*******************************************************************************
 *  FUNCTION POINTER DECLARATION
 ******************************************************************************/
typedef void (*Task_Fptr_t)(void);
/*******************************************************************************
 *  MACRO DEFNITION
 ******************************************************************************/
#define TS_INTERVAL 5 /* 5 millisec*/
#define TS_PERIODICTY(x) (x / TS_INTERVAL)
// #define TOTAL_TASK  6

#define INIT_VALUE  0u

/*******************************************************************************
 *  STRUCTURES, ENUMS and TYPEDEFS
 ******************************************************************************/
#pragma pack(1)
typedef struct
{
    const Task_Fptr_t Task_Fptr;
    const uint16_t Periodicity_u16;
    uint16_t TaskTimerCounter_u16;    
} TaskConf_St_t;
#pragma unpack

#pragma pack(1)
typedef struct
{
    void (*Task_Fptr)(void);    // Initialization function pointer
    uint8_t TaskExecutedFlag_u8;   // Flag to ensure it runs only once
} InitTaskConf_St_t;
#pragma unpack

typedef enum
{
    TASK_START_E = 0,
    TASK1_E = TASK_START_E,
     Task2_E,
     Task3_E,
     Task4_E,
    //  Task5_E,
    //  Task6_E,
    // Task7_E,
    TOTAL_TASK
} Task_En_t;

typedef enum
{
    INIT_TASK_START_E = 0,
    INIT_TASK1_E = TASK_START_E,
    //INIT_Task2_E,
    // INIT_Task3_E,  
    TOTAL_INIT_TASK,
}InitTask_En_t;
/*******************************************************************************
 *  EXTERN GLOBAL VARIABLES
 ******************************************************************************/
extern TaskConf_St_t TaskConf_St[TOTAL_TASK];
extern InitTaskConf_St_t InitTaskConf_St[];
extern const uint8_t Total_task_u8;
extern const uint8_t Total_Init_task_u8;
/*******************************************************************************
 *  EXTERN FUNCTION
 ******************************************************************************/

#endif /* TASK_SCHD_CONF */
/*---------------------- End of File -----------------------------------------*/