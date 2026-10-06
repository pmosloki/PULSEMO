/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : board_init.c
|    Project        : EMBDES_GSHIFTER
|    Description    : The file implements the board initialization.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 14/03/2022       Jeevan Jestin N             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"ext_eeprom.h"
#include "iso14229_serv27.h"
//#include"sys_utills.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/
#define EA_IIC_FAIL     1u
/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/
bool Ea_Access_Lock_b;
Ea_Result_En_t Ea_Result_En;
uint32_t Ea_Write_Cycletime_u32;
uint8_t Ea_Job_Completed_u8;
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/
static Ea_Data_St_t Ea_Data_aSt[QUEUE_SIZE];
Ea_Data_St_t Ea_Access_St;
Ea_Queue_St_t Ea_Queue_St;
/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
void Ea_Reqproc(void);
bool Is_Eaqueue_Full(void);
void Ea_Dequeue(void);
bool Is_Eaqueue_Empty(void);
void Ea_Enqueue(void);
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
uint32_t GetTimeDelayDiff(uint32_t ActualTime_u32,uint32_t CapturedTime_u32);

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void EepromInit(void)
{
    Ea_Queue_St.Data_pSt = Ea_Data_aSt;
    Ea_Queue_St.Front_u8 = QUEUE_START;
    Ea_Queue_St.Rear_u8 = QUEUE_START;
    return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void Ea_Scheduler(void)
{
    if(NULL != Ea_Queue_St.Data_pSt)
    {
        if ((EA_REQ_NULL == Ea_Result_En) && (!Is_Eaqueue_Empty()))
        {
            uint8_t Front_u8 = Ea_Queue_St.Front_u8;
            Ea_Result_En = EA_REQ_INPROGRESS_E;
            Ea_Access_St.Buff_Addr_pu8 = Ea_Queue_St.Data_pSt[Front_u8].Buff_Addr_pu8;
            Ea_Access_St.Ea_Addr_u32 = Ea_Queue_St.Data_pSt[Front_u8].Ea_Addr_u32;
            Ea_Access_St.Identifier_u32 = Ea_Queue_St.Data_pSt[Front_u8].Identifier_u32;
            Ea_Access_St.EaLen_u16 = Ea_Queue_St.Data_pSt[Front_u8].EaLen_u16;
            Ea_Access_St.Mode_u8 = Ea_Queue_St.Data_pSt[Front_u8].Mode_u8;

            Ea_Dequeue();
        }

        if ((EA_REQ_INPROGRESS_E == Ea_Result_En) && (false == Ea_Access_Lock_b))
        {
            Ea_Reqproc();
            if (EA_REQ_PASS_E == Ea_Result_En)
            {
                Ea_Result_En = EA_REQ_NULL;
                Ea_Req_CompleteCallback(Ea_Access_St.Identifier_u32, EA_REQ_PASS);
            }
            else if (EA_REQ_FAIL_E == Ea_Result_En)
            {
                Ea_Result_En = EA_REQ_NULL;
                Ea_Req_CompleteCallback(Ea_Access_St.Identifier_u32, EA_REQ_FAIL);
            }
            else
            {
            }
        }
        else if (true == Ea_Access_Lock_b)
        {
            if (GetTimeDelayDiff(GET_TIME_MS(), Ea_Write_Cycletime_u32) > EEPROM_CYCLETIME)
            {
                Ea_Access_Lock_b = false;
            }
        }
        else
        {
        }
    }


    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void Ea_Reqproc(void)
{
    uint8_t State_u8 = 0;
    bool Ea_Req_Complete_b = false;

    if ((NULL == Ea_Queue_St.Data_pSt)||(NULL == Ea_Access_St.Buff_Addr_pu8))
    {
        return;
    }

    if(EA_MODE_WRITE == Ea_Access_St.Mode_u8)
    {
        if(0u == (Ea_Access_St.Ea_Addr_u32%EEPROM_PAGE_SIZE))
        {
            if(Ea_Access_St.EaLen_u16 > EEPROM_PAGE_SIZE)
            {
                Ea_Access_Lock_b = true;
                State_u8 = EepromWrite(Ea_Access_St.Ea_Addr_u32,Ea_Access_St.Buff_Addr_pu8,EEPROM_PAGE_SIZE);
                Ea_Write_Cycletime_u32 = GET_TIME_MS();
                Ea_Access_St.EaLen_u16 -= EEPROM_PAGE_SIZE;
                Ea_Access_St.Ea_Addr_u32 += EEPROM_PAGE_SIZE;
                Ea_Access_St.Buff_Addr_pu8 += EEPROM_PAGE_SIZE;
            }
            else
            {
                Ea_Access_Lock_b = true;
                State_u8 = EepromWrite(Ea_Access_St.Ea_Addr_u32,Ea_Access_St.Buff_Addr_pu8,Ea_Access_St.EaLen_u16);
                Ea_Write_Cycletime_u32 = GET_TIME_MS();
                Ea_Access_St.EaLen_u16 = 0u;
                Ea_Req_Complete_b = true;
            }
        }
        else
        {
            uint8_t Ea_Size_u8 = (uint8_t)(EEPROM_PAGE_SIZE - (Ea_Access_St.Ea_Addr_u32 %EEPROM_PAGE_SIZE));
            if(Ea_Access_St.EaLen_u16 > Ea_Size_u8)
            {
                Ea_Access_Lock_b = true;
                State_u8 = EepromWrite(Ea_Access_St.Ea_Addr_u32,Ea_Access_St.Buff_Addr_pu8,Ea_Size_u8);
                Ea_Write_Cycletime_u32 = GET_TIME_MS();
                Ea_Access_St.EaLen_u16 -= Ea_Size_u8;
                Ea_Access_St.Ea_Addr_u32 += Ea_Size_u8;
                Ea_Access_St.Buff_Addr_pu8 += Ea_Size_u8;
            }
            else
            {
                Ea_Access_Lock_b = true;
                State_u8 = EepromWrite(Ea_Access_St.Ea_Addr_u32,Ea_Access_St.Buff_Addr_pu8,Ea_Size_u8);
                Ea_Write_Cycletime_u32 = GET_TIME_MS();
                Ea_Access_St.EaLen_u16 = 0u;
                Ea_Req_Complete_b = true;
            }

        }
    }
    else if(EA_MODE_READ == Ea_Access_St.Mode_u8)
    {
        if(Ea_Access_St.EaLen_u16 > EEPROM_READ_ONESHOT)
        {
            State_u8 = EepromRead(Ea_Access_St.Ea_Addr_u32,Ea_Access_St.Buff_Addr_pu8,EEPROM_READ_ONESHOT);
            Ea_Access_St.Ea_Addr_u32 += EEPROM_READ_ONESHOT;
            Ea_Access_St.Buff_Addr_pu8 += EEPROM_READ_ONESHOT;
            Ea_Access_St.EaLen_u16 -= EEPROM_READ_ONESHOT;
        }
        else
        {
            State_u8 = EepromRead(Ea_Access_St.Ea_Addr_u32,Ea_Access_St.Buff_Addr_pu8,Ea_Access_St.EaLen_u16);
            Ea_Req_Complete_b = true;
            Ea_Access_St.EaLen_u16 = 0u;
        }
    }
    else
    {
    }

    if(EA_IIC_FAIL == State_u8)
    {
        Ea_Result_En = EA_REQ_FAIL_E;
    }
    else if(true == Ea_Req_Complete_b)
    {
        Ea_Result_En = EA_REQ_PASS_E;
    }
    else
    {
    }

    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
bool Read_Eeprom(uint32_t Identifier_u32, uint32_t Mem_Addr_u32, uint8_t* Buff_Addr_pu8,uint16_t Read_Len_u16)
{
    bool Enqueue_Success_b = false;

    if((!Is_Eaqueue_Full())&&(NULL != Ea_Queue_St.Data_pSt))
    {
        uint8_t Rear_u8 = Ea_Queue_St.Rear_u8;
        Enqueue_Success_b = true;
        Ea_Queue_St.Data_pSt[Rear_u8].Buff_Addr_pu8 = Buff_Addr_pu8;
        Ea_Queue_St.Data_pSt[Rear_u8].Ea_Addr_u32 = Mem_Addr_u32;
        Ea_Queue_St.Data_pSt[Rear_u8].EaLen_u16 = Read_Len_u16;
        Ea_Queue_St.Data_pSt[Rear_u8].Identifier_u32 = Identifier_u32;
        Ea_Queue_St.Data_pSt[Rear_u8].Mode_u8 = EA_MODE_READ;
        Ea_Enqueue();
    }
    else
    {
        Enqueue_Success_b = false;
    }

    return Enqueue_Success_b;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
bool Write_Eeprom(uint32_t Identifier_u32, uint32_t Mem_Addr_u32, uint8_t* Buff_Addr_pu8,uint16_t Read_Len_u16)
{
    bool Enqueue_Success_b = false;
    if((!Is_Eaqueue_Full())&&(NULL != Ea_Queue_St.Data_pSt))
    {
        uint8_t Rear_u8 = Ea_Queue_St.Rear_u8;
        Enqueue_Success_b = true;
        Ea_Queue_St.Data_pSt[Rear_u8].Buff_Addr_pu8 = Buff_Addr_pu8;
        Ea_Queue_St.Data_pSt[Rear_u8].Ea_Addr_u32 = Mem_Addr_u32;
        Ea_Queue_St.Data_pSt[Rear_u8].EaLen_u16 = Read_Len_u16;
        Ea_Queue_St.Data_pSt[Rear_u8].Identifier_u32 = Identifier_u32;
        Ea_Queue_St.Data_pSt[Rear_u8].Mode_u8 = EA_MODE_WRITE;
        Ea_Enqueue();
    }
    else
    {
        Enqueue_Success_b = false;
    }

    return Enqueue_Success_b;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
bool Is_Eaqueue_Full(void)
{
    bool Status_b = false;

    if(((uint8_t)(QUEUE_END) == Ea_Queue_St.Rear_u8)&&
        (QUEUE_START) == Ea_Queue_St.Front_u8)
    {
        /* Queue is full */
        Status_b = true;
    }
    else if((Ea_Queue_St.Rear_u8+1u) == Ea_Queue_St.Front_u8)
    {
        /* Queue is full */
        Status_b = true;
    }
    else
    {
        /* Queue is not full */
        Status_b = false;
    }

    return Status_b;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
bool Is_Eaqueue_Empty(void)
{
    bool Status_b = false;

    if(Ea_Queue_St.Rear_u8 == Ea_Queue_St.Front_u8)
    {
        /* Queue is empty */
        Status_b = true;
    }
    else
    {
        /* Queue is not empty */
        Status_b = false;
    }
    return Status_b;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void Ea_Dequeue(void)
{
    if(Ea_Queue_St.Rear_u8 != Ea_Queue_St.Front_u8)
    {
        if((uint8_t)QUEUE_END == Ea_Queue_St.Front_u8)
        {
            Ea_Queue_St.Front_u8 = QUEUE_START;
        }
        else
        {
            Ea_Queue_St.Front_u8++;
        }
    }

    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void Ea_Enqueue(void)
{
    if ((uint8_t)QUEUE_END == Ea_Queue_St.Rear_u8)
    {
        Ea_Queue_St.Rear_u8 = QUEUE_START;
    }
    else
    {
        Ea_Queue_St.Rear_u8++;
    }

    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NONE
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint32_t GetTimeDelayDiff(uint32_t ActualTime_u32,uint32_t CapturedTime_u32)
{
    uint32_t TimeDiff_u32 = 0;
    if(ActualTime_u32 >= CapturedTime_u32)
    {
        TimeDiff_u32 = ActualTime_u32 - CapturedTime_u32;
    }
    else
    {
        TimeDiff_u32 = (GET_TIME_MAXVALUE() - CapturedTime_u32) +  ActualTime_u32;
    }

    return TimeDiff_u32;
}
/*---------------------- End of File -----------------------------------------*/
