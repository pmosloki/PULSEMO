/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File      		: iso14229_serv27_callback.c
|    Project      	: MIL_PBL_CV
|    Description    : callback function description for UDS service - Security
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date             Name                        Company
| ----------     ---------------     -----------------------------------
|31/07/2024       Manikandan S         Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "iso14229_serv27.h"
#include "iso14229_serv27_conf.h"
#include "uds_conf.h"
#include "aes_128.h"
#include "stdlib.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION
 ******************************************************************************/
uint32_t CipherText_u32 = 0u;
uint32_t PlainText_u32 = 0u;
/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
/*LDRA_INSPECTED 458 S */
#if (TRUE == ONES_COMPLIMENT)
static SPLIT_RAND_Un_t SPLIT_RAND_Un;
#endif
static AES_128_Un_t AES_128_Un;
/* the cipher key */
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
/* -----------------------------------------------------------------------------
 *  FUNCTION DESCRIPTION
 *  -----------------------------------------------------------------------------
 *   Function Name : NONE
 *   Description   : NONE
 *   Parameters    : None
 *   Return Value  : None
 *  ---------------------------------------------------------------------------*/
/* LDRA_EXCLUDE_START 65 D */
void Uds27ServInitCallback(void)
{
    return;
}
/* LDRA_EXCLUDE_END 65 D */
/* LDRA_EXCLUDE_START 436 S */
/*LDRA_INSPECTED 8 D */
uint32_t rand32() {
    return ((uint32_t)rand() << 16) | (uint32_t)rand();
}
void GenerateSeedCallback(uint8_t *databuff_pu8, uint8_t Len_u8, uint8_t* Seed_pu8) /* Pointer to an array of bytes */
{
    uint8_t i = 0;
#if (TRUE == ONES_COMPLIMENT)
    do
    {
#if (TRUE == RANDOM_NUM_GENERATION)
        /*generating the random number*/
        SPLIT_RAND_Un.random_number = GET_TIME_MS();

#elif (TRUE == RAND_FUNCTION_GENERATION)
        /*generating from rand function*/
        SPLIT_RAND_Un.random_number = rand32();
#endif
    } while ((SPLIT_RAND_Un.random_number == 0) || (SPLIT_RAND_Un.random_number == 0xFFFFFFFF));

    for (i = 0; i < 4; i++)
    {
        databuff_pu8[i] = SPLIT_RAND_Un.byte_buff[i];
    }
#endif
#if (TRUE == AES32_ALGORITHM)
    srand(PblParam_St.RandomNum_u32);
    for (i = 0; i < CIPHER_LEN; i++)
    {
        AES_128_Un.random_number_au8[i] = (uint8_t)rand();
        databuff_pu8[i] = AES_128_Un.random_number_au8[i];
        Seed_pu8[i] = AES_128_Un.random_number_au8[i];
    }
    PblParam_St.RandomNum_u32 = AES_128_Un.random_number_au32[0];
    UpdateNVMblock(PBL_NVM_BLOCK);
#endif
    return;
}

/*LDRA_EXCLUDE_END 436 S */
void GenerateKeyCallback(uint8_t *databuff_pu8,uint8_t *Seed_pu8, uint8_t keyLen_u8,uint8_t SeedLen_u8, uint8_t SecLvl_u8)
{
    uint32_t CalSeed_u32 = 0u;
    uint8_t i = 0u;
    uint8_t LoopCnt_u8;
    for(LoopCnt_u8 = 0U;LoopCnt_u8<(uint8_t)TOTAL_SECURITY_LVL;LoopCnt_u8++)
    {
        if(SecLvl_u8 == UdsSecurityAccess_aSt[LoopCnt_u8].SecurityLvl_u8)
        {
            break;
        }
    }
#if (TRUE == ONES_COMPLIMENT)
    SPLIT_RAND_Un_t key_data_Un;  /*TODO: is this variable is required*/
#endif
    PlainText_u32 |= (((uint32_t)AES_128_Un.random_number_au8[0] & 0xFF) << 24);
    PlainText_u32 |= (((uint32_t)AES_128_Un.random_number_au8[1] & 0xFF) << 16);
    PlainText_u32 |= (((uint32_t)AES_128_Un.random_number_au8[2] & 0xFF) << 8);
    PlainText_u32 |= (((uint32_t)AES_128_Un.random_number_au8[3] & 0xFF));
    

    CipherText_u32 |= (((uint32_t)databuff_pu8[0] & 0xFF) << 24);
    CipherText_u32 |= (((uint32_t)databuff_pu8[1] & 0xFF) << 16);
    CipherText_u32 |= (((uint32_t)databuff_pu8[2] & 0xFF) << 8);
    CipherText_u32 |= (((uint32_t)databuff_pu8[3] & 0xFF));

    GenerateTables();
    
#if (TRUE == ONES_COMPLIMENT)

   // SPLIT_RAND_Un_t key_data_Un;  /*TODO: is this variable is required*/
    /*compliments the generated random number*/
    key_data_Un.random_number = ~SPLIT_RAND_Un.random_number;

    for (i = 0; i < 4; i++)
    {
        if (databuff_pu8[i + 1] != key_data_Un.byte_buff[i])
        {
            return FALSE;
        }
    }
       if(i == 4)
    {
	   return TRUE; 
    }

#endif
#if (TRUE == AES32_ALGORITHM)
    CalSeed_u32 = decryptCipher(CipherText_u32,LoopCnt_u8);
    for (i = 0; i < 4; i++)
    {
        Seed_pu8[i] = (CalSeed_u32 >> (8 * (3 - i))) & 0xFF;
    }

#endif
}
/*---------------------- End of File -----------------------------------------*/
