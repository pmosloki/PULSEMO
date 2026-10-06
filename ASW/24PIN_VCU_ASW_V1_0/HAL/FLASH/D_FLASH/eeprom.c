/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : eeprom_conf.c
|    Project        : RH850/F1KM S1-80pin
|    Description    : The file implements application for accessing the eeprom.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 18/08/2022       Jeevan Jestin N             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "r_typedefs.h"
#include "r_fdl.h"
#include "fdl_descriptor.h"
//#include "target.h"
#include "eeprom.h"
#include"eeprom_conf.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION
 ******************************************************************************/
volatile uint16_t MaximumBlock_u16 = 0; /* Maximum block can be accessed by 
                                        the application*/
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION
 ******************************************************************************/
uint32_t FDLReaddataBuff_au32[(BYTE_SIZE_OF_BLOCK * 2)/MEMORY_ACCESS_SIZE] = {0};
uint32_t FDLWritedataBuff_au8[(BYTE_SIZE_OF_BLOCK * 2)/MEMORY_ACCESS_SIZE] = {0};

uint8_t*const FdlWriteBuff_pu8 = (uint8_t*)FDLWritedataBuff_au8;
uint8_t*const FdlReadBuff_pu8 = (uint8_t*)FDLReaddataBuff_au32;


EEprom_St_t EEprom_St;
/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : DFlash_Init
 *   Description   : The function initialize the Data flash
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/
bool DFlash_Init(void)
{
    r_fdl_status_t fdlRet;
    r_fdl_request_t InitReq;
    uint16_t Indx_u16 = 0;
    
    uint16_t BlockSize_u16 = 0;
    uint16_t ByteStartPos_u16 = 0;

    DCIB.EEPRDCYCL = 0x0F;

    /* 1st initialize the FDL */
    fdlRet = R_FDL_Init(&sampleApp_fdlConfig_enu);
    if (R_FDL_OK != fdlRet)
    {
        /* Error handler */
        return false;
    }
#ifndef R_FDL_LIB_V1_COMPATIBILITY
    /* Prepare the environment */
    InitReq.command_enu = R_FDL_CMD_PREPARE_ENV;
    InitReq.idx_u32 = 0;
    InitReq.cnt_u16 = 0;
    InitReq.accessType_enu = R_FDL_ACCESS_NONE;
    R_FDL_Execute(&InitReq);

    while (R_FDL_BUSY == InitReq.status_enu)
    {
        R_FDL_Handler();
    }
    if (R_FDL_OK != InitReq.status_enu)
    {
        /* Error handler */
        NOP();
        return false;
    }
#endif

    if((TOTAL_BLOCK*BYTE_SIZE_OF_BLOCK)<= EEPROMconf_St.VirtualStartAddr_u32)
    {
        return false;
    }

    EEprom_St.BlockStartPos_u16 = EEPROMconf_St.VirtualStartAddr_u32 / BYTE_SIZE_OF_BLOCK;
    BlockSize_u16 = (EEPROMconf_St.VirtualStartAddr_u32 % BYTE_SIZE_OF_BLOCK) + EEPROMconf_St.ByteSize_u16;
    EEprom_St.NumOfBlock_u16 = BlockSize_u16 / BYTE_SIZE_OF_BLOCK;

    if(0 != (BlockSize_u16 % BYTE_SIZE_OF_BLOCK))
    {
        EEprom_St.NumOfBlock_u16++;
    }
    EEprom_St.BlockEndPos_u16 =  EEprom_St.BlockStartPos_u16 + EEprom_St.NumOfBlock_u16;
    ByteStartPos_u16 = EEprom_St.BlockStartPos_u16 * BYTE_SIZE_OF_BLOCK;
    EEprom_St.NumOfbytes_u16 = EEprom_St.NumOfBlock_u16 * BYTE_SIZE_OF_BLOCK;
    
    if(TOTAL_BLOCK < EEprom_St.BlockEndPos_u16)
    {
        /* EEPROM has 1024 block. each block has 64 bytes(16 word)*/
        return false;
    }

    for (Indx_u16 = 0; Indx_u16 < EEprom_St.NumOfbytes_u16; Indx_u16 += MEMORY_ACCESS_SIZE)
    {
        /* Blankcheck operation */
        InitReq.command_enu = R_FDL_CMD_BLANKCHECK;
        InitReq.idx_u32 = ByteStartPos_u16;
        InitReq.cnt_u16 = 1;
        InitReq.accessType_enu = R_FDL_ACCESS_USER;
        R_FDL_Execute(&InitReq);
        while (R_FDL_BUSY == InitReq.status_enu)
        {
            R_FDL_Handler();
        }

        if (R_FDL_OK == InitReq.status_enu)
        {
            /* If the data flash is blank fill with dummy data */
            InitReq.command_enu = R_FDL_CMD_WRITE;
            InitReq.idx_u32 = ByteStartPos_u16;
            InitReq.cnt_u16 = 1;
            InitReq.bufAddr_u32 = (uint32_t)&EEPROMconf_St.DummyData_u32;
            InitReq.accessType_enu = R_FDL_ACCESS_USER;
            R_FDL_Execute(&InitReq);
            while (R_FDL_BUSY == InitReq.status_enu)
            {
                R_FDL_Handler();
            }
            if (R_FDL_OK != InitReq.status_enu)
            {
                return false;
            }
        }
        else if(R_FDL_OK != InitReq.status_enu)
        {
            NOP();
        }
        else
        {
            ;
        }
	    ByteStartPos_u16 += MEMORY_ACCESS_SIZE;
    }

    /* The blank check is successfully is varified */
    EEprom_St.BlankCheckDone_b = true;

    return true;
}

/* -----------------------------------------------------------------------------
 *  FUNCTION DECLERATION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : FDL_write
 *   Description   : The function writes to the Data flash.
 *   Parameters    : WriteVirtualAddr_u16 :- The virtual start address of Data flash
 *                   DataSize_u16 :- Data size to be write.
 *                   DataBuff_u8  :- It holds the data to be write
 *   Return Value  : None
 *******************************************************************************/
 
 
bool FDL_write(uint16_t WriteVirtualAddr_u16, uint16_t DataSize_u16, uint8_t* const DataBuff_u8)
{
    r_fdl_request_t WriteReq_St;
    uint32_t FdlVirtualAddr_u32 = 0;
    uint16_t NumOfBlckToErase_u16 = 0;
    uint16_t NumOfBlckToWrite_u16 = 0;
    uint16_t DataBytesToread_u16 = 0;
    uint16_t FdlEraseVirtualAddr_u16 = 0;
    uint16_t FdlBlockPosition_u16 = 0;
    uint16_t PosAtBlock_u8 = 0;
    uint8_t BuffInd_u8 = 0;
    
    FdlBlockPosition_u16 = WriteVirtualAddr_u16 / BYTE_SIZE_OF_BLOCK;
    PosAtBlock_u8 = WriteVirtualAddr_u16 % BYTE_SIZE_OF_BLOCK;
    
    if(FdlBlockPosition_u16 < EEprom_St.BlockStartPos_u16)
    {
	    return false;
    }

    FdlVirtualAddr_u32 = FdlBlockPosition_u16 * BYTE_SIZE_OF_BLOCK;
    FdlEraseVirtualAddr_u16 = FdlBlockPosition_u16;
    NumOfBlckToErase_u16 = ((PosAtBlock_u8 + DataSize_u16) / BYTE_SIZE_OF_BLOCK);

    if (0 != ((PosAtBlock_u8 + DataSize_u16) % BYTE_SIZE_OF_BLOCK))
    {
        /* if byte size is less than 1 block(64bytes)*/
        NumOfBlckToErase_u16 = NumOfBlckToErase_u16 + 1;
    }

    if((FdlBlockPosition_u16 + NumOfBlckToErase_u16) >  EEprom_St.BlockEndPos_u16)
    {
        /* If the block is to be accessed more than configured block*/
        return false;
    }
    
    if (NumOfBlckToErase_u16 > 2)
    {
        /* FDLdataBuff_au8 can store only 2 block data */
        return false;
    }

    NumOfBlckToWrite_u16 = NumOfBlckToErase_u16 * WORD_SIZE_OF_BLOCK;
    DataBytesToread_u16 = BYTE_SIZE_OF_BLOCK * NumOfBlckToErase_u16;

    /* Read the data before erasing the block and store it in local buffer*/
    FDL_Read(FdlVirtualAddr_u32, DataBytesToread_u16, &FdlWriteBuff_pu8[0]);

    for (BuffInd_u8 = 0; BuffInd_u8 < DataSize_u16; BuffInd_u8++)
    {
        /* Update the local buffer with new data recieved from application  */
        FdlWriteBuff_pu8[BuffInd_u8 + PosAtBlock_u8] = DataBuff_u8[BuffInd_u8];
    }

    /* EEPROM "ERASE" command execution */
    WriteReq_St.command_enu = R_FDL_CMD_ERASE;
    WriteReq_St.idx_u32 = FdlEraseVirtualAddr_u16;
    WriteReq_St.cnt_u16 = NumOfBlckToErase_u16;
    WriteReq_St.accessType_enu = R_FDL_ACCESS_USER;
   
    R_FDL_Execute(&WriteReq_St);

    while (R_FDL_BUSY == WriteReq_St.status_enu)
    {
        R_FDL_Handler();
    }
    if (R_FDL_OK != WriteReq_St.status_enu)
    {
        /* Error handler */
        return false;
    }

    /* EEPROM "WRITE" command executuion */
    WriteReq_St.command_enu = R_FDL_CMD_WRITE;
    WriteReq_St.idx_u32 = FdlVirtualAddr_u32;
    WriteReq_St.cnt_u16 = NumOfBlckToWrite_u16;
    WriteReq_St.bufAddr_u32 = (uint32_t)(&FdlWriteBuff_pu8[0]); /* Write data stored in the locla buffer*/
    WriteReq_St.accessType_enu = R_FDL_ACCESS_USER;

    R_FDL_Execute(&WriteReq_St);
    while (R_FDL_BUSY == WriteReq_St.status_enu)
    {
        R_FDL_Handler();
	
    }
    if (R_FDL_OK != WriteReq_St.status_enu)
    {
        /* Error handler */
        return false;
	
    }

    /* Write operation to eeprom is succesfull*/
    return true;
}

uint32_t addrs;
void fun_fdl_write(uint16_t ReadVirtualAddr_u16,uint16_t WriteVirtualAddr_u16, uint16_t DataSize_u16, uint8_t*  DataBuff_u8)
{
	addrs = ((ReadVirtualAddr_u16*64) + WriteVirtualAddr_u16);
	//FDL_write(((ReadVirtualAddr_u16*64) + WriteVirtualAddr_u16) ,DataSize_u16,(uint8_t*)&DataBuff_u8[0]);
 return ; 
}
/* -----------------------------------------------------------------------------
 *  FUNCTION DECLERATION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : FDL_Read
 *   Description   : The function read the data from eeprom
 *   Parameters    : ReadVirtualAddr_u16 :- The byte address of eeprom 
 *                   (Note: The address should be virtual address, 
 *                          do not pass the physical address)  
 *                   DataSize_u16:- The size of the data to be read.
 *                   DataBuff_u8:- The pointer of the data buffer where data is to be stored
 *   Return Value  : true/false
 *******************************************************************************/
bool FDL_Read(uint16_t ReadVirtualAddr_u16, uint16_t DataSize_u16, uint8_t* const DataBuff_u8)
{
    r_fdl_request_t ReadReq_St;
    uint32_t FdlReadVirtualAddr_u32 = 0;
    uint16_t ReadDataByteSize_u16 = 0;
    uint16_t FdlBlockNum_u16 = 0;
    uint16_t PosAtBlock_u8 = 0;
    uint8_t BuffInd_u8 = 0;
    uint8_t BuffReadPos_u8 = 0;
   

    FdlBlockNum_u16 = ReadVirtualAddr_u16 / BYTE_SIZE_OF_BLOCK;
    PosAtBlock_u8 = ReadVirtualAddr_u16 % BYTE_SIZE_OF_BLOCK;
    
     if(false == EEprom_St.BlankCheckDone_b)
    {
        /* The blank check is not successfully varified.*/
        return false;
    }

    if(FdlBlockNum_u16 < EEprom_St.BlockStartPos_u16)
    {
	    return false;
    }
    
    if((ReadVirtualAddr_u16 + DataSize_u16) > ( EEprom_St.BlockEndPos_u16 * BYTE_SIZE_OF_BLOCK))
    {
        return false;
    }

    FdlReadVirtualAddr_u32 = ReadVirtualAddr_u16 - ( ReadVirtualAddr_u16%MEMORY_ACCESS_SIZE);
    ReadDataByteSize_u16 = (PosAtBlock_u8 % 4) + DataSize_u16;

    if ((ReadDataByteSize_u16 % MEMORY_ACCESS_SIZE) != 0)
    {
        ReadDataByteSize_u16 = ReadDataByteSize_u16 + (MEMORY_ACCESS_SIZE - (ReadDataByteSize_u16%MEMORY_ACCESS_SIZE));
    }

    if(ReadDataByteSize_u16 > (BYTE_SIZE_OF_BLOCK * 2))
    {
        /* FDLReaddataBuff_au8 can store only 2 block data*/
        return false;
    }

    ReadReq_St.command_enu = R_FDL_CMD_READ;
    ReadReq_St.idx_u32 = FdlReadVirtualAddr_u32;
    ReadReq_St.cnt_u16 = ReadDataByteSize_u16 / MEMORY_ACCESS_SIZE;
    ReadReq_St.bufAddr_u32 = (uint32_t)(&FdlReadBuff_pu8[0]);
    ReadReq_St.accessType_enu = R_FDL_ACCESS_USER;


    R_FDL_Execute(&ReadReq_St);
    while (R_FDL_BUSY == ReadReq_St.status_enu)
    {
        R_FDL_Handler();
    }
    if (R_FDL_OK != ReadReq_St.status_enu)
    {
        /* Error handler */
        NOP();
        return false;
    }

    BuffReadPos_u8 = PosAtBlock_u8 % MEMORY_ACCESS_SIZE;
    for (BuffInd_u8 = 0; BuffInd_u8 < DataSize_u16; BuffInd_u8++)
    {
        DataBuff_u8[BuffInd_u8] = FdlReadBuff_pu8[BuffInd_u8 + BuffReadPos_u8];
    }
    return true;
}

void fun_fdl_Read(uint16_t ReadVirtualAddr_u16,uint16_t WriteVirtualAddr_u16, uint16_t DataSize_u16, uint8_t*  DataBuff_u8)
{
	addrs = ((ReadVirtualAddr_u16*64) + WriteVirtualAddr_u16);
	//FDL_Read(((ReadVirtualAddr_u16*64) + WriteVirtualAddr_u16) ,DataSize_u16,(uint8_t*)&DataBuff_u8[0]);
	return ;
}
bool Fdl_Erase(uint16_t Sector_u16,uint16_t TotalSector_u16)
{
    r_fdl_request_t EraseReq_St;
    uint16_t FdlEraseVirtualAddr_u16 = 0; 
    uint16_t LoopCnt_u16 = 0;

    for(LoopCnt_u16 = 0;LoopCnt_u16<BYTE_SIZE_OF_BLOCK;LoopCnt_u16++)
    {
        FdlWriteBuff_pu8[LoopCnt_u16] = 0;
    }

    for(LoopCnt_u16 = Sector_u16;LoopCnt_u16<(Sector_u16+TotalSector_u16);LoopCnt_u16++)
    {

        if(LoopCnt_u16 > TOTAL_BLOCK)
        {
            return false;
        }

        FdlEraseVirtualAddr_u16 = LoopCnt_u16*BYTE_SIZE_OF_BLOCK;
        /* EEPROM "ERASE" command execution */
        EraseReq_St.command_enu = R_FDL_CMD_ERASE;
        EraseReq_St.idx_u32 = LoopCnt_u16;
        EraseReq_St.cnt_u16 = 1;
        EraseReq_St.accessType_enu = R_FDL_ACCESS_USER;
        R_FDL_Execute(&EraseReq_St);

        while (R_FDL_BUSY == EraseReq_St.status_enu)
        {
            R_FDL_Handler();
        }
        if (R_FDL_OK != EraseReq_St.status_enu)
        {
            /* Error handler */
            return false;
        }

        /* EEPROM "WRITE" command executuion */
        EraseReq_St.command_enu = R_FDL_CMD_WRITE;
        EraseReq_St.idx_u32 = FdlEraseVirtualAddr_u16;
        EraseReq_St.cnt_u16 = WORD_SIZE_OF_BLOCK;
        EraseReq_St.bufAddr_u32 = (uint32_t)(&FdlWriteBuff_pu8[0]); /* Write data stored in the locla buffer*/
        EraseReq_St.accessType_enu = R_FDL_ACCESS_USER;

        R_FDL_Execute(&EraseReq_St);
        while (R_FDL_BUSY == EraseReq_St.status_enu)
        {
            R_FDL_Handler();
        }
        if (R_FDL_OK != EraseReq_St.status_enu)
        {
            /* Error handler */
            return false;
        }

        
    }

    return 0;
}
/*---------------------- End of File -----------------------------------------*/





