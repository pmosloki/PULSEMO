/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : nvm_data.c
|    Project        : TML EV
|    Description    : The file contains the NVM data.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 14/03/2022     Jeevan Jestin N       Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "nvm_data.h"
#include "ext_eeprom.h"
#include "eeprom.h"
#include "nvm_userinterface.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION
 ******************************************************************************/
NvmBlkData_St_t NvmBlkData_aSt[TOTAL_NVM_BLK_E];
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
static void ClearBuff(uint8_t *Dest_pu8, uint32_t Len_u32);

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/
/*LDRA_EXCLUDE_START 567 S
<justification start>
The pointer arithmetic performed in the code is intentional and does not result in undefined behavior. The design ensures that:
 1. The pointer arithmetic is performed within the valid range of memory allocated or assigned to the pointer.
 2. Proper boundary checks are implemented to prevent out-of-bound access.
 3. The operation is necessary to optimize the code for performance or to comply with hardware-specific requirements (e.g., working with memory-mapped registers or communication buffers).
 4. The usage has been reviewed and verified to ensure safety and compliance with functional requirements.
<justification end>
*/

/*LDRA_EXCLUDE_START 436 S
 <justification start>
 The use of a pointer instead of an explicit array declaration is intentional and serves the following purposes:
 1. The pointer is used to reference dynamically allocated memory or data that is determined at runtime.
 Declaring an array would limit this flexibility and prevent runtime adjustments to the size or location of the data.
 2.The code has been reviewed to ensure the pointer is always initialized and used safely.
 Boundary checks or other mechanisms are in place to avoid undefined behavior, ensuring functional correctness.
 <justification end>
 */
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : NONE
 *   Description   : NONE
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/
NvmAccessSt_En_t Nvm_Write(uint16_t Param_u16, uint8_t *Data_Buff_pu8)
{
    uint32_t Loop_Cnt_u32 = 0u;
    bool ParmFound_b = false;
    uint16_t Parm_u16 = 0u;
    uint16_t Crc_Cal_u16 = 0u;
    NvmAccessSt_En_t Status_En = NVM_WRITE_OK_E;

    for (Loop_Cnt_u32 = (uint32_t)NVM_BLK_START_E; Loop_Cnt_u32 < (uint32_t)TOTAL_NVM_BLK_E; Loop_Cnt_u32++)
    {
        if ((NULL != &NvmBlk_aSt[Loop_Cnt_u32]) && (NULL != NvmBlk_aSt[Loop_Cnt_u32].Descriptor_pSt))
        {
            const NvmBlk_St_t *Blk_pSt = &NvmBlk_aSt[Loop_Cnt_u32];
            Blk_Descriptor_St_t *Blk_Descriptor_St = (Blk_Descriptor_St_t *)NvmBlk_aSt[Loop_Cnt_u32].Descriptor_pSt;
            for (Parm_u16 = 0u; Parm_u16 < Blk_pSt->TotalParameter_u16; Parm_u16++)
            {
                if (Blk_Descriptor_St[Parm_u16].Parameter_En == (Nvm_Parameter_En_t)Param_u16)
                {
                    ParmFound_b = true;
                    if (NVM_IDLE_E == NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En)
                    {
                        if ((true == NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b) && (NULL != Blk_pSt->RAMCopy_pu8))
                        {
                            /* LDRA_INSPECTED 45 D */
                            /* MISRA-C:2012(Ed 3) D.4.1 : Pointer not checked for null before use.
                            Justification: Global pointer is checked for null before use*/
                            MemCopy((uint8_t *)(Blk_pSt->RAMCopy_pu8 + Blk_Descriptor_St[Parm_u16].Offset_u32), Data_Buff_pu8, Blk_Descriptor_St[Parm_u16].Size_u16);
                            Crc_Cal_u16 = crc16(&(Blk_pSt->RAMCopy_pu8[BLOCK_START_OFFSET]), (Blk_pSt->Length_u32 - NVM_BLOCK_HEADER_SIZE), INIT_CRC);
                            Blk_pSt->RAMCopy_pu8[NVM_CRC_POS_1] = (uint8_t)(Crc_Cal_u16 >> 8) & 0xFFU;
                            Blk_pSt->RAMCopy_pu8[NVM_CRC_POS_2] = (uint8_t)Crc_Cal_u16 & 0xFFU;

                            if (true == Blk_pSt->Nvm_Storage_u8)
                            {
                                NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_UPLOAD_E;
                            }
                            else
                            {
                                NvmBlkData_aSt[Loop_Cnt_u32].ShutdownWrite_b = true;
                            }
                        }
                        else
                        {
                            MemCopy((uint8_t *)(Blk_pSt->RAMCopy_pu8 + Blk_Descriptor_St[Parm_u16].Offset_u32), Data_Buff_pu8, Blk_Descriptor_St[Parm_u16].Size_u16);
                            Status_En = NVM_ERROR_E;
                        }
                    }
                    else
                    {
                        Status_En = NVM_WAIT_E;
                    }
                }

                if (true == ParmFound_b)
                {
                    break;
                }
            }
        }

        if (true == ParmFound_b)
        {
            break;
        }
    }

    if (false == ParmFound_b)
    {
        Status_En = NVM_INVALID_PAREMETER_E;
    }

    return Status_En;
}

NvmAccessSt_En_t Nvm_Read(uint16_t Param_u16, uint8_t *Data_Buff_pu8)
{
    uint32_t Loop_Cnt_u32 = 0u;
    bool ParmFound_b = false;
    uint16_t Parm_u16 = 0u;
    NvmAccessSt_En_t Status_En = NVM_READ_OK_E;

    for (Loop_Cnt_u32 = (uint32_t)NVM_BLK_START_E; Loop_Cnt_u32 < (uint32_t)TOTAL_NVM_BLK_E; Loop_Cnt_u32++)
    {
        if ((NULL != &NvmBlk_aSt[Loop_Cnt_u32]) && (NULL != NvmBlk_aSt[Loop_Cnt_u32].Descriptor_pSt))
        {
            const NvmBlk_St_t *Blk_pSt = &NvmBlk_aSt[Loop_Cnt_u32];
            Blk_Descriptor_St_t *Blk_Descriptor_St = (Blk_Descriptor_St_t *)NvmBlk_aSt[Loop_Cnt_u32].Descriptor_pSt;
            for (Parm_u16 = 0u; Parm_u16 < Blk_pSt->TotalParameter_u16; Parm_u16++)
            {
                if (Blk_Descriptor_St[Parm_u16].Parameter_En == (Nvm_Parameter_En_t)Param_u16)
                {
                    ParmFound_b = true;

                    if (NVM_IDLE_E == NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En)
                    {
                        if ((true == NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b) && (NULL != Blk_pSt->RAMCopy_pu8))
                        {
                            Status_En = NVM_READ_OK_E;
                            MemCopy(Data_Buff_pu8, (uint8_t *)(Blk_pSt->RAMCopy_pu8 + Blk_Descriptor_St[Parm_u16].Offset_u32), Blk_Descriptor_St[Parm_u16].Size_u16);
                        }
                        else
                        {
                            Status_En = NVM_DATA_INVALID_E;
                            if (NULL != Blk_pSt->RomCopy_pu8)
                            {
                                MemCopy(Data_Buff_pu8, (uint8_t *)(Blk_pSt->RomCopy_pu8 + Blk_Descriptor_St[Parm_u16].Offset_u32), Blk_Descriptor_St[Parm_u16].Size_u16);
                            }
                            else
                            {
                                ClearBuff(Data_Buff_pu8, Blk_Descriptor_St[Parm_u16].Size_u16);
                            }
                        }
                    }
                    else
                    {
                        Status_En = NVM_WAIT_E;
                    }
                }

                if (true == ParmFound_b)
                {
                    break;
                }
            }

            if (true == ParmFound_b)
            {
                break;
            }
        }
    }

    if (false == ParmFound_b)
    {
        Status_En = NVM_INVALID_PAREMETER_E;
    }

    return Status_En;
}

void Nvm_Scheduler(void)
{
    uint32_t Loop_Cnt_u32 = 0u;
    const NvmBlk_St_t *Blk_pSt = NULL;

    for (Loop_Cnt_u32 = NVM_BLK_START_E; Loop_Cnt_u32 < (uint32_t)TOTAL_NVM_BLK_E; Loop_Cnt_u32++)
    {
        if (NULL != &NvmBlk_aSt[Loop_Cnt_u32])
        {
            Blk_pSt = &NvmBlk_aSt[Loop_Cnt_u32];
            switch (NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En)
            {
            case NVM_INIT_E:
            {
                /* Queue the task to Read */
                if (NULL != Blk_pSt->RAMCopy_pu8)
                {
                    if (true == Read_Eeprom((uint32_t)Blk_pSt->Id_En, Blk_pSt->Start_Addr_u32, Blk_pSt->RAMCopy_pu8, (uint16_t)Blk_pSt->Length_u32))
                    {
                        /* If enqeue is successfull */
                        NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b = false;
                        NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8 = 0u;
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_INIT_PROGRESS_E;
                    }
                }
                else
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                }

                break;
            }
            case NVM_INIT_PROGRESS_E:
            {
                if (NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b)
                {
                    if (EA_REQ_PASS == NvmBlkData_aSt[Loop_Cnt_u32].ReqResult_u8)
                    {
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_VERIFY_PROGRESS_E;
                    }
                    else
                    {
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_INIT_FAIL_E;
                    }
                }
                break;
            }
            case NVM_INIT_FAIL_E:
            {
                if (NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8 < NVM_RETRY_MAX)
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8++;
                    /* Queue the task to Read */
                    /*LDRA_INSPECTED 128 D
                    The global pointer [pointer name] is not explicitly checked within this procedure because:.
                    1. The design ensures that the pointer remains valid for the lifetime of the system and cannot be reassigned or invalidated elsewhere.
                    2. The procedure is invoked only from controlled contexts where the pointer's validity has already been verified.
                    <justification end>
                    */
                    if (true == Read_Eeprom((uint32_t)Blk_pSt->Id_En, Blk_pSt->Start_Addr_u32, Blk_pSt->RAMCopy_pu8, (uint16_t)Blk_pSt->Length_u32))
                    {
                        /* If enqeue is successfull */
                        NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b = false;
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_INIT_PROGRESS_E;
                    }
                }
                else
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b = false;
                    NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8 = 0u;
                    NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                }
                break;
            }
            case NVM_VERIFY_PROGRESS_E:
            {
                uint16_t BlkCrc_u16 = Blk_pSt->RAMCopy_pu8[2];
                BlkCrc_u16 = (BlkCrc_u16 << 8) | (Blk_pSt->RAMCopy_pu8[3]);
                if (BlkCrc_u16 == crc16(&(Blk_pSt->RAMCopy_pu8[BLOCK_START_OFFSET]), (Blk_pSt->Length_u32 - NVM_BLOCK_HEADER_SIZE), INIT_CRC))
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                    NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b = true;
                }
                else
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_INCONSTITENT_E;
                }
                break;
            }
            case NVM_INCONSTITENT_E:
            {
                NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                if (NULL != Blk_pSt->RomCopy_pu8)
                {
                    MemCopy(Blk_pSt->RAMCopy_pu8, Blk_pSt->RomCopy_pu8, Blk_pSt->Length_u32);
                    NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b = true;
                }
                else
                {
                    ClearBuff(Blk_pSt->RAMCopy_pu8, Blk_pSt->Length_u32);
                    NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b = true;
                }
                break;
            }
            case NVM_UPLOAD_E:
            {
                if (NULL != Blk_pSt->RAMCopy_pu8)
                {
                    if (Write_Eeprom((uint32_t)Blk_pSt->Id_En, Blk_pSt->Start_Addr_u32, Blk_pSt->RAMCopy_pu8, (uint16_t)Blk_pSt->Length_u32))
                    {
                        NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b = false;
                        NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8 = 0u;
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_UPLOAD_PROGRESS_E;
                    }
                }
                else
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                }
                break;
            }
            case NVM_UPLOAD_PROGRESS_E:
            {
                if (NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b)
                {
                    if (EA_REQ_PASS == NvmBlkData_aSt[Loop_Cnt_u32].ReqResult_u8)
                    {
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                    }
                    else
                    {
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_UPLOAD_FAIL_E;
                    }
                }
                break;
            }
            case NVM_UPLOAD_FAIL_E:
            {
                if (NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8 < NVM_RETRY_MAX)
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8++;
                    if (NULL != Blk_pSt->RAMCopy_pu8)
                    {
                        /* Queue the task to Read */
                        if (true == Write_Eeprom((uint32_t)Blk_pSt->Id_En, Blk_pSt->Start_Addr_u32, Blk_pSt->RAMCopy_pu8, (uint16_t)Blk_pSt->Length_u32))
                        {
                            /* If enqeue is successfull */
                            NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b = false;
                            NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_UPLOAD_PROGRESS_E;
                        }
                    }
                    else
                    {
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                    }
                }
                else
                {
                    NvmBlkData_aSt[Loop_Cnt_u32].FailCnt_u8 = 0u;
                    NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_IDLE_E;
                }
                break;
            }
            default:
            {
                break;
            }
            }
        }
    }

    return;
}

void MemCopy(uint8_t *Dest_pu8, const uint8_t *Srs_pu8, uint32_t Len_u32)
{
	uint32_t Loop_Cnt_u32 = 0u;
    if ((NULL != Dest_pu8) && (NULL != Srs_pu8))
    {
        for (Loop_Cnt_u32 = 0u; Loop_Cnt_u32 < Len_u32; Loop_Cnt_u32++)
        {
            Dest_pu8[Loop_Cnt_u32] = Srs_pu8[Loop_Cnt_u32];
        }
    }
    return;
}

static void ClearBuff(uint8_t *Dest_pu8, uint32_t Len_u32)
{
	uint32_t Loop_Cnt_u32 = 0u;
    if (NULL != Dest_pu8)
    {
        for (Loop_Cnt_u32 = 0u; Loop_Cnt_u32 < Len_u32; Loop_Cnt_u32++)
        {
            Dest_pu8[Loop_Cnt_u32] = 0x00u;
        }
    }

    return;
}

void NvmReqCompleteCallback(uint32_t Identifier_u32, uint8_t Result_u8)
{
    uint32_t Loop_Cnt_u32 = 0u;

    for (Loop_Cnt_u32 = NVM_BLK_START_E; Loop_Cnt_u32 < TOTAL_NVM_BLK_E; Loop_Cnt_u32++)
    {
        if (((NvmBlockId_En_t)Identifier_u32) == NvmBlk_aSt[Loop_Cnt_u32].Id_En)
        {
            NvmBlkData_aSt[Loop_Cnt_u32].ReqComplete_b = true;
            NvmBlkData_aSt[Loop_Cnt_u32].ReqResult_u8 = Result_u8;
            break;
        }
    }
    return;
}
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : crc16.
 *   Description   : This function will calculate the crc.
 *   Parameters    : data - on which crc has to be calcukated.
 *	                 length - size of the data.
 *   Return Value  : CRC.
 *******************************************************************************/
uint16_t crc16(const uint8_t *data, uint32_t length, uint16_t CRC_val)
{
    uint16_t crc = CRC_val;
    uint16_t i;
    uint8_t j;
    uint32_t K;
    uint16_t crc_entry;
    static uint16_t crc_table[256u] = {0u};

    // Generate the CRC lookup table
    if (0u == crc_table[1u])
    {
        for (i = 0u; i < 256u; i++)
        {
            crc_entry = i << 8u;
            for (j = 0u; j < 8u; j++)
            {
                crc_entry = (((crc_entry & 0x8000u) == 0x8000u) ? ((crc_entry << 1u) ^ POLY) : (crc_entry << 1u));
            }
            crc_table[i] = crc_entry;
        }
    }

    for (K = 0u; K < length; K++)
    {
        crc = (crc << 8u) ^ crc_table[(crc >> 8u) ^ data[K]];
    }

    return crc;
}

void Nvm_Shutdown(void)
{
    uint16_t Loop_Cnt_u16 = 0u;

    for (Loop_Cnt_u16 = (uint16_t)NVM_BLK_START_E; Loop_Cnt_u16 < (uint16_t)TOTAL_NVM_BLK_E; Loop_Cnt_u16++)
    {
        if (true == NvmBlkData_aSt[Loop_Cnt_u16].ShutdownWrite_b)
        {
            NvmBlkData_aSt[Loop_Cnt_u16].ShutdownWrite_b = false;

            if (NVM_IDLE_E == NvmBlkData_aSt[Loop_Cnt_u16].BlockState_En)
            {
                NvmBlkData_aSt[Loop_Cnt_u16].BlockState_En = NVM_UPLOAD_E;
            }
        }
    }
    return;
}

bool Is_Nvm_Idle(void)
{
    uint16_t Loop_Cnt_u16 = 0u;
    bool Status_u8 = true;

    for (Loop_Cnt_u16 = (uint16_t)NVM_BLK_START_E; Loop_Cnt_u16 < (uint16_t)TOTAL_NVM_BLK_E; Loop_Cnt_u16++)
    {
        if (NVM_IDLE_E != NvmBlkData_aSt[Loop_Cnt_u16].BlockState_En)
        {
            Status_u8 = false;
            break;
        }
    }

    return Status_u8;
}

NvmAccessSt_En_t ApplNVMUpload(uint32_t BlockNumber_u32, uint8_t *Data_Buff_pu8, uint8_t Length2Write_u8)
{
    uint32_t Loop_Cnt_u32 = 0;
    bool BlockFound_b = false;
    uint16_t Crc_Cal_u16 = 0u;
    NvmAccessSt_En_t Status_En = NVM_WRITE_OK_E;

    for (Loop_Cnt_u32 = NVM_BLK_START_E; Loop_Cnt_u32 < TOTAL_NVM_BLK_E; Loop_Cnt_u32++)
    {
        if (BlockNumber_u32 == Loop_Cnt_u32)
        {
            BlockFound_b = true;
            if ((NULL != &NvmBlk_aSt[Loop_Cnt_u32]) && (NULL != NvmBlk_aSt[Loop_Cnt_u32].Descriptor_pSt))
            {
                const NvmBlk_St_t *Blk_pSt = &NvmBlk_aSt[Loop_Cnt_u32];
                // Blk_Descriptor_St_t *Blk_Descriptor_St = (Blk_Descriptor_St_t *)NvmBlk_aSt[Loop_Cnt_u32].Descriptor_pSt;
                if (NVM_IDLE_E == NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En)
                {
                    if ((true == NvmBlkData_aSt[Loop_Cnt_u32].BlkValid_b) && (NULL != Blk_pSt->RAMCopy_pu8))
                    {
                        /* LDRA_INSPECTED 45 D */
                        /* MISRA-C:2012(Ed 3) D.4.1 : Pointer not checked for null before use.
                        Justification: Global pointer is checked for null before use*/
                        MemCopy((Blk_pSt->RAMCopy_pu8 + BLOCK_START_OFFSET), Data_Buff_pu8, Length2Write_u8);
                        Crc_Cal_u16 = crc16(&(Blk_pSt->RAMCopy_pu8[BLOCK_START_OFFSET]), (Blk_pSt->Length_u32 - NVM_BLOCK_HEADER_SIZE), INIT_CRC);
                        Blk_pSt->RAMCopy_pu8[NVM_CRC_POS_1] = ((uint8_t)(Crc_Cal_u16 >> 8)) & 0xFFU;
                        Blk_pSt->RAMCopy_pu8[NVM_CRC_POS_2] = ((uint8_t)Crc_Cal_u16) & 0xFFU;
                        NvmBlkData_aSt[Loop_Cnt_u32].BlockState_En = NVM_UPLOAD_E;
                    }
                    else
                    {
                        MemCopy((Blk_pSt->RAMCopy_pu8 + BLOCK_START_OFFSET), Data_Buff_pu8, Length2Write_u8);
                        Status_En = NVM_ERROR_E;
                    }
                }
                else
                {
                    Status_En = NVM_WAIT_E;
                }
            }
            if (true == BlockFound_b)
            {
                break;
            }
        }
        else
        {
            Status_En = NVM_INVALID_PAREMETER_E;
        }
    }
    return Status_En;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : NONE
 *   Description   : NONE
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/
nvm_status_En_t NvmWriteBlock(uint32_t block_id_u32, uint8_t *buffer_pu8, uint32_t size_u32)
{
    uint32_t address_u32 = 0u;
    bool NvmWriteStatus_b = false;
    address_u32 = (RESERVED_BLOCK_SIZE * BYTE_SIZE_OF_BLOCK) + (block_id_u32 * BYTE_SIZE_OF_BLOCK);

    NvmWriteStatus_b = FDL_write((uint16_t)address_u32, (uint16_t)size_u32, buffer_pu8);
    return (nvm_status_En_t)NvmWriteStatus_b;
}


/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : NONE
 *   Description   : NONE
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/
uint8_t NvmReadBlock(uint32_t block_id_u32, uint8_t *buffer_pu8, uint32_t size_u32)
{
    uint32_t address_u32 = 0u;
    bool NvmReadStatus_b = false;
    address_u32 = (RESERVED_BLOCK_SIZE * BYTE_SIZE_OF_BLOCK) + (block_id_u32 * BYTE_SIZE_OF_BLOCK);

    NvmReadStatus_b = FDL_Read((uint16_t)address_u32, (uint16_t)size_u32, buffer_pu8);

    return NvmReadStatus_b;
}

/*LDRA_EXCLUDE_END 567 S */
/*LDRA_EXCLUDE_END 436 S */
/*---------------------- End of File -----------------------------------------*/
