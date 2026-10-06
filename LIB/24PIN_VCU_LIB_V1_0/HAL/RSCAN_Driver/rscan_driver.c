/*******************************************************************************
|-------------------------------------------------------------------------------
|   FILE DESCRIPTION
|-------------------------------------------------------------------------------
|    File           : rscan_driver.h
|    Project        : CAN Driver development
|    Description    : The file is implements the RSCAN driver
|    Version        : V1_00
|-------------------------------------------------------------------------------
|
|-------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|-------------------------------------------------------------------------------
|   Date              Name                        Company
| ----------     ---------------     -----------------------------------
| 17/10/2023       Jeevan Jestin N             Sloki Software Technologies LLP
|-------------------------------------------------------------------------------
|******************************************************************************/

/*******************************************************************************
 *  HEADER FILE INCLUDES
 ******************************************************************************/
#include"rscan_driver.h"
#include"rscan_typedefs.h"
#include"rscan_reg.h"
/*******************************************************************************
 *  MACRO DEFINITION
 ******************************************************************************/

/*******************************************************************************
 *  GLOBAL VARIABLES DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STRUCTURE AND ENUM DEFNITION 
 ******************************************************************************/

/*******************************************************************************
 *  STATIC FUNCTION PROTOTYPES
 ******************************************************************************/
uint32_t GetRSCANbaseAddr(uint8_t Unit_u8);
void EnableRSCANunit0Interrupt(void);
uint8_t GetDlcData(uint8_t Length_u8);
CanRxCallback_Fptr_t CanRxCallback_Fptr = NULL;
/*******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : RSCANinit
*   Description   : The function initializes the RSCAN of RH850
*   Parameters    : Unit_u8 : RSCAN unit number
*                   RSCANchannelConf_pSt : Pointing conf struct of RSCAN
*   Return Value  : state of CAN init. refer "RsCanInitSt_En_t" enum
*  ---------------------------------------------------------------------------*/
uint8_t RSCANinit(uint8_t Unit_u8, RSCANchannelConf_St_t *RSCANchannelConf_pSt, RSCANBaudconfg_St_t *RSCANbaudconfg_pSt)
{
    uint8_t state_u8 = RSCAN_INIT_OK_E;
    uint8_t Channel_u8 = 0;
    uint16_t ReceieveRulePage_u16 = 0;
    uint16_t WindowReg_u16 = 0;
    uint16_t PageCnt_u16 = 0;
    uint16_t MsgCnt_u16 = 0;
    uint16_t RxFilterCnt_u16 = 0;
    uint16_t LoopCnt_u16 = 0;
    uint32_t Base_u32 = GetRSCANbaseAddr(Unit_u8);
    uint16_t i;

    if(0 == Base_u32)
    {
        return RSCAN_INVALID_UNIT_E;
    }

    
    while(CFDGSTS(Base_u32) & CAN_RAM_INIT_STS_MASK); /* CAN RAM initialization */
    CFDGCTR(Base_u32) =  (CFDGCTR(Base_u32) & GLOBAL_MODE_MASK_INV)|GLOBAL_RESET_MODE; /* Transition to global reset mode*/
    while((CFDGSTS(Base_u32) & GLOBAL_MODE_MASK) != GLOBAL_RESET_MODE); /* Wait for the transition to global reset mode */

    CFDGAFLECTR(Base_u32) = RECIEVE_RULE_TABLE_ENABLE|INIT_RECEIVE_PAGE_NUMBER;
    ReceieveRulePage_u16 = 0;

    CFDGCFG(Base_u32) &= ~CLKC; /* CAN module clock will used for the Buadrate generator*/
    /* Select external clock for buadrate generation */
    
    CFDGAFLCFG0(Base_u32) = 0x00; 

    for(Channel_u8 = 0;Channel_u8<RSCANconfg_St[Unit_u8].TotalChannel_u8;Channel_u8++)
    {
        LoopCnt_u16 = 0;
        CFDCmCTR(Base_u32, Channel_u8) = (CFDCmCTR(Base_u32, Channel_u8) & CHANNEL_MODE_MASK_INV) | CHANNEL_RESET_MODE;
        while ((CFDCmCTR(Base_u32, Channel_u8) & CHANNEL_MODE_MASK) != CHANNEL_RESET_MODE);
        
        if((true == RSCANchannelConf_pSt[Channel_u8].ChannelEn_u8)&&(Channel_u8 == RSCANchannelConf_pSt[Channel_u8].CanChannel_u8))
        {
            if ((RSCANbaudconfg_pSt[Channel_u8].NominalBitrate_En > _500K_BAUDRATE_E) && (RSCANbaudconfg_pSt[Channel_u8].DataBitrate_En > _2MBPS_BAUDRATE_E))
            {
                /* As per RH850 F1KMS1 
                Data bitrate max is 5MBPS(Condition NominalBitrate <= 500kbps)
                Data bitrate max is 2MBPS(If Nominal Bitrate is > 500kbps )*/
                state_u8 = RSCAN_INCORRECT_BAUDRATE_E;
            }
            else
            {
                switch (RSCANbaudconfg_pSt[Channel_u8].NominalBitrate_En)
                {
                    case _125K_BUADRATE_E:
                    {
                        CFDCmNCFG(Base_u32, Channel_u8) = NSJW | NTSEG_1 | NTSEG_2 | NBRP_125KBPS;
                        break;
                    }
                    case _250K_BAUDRATE_E:
                    {
                        CFDCmNCFG(Base_u32, Channel_u8) = NSJW | NTSEG_1 | NTSEG_2 | NBRP_250KBPS;
                        break;
                    }
                    case _500K_BAUDRATE_E:
                    {
                        CFDCmNCFG(Base_u32, Channel_u8) = NSJW | NTSEG_1 | NTSEG_2 | NBRP_500KBPS;
                        break;
                    }
                    case _1MBPS_BAUDRATE_E:
                    {
                        CFDCmNCFG(Base_u32, Channel_u8) = NSJW | NTSEG_1 | NTSEG_2 | NBRP_1MBPS;
                        break;
                    }
                    default:
                    {
                        /* By default baudrate will be set to 500kbps */
                        CFDCmNCFG(Base_u32, Channel_u8) = NSJW | NTSEG_1 | NTSEG_2 | NBRP_500KBPS;
                    }
                }

                if(CAN_FD_MODE_E == RSCANchannelConf_pSt[Channel_u8].CANMode_u8)
                {
                    switch (RSCANbaudconfg_pSt[Channel_u8].DataBitrate_En)
                    {
                        case _125K_BUADRATE_E:
                        {
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_1 | DTSEG_2 | DBRP_125KBPS;
                            break;
                        }
                        case _250K_BAUDRATE_E:
                        {
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_1 | DTSEG_2 | DBRP_250KBPS;
                            break;
                        }
                        case _500K_BAUDRATE_E:
                        {
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_1 | DTSEG_2 | DBRP_500KBPS;
                            break;
                        }
                        case _1MBPS_BAUDRATE_E:
                        {
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_1 | DTSEG_2 | DBRP_1MBPS;
                            break;
                        }
                        case _2MBPS_BAUDRATE_E:
                        {
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_2_2MBPS | DTESG_1_2MBPS | DBRP_2MBPS;
                            break;
                        }
                        case _4MBPS_BAUDRATE_E:
                        {
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_2_4MBPS | DTSEG_1_4MBPS | DBRP_4MBPS;
                            break;
                        }
                        case _5MBPS_BAUDRATE_E:
                        {
                            /* Change the TDC offset value based on the CAN bus length. If suitable TDC value is not applied
                            Chances of buss error is high in CAN FD at higher baudrate */
                            CFDCmFDCFG(Base_u32,Channel_u8) |= TDC_OFFSET|TDC_ENABLE|TDC_OFFSET_EN;
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_1 | DTSEG_2 | DBRP_5MBPS;
                            break;
                        }
                        default:
                        {
                            /* By default baudrate will be set to 500kbps */
                            CFDCmDCFG(Base_u32, Channel_u8) = DSJW | DTSEG_1 | DTSEG_2 | DBRP_500KBPS;
                        }
                    }
                }
            }

            if(RxFilterCnt_u16+RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8 > MAX_RX_RULE_PER_CHANNEL)
            {
                state_u8 = RSCAN_MAX_RX_CHN_RULE_E;
            }
            else if (RxFilterCnt_u16+RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8 > (MAX_RX_RULE_ALLOWED*RSCANconfg_St[Unit_u8].TotalChannel_u8))
            {
                state_u8 = RSCAN_MAX_RX_UNIT_RULE_E;
            }
            else if((0 == RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8)||(0 == RSCANchannelConf_pSt[Channel_u8].CanRxFilterConf_pSt))
            {
                state_u8 = RSCAN_RX_FILTER_NULL_E;
            }
            else
            {
                switch(Channel_u8)
                {
                    case 0:
                    {
                        CFDGAFLCFG0(Base_u32) |= RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8 << CHANNEL_0_RULE_POS;
                        RxFilterCnt_u16 += RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8;
                        break;
                    }
                    case 1:
                    {   
                        CFDGAFLCFG0(Base_u32) |= RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8 << CHANNEL_1_RULE_POS;
                        RxFilterCnt_u16 += RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8;
                        break;
                    }
                    case 2:
                    {
                        CFDGAFLCFG0(Base_u32) |= RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8 << CHANNEL_2_RULE_POS;
                        RxFilterCnt_u16 += RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8;
                        break;
                    }
                    default:
                    {

                    }
                }

                for(PageCnt_u16 = ReceieveRulePage_u16; PageCnt_u16 < TOTAL_PAGE; PageCnt_u16++)
                {
                    CFDGAFLECTR(Base_u32) |= ReceieveRulePage_u16;

                    for(MsgCnt_u16 = WindowReg_u16;MsgCnt_u16<MAX_RULE_PER_PAGE;MsgCnt_u16++)
                    {
                        CFDGAFLIDj(Base_u32,MsgCnt_u16) = 0x00;
                        CFDGAFLMj(Base_u32,MsgCnt_u16) = 0x00;
                        CFDGAFLP1j(Base_u32,MsgCnt_u16) = 0x00;
                        if(STD_IDE_E == RSCANchannelConf_pSt[Channel_u8].CanRxFilterConf_pSt[LoopCnt_u16].Ide_u8)
                        {
                            CFDGAFLIDj(Base_u32,MsgCnt_u16) &= (~(1 << IDE_POS)); 
                            CFDGAFLIDj(Base_u32,MsgCnt_u16) |= (0x7FF & RSCANchannelConf_pSt[Channel_u8].CanRxFilterConf_pSt[LoopCnt_u16].CanId_u32);
                            CFDGAFLMj(Base_u32,MsgCnt_u16) |= (0x7FF & RSCANchannelConf_pSt[Channel_u8].CanRxFilterConf_pSt[LoopCnt_u16].MaskId_u32);
                        }
                        else
                        {
                            CFDGAFLIDj(Base_u32,MsgCnt_u16) |= (uint32_t)(1 << IDE_POS);
                            CFDGAFLIDj(Base_u32,MsgCnt_u16) |= (0x1FFFFFFF & RSCANchannelConf_pSt[Channel_u8].CanRxFilterConf_pSt[LoopCnt_u16].CanId_u32);
                            CFDGAFLMj(Base_u32,MsgCnt_u16) |= (0x1FFFFFFF & RSCANchannelConf_pSt[Channel_u8].CanRxFilterConf_pSt[LoopCnt_u16].MaskId_u32);
                        }

                        CFDGAFLMj(Base_u32,MsgCnt_u16) |= (uint32_t)(1 << IDE_POS);
                        CFDGAFLP1j(Base_u32,MsgCnt_u16) = (1<<(TXRX_FIFO_SEL_POS+(Channel_u8 * RSCAN_UNIT0_TXRX_FIFO_BUFF_PER_CHANNEL)));
                        LoopCnt_u16++;

                        WindowReg_u16++;
                        if(LoopCnt_u16 == RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8)
                        {
                            break;
                        }
                    }


                    if(MsgCnt_u16 == MAX_RULE_PER_PAGE)
                    {
                        ReceieveRulePage_u16++;
                        WindowReg_u16 = 0;
                    }

                    if (LoopCnt_u16 == RSCANchannelConf_pSt[Channel_u8].TotalRxFilter_u8)
                    {
                        break;
                    }
                }
                
            }

        }
    }
    
    CFDGAFLECTR(Base_u32) &= RECIEVE_RULE_TABLE_DISABLE;
    
    for (Channel_u8 = 0; Channel_u8 < RSCANconfg_St[Unit_u8].TotalChannel_u8; Channel_u8++)
    {
        /* 1st TXRX FIFO of channel is configures as receive mode*/
        CFDCFCCk(Base_u32,((Channel_u8*RSCAN_UNIT0_TXRX_FIFO_BUFF_PER_CHANNEL))) = ON_MSG_INTC|TXRX_FIFO_DEPTH|RECEIVE_MODE|TXRXFIFO_PAYLOAD_SIZE; 
    }

    for (Channel_u8 = 0; Channel_u8 < RSCANconfg_St[Unit_u8].TotalChannel_u8; Channel_u8++)
    {
        /* 2nd TXRX FIFO of channel is configures as transmit mode*/
        CFDCFCCk(Base_u32,((Channel_u8*RSCAN_UNIT0_TXRX_FIFO_BUFF_PER_CHANNEL)+1)) = ON_MSG_INTC|TXRX_FIFO_DEPTH|TRANSMIT_MODE|TXRXFIFO_PAYLOAD_SIZE; 
    }

    EnableRSCANunit0Interrupt();

    CFDGCTR(Base_u32) =  (CFDGCTR(Base_u32) & GLOBAL_MODE_MASK_INV); /* Transition to global reset mode*/
    while((CFDGSTS(Base_u32) & GLOBAL_MODE_MASK) != GLOBAL_OPERATING_MODE); /* Wait for the transition to global reset mode */

    for (Channel_u8 = 0; Channel_u8 < RSCANconfg_St[Unit_u8].TotalChannel_u8; Channel_u8++)
    {
        /* TXRX FIFO RX_MODE need to be enable after unit is communication mode */
        CFDCFCCk(Base_u32,(Channel_u8*RSCAN_UNIT0_TXRX_FIFO_BUFF_PER_CHANNEL)) |= TXRX_FIFO_EN|TXRX_RECEIVE_INT_EN;
    }

    for(Channel_u8 = 0;Channel_u8<RSCANconfg_St[Unit_u8].TotalChannel_u8;Channel_u8++)
    {
        /* Transition all the channel to halt mode to enable the TXRX FIFO as transmit mode */
        if((true == RSCANchannelConf_pSt[Channel_u8].ChannelEn_u8)&&(Channel_u8 == RSCANchannelConf_pSt[Channel_u8].CanChannel_u8))
        {
            CFDCmCTR(Base_u32, Channel_u8) = (CFDCmCTR(Base_u32, Channel_u8) & CHANNEL_MODE_MASK_INV) | CHANNEL_HALT_MODE;
            while ((CFDCmCTR(Base_u32, Channel_u8) & CHANNEL_MODE_MASK) != CHANNEL_HALT_MODE);
        }
    }
    for(i = 0;i<0xFF;i++);

    for (Channel_u8 = 0; Channel_u8 < RSCANconfg_St[Unit_u8].TotalChannel_u8; Channel_u8++)
    {
        /* TXRX FIFO TX_MODE need to be enable after channel is in communication mode  */
        CFDCFCCk(Base_u32,((Channel_u8*RSCAN_UNIT0_TXRX_FIFO_BUFF_PER_CHANNEL)+1)) = TXRX_FIFO_EN;
        for(i = 0;i<0xFF;i++);
    }

    for(Channel_u8 = 0;Channel_u8<RSCANconfg_St[Unit_u8].TotalChannel_u8;Channel_u8++)
    {
        /* Transition all the channels to communication mode */
        if((true == RSCANchannelConf_pSt[Channel_u8].ChannelEn_u8)&&(Channel_u8 == RSCANchannelConf_pSt[Channel_u8].CanChannel_u8))
        {
            CFDCmCTR(Base_u32, Channel_u8) = (CFDCmCTR(Base_u32, Channel_u8) & CHANNEL_MODE_MASK_INV) | CHANNEL_OPERATING_MODE;
            while ((CFDCmCTR(Base_u32, Channel_u8) & CHANNEL_MODE_MASK) != CHANNEL_OPERATING_MODE);
        }
    }

    return state_u8;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : RSCANtransmit
*   Description   : The function transmit the CAN message
*   Parameters    : Unit_u8 : RSCAN unit number
*                   Channel_u8 : RSCAN channel number of the unit.
*                   DataBuff_pu8 : pointing to the data buffer to be transmitted.
*                   Dlc_u8 : Length of data to be transmitted.
*                   CanMode_u8 : 0 - Classic CAN
*                                1 - CANFD 
*                   Ide_u8 : 0 - Standard IDE
*                            1 - Extended IDE. 
*   Return Value  : Transmit State
*  ---------------------------------------------------------------------------*/
uint8_t RSCANtransmit(uint8_t Unit_u8,uint8_t Channel_u8,uint8_t* DataBuff_pu8,uint8_t Dlc_u8,uint8_t CanMode_u8,uint8_t Ide_u8,uint32_t CanId_u32)
{
    uint8_t TxRxFifo_u8 = 0;
    uint8_t State_u8 = RSCAN_TX_SUCCESS_E;
    volatile uint8_t* CanTxBuff_pu8 = 0;
    uint32_t Base_u32 = GetRSCANbaseAddr(Unit_u8);
    uint8_t i = 0;

    /* 2nd Fifobuffer of TXRX is configured as TX mode(So add one)*/
    TxRxFifo_u8 = (Channel_u8*RSCANconfg_St[Unit_u8].TransReceivefifoBuff_u8) + 1;

    if(0 == Base_u32)
    {
        State_u8 = RSCAN_TX_INVALID_UNIT_E;
    }
    else if(Channel_u8 >= RSCANconfg_St[Unit_u8].TotalChannel_u8)
    {
        State_u8 = RSCAN_TX_INVALID_CHANNEL_E;
    }
    else if((CLASSICAL_CAN_MODE_E == CanMode_u8) && (Dlc_u8 > CLASSIC_CAN_MAX_LEN))
    {
        State_u8 = RSCAN_INVALID_TX_LEN_E;
    }
    else if((CAN_FD_MODE_E == CanMode_u8) && (Dlc_u8 > CANFD_MAX_LEN))
    {
        State_u8 = RSCAN_INVALID_TX_LEN_E;
    }
    else if(CFDCFSTSk(Base_u32, TxRxFifo_u8) & TXRX_FIFO_FULL)
    {
        State_u8 = RSCAN_TX_FIFO_FULL_E;
    }
    else
    {
        if(Ide_u8 == EXT_IDE_E)
        {
            CFDCFIDk(Base_u32, TxRxFifo_u8) = EXT_IDE | (CanId_u32 & EXTENDED_ID_MASK);
        }
        else
        {
            CFDCFIDk(Base_u32, TxRxFifo_u8) = STD_IDE | (CanId_u32 & STANDARD_ID_MASK);
        }

        CFDCFPTRk(Base_u32, TxRxFifo_u8) = (GetDlcData(Dlc_u8)<<DLC_POS)&DLC_MASK;

        if(CAN_FD_MODE_E == CanMode_u8)
        {
            CFDCFFDCSTSk(Base_u32, TxRxFifo_u8) |= CANFD_FRAME;    
        }
        else
        {
            CFDCFFDCSTSk(Base_u32, TxRxFifo_u8) = 0;
        }
        CanTxBuff_pu8 = (volatile uint8_t *)(Base_u32 + 0x640C + (0x80 * TxRxFifo_u8));

        for (i = 0; i < CANFD_MAX_LEN; i++)
        {
            /*Initialize the CAN data buffer to 0x00 */
            CanTxBuff_pu8[i] = 0x00;
        }

        for (i = 0; i < Dlc_u8; i++)
        {
            /* Filling the CAN data buffer with actual data */
            CanTxBuff_pu8[i] = DataBuff_pu8[i];
        }
        CFDCFPCTRk(Base_u32, TxRxFifo_u8) = 0xFF;
    }
    return State_u8;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : GetDlcData
*   Description   : The function provides the DLC data based on the length
*   Parameters    : Length_u8 : Length to be transmitted. 
*   Return Value  : Dlc data.
*  ---------------------------------------------------------------------------*/
uint8_t GetDlcData(uint8_t Length_u8)
{
    uint8_t DlcData_u8;

    if(Length_u8 <= CLASSIC_CAN_MAX_LEN)
    {
        DlcData_u8 = Length_u8;
    }
    else if(Length_u8 <= CANFD_LEN_12)
    {
        DlcData_u8 = DLC_DATA_12;
    }
    else if(Length_u8 <= CANFD_LEN_16)
    {
        DlcData_u8 = DLC_DATA_16;
    }
    else if (Length_u8 <= CANFD_LEN_20)
    {
        DlcData_u8 = DLC_DATA_20;        
    }
    else if(Length_u8 <= CANFD_LEN_24)
    {
        DlcData_u8 = DLC_DATA_24;
    }
    else if(Length_u8 <= CANFD_LEN_32)
    {
        DlcData_u8 = DLC_DATA_32;
    }
    else if (Length_u8 <= CANFD_LEN_48)
    {
        DlcData_u8 = DLC_DATA_48;        
    }
    else if(Length_u8 <= CANFD_LEN_64)
    {
        DlcData_u8 = DLC_DATA_64;
    }
    else
    {
        DlcData_u8 = CLASSIC_CAN_MAX_LEN;
    }
    
    return DlcData_u8;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : GetRSCANbaseAddr
*   Description   : NONE
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
#pragma interrupt CAN_Rx_FIFO_ISR(enable=false, channel=23, fpu=true, callt=false)
void RSCANunitoRxFifoISR(void)
{
    
    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CAN0_TxRxFIFO_Receive_ISR
*   Description   : The ISR of CAN0 TXRX FIFO buffer
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
#pragma interrupt CAN0_TxRxFIFO_Receive_ISR(enable=false, channel=218, fpu=true, callt=false)
void CAN0_TxRxFIFO_Receive_ISR(void)
{
    static uint8_t CanData_au8[64];
    uint32_t Base_u32 = GetRSCANbaseAddr(UNIT_0);
    uint8_t TxRxFifo_u8 = TXRXFIFO_CHNL_0;
    uint8_t CanIde_u8 = STD_IDE_E;
    uint32_t CanId_u32 = 0;
    uint8_t Dlc_u8 = 0;
    uint8_t CanMode = CLASSICAL_CAN_MODE_E;
    uint8_t DataCnt_u8 = 0;
    volatile uint8_t* DataBuffPtr_pu8 = 0;
    uint8_t i = 0;

    for(i = 0;i<RSCANconfg_St[UNIT_0].TransReceivefifoBuff_u8;i++)
    {
        TxRxFifo_u8 += i;
        if((CFDCFSTSk(Base_u32,TxRxFifo_u8) & TXRXFIFO_RX_INT_MASK))
        {
            /* Clear TXRX FIFO rx interrupt */
            CFDCFSTSk(Base_u32, TxRxFifo_u8) &= (~TXRXFIFO_RX_INT_MASK);
            while (!(CFDCFSTSk(Base_u32, TxRxFifo_u8) & TXRXFIFO_EMPTY_MASK))
            {
                if (CFDCFIDk(Base_u32, TxRxFifo_u8) & TXRXFIFO_IDE_MASK)
                {
                    /* Recieved CAN ID is extended */
                    CanId_u32 = CFDCFIDk(Base_u32, TxRxFifo_u8) & EXTENDED_ID_MASK;
                    CanIde_u8 = EXT_IDE_E;
                }
                else
                {
                    /* Recieved CAN ID is standard */
                    CanId_u32 = CFDCFIDk(Base_u32, TxRxFifo_u8) & STANDARD_ID_MASK;
                    CanIde_u8 = STD_IDE_E;
                }

                if (!(CFDCFIDk(Base_u32, TxRxFifo_u8) & TXRXFIFO_RTR_MASK))
                {
                    /* Length of the received message */
                    Dlc_u8 = (CFDCFPTRk(Base_u32, TxRxFifo_u8) & DLC_MASK) >> DLC_POS;
                }

                if (CFDCFFDCSTSk(Base_u32, TxRxFifo_u8) & CAN_FRAME_MASK)
                {
                    CanMode = CAN_FD_MODE_E;
                    switch (Dlc_u8)
                    {
                    case 9:
                    {
                        Dlc_u8 = 12;
                    }
                    case 10:
                    {
                        Dlc_u8 = 16;
                        break;
                    }
                    case 11:
                    {
                        Dlc_u8 = 20;
                        break;
                    }
                    case 12:
                    {
                        Dlc_u8 = 24;
                        break;
                    }
                    case 13:
                    {
                        Dlc_u8 = 32;
                        break;
                    }
                    case 14:
                    {
                        Dlc_u8 = 48;
                        break;
                    }
                    case 15:
                    {
                        Dlc_u8 = 64;
                    }
                    }
                }
                else
                {
                    CanMode = CLASSICAL_CAN_MODE_E;
                }

                /* Pointing to the Data buffer of TxRxFIO data buffer*/
                DataBuffPtr_pu8 = (volatile uint8_t *)(Base_u32 + 0x640C + (0x80 * TxRxFifo_u8));
                for (DataCnt_u8 = 0; DataCnt_u8 < Dlc_u8; DataCnt_u8++)
                {
                    CanData_au8[DataCnt_u8] = DataBuffPtr_pu8[DataCnt_u8];
                }
                /* Move the FIFO buffer to next message */
                CFDCFPCTRk(Base_u32, TxRxFifo_u8) = NEXT_TXRXFIFO_MSG;
                if(CanRxCallback_Fptr)
                {
                    CanRxCallback_Fptr(UNIT_0,CAN_CHANNEL_1_E,CanData_au8,Dlc_u8,CanMode,CanIde_u8,CanId_u32);
                }
            }
        }
        
        if((CFDCFSTSk(Base_u32,TxRxFifo_u8) & TXRXFIFO_TX_INT_MASK))
        {
            CFDCFSTSk(Base_u32,TxRxFifo_u8) &= (~TXRXFIFO_TX_INT_MASK);
        }
    }
    return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CAN1_TxRxFIFO_Receive_ISR
*   Description   : The ISR of CAN0 TXRX FIFO buffer
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
#pragma interrupt CAN1_TxRxFIFO_Receive_ISR(enable=false, channel=218, fpu=true, callt=false)
void CAN1_TxRxFIFO_Receive_ISR(void)
{
    static uint8_t CanData_au8[64];
    uint32_t Base_u32 = GetRSCANbaseAddr(UNIT_0);
    uint8_t TxRxFifo_u8 = TXRXFIFO_CHNL_1;
    uint8_t CanIde_u8 = STD_IDE_E;
    uint32_t CanId_u32 = 0;
    uint8_t Dlc_u8 = 0;
    uint8_t CanMode = CLASSICAL_CAN_MODE_E;
    uint8_t DataCnt_u8 = 0;
    volatile uint8_t* DataBuffPtr_pu8 = 0;
    uint8_t i = 0;

    for(i = 0;i<RSCANconfg_St[UNIT_0].TransReceivefifoBuff_u8;i++)
    {
        TxRxFifo_u8 += i;
        if((CFDCFSTSk(Base_u32,TxRxFifo_u8) & TXRXFIFO_RX_INT_MASK))
        {
            /* Clear TXRX FIFO rx interrupt */
            CFDCFSTSk(Base_u32, TxRxFifo_u8) &= (~TXRXFIFO_RX_INT_MASK);
            while (!(CFDCFSTSk(Base_u32, TxRxFifo_u8) & TXRXFIFO_EMPTY_MASK))
            {
                if (CFDCFIDk(Base_u32, TxRxFifo_u8) & TXRXFIFO_IDE_MASK)
                {
                    /* Recieved CAN ID is extended */
                    CanId_u32 = CFDCFIDk(Base_u32, TxRxFifo_u8) & EXTENDED_ID_MASK;
                    CanIde_u8 = EXT_IDE_E;
                }
                else
                {
                    /* Recieved CAN ID is standard */
                    CanId_u32 = CFDCFIDk(Base_u32, TxRxFifo_u8) & STANDARD_ID_MASK;
                    CanIde_u8 = STD_IDE_E;
                }

                if (!(CFDCFIDk(Base_u32, TxRxFifo_u8) & TXRXFIFO_RTR_MASK))
                {
                    /* Length of the received message */
                    Dlc_u8 = (CFDCFPTRk(Base_u32, TxRxFifo_u8) & DLC_MASK) >> DLC_POS;
                }

                if (CFDCFFDCSTSk(Base_u32, TxRxFifo_u8) & CAN_FRAME_MASK)
                {
                    CanMode = CAN_FD_MODE_E;
                    switch (Dlc_u8)
                    {
                    case 9:
                    {
                        Dlc_u8 = 12;
                    }
                    case 10:
                    {
                        Dlc_u8 = 16;
                        break;
                    }
                    case 11:
                    {
                        Dlc_u8 = 20;
                        break;
                    }
                    case 12:
                    {
                        Dlc_u8 = 24;
                        break;
                    }
                    case 13:
                    {
                        Dlc_u8 = 32;
                        break;
                    }
                    case 14:
                    {
                        Dlc_u8 = 48;
                        break;
                    }
                    case 15:
                    {
                        Dlc_u8 = 64;
                    }
                    }
                }
                else
                {
                    CanMode = CLASSICAL_CAN_MODE_E;
                }

                /* Pointing to the Data buffer of TxRxFIO data buffer*/
                DataBuffPtr_pu8 = (volatile uint8_t *)(Base_u32 + 0x640C + (0x80 * TxRxFifo_u8));
                for (DataCnt_u8 = 0; DataCnt_u8 < Dlc_u8; DataCnt_u8++)
                {
                    CanData_au8[DataCnt_u8] = DataBuffPtr_pu8[DataCnt_u8];
                }
                /* Move the FIFO buffer to next message */
                CFDCFPCTRk(Base_u32, TxRxFifo_u8) = NEXT_TXRXFIFO_MSG;

                if(CanRxCallback_Fptr)
                {
                    CanRxCallback_Fptr(UNIT_0,CAN_CHANNEL_2_E,CanData_au8,Dlc_u8,CanMode,CanIde_u8,CanId_u32);
                }
            }
        }
        
        if((CFDCFSTSk(Base_u32,TxRxFifo_u8) & TXRXFIFO_TX_INT_MASK))
        {
            CFDCFSTSk(Base_u32,TxRxFifo_u8) &= (~TXRXFIFO_TX_INT_MASK);
        }
    }
    

    return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : CAN2_TxRxFIFO_Receive_ISR
*   Description   : The ISR of CAN0 TXRX FIFO buffer
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
#pragma interrupt CAN2_TxRxFIFO_Receive_ISR(enable=false, channel=218, fpu=true, callt=false)
void CAN2_TxRxFIFO_Receive_ISR(void)
{
    static uint8_t CanData_au8[64];
    uint32_t Base_u32 = GetRSCANbaseAddr(UNIT_0);
    uint8_t TxRxFifo_u8 = TXRXFIFO_CHNL_2;
    uint8_t CanIde_u8 = STD_IDE_E;
    uint32_t CanId_u32 = 0;
    uint8_t Dlc_u8 = 0;
    uint8_t CanMode = CLASSICAL_CAN_MODE_E;
    uint8_t DataCnt_u8 = 0;
    volatile uint8_t* DataBuffPtr_pu8 = 0;
    uint8_t i = 0;

    for(i = 0;i<RSCANconfg_St[UNIT_0].TransReceivefifoBuff_u8;i++)
    {
        TxRxFifo_u8 += i;
        if((CFDCFSTSk(Base_u32,TxRxFifo_u8) & TXRXFIFO_RX_INT_MASK))
        {
            /* Clear TXRX FIFO rx interrupt */
            CFDCFSTSk(Base_u32, TxRxFifo_u8) &= (~TXRXFIFO_RX_INT_MASK);
            while (!(CFDCFSTSk(Base_u32, TxRxFifo_u8) & TXRXFIFO_EMPTY_MASK))
            {
                if (CFDCFIDk(Base_u32, TxRxFifo_u8) & TXRXFIFO_IDE_MASK)
                {
                    /* Recieved CAN ID is extended */
                    CanId_u32 = CFDCFIDk(Base_u32, TxRxFifo_u8) & EXTENDED_ID_MASK;
                    CanIde_u8 = EXT_IDE_E;
                }
                else
                {
                    /* Recieved CAN ID is standard */
                    CanId_u32 = CFDCFIDk(Base_u32, TxRxFifo_u8) & STANDARD_ID_MASK;
                    CanIde_u8 = STD_IDE_E;
                }

                if (!(CFDCFIDk(Base_u32, TxRxFifo_u8) & TXRXFIFO_RTR_MASK))
                {
                    /* Length of the received message */
                    Dlc_u8 = (CFDCFPTRk(Base_u32, TxRxFifo_u8) & DLC_MASK) >> DLC_POS;
                }

                if (CFDCFFDCSTSk(Base_u32, TxRxFifo_u8) & CAN_FRAME_MASK)
                {
                    CanMode = CAN_FD_MODE_E;
                    switch (Dlc_u8)
                    {
                    case 9:
                    {
                        Dlc_u8 = 12;
                    }
                    case 10:
                    {
                        Dlc_u8 = 16;
                        break;
                    }
                    case 11:
                    {
                        Dlc_u8 = 20;
                        break;
                    }
                    case 12:
                    {
                        Dlc_u8 = 24;
                        break;
                    }
                    case 13:
                    {
                        Dlc_u8 = 32;
                        break;
                    }
                    case 14:
                    {
                        Dlc_u8 = 48;
                        break;
                    }
                    case 15:
                    {
                        Dlc_u8 = 64;
                    }
                    }
                }
                else
                {
                    CanMode = CLASSICAL_CAN_MODE_E;
                }

                /* Pointing to the Data buffer of TxRxFIO data buffer*/
                DataBuffPtr_pu8 = (volatile uint8_t *)(Base_u32 + 0x640C + (0x80 * TxRxFifo_u8));
                for (DataCnt_u8 = 0; DataCnt_u8 < Dlc_u8; DataCnt_u8++)
                {
                    CanData_au8[DataCnt_u8] = DataBuffPtr_pu8[DataCnt_u8];
                }

                /* Move the FIFO buffer to next message */
                CFDCFPCTRk(Base_u32, TxRxFifo_u8) = NEXT_TXRXFIFO_MSG;
                if(CanRxCallback_Fptr)
                {
                    CanRxCallback_Fptr(UNIT_0,CAN_CHANNEL_3_E,CanData_au8,Dlc_u8,CanMode,CanIde_u8,CanId_u32);
                }
            }
        }

        if((CFDCFSTSk(Base_u32,TxRxFifo_u8) & TXRXFIFO_TX_INT_MASK))
        {
            CFDCFSTSk(Base_u32,TxRxFifo_u8) &= (~TXRXFIFO_TX_INT_MASK);
        }
    }

    return;
}

/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : EnableRSCANunit0Interrupt
*   Description   : The function enable the CAN interrupt
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
void EnableRSCANunit0Interrupt(void)
{
    /* The interrupts are table reference method */

    /* CAN0 error interrupt */
    INTC1.ICRCAN0ERR.BIT.TBRCAN0ERR = 1;      
    INTC1.ICRCAN0ERR.BIT.MKRCAN0ERR = 0;
    INTC1.ICRCAN0ERR.BIT.RFRCAN0ERR = 0;
    
    /* CAN0 Transmit interrupt */
    INTC1.ICRCAN0TRX.BIT.TBRCAN0TRX = 1;
    INTC1.ICRCAN0TRX.BIT.MKRCAN0TRX = 0;
    INTC1.ICRCAN0TRX.BIT.RFRCAN0TRX = 0;
 
    /* CAN0 TxRx FIFO interrupt */
    INTC1.ICRCAN0REC.BIT.TBRCAN0REC = 1;
    INTC1.ICRCAN0REC.BIT.MKRCAN0REC = 0;
    INTC1.ICRCAN0REC.BIT.RFRCAN0REC = 0;
   
    /* CAN1 error interrupt */
    INTC2.ICRCAN1ERR.BIT.TBRCAN1ERR = 1;      
    INTC2.ICRCAN1ERR.BIT.MKRCAN1ERR = 0;
    INTC2.ICRCAN1ERR.BIT.RFRCAN1ERR = 0;
    
    /* CAN1 Transmit interrupt */
    INTC2.ICRCAN1TRX.BIT.TBRCAN1TRX = 1;
    INTC2.ICRCAN1TRX.BIT.MKRCAN1TRX = 0;
    INTC2.ICRCAN1TRX.BIT.RFRCAN1TRX = 0;
 
    /* CAN1 TxRx FIFO interrupt */
    INTC2.ICRCAN1REC.BIT.TBRCAN1REC = 1;
    INTC2.ICRCAN1REC.BIT.MKRCAN1REC = 0;
    INTC2.ICRCAN1REC.BIT.RFRCAN1REC = 0;
   
    /* CAN2 error interrupt */
    INTC2.ICRCAN2ERR.BIT.TBRCAN2ERR = 1;      
    INTC2.ICRCAN2ERR.BIT.MKRCAN2ERR = 0;
    INTC2.ICRCAN2ERR.BIT.RFRCAN2ERR = 0;
    
    /* CAN2 Transmit interrupt */
    INTC2.ICRCAN2TRX.BIT.TBRCAN2TRX = 1;
    INTC2.ICRCAN2TRX.BIT.MKRCAN2TRX = 0;
    INTC2.ICRCAN2TRX.BIT.RFRCAN2TRX = 0;
 
    /* CAN2 TxRx FIFO interrupt */
    INTC2.ICRCAN2REC.BIT.TBRCAN2REC = 1;
    INTC2.ICRCAN2REC.BIT.MKRCAN2REC = 0;
    INTC2.ICRCAN2REC.BIT.RFRCAN2REC = 0;
   
    /* UNIT_0 CAN Global error interrupt */
    INTC1.ICRCANGERR0.BIT.TBRCANGERR0 = 1;
    INTC1.ICRCANGERR0.BIT.MKRCANGERR0 = 0;
    INTC1.ICRCANGERR0.BIT.RFRCANGERR0 = 0;

    /* UNIT_0 CAN receive interrupt interrupt */
    INTC1.ICRCANGRECC0.BIT.TBRCANGRECC0 = 1;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 0;
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = 0;
    return;
}
/* -----------------------------------------------------------------------------
*  FUNCTION DESCRIPTION
*  -----------------------------------------------------------------------------
*   Function Name : GetRSCANbaseAddr
*   Description   : The function returns the base address
*   Parameters    : None
*   Return Value  : None
*  ---------------------------------------------------------------------------*/
uint32_t GetRSCANbaseAddr(uint8_t Unit_u8)
{
    uint32_t BaseAddr_u32 = 0;
    switch(Unit_u8)
    {
        case 0:
        {
            BaseAddr_u32 = RSCANFD0_BASE;
            break;
        }
        default:
        {

        }
    }

    return BaseAddr_u32;
}


/*---------------------- End of File -----------------------------------------*/