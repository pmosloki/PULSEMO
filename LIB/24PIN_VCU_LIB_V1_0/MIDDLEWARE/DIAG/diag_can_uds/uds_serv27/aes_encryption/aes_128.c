/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File	        : aes_128.c
|    Project	    : PBL_HVAC
|    Description    : This file contains the implementation of the AES algorithm, 
|                     specifically ECB, CTR and CBC mode.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date     	      Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S           Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

#ifndef AES_128_C
#define AES_128_C

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include <stdint.h>
#include "aes_128.h"

/*******************************************************************************
 *  Define & Macros
 ******************************************************************************/
uint32_t SecretKeys_au8[TOTAL_SECRET_KEY_E] = 
{
  0xE5E6E7E9,
  0xA392A2B2,
  0xD2564890,
};

uint8_t ltable[256];
uint8_t atable[256];


unsigned char mul_inverse(unsigned char in);
unsigned char s_box(unsigned char data_in);
unsigned char inv_sbox(unsigned char data_in);


void GenerateTables(void)
{
  unsigned char c;
  unsigned char a = 1u;
  unsigned char d;

  for(c = 0; c<255; c++)
  {
    atable[c] = a;
    /* Multiply by three*/
    d = a & 0x80u;
    a <<= 1u;
    if(d == 0x80u)
    {
      a ^= 0x1bu;
    }
    a ^= atable[c];
    /* Set the log table value */
    ltable[atable[c]] = c;
  }
  atable[255] = atable[0];
  ltable[0] = 0u;

  return;
}

unsigned char mul_inverse(unsigned char in)
{
  if(in == 0u)
  {
    return 0u;
  }
  else
  {
    return atable[(255 - ltable[in])];
  }
}

unsigned char s_box(unsigned char data_in)
{
  unsigned char c,s,x;

  s = x = mul_inverse(data_in);
  for(c = 0u; c < 4u; c++)
  {
    s = (uint8_t)((s << 1u) | (s >> 7u));
    x ^= s;
  }
  x ^= 0x63u;

  return x;
}


void setBit(uint8_t *bitset, uint8_t index, uint8_t value) 
{
    if (value)
    {
      *bitset |= (1u << index);  // Set the bit at the given index
    }
    else
    {
      *bitset &= ~(1u << index); // Clear the bit at the given index
    }

}

uint8_t getBit(uint8_t bitset, uint8_t index) 
{
    return (bitset >> index) & 1u; // Get the bit value at the given index
}


unsigned char inv_sbox(unsigned char data_in) {
    uint8_t bit_data = data_in;
    uint8_t bit_b = data_in;
    uint8_t bit_d = 0x05u;
    uint8_t TmpValue_u8 = 0;

    unsigned char j, x;

    for (j = 0u; j < 8u; j++)
    {
        setBit(&bit_data, j, (getBit(bit_b, ((j + 2u) % 8u))));
        TmpValue_u8 = getBit(bit_data, j) ^ getBit(bit_b, ((j + 5u) % 8u));
        setBit(&bit_data, j, TmpValue_u8);
        TmpValue_u8 = getBit(bit_data, j) ^ getBit(bit_b, ((j + 7u) % 8u));
        setBit(&bit_data, j, TmpValue_u8);
        TmpValue_u8 = getBit(bit_data, j) ^ getBit(bit_d, j);
        setBit(&bit_data, j, TmpValue_u8);
    }

    x = mul_inverse(bit_data);

    return x;
}

unsigned long  encrypt_data(unsigned long rxdata,unsigned long secret_key)
{
  unsigned char databyte[5];
  unsigned char encdatabyte[5];
  unsigned long return_data = 0;
  int i;

  for(i = 0;i<4;i++)
  {
    databyte[i] = (rxdata >> (8 * (3 - i))) & 0xFF;
    encdatabyte[i] = s_box(databyte[i]);
    return_data += (uint32_t)encdatabyte[i] << (8 * (3 - i));
  }

  return return_data ^ secret_key;
}

unsigned long decrypt_data(unsigned long rxdata, unsigned long secret_key)
{
  unsigned char databytes[5];
  unsigned char decdatabytes[5];
  unsigned long return_data = 0L;
  int i;

  rxdata ^= secret_key;
  for(i = 0; i < 4 ;i++)
  {
    databytes[i] = (rxdata >> (8 * (3 - i))) & 0xFF;
    decdatabytes[i] = inv_sbox(databytes[i]);
    return_data += (uint32_t)decdatabytes[i] << (8 * (3 - i));
  }
  return return_data;
}

uint32_t decryptCipher(uint32_t Cipher_u32, uint8_t KeyNum_u8)
{
  uint32_t PlainData_u32 = 0;
  if(KeyNum_u8 < TOTAL_SECRET_KEY_E)
  {
    GenerateTables();
    PlainData_u32 = decrypt_data(Cipher_u32, SecretKeys_au8[KeyNum_u8]);
  }
	return PlainData_u32;
}


uint32_t EncryptSeed(unsigned long Plaintext, uint8_t Keynum_u8)
{
  uint32_t CipherData_u32 = 0;
  if (Keynum_u8 < TOTAL_SECRET_KEY_E)
  {
    GenerateTables();
    CipherData_u32 = encrypt_data(Plaintext, SecretKeys_au8[Keynum_u8]);
  }

  return CipherData_u32;
}
#endif /* AES_128_C */
/*---------------------- End of File -----------------------------------------*/
