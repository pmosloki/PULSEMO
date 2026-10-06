
/*******************************************************************************
*    FILENAME    : j1939_81_nmac.c
*
*    @brief      : Contains all general J1939-81 address claim mechanism
*                    
*    
*
*******************************************************************************
* Revision history
* 
* Ver Author       Date       Description
* 1   Sloki
*******************************************************************************
*/

/*
*******************************************************************************
*    Includes
*******************************************************************************
*/
#include "can_if.h"
#include "cil_can_conf.h"
#include "j1939.h"
#include "diag_sys_conf.h"
/*
****************************************************************************************
*    Static Variables
****************************************************************************************
*/

/*
****************************************************************************************
*    Static functions
****************************************************************************************
*/
/**
*  @brief   Function to process the transmission frame right before
*	    sending the frame     
*
*  @param   pgn_u16                 PGN value
*           SourceAddr_u8	    Source address.
*
*  @return  None *
*/ 

static void J1939_81_TransmitMessage(uint32_t pgn_u16, uint8_t SourceAddr_u8) ;

/**
*  @brief   This routine is called if Arbitration bit field is set in NAME frame.
*			If our CA's NAME has lower priority than received CA's NAME then it
*           recalculate address to new one for claiming,
*                                 
*
*  @param   oldAddress_pu8	pointer which holding a old denied address.
*           minAddrRange    minimum address for claiming
*           maxAddRange     max address for claiming
*
*  @return  bool	true  if address is available for claiming.
*					false if no more address available for claiming.
*/ 
static bool CalcNextClaimAddress(void);


/**
*  @brief   This function compare the received NAME and this CA's NAME..
*                                 
*
*  @param   otherName_pu8	Structure of NAME.
*           Name_pu8        This CA's NAME 
*  @return  0 : if CA's NAME is less than the other name.
*	    1 : if CA's NAME is greater than or equal to other name.
*/
static bool     IsMyNameHigherPriority(J1939_81_NMAC_Name_St_t otherName_St, J1939_81_NMAC_Name_St_t MyName_St);


/**
*  @brief   This routine is called either when the CA claims its address on bus or
*            another CA is trying to claim the same address on the bus.
*                                 
*
*  @param   mode:       J1939_81_NMAC_TX_MODE: indicates that CA is initiating 
*			a claim to its address.
*	    J1939_81_NMAC_RX_MODE: indicates an address claim message
*				   has been received and this CA must
*				   defend or give up its address.   
*           rxName_St       received CA's NAME
*
*  @return  int8_t       -1     No response
*                        0     Can not claim the address
*                        1     Start the Address Claim
*/ 
static uint8_t    AddressClaimHandling(uint8_t modeRxTx, J1939_81_NMAC_Name_St_t  rxName_St);

/**
*  @brief    Function to handle All Address Claim message.
*  @param    srcAddr_u8    claimed address
*  @return   none
*/
static void     SendAddressClaimResponse(uint8_t srcAddr_u8);
/*
****************************************************************************************
*    Global Variables
****************************************************************************************
*/

/*
@@ SYMBOL     = J1939_NMAC_ECU_ClaimedAddr_u8
@@ A2L_TYPE   = MEASURE
@@ DATA_TYPE  = $uint8_t$ 
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "J1939 Claimed address"
@@ END
*/
uint8_t J1939_NMAC_ECU_ClaimedAddr_u8;

J1939_81_NMAC_Name_St_t CA_Name_St;
J1939_81_NMAC_Name_St_t OtherCA_Name_St;

J1939_81_NMAC_FrameIdentifier_Uni_t J1939_81_NMAC_FrameIdentifier_Uni;

/*
@@ SYMBOL     = J1939_81_NMAC_State_En
@@ A2L_TYPE   = MEASURE
@@ DATA_TYPE  = $int32_t$ 
@@ CONVERSION = LINEAR $RADIX_0$ ""
@@ DESCRIPTION= "State of J1939-81 state machine"
@@ END
*/
J1939_NMAC_State_En_t J1939_81_NMAC_State_En = J1939_NMAC_NOT_DONE_E;


/*
*********************************************************************************
*    Module Function Prototypes
*********************************************************************************
*/


/*
*********************************************************************************
*    Function Definitions
*********************************************************************************
*/

/**
*  @brief    J1939_81_NMAC_Init: Init function to set the default values.
*  @param    none
*  @return   none
*/
void J1939_81_NMAC_Init(void)
{	
    /*Assign start address for claiming*/
    J1939_NMAC_ECU_ClaimedAddr_u8 = J1939_81_NMAC_STARTADDRESS;
    
    
    /*Clear the NAME*/
    CA_Name_St.LSBName_Uni.Name_u32 = 0U;
    CA_Name_St.MSBName_Uni.Name_u32 = 0U;
    
#if(NMAC_NAME_VIA_CALRAM)
    CA_Name_St.MSBName_Uni.Name.Arbitrary_Address_b1	=	CALRAM.J1939_81_NMAC.ARBITRARY_ADDRESS_b1;
    CA_Name_St.MSBName_Uni.Name.Industry_group_b3 	= 	CALRAM.J1939_81_NMAC.INDUSTRY_GROUP_b3;
    CA_Name_St.MSBName_Uni.Name.Vehivle_Instance_b4 	= 	CALRAM.J1939_81_NMAC.VEHICLE_INSTANCE_b4;
    CA_Name_St.MSBName_Uni.Name.Vehicle_System_b7	=	CALRAM.J1939_81_NMAC.VEHICLE_SYSTEM_b7;
    CA_Name_St.MSBName_Uni.Name.Function_b8		=	CALRAM.J1939_81_NMAC.FUNCTION_b8;
    CA_Name_St.MSBName_Uni.Name.Function_Instance_b5	=	CALRAM.J1939_81_NMAC.FUNCTION_INSTANCE_b5;
    CA_Name_St.MSBName_Uni.Name.ECU_Instance_b3	 	=	CALRAM.J1939_81_NMAC.ECU_INSTANCE_b3;
    CA_Name_St.LSBName_Uni.Name.Manufacturer_Code_b11	=	CALRAM.J1939_81_NMAC.MANUFACTURER_CODE_b11;
    CA_Name_St.LSBName_Uni.Name.Indentity_Number_21	=	CALRAM.J1939_81_NMAC.IDENTITY_NUMBER_b21;
#else	
    CA_Name_St.MSBName_Uni.Name.Arbitrary_Address_b1	=	J1939_81_NMAC_ARBITRARY_ADDRESS;
    CA_Name_St.MSBName_Uni.Name.Industry_group_b3 	= 	J1939_81_NMAC_INDUSTRY_GROUP;
    CA_Name_St.MSBName_Uni.Name.Vehivle_Instance_b4 	= 	J1939_81_NMAC_VEHICLE_INSTANCE;
    CA_Name_St.MSBName_Uni.Name.Vehicle_System_b7	=	J1939_81_NMAC_VEHICLE_SYSTEM;
    CA_Name_St.MSBName_Uni.Name.Function_b8		=	J1939_81_NMAC_FUNCTION;
    CA_Name_St.MSBName_Uni.Name.Function_Instance_b5	=	J1939_81_NMAC_FUNCTION_INSTANCE;
    CA_Name_St.MSBName_Uni.Name.ECU_Instance_b3	 	=	J1939_81_NMAC_ECU_INSTANCE;
    CA_Name_St.LSBName_Uni.Name.Manufacturer_Code_b11	=	J1939_81_NMAC_MANUFACTURER_CODE;
    CA_Name_St.LSBName_Uni.Name.Indentity_Number_21	=	J1939_81_NMAC_IDENTITY_NUMBER;
    
#endif
    
    /*Set the state as Address claim is in idle state*/
    J1939_81_NMAC_State_En = J1939_NMAC_IDLE_E;
    
    /*Set the default state*/
    J1939_81_NMAC_FrameIdentifier_Uni.Identifiers_u16 = 0U;
    
    
    return;
}



/**
*  @brief   Receive callback of J1939-81 network management address claim message  
*
*  @param   signalName      ENUM of configured CAN message
*           canMsgRx        Buffer for received CAN message
*
*  @return  None 
*/

void J1939_81_RXNMAC_Callback (uint16_t signalName, CAN_MessageFrame_St_t* canMsgRx)
{    
    uint32_t CanId_u32 = canMsgRx->ID_u32;
    
    PARAM_NOTUSED(signalName);    
    
    /*Get the PF and SA*/
    J1939_81_NMAC_FrameIdentifier_Uni.Identifiers.PDUFormat_u8 		= (uint8_t)((CanId_u32 >> 16U) & 0xffU);
    J1939_81_NMAC_FrameIdentifier_Uni.Identifiers.SourceAddress_u8 	= (uint8_t)(CanId_u32 & 0xffU);
    
    /*Fetch the NAME*/   
    J1939_tp_dataCopy(canMsgRx->DataBytes_au8, OtherCA_Name_St.MSBName_Uni.Name_au8, J1939_81_MSBNAME_LENGTH);
    J1939_tp_dataCopy(&canMsgRx->DataBytes_au8[4], OtherCA_Name_St.LSBName_Uni.Name_au8, J1939_81_LSBNAME_LENGTH);
    
    /*Identify message type and accordingly process the data*/
    switch(J1939_81_NMAC_FrameIdentifier_Uni.Identifiers.PDUFormat_u8)
    {
        /*Address Claimed Message*/
    case J1939_PF_ADDRESS_CLAIMED:   /* 0xEE*/
        {				    
            /*Check is address claim is done and is our priority high.*/
            bool IsMyPHigh_b = IsMyNameHigherPriority(OtherCA_Name_St, CA_Name_St);
            if((J1939_NMAC_DONE_E == J1939_81_NMAC_State_En) && IsMyPHigh_b)
            {  			    
                SendAddressClaimResponse(J1939_NMAC_ECU_ClaimedAddr_u8);
            }
            else
            {          
                /*Change the state and claim for address.  	*/		    
                J1939_81_NMAC_State_En = J1939_NMAC_IDLE_E;
            }
            break;
        }
    default:
        {
            break;
        }
    }
}

/**
*  @brief   Function to process the transmission frame right before
*	    sending the frame     
*
*  @param   pgn_u16                 PGN value
*           SourceAddr_u8	    Source address.
*
*  @return  None *
*/ 

static void J1939_81_TransmitMessage(uint32_t pgn_u16, uint8_t SourceAddr_u8) 
{
    CAN_MessageFrame_St_t Can_Applidata_St;
    /*Prepare the message*/
    Can_Applidata_St.DataLength_u8 = J1939_81_DATA_LENGTH;
    Can_Applidata_St.ID_u32 = (uint32_t)(J1939_BASE_ID + (pgn_u16 << 16U) + ((uint16_t)(J1939_GLOBAL_ADDR << 8U)) + SourceAddr_u8);
    
    /*Get the configured NAME*/  
    J1939_tp_dataCopy(CA_Name_St.MSBName_Uni.Name_au8, Can_Applidata_St.DataBytes_au8, J1939_81_MSBNAME_LENGTH);
    J1939_tp_dataCopy(CA_Name_St.LSBName_Uni.Name_au8, &Can_Applidata_St.DataBytes_au8[4], J1939_81_LSBNAME_LENGTH);
    
    /*Transmit the message*/
    Can_Applidata_St.MessageType_u8 = EXT_E;
#if(TRUE == DIAG_CONF_J1939_SUPPORTED)
    CIL_CAN_Tx_DynamicMsg(CIL_J1939_ACK_TX_E, Can_Applidata_St);
#endif
    
    /*Disable self reception*/
//    DisableSelfRx();
    return;
}


/**
*  @brief    J1939_81_nmac_50ms: 50ms task for address claim operation.
*  @param    none
*  @return   none
*/
void J1939_81_nmac_50ms(void)
{
    uint32_t	NMAC_ProtocolTimer_u32 = 0U;
    
    /*Check is Address claiming	enabled*/
    if (! J1939_ADDRESSCLAIMING_ACTIVE_FLAG_b)
    {                                               
        J1939_81_NMAC_State_En = J1939_NMAC_DONE_E;
        J1939_NMAC_ECU_ClaimedAddr_u8 = J1939_ECU_ID;
        CIL_CAN_SetClaimedAddr(J1939_NMAC_ECU_ClaimedAddr_u8); //Todo : sandeep
    }
    else
    {     
        /* state machine for address claim*/
        switch(J1939_81_NMAC_State_En)
        {
        	case J1939_NMAC_IDLE_E:
            {
                uint8_t retVal_s8; 		    
                /*Check is it request*/
                if(J1939_PF_ADDRESS_CLAIMED == J1939_81_NMAC_FrameIdentifier_Uni.Identifiers.PDUFormat_u8)
                {				
                    /*Process the received address claimed request.*/
                    retVal_s8 = AddressClaimHandling(J1939_81_NMAC_RX_MODE, OtherCA_Name_St);  				
                }
                else
                {
                    /*start the address claiming process*/
                    retVal_s8 = AddressClaimHandling(J1939_81_NMAC_TX_MODE, OtherCA_Name_St);
                }
                switch(retVal_s8)
                {
                case J1939_81_CANNOTCLAIM_E:
                    {
                        J1939_81_NMAC_State_En = J1939_NMAC_CANNOT_CLAIM_E;
                        break;
                    }
                case J1939AP_81_START_PROCESS_E:
                    {
                        J1939_81_NMAC_State_En = J1939_NMAC_PROCESSING_ADDR_CLAIM_E;
                        break;
                    }
                default:
                    {
                        /*Set the state as In progress*/
                        J1939_81_NMAC_State_En = J1939_NMAC_INPROGRESS_E;
                        break;
                    }
                }
                
                NMAC_ProtocolTimer_u32 = 0U;
                break;	
            }
        	case J1939_NMAC_INPROGRESS_E:
            {
                /*get elapsed time*/
                uint32_t msElapsed_u32 = TIME_DIFF_MS(NMAC_ProtocolTimer_u32);
                
                /*Check Address Claiming is in process*/
                if(msElapsed_u32 >= J1939_81_CONTENTION_WAIT_TIME_MS)
                {								
                    /*Set the flag as Address Claimed successfully*/
                    J1939_81_NMAC_State_En = J1939_NMAC_DONE_E;
                }
                break;	
            }
        	case J1939_NMAC_PROCESSING_ADDR_CLAIM_E:
            {
                /*Check is it Address Claimed from other CA.*/
                if(J1939_PF_ADDRESS_CLAIMED == J1939_81_NMAC_FrameIdentifier_Uni.Identifiers.PDUFormat_u8)
                {
                    /*Clear the Received info.*/
                    J1939_81_NMAC_FrameIdentifier_Uni.Identifiers_u16 = 0U;
                }
                
                /*Send the Address message*/
                SendAddressClaimResponse(J1939_NMAC_ECU_ClaimedAddr_u8);
                
                /*Get the current time*/
                NMAC_ProtocolTimer_u32 = GET_TIME_MS();
                
                /*Set the state as In progress*/
                J1939_81_NMAC_State_En = J1939_NMAC_INPROGRESS_E;
                break;	
            }
        	case J1939_NMAC_CANNOT_CLAIM_E:
            {
                SendAddressClaimResponse(J1939_NULL_ADDR);
                break;	
            }
        	case J1939_NMAC_DONE_E:
            {
                /*Reset the wait time*/
                NMAC_ProtocolTimer_u32 = 0U;
                
                /* Set the address for cil layer */
                CIL_CAN_SetClaimedAddr(J1939_NMAC_ECU_ClaimedAddr_u8);  //Todo : sandeep 
                break;	
            }
        	default:
            {
                /*Do nothing*/
                break;
            }
        }  
    }
    
}




/**
*  @brief   This function compare the received NAME and this CA's NAME..
*                                 
*
*  @param   otherName_St	Structure of NAME.
*           MyName_St        This CA's NAME 
*  @return  0 : if CA's NAME is less than the other name.
*			1  : if CA's NAME is greater than or equal to other name.
*/ 
static bool IsMyNameHigherPriority(J1939_81_NMAC_Name_St_t otherName_St, J1939_81_NMAC_Name_St_t MyName_St)
{
    bool IsPriorityHigh_b = (bool)TRUE;
    /*First check the MSB of NAME */
    if(otherName_St.MSBName_Uni.Name_u32 <= MyName_St.MSBName_Uni.Name_u32)
    {
        IsPriorityHigh_b = FALSE;
    }
    /*In MSB of both are different then LSB values doesn't matter
    But both MSBs are equal then LSB comes into picture to decide the 
    priority*/    
    if((otherName_St.MSBName_Uni.Name_u32 == MyName_St.MSBName_Uni.Name_u32) &&
       otherName_St.LSBName_Uni.Name_u32 <= MyName_St.LSBName_Uni.Name_u32)
    {
        IsPriorityHigh_b = FALSE;
    }
    return IsPriorityHigh_b;
}


/**
*  @brief   This routine is called if Arbitration bit field is set in NAME frame.
*			If our CA's NAME has lower priority than received CA's NAME then it
*           recalculate address to new one for claiming,
*                                 
*
*  @param   oldAddress_pu8	pointer which holding a old denied address.
*           minAddrRange    minimum address for claiming
*           maxAddRange     max address for claiming
*
*  @return  bool	true  if address is available for claiming.
*					false if no more address available for claiming.
*/ 
static bool CalcNextClaimAddress(void)
{	
    bool State_b = FALSE;
    if (((J1939_NMAC_ECU_ClaimedAddr_u8 > J1939_81_NMAC_MINADDRESS) || (0U == J1939_NMAC_ECU_ClaimedAddr_u8)) 
        && (J1939_NMAC_ECU_ClaimedAddr_u8 <= J1939_81_NMAC_MAXADDRESS))
    {
        J1939_NMAC_ECU_ClaimedAddr_u8++;
        State_b = (bool)TRUE;
    }
    
    /*no more address to try*/
    return State_b; 
}

/**
*  @brief   This routine is called either when the CA claims its address on bus or
*            another CA is trying to claim the same address on the bus.
*                                 
*
*  @param   mode:       J1939_81_NMAC_TX_MODE: indicates that CA is initiating 
*			a claim to its address.
*	    J1939_81_NMAC_RX_MODE: indicates an address claim message
*				   has been received and this CA must
*				   defend or give up its address.   
*           rxName_Uni       received CA's NAME
*
*  @return  int8_t       -1     No response
*                        0     Can not claim the address
*                        1     Start the Address Claim
*/ 
static uint8_t AddressClaimHandling(uint8_t modeRxTx, J1939_81_NMAC_Name_St_t  rxName_St)
{	
    uint8_t MachineState_u8 = J1939_81_NO_RESP_E;
    
    /*Check Address Claiming is the result of Request or ECU start.*/
    if (J1939_81_NMAC_TX_MODE == modeRxTx)
    {	
        /*Is address is in the range of min and max*/
        if (((J1939_NMAC_ECU_ClaimedAddr_u8 > J1939_81_NMAC_MINADDRESS) || (0U == J1939_NMAC_ECU_ClaimedAddr_u8)) 
            && (J1939_NMAC_ECU_ClaimedAddr_u8 <= J1939_81_NMAC_MAXADDRESS))
        {  
            /*If address available start the process 	*/	    
            MachineState_u8 = J1939AP_81_START_PROCESS_E;			
        }
        else
        {
            /*if address is not available the send as cannot claim address*/
            MachineState_u8 = J1939_81_CANNOTCLAIM_E;
        }
    }
    else
    {       
        
        /*If other CA try to claim address and if SA is  not same as its SA then don't send anything. */
        if (J1939_81_NMAC_FrameIdentifier_Uni.Identifiers.SourceAddress_u8 != J1939_NMAC_ECU_ClaimedAddr_u8)
        {
            MachineState_u8 = J1939_81_NO_RESP_E;
        }
        else
        {
            
            /*But if Both SAs are different then check for priority*/
            if (!IsMyNameHigherPriority (rxName_St, CA_Name_St))
            {
                /* Our priority is lower than other node.
                Two sections should be here
                1. Is CA supporting Arbitration Address mode? If yes, change the source address and again claim the new address
                2. If not means either CA support Single Address mode or there are no address for claiming then send cannot claim address message*/
                if ( CA_Name_St.MSBName_Uni.Name.Arbitrary_Address_b1)
                {
                    if (CalcNextClaimAddress())
                    {
                        /*Claim with different address
                        Set the flag as processing the address claimed msg*/
                        MachineState_u8 = J1939AP_81_START_PROCESS_E;	
                    }
                    else
                    {
                        
                        /*send as cannot claim address	
                        Set the flag as cannot claim address	*/
                        MachineState_u8 = J1939_81_CANNOTCLAIM_E;	
                    }
                }
                else
                {
                    
                    /*send as cannot claim address	
                    Set the flag as cannot claim address	*/
                    MachineState_u8 = J1939_81_CANNOTCLAIM_E;	
                }
            }
            else
            {
                /*our priority is higher than other CA, so re-claim 
                the current address as per mentioned in J1939-81 FIGURE D1  
                Page 58*/
                MachineState_u8 = J1939AP_81_START_PROCESS_E;
            }	
        }
    }
    
    return MachineState_u8;
}


/**
*  @brief    Function to handle All Address Claim message.
*  @param    srcAddr_u8    claimed address
*  @return   none
*/
static void SendAddressClaimResponse(uint8_t srcAddr_u8)
{
    /*Send as claimed Address */
    J1939_81_TransmitMessage(J1939_PF_CANNOT_CLAIM_ADDRESS, srcAddr_u8);
}

/**
*  @brief    Function to send the response to request for Address Claimed.
*  @param    none
*  @return   none
*/
void J1939_81_SendReqPGNResp(void)
{
    /*Check for Address Claiming process state*/
    switch(J1939_81_NMAC_State_En)
    {
    case J1939_NMAC_DONE_E:
        {
            /*Send the Address message*/
            SendAddressClaimResponse(J1939_NMAC_ECU_ClaimedAddr_u8);
            break;
        }
    case J1939_NMAC_CANNOT_CLAIM_E:
        {
            SendAddressClaimResponse(J1939_NULL_ADDR);
            break;
        }
    default:
        {
            break;
        }
    }    
}
