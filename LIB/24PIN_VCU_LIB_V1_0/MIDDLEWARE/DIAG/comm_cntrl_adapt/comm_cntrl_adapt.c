/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : comm_cntrl_adapt.c
|    Project        : EMBDES_GSHIFTER
|    Description    : The file contatins the common control tasks.
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 31/07/2024       Manikandan S             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include "comm_cntrl_adapt.h"
#include "can_sched.h"
/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NormalCommMsg_Adapt
*   Description   : The function used for normal communication.
*   Parameters    : uint8_t Rx_Status - Rx Message status,  
					uint8_t Tx_Status - Tx Message status
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void NormalCommMsg_Adapt(bool Rx_Status_b,  bool Tx_Status_b)
{
	uint8_t CommStatusMask_u8 = 0u;
	if((true == Rx_Status_b) && (true == Tx_Status_b))
	{
		CommStatusMask_u8 = CAN_RX_TX_ENABLE;
	}
	else if((true == Rx_Status_b) && (false == Tx_Status_b))
	{
		CommStatusMask_u8 = CAN_RX_ONLY;
	}
	else if ((false == Rx_Status_b) && (true == Tx_Status_b))
	{
		CommStatusMask_u8 = CAN_TX_ONLY;
	}
	else
	{
		CommStatusMask_u8 = CAN_RX_TX_DISABLE;
	}

	CilCanStatusMaskSet(J1939_CAN_MSG_E,CommStatusMask_u8);
	return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NWMngmntCommMsg_Adapt
*   Description   : The function used for network communication.
*   Parameters    : uint8_t Rx_Status - Rx Message status,  
					uint8_t Tx_Status - Tx Message status
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void NWMngmntCommMsg_Adapt(bool Rx_Status_b,  bool Tx_Status_b)
{
	/* No Network available */
	return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : NWMngmnt_NormalCommMsg_Adapt
*   Description   : The function used for normal and network communication.
*   Parameters    : uint8_t Rx_Status - Rx Message status,  
					uint8_t Tx_Status - Tx Message status
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void NWMngmnt_NormalCommMsg_Adapt(bool Rx_Status_b,  bool Tx_Status_b)
{
	uint8_t CommStatusMask_u8 = 0u;
	if((true == Rx_Status_b) && (true == Tx_Status_b))
	{
		CommStatusMask_u8 = CAN_RX_TX_ENABLE;
	}
	else if((true == Rx_Status_b) && (false == Tx_Status_b))
	{
		CommStatusMask_u8 = CAN_RX_ONLY;
	}
	else if ((false == Rx_Status_b) && (true == Tx_Status_b))
	{
		CommStatusMask_u8 = CAN_TX_ONLY;
	}
	else
	{
		CommStatusMask_u8 = CAN_RX_TX_DISABLE;
	}

	CilCanStatusMaskSet(J1939_CAN_MSG_E,CommStatusMask_u8);
}
