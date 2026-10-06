/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2018, 2025 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_PORT.c
* Component Version: 1.8.0
* Device(s)        : R7F701687
* Description      : This file implements device driver for Config_PORT.
***********************************************************************************************************************/
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_PORT.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint32_t g_cg_sync_read;
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_PORT_Create
* Description  : This function initializes the PORT
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_PORT_Create(void)
{
    PORT.PIBC10 = _PORT_PIBC_INIT;
    PORT.PIBC11 = _PORT_PIBC_INIT;
    PORT.PBDC10 = _PORT_PBDC_INIT;
    PORT.PBDC11 = _PORT_PBDC_INIT;
    PORT.PM10 = _PORT_PM_INIT;
    PORT.PM11 = _PORT_PM_INIT;
    PORT.PMC10 = _PORT_PMC_INIT;
    PORT.PMC11 = _PORT_PMC_INIT;
    PORT.PIPC10 = _PORT_PIPC_INIT;
    PORT.PIPC11 = _PORT_PIPC_INIT;
    /* PORT10 setting */
    PORT.PPCMD10 = _WRITE_PROTECT_COMMAND;
    PORT.PDSC10 = _PORT_PDSCn3_SLOW_MODE_SELECT | _PORT_PDSCn4_SLOW_MODE_SELECT | _PORT_PDSCn5_SLOW_MODE_SELECT | 
                  _PORT_PDSCn10_SLOW_MODE_SELECT | _PORT_PDSCn13_SLOW_MODE_SELECT | _PORT_PDSCn14_SLOW_MODE_SELECT;
    PORT.PDSC10 = (uint32_t) ~(_PORT_PDSCn3_SLOW_MODE_SELECT | _PORT_PDSCn4_SLOW_MODE_SELECT | 
                  _PORT_PDSCn5_SLOW_MODE_SELECT | _PORT_PDSCn10_SLOW_MODE_SELECT | _PORT_PDSCn13_SLOW_MODE_SELECT | 
                  _PORT_PDSCn14_SLOW_MODE_SELECT);
    PORT.PDSC10 = _PORT_PDSCn3_SLOW_MODE_SELECT | _PORT_PDSCn4_SLOW_MODE_SELECT | _PORT_PDSCn5_SLOW_MODE_SELECT | 
                  _PORT_PDSCn10_SLOW_MODE_SELECT | _PORT_PDSCn13_SLOW_MODE_SELECT | _PORT_PDSCn14_SLOW_MODE_SELECT;
    PORT.PPCMD10 = _WRITE_PROTECT_COMMAND;
    PORT.PODC10 = _PORT_PODCn3_PUSH_PULL | _PORT_PODCn4_PUSH_PULL | _PORT_PODCn5_PUSH_PULL | _PORT_PODCn10_PUSH_PULL | 
                  _PORT_PODCn13_PUSH_PULL | _PORT_PODCn14_PUSH_PULL;
    PORT.PODC10 = (uint32_t) ~(_PORT_PODCn3_PUSH_PULL | _PORT_PODCn4_PUSH_PULL | _PORT_PODCn5_PUSH_PULL | 
                  _PORT_PODCn10_PUSH_PULL | _PORT_PODCn13_PUSH_PULL | _PORT_PODCn14_PUSH_PULL);
    PORT.PODC10 = _PORT_PODCn3_PUSH_PULL | _PORT_PODCn4_PUSH_PULL | _PORT_PODCn5_PUSH_PULL | _PORT_PODCn10_PUSH_PULL | 
                  _PORT_PODCn13_PUSH_PULL | _PORT_PODCn14_PUSH_PULL;
    PORT.PBDC10 = _PORT_PBDCn3_MODE_DISABLED | _PORT_PBDCn4_MODE_DISABLED | _PORT_PBDCn5_MODE_DISABLED | 
                  _PORT_PBDCn10_MODE_DISABLED | _PORT_PBDCn13_MODE_DISABLED | _PORT_PBDCn14_MODE_DISABLED;
    PORT.PU10 = _PORT_PUn0_PULLUP_OFF | _PORT_PUn1_PULLUP_OFF | _PORT_PUn2_PULLUP_OFF | _PORT_PUn6_PULLUP_OFF | 
                _PORT_PUn7_PULLUP_OFF | _PORT_PUn8_PULLUP_OFF | _PORT_PUn9_PULLUP_OFF | _PORT_PUn11_PULLUP_OFF | 
                _PORT_PUn12_PULLUP_OFF | _PORT_PUn15_PULLUP_OFF;
    PORT.PD10 = _PORT_PDn0_PULLDOWN_OFF | _PORT_PDn1_PULLDOWN_OFF | _PORT_PDn2_PULLDOWN_OFF | 
                _PORT_PDn6_PULLDOWN_OFF | _PORT_PDn7_PULLDOWN_OFF | _PORT_PDn8_PULLDOWN_OFF | 
                _PORT_PDn9_PULLDOWN_OFF | _PORT_PDn11_PULLDOWN_OFF | _PORT_PDn12_PULLDOWN_OFF | 
                _PORT_PDn15_PULLDOWN_OFF;
    PORT.PIS10 = _PORT_PISn0_TYPE_SHMT4 | _PORT_PISn1_TYPE_SHMT4 | _PORT_PISn2_TYPE_SHMT4 | _PORT_PISn6_TYPE_SHMT4 | 
                 _PORT_PISn7_TYPE_SHMT4 | _PORT_PISn8_TYPE_SHMT4 | _PORT_PISn9_TYPE_SHMT1 | _PORT_PISn11_TYPE_SHMT4 | 
                 _PORT_PISn12_TYPE_SHMT4 | _PORT_PISn15_TYPE_SHMT4;
    PORT.P10 = _PORT_Pn3_OUTPUT_LOW | _PORT_Pn4_OUTPUT_LOW | _PORT_Pn5_OUTPUT_LOW | _PORT_Pn10_OUTPUT_LOW | 
               _PORT_Pn13_OUTPUT_LOW | _PORT_Pn14_OUTPUT_LOW;
    PORT.PM10 = _PORT_PMn0_MODE_UNUSED | _PORT_PMn1_MODE_UNUSED | _PORT_PMn2_MODE_UNUSED | _PORT_PMn3_MODE_OUTPUT | 
                _PORT_PMn4_MODE_OUTPUT | _PORT_PMn5_MODE_OUTPUT | _PORT_PMn6_MODE_UNUSED | _PORT_PMn7_MODE_UNUSED | 
                _PORT_PMn8_MODE_UNUSED | _PORT_PMn9_MODE_INPUT | _PORT_PMn10_MODE_OUTPUT | _PORT_PMn11_MODE_UNUSED | 
                _PORT_PMn12_MODE_UNUSED | _PORT_PMn13_MODE_OUTPUT | _PORT_PMn14_MODE_OUTPUT | _PORT_PMn15_MODE_UNUSED;
    PORT.PIBC10 = _PORT_PIBCn9_INPUT_BUFFER_ENABLE;
    /* PORT11 setting */
    PORT.PPCMD11 = _WRITE_PROTECT_COMMAND;
    PORT.PDSC11 = _PORT_PDSCn1_SLOW_MODE_SELECT;
    PORT.PDSC11 = (uint32_t) ~(_PORT_PDSCn1_SLOW_MODE_SELECT);
    PORT.PDSC11 = _PORT_PDSCn1_SLOW_MODE_SELECT;
    PORT.PPCMD11 = _WRITE_PROTECT_COMMAND;
    PORT.PODC11 = _PORT_PODCn1_PUSH_PULL;
    PORT.PODC11 = (uint32_t) ~(_PORT_PODCn1_PUSH_PULL);
    PORT.PODC11 = _PORT_PODCn1_PUSH_PULL;
    PORT.PBDC11 = _PORT_PBDCn1_MODE_DISABLED;
    PORT.P11 = _PORT_Pn1_OUTPUT_LOW;
    PORT.PM11 = _PORT_PM11_DEFAULT_VALUE | _PORT_PMn0_MODE_UNUSED | _PORT_PMn1_MODE_OUTPUT | _PORT_PMn2_MODE_UNUSED | 
                _PORT_PMn3_MODE_UNUSED | _PORT_PMn4_MODE_UNUSED;
    /* Synchronization processing */
    g_cg_sync_read = PORT.PM10;
    __syncp();

    R_Config_PORT_Create_UserInit();
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
