/******************************************************************************
 *    FILENAME    : uds_DTC.c
 *    DESCRIPTION : Utility functions for DTC services (14h & 19h).
 ******************************************************************************
 * Revision history
 *  
 * Ver Author          Date                     Description
 * 1   Sushil      13/01/2019		   Initial version
 ******************************************************************************
 */

/*
 **************************************************************************************************
 *    Includes
 **************************************************************************************************
 */
#include "uds_DTC.h"
#include "uds_conf.h"
#include "iso14229_serv14.h"
#include "diag_sys_conf.h"
//#if(TRUE == DIAG_CONF_FM_SUPPORTED)
#include "fm.h"
#include "fmff_conf.h"
#include "fm_level1.h"
#include "fm_level2.h"
#include "uds_DID_conf.h"
//#endif

#define TEST_FAILED                                             0x00
#define TEST_FAILED_THIS_OPERATION_CYCLE                        0x01
#define PENDING_DTC                                             0x02
#define CONFIRMED_DTC                                           0x03
#define TEST_NOT_COMPLETE_SINCE_LAST_CLEAR                      0x04
#define TEST_FAILED_SINCE_LAST_CLEAR                            0x05
#define TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE                 0x06
#define WARNING_INDICATOR_REQUESTED                             0x07

#define SAE_J2012_DA_DTCFORMAT_00                               0x00
#define ISO14229_1DTCFORMAT                                     0x01
#define ZERO_SER19                                              0x00

#define SUBFUN_1_RESP_LEN                                       0x05
/*
 **************************************************************************************************
 *    Global Variables
 **************************************************************************************************
 */

/*
 **************************************************************************************************
 *    Function Prototype
 **************************************************************************************************
 */

/**
 *    @brief  Converts FM status bits frame format to UDS status frame format
 *
 *    @param  status_u8 FM Status
 *    @return UDS status
 */
//static uint8_t StatusDecode(uint8_t status_u8);
/*
 **************************************************************************************************
 *    Function Definition
 **************************************************************************************************
 */
/**
 *    @brief  Converts FM status bits frame format to UDS status frame format
 *
 *    @param  status_u8 FM Status
 *    @return UDS status
 */
// static uint8_t StatusDecode(FM_DTCStatus_Un_t FM_DTCStatus_Un)
// {

// uint8_t actual_status_u8 = 0x00u ;
// if (FM_DTCStatus_Un.status_bit.L1_b1)
// {
// actual_status_u8 = actual_status_u8 | 0x01 ;
// }
// if((FM_DTCStatus_Un.status_bit.L2P_b2) || ())
// {
// actual_status_u8 = actual_status_u8 | 0x02 ;
// }

// if(0x01 & status_u8){
// actual_status_u8 = actual_status_u8 | 0x03 ;
// }
// if(0x02 & status_u8){
// actual_status_u8 = actual_status_u8 | 0x04 ;
// }
// if(0x04 & status_u8){
// actual_status_u8 = actual_status_u8 | 0x08 ;
// }
// if(0x20 & status_u8){
// actual_status_u8 = actual_status_u8 | 0x50 ;
// }
// if(0x40 & status_u8){
// actual_status_u8 = actual_status_u8 | 0x02 ;
// }
// if(0x80 & status_u8){
// // todo actual_status_u8 = actual_status_u8 | 0x80 ;
// }
// return actual_status_u8 ;
// }
/**
 *    @brief  This function will take one status mask value which is 1-byte.
 *            Do AND operation between status_mask_u8 and DTC actual status mask if it results one
 *            then increment the numberoffaults variable by 1 and at last return it.
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */

uint8_t check_st_u8;
uint16_t GetNumOfFaultsByMask(uint8_t* service_buffer_pu8)
{                        //0x01
	uint32_t DTC_u32 = 0x00u;   
	uint16_t response_length_s16 = 0x00u;
    uint16_t number_of_faults_u16   = 0x00u ;
//    uint16_t j                      = 0x00u ;
	uint8_t i = 0x00u;
	uint8_t status_mask_u8 = 0x00u;
	uint8_t actual_status_u8 = 0x00u;
	//uint8_t status_of_DTC_u8        = 0x00u ;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
	int8_t number_of_DTCs_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;
	uint8_t StatusAvalMask_u8 = FM_GetStatusAvailabiltyMask();
	status_mask_u8 = service_buffer_pu8[1];
	
	//number_of_DTCs_u8 = FM_GetTestActiveDTCs(DTCbuffer_au32) ;
	number_of_DTCs_u8 = FM_GetAllSupportedDTCs(DTCbuffer_au32, FM_PROTO_UDS_E); // TODO: Hareesh (19/06)
	if (ZERO_SER19 <= number_of_DTCs_u8)
	{
		for (i = 0; i < number_of_DTCs_u8; i++)
		{
			DTC_u32 = DTCbuffer_au32[i];
			FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
			actual_status_u8 = FM_DTCStatus_Un.Status_u8;
			if ((status_mask_u8) & (actual_status_u8))
			{
				number_of_faults_u16++;
			}
		}
		service_buffer_pu8[0] = SERV19_NUMBER_OF_DTC_BY_STATUS_MASK_E;
		service_buffer_pu8[1] = StatusAvalMask_u8;
		service_buffer_pu8[2] = SAE_J2012_DA_DTCFORMAT_00;
		service_buffer_pu8[3] = number_of_faults_u16 >> 8;
		service_buffer_pu8[4] = number_of_faults_u16 & 0xff;
		response_length_s16 = SUBFUN_1_RESP_LEN;
	}
#endif
	return response_length_s16;
}

/**
 *    @brief  This function will take one buffer address as an argument and fault manager has to write the DTCs. 
 *            fault manager has to write immediate next to their respective DTCs. 
 *    @param DTCBuff    :   address of buffer    
 *    @return length of data
 */
uint16_t GetDTCsByStatusMask(uint8_t* service_buffer_pu8)
{                         //0x02
	uint32_t DTC_u32 = 0x00u;
	uint16_t response_length_s16 = 0x00u;
//    uint16_t j                      = 0x00u ;
	uint8_t i = 0x00u;
	uint8_t status_mask_u8 = 0x00u;
	uint8_t main_buffer_index_u8 = 0x02u;
	uint8_t actual_status_u8 = 0x00u;
//    uint8_t status_of_DTC_u8        = 0x00u ;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
	int8_t number_of_DTCs_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	status_mask_u8 = service_buffer_pu8[1];

	//number_of_DTCs_u8 = FM_GetTestActiveDTCs(DTCbuffer_au32) ;
	number_of_DTCs_u8 = FM_GetAllSupportedDTCs(DTCbuffer_au32, FM_PROTO_UDS_E); // TODO: Hareesh (19/06)

	if (ZERO_SER19 <= number_of_DTCs_u8)
	{
		service_buffer_pu8[0] = SERV19_DTC_BY_STATUS_MASK_E;
		service_buffer_pu8[1] = FM_GetStatusAvailabiltyMask();

		for (i = 0; i < number_of_DTCs_u8; i++)
		{
			DTC_u32 = DTCbuffer_au32[i];
			//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32);
			//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
			FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
			actual_status_u8 = FM_DTCStatus_Un.Status_u8;

			if (status_mask_u8 & actual_status_u8)
			{
				service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 16);
				main_buffer_index_u8++;
				service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 8);
				main_buffer_index_u8++;
				service_buffer_pu8[main_buffer_index_u8] = DTC_u32;
				main_buffer_index_u8++;
				service_buffer_pu8[main_buffer_index_u8] = actual_status_u8;
				main_buffer_index_u8++;
			}
		}
		response_length_s16 = main_buffer_index_u8 - 1;
	}
#endif
	return (response_length_s16 + 1);
}
/**
 *    @brief  This function will take one buffer address as an argument.
 *               fault manager has to write the snapshot dtc values with their individual record numbers
 *               the DTC and the record value should be immediate next to each other
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetDTCSnapshotIdentification(uint8_t* service_buffer_pu8)
{                //0x03
	uint32_t DTC_u32 = 0x00u;
	uint16_t response_length_s16 = 0x00u;
	uint16_t main_buffer_index_u16 = 0x01u;
	uint8_t number_of_records_u8 = 0x00u;
	uint8_t snapshot_records_ID_au8[10] =
	{ 0 };
	uint32_t DTC_au32[100] =
	{ 0 };
	uint8_t number_of_DTCs = 0x00u;
	uint8_t i = 0x00u;
	uint8_t j = 0x00u;
	service_buffer_pu8[0] = SERV19_DTC_SNAPSHOT_IDENTIFICATION_E;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	number_of_DTCs = FM_GetAllSnapshotRecordDTCs(DTC_au32);
	for (i = 0; i < number_of_DTCs; i++)
	{
		DTC_u32 = DTC_au32[i];
		number_of_records_u8 = FM_GetAllSnapshotRecordIDByDTC(DTC_u32,
				snapshot_records_ID_au8);
		for (j = 0; j < number_of_records_u8; j++)
		{
			service_buffer_pu8[main_buffer_index_u16] = (DTC_u32 >> 16);
			main_buffer_index_u16++;
			service_buffer_pu8[main_buffer_index_u16] = (DTC_u32 >> 8);
			main_buffer_index_u16++;
			service_buffer_pu8[main_buffer_index_u16] = DTC_u32;
			main_buffer_index_u16++;
			service_buffer_pu8[main_buffer_index_u16] =
					snapshot_records_ID_au8[j];
			main_buffer_index_u16++;
		}
	}
#endif
	response_length_s16 = main_buffer_index_u16;
	return (response_length_s16);
}

/**
 *    @brief  In this function we will send data buffer address which have DTC value and DTC snapshot record number.
 *               fault manager has to take the values and rewrite the buffer in following way
 *               DTC which is                         3 bytes
 *               DTCstatus                            1 byte
 *               DTCSnapshotRecordNumber              1 byte
 *               DTCSnapshotRecordNumberOfIdentifiers 1 byte
 *               data identifiers                     2bytes
 *               DTC snapshot records                 ...
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetDTCSnapshotRecordByDTCNumber(uint8_t* service_buffer_pu8)     //0x04
{
	uint32_t idxConfigTab_u32 = 0;
	uint32_t idxTempBuff_u32 = 0;
	u16_FaultPath_t FaultPath_En;
	uint8_t paramSize_u8 =0;
	uint32_t idxFrzFrmStorage_u32 = 0;
	uint32_t DTC_u32 = 0x00u;
	int16_t response_length_s16 = -1;
	int16_t Ret_s16 = 0;
	uint16_t did_u16 = 0;
	uint16_t NumOfDIDs_u16 = 0;
	uint8_t record_number_u8 = 0x00u;
//	uint8_t  status_of_DTC_u8                	= 0x00u ;
	uint8_t actual_status_u8 = 0x00u;
	uint8_t i = 0x00u;
	uint8_t j = 0x00u;
	uint8_t FltType_u8 = 0;
	int8_t FltIndex_s8 = -1;
	bool DtcPresentInL1_b = FALSE;
	bool DtcPresentInL2_b = FALSE;
	uint8_t tempFrzFrmBuff_au8[100];
//	uint8_t	paramSize_u8                    = 0;        

	DTC_u32 = service_buffer_pu8[1];
	DTC_u32 = (DTC_u32 << 8) | service_buffer_pu8[2];
	DTC_u32 = (DTC_u32 << 8) | service_buffer_pu8[3];
	record_number_u8 = service_buffer_pu8[4];

#if(TRUE == DIAG_CONF_FM_SUPPORTED)
	FaultPath_En = 0;

	Ret_s16 = GetFltPathFltType_ByDTC(DTC_u32, &FaultPath_En, &FltType_u8,
			FM_PROTO_UDS_E);
	if (-1 == Ret_s16)
	{
		response_length_s16 = -1;
	}
	else
	{
		FltIndex_s8 = FML2_GetFltIndxOfPath((FM_FaultPath_En_t) FaultPath_En,
				FltType_u8);
		//FltIndex_s8 = FML2_GetFaultIndexByDTC(DTC_u32);
		// Valid DTC request.
		if (-1 == FltIndex_s8)
		{
			if ((SERV19_RECORDNUMBER_SUPPORTED == record_number_u8) || (SERV19_RECORDNUMBER_SUPPORTED_ALL == record_number_u8))
			{
				FM_DTCStatus_Un_t FM_DTCStatus_Un1;
				FM_DTCStatus_Un1 = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
				actual_status_u8 = FM_DTCStatus_Un1.Status_u8;
				service_buffer_pu8[4] = actual_status_u8;
				service_buffer_pu8[5] = SERV19_RECORDNUMBER_SUPPORTED;
				response_length_s16 = 7;
			}
		}
		else
		{
			// higher priority to L2 for freeze frames.
			DtcPresentInL2_b = TRUE;
		}

		if (DtcPresentInL1_b || DtcPresentInL2_b)
		{
			/*If record number is other than 0xFF then send respective DTC's snapshot record
			 otherwise get the all DTCs having snapshotrecord and send it to client
			 [ISO 14229-1:2013(E)] Page 190. But we are not supporting 0xFF, we support according to customer REQ*/
			if ((SERV19_RECORDNUMBER_SUPPORTED == record_number_u8) || (SERV19_RECORDNUMBER_SUPPORTED_ALL == record_number_u8))
			{
				//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32);
				//actual_status_u8 = StatusDecode(status_of_DTC_u8);
				FM_DTCStatus_Un_t FM_DTCStatus_Un;
				FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32,
						FM_PROTO_UDS_E);
				actual_status_u8 = FM_DTCStatus_Un.Status_u8;

				service_buffer_pu8[4] = actual_status_u8;
				service_buffer_pu8[5] = SERV19_RECORDNUMBER_SUPPORTED;
#if (TRUE == DIAG_CONF_UDS_SUPPORTED)
				NumOfDIDs_u16 =
					(FMFF_CONF_UDS_GLBSS_ENTRIES + FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].NumOfEntries_u16);
				service_buffer_pu8[6] = NumOfDIDs_u16;

				// Read DIDs of the Global snapshot table
				response_length_s16 = 7;
				 paramSize_u8 = 0;
				for (i = 0; i < FMFF_CONF_UDS_GLBSS_ENTRIES; i++)
				{
					paramSize_u8 = Get_FreezeFrame_ParamSize(FM_Global_Snapshot_UDS_au16[i]);

					did_u16 = FM_Global_Snapshot_UDS_au16[i];
					tempFrzFrmBuff_au8[idxTempBuff_u32] =
							((did_u16 >> 8) & 0xFF);
					idxTempBuff_u32++;
					tempFrzFrmBuff_au8[idxTempBuff_u32] = ((did_u16) & 0xFF);
					idxTempBuff_u32++;

					if (DtcPresentInL2_b)
					{
						for (j = 0; j < paramSize_u8; j++)
						{
							tempFrzFrmBuff_au8[idxTempBuff_u32] =
								FML2_FaultMem_aSt[FltIndex_s8].L2FrzFrm_UDS_au8[idxFrzFrmStorage_u32 % FMFF_CONF_UDS_FRZFRM_TOTALBYTES];
							idxFrzFrmStorage_u32++;
							idxTempBuff_u32++;
						}
					}
					else if (DtcPresentInL1_b)
					{
						for (j = 0; j < paramSize_u8; j++)
						{
							tempFrzFrmBuff_au8[idxTempBuff_u32] =FaultPath_aSt[FaultPath_En].L1FrzFrm_UDS_au8[idxFrzFrmStorage_u32 % FMFF_CONF_UDS_FRZFRM_TOTALBYTES];
							idxFrzFrmStorage_u32++;
							idxTempBuff_u32++;
						}
					}
					else
					{
						// do nothing
					}
					idxConfigTab_u32++;
				}
				idxConfigTab_u32 = 0;
				// Read DIDs of the Local snapshot table for the given DTC
				for (i = 0;
					 i < FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].NumOfEntries_u16;
					 i++)
				{
					// Note, we are using fault path as index for freezeframe table, as the size of 
					// "FM_FrzFrmSignalConf_En_t" and the total # of fault paths are the same.
					did_u16 =
							FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].FM_FrzFrm_LocalEntries[i];
					tempFrzFrmBuff_au8[idxTempBuff_u32] =
							((did_u16 >> 8) & 0xFF);
					idxTempBuff_u32++;
					tempFrzFrmBuff_au8[idxTempBuff_u32] = ((did_u16) & 0xFF);
					idxTempBuff_u32++;
					paramSize_u8 = Get_FreezeFrame_ParamSize(FM_FrzFrmMasterConfig_UDS_aSt[FaultPath_En].FM_FrzFrm_LocalEntries[i]);
						if (DtcPresentInL2_b)
						{
							//						idxFrzFrmStorage_u32 = idxFrzFrmStorage_u32 + FaultPath_En;
							for (j = 0; j < paramSize_u8; j++)
							{
								tempFrzFrmBuff_au8[idxTempBuff_u32] =
									FML2_FaultMem_aSt[FltIndex_s8].L2FrzFrm_UDS_au8[idxFrzFrmStorage_u32 % FMFF_CONF_UDS_FRZFRM_TOTALBYTES];
								idxFrzFrmStorage_u32++;
								idxTempBuff_u32++; /*FltIndex_s8   FaultPath_En*/ /*FM:Jeevan*/
							}
						}
						else if (DtcPresentInL1_b)
						{
							for (j = 0; j < paramSize_u8; j++)
							{
								tempFrzFrmBuff_au8[idxTempBuff_u32]= FaultPath_aSt[FaultPath_En].L1FrzFrm_UDS_au8[idxFrzFrmStorage_u32 % FMFF_CONF_UDS_FRZFRM_TOTALBYTES];
								idxFrzFrmStorage_u32++;
								idxTempBuff_u32++;
							}
						}
					// }
					idxConfigTab_u32++;
				}
#endif

				for (i = 0; i < idxTempBuff_u32; i++)
				{
					service_buffer_pu8[7 + i] = tempFrzFrmBuff_au8[i];
				}
				response_length_s16 += idxTempBuff_u32;
			}
		}
	}
#endif
	return response_length_s16;
}

/**
 *    @brief  this function given the extended data record number for the provided DTCmaskrecord..
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetExtendedDataRecordByDTCNumber(uint8_t* service_buffer_pu8)
{             //0x06

	uint32_t DTC_u32 = 0x00u;
	int16_t response_length_s16 = 0x00u;
	uint8_t extended_record_number_u8 = 0x00u;
//  uint8_t   status_of_DTC_u8               = 0x00u ;
	uint8_t actual_status_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	DTC_u32 = service_buffer_pu8[1];
	DTC_u32 = (DTC_u32 << 8) | service_buffer_pu8[2];
	DTC_u32 = (DTC_u32 << 8) | service_buffer_pu8[3];

	extended_record_number_u8 = service_buffer_pu8[4];
	//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32);
	//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
	FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
	actual_status_u8 = FM_DTCStatus_Un.Status_u8;

	service_buffer_pu8[4] = actual_status_u8;
	response_length_s16 = FM_GetExtendedDataRecordByDTCNumber(DTC_u32,
			extended_record_number_u8, service_buffer_pu8, FM_PROTO_UDS_E);
#endif
	return response_length_s16;
}

/**
 *    @brief  this function will pass the severity_mask_u8 and status_mask_u8 as arguments.
 *               fault manager has to ((statusOfDTC & DTCStatusMask) & (severity & DTCSeverityMask)) != 0 do this operation for every DTC
 *               if its satisfies the 8 bit number has to be incremented. and at last it has to be returned.
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetNumberOfDTCBySeverityMaskRecord(uint8_t* service_buffer_pu8)
{          //0x07
	uint32_t DTC_u32 = 0x00u;
	uint16_t response_length_s16 = 0x00u;
	uint16_t number_of_faults_u16 = 0x00u;
	uint8_t i = 0x00u;
	uint8_t requested_status_mask_u8 = 0x00u;
	uint8_t actual_status_u8 = 0x00u;
	uint8_t severity_mask_u8 = 0x00u;
	uint8_t actual_severity_mask_u8 = 0x00u;
//    uint8_t status_of_DTC_u8        = 0x00u ;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
	int8_t number_of_DTCs_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	severity_mask_u8 = service_buffer_pu8[1];
	requested_status_mask_u8 = service_buffer_pu8[2];

	//number_of_DTCs_u8 = FM_GetTestActiveDTCs(DTCbuffer_au32) ;
	number_of_DTCs_u8 = FM_GetAllSupportedDTCs(DTCbuffer_au32, FM_PROTO_UDS_E); // TODO: Hareesh (19/06)

	if (ZERO_SER19 <= number_of_DTCs_u8)
	{
		for (i = 0; i < number_of_DTCs_u8; i++)
		{
			DTC_u32 = DTCbuffer_au32[i];
			//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32);
			//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
			FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
			actual_status_u8 = FM_DTCStatus_Un.Status_u8;

			actual_severity_mask_u8 = FM_GetSeverityMaskByDTC(DTC_u32,
					FM_PROTO_UDS_E);
			if (requested_status_mask_u8 & actual_status_u8)
			{
				if (actual_severity_mask_u8 & severity_mask_u8)
				{
					number_of_faults_u16++;
				}
			}
		}
		service_buffer_pu8[0] = SERV19_NUMBER_OF_DTC_BY_SEVERITY_MASK_RECORD_E;
		service_buffer_pu8[1] = FM_GetStatusAvailabiltyMask(); ///////////////////////////////////////////////
		service_buffer_pu8[2] = SAE_J2012_DA_DTCFORMAT_00;
		service_buffer_pu8[3] = number_of_faults_u16 >> 8;
		service_buffer_pu8[4] = number_of_faults_u16;
		response_length_s16 = 0x05;
	}
#endif
	return response_length_s16;
}

/**
 *    @brief  this function will pass the data buffer address severity_mask_u8 and status_mask_u8 as arguments.
 *               fault manager has to ((statusOfDTC & DTCStatusMask) & (severity & DTCSeverityMask)) != 0 do this operation for every DTC
 *               if its satisfies it has to write the values to the buffer as following
 *               DTCStatusAvailabilityMask        1 byte
 *               DTCSeverity                      1 byte
 *               DTCFunctionalUnit                1 byte
 *               DTC                              3 bytes
 *               status of DTC                    1 byte
 *
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetDTCBySeverityMaskRecord(uint8_t* service_buffer_pu8)
{                  //0x08
	uint32_t DTC_u32 = 0x00u;
	uint8_t StatusAvalMask_u8=0;
	uint16_t response_length_s16 = 0x00u;
//    uint16_t j                      = 0x00u ;
	uint8_t i = 0x00u;
	uint8_t requested_status_mask_u8 = 0x00u;
	uint8_t actual_status_u8 = 0x00u;
	uint8_t main_buffer_index_u8 = 0x01u;
	uint8_t severity_mask_u8 = 0x00u;
	uint8_t actual_severity_mask_u8 = 0x00u;
//    uint8_t status_of_DTC_u8        = 0x00u ;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
	int8_t number_of_DTCs_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	severity_mask_u8 = service_buffer_pu8[1];
	requested_status_mask_u8 = service_buffer_pu8[2];
	 StatusAvalMask_u8 = FM_GetStatusAvailabiltyMask();
	service_buffer_pu8[main_buffer_index_u8] = StatusAvalMask_u8;
	main_buffer_index_u8++;
	;
	//number_of_DTCs_u8 = FM_GetTestActiveDTCs(DTCbuffer_au32) ;
	number_of_DTCs_u8 = FM_GetAllSupportedDTCs(DTCbuffer_au32, FM_PROTO_UDS_E); // TODO: Hareesh (19/06)

	if (ZERO_SER19 <= number_of_DTCs_u8)
	{
		for (i = 0; i < number_of_DTCs_u8; i++)
		{
			DTC_u32 = DTCbuffer_au32[i];
			//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32);
			//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
			FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
			actual_status_u8 = FM_DTCStatus_Un.Status_u8;

			actual_severity_mask_u8 = FM_GetSeverityMaskByDTC(DTC_u32,
					FM_PROTO_UDS_E);
			if (requested_status_mask_u8 & actual_status_u8)
			{
				if (actual_severity_mask_u8 & severity_mask_u8)
				{
					service_buffer_pu8[main_buffer_index_u8] =
							actual_severity_mask_u8;
					main_buffer_index_u8++;
					service_buffer_pu8[main_buffer_index_u8] =
							FM_GetFunctionalUnitOfDTCByDTC(DTC_u32,
									FM_PROTO_UDS_E);
					main_buffer_index_u8++;
					service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 16);
					main_buffer_index_u8++;
					service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 8);
					main_buffer_index_u8++;
					service_buffer_pu8[main_buffer_index_u8] = DTC_u32;
					main_buffer_index_u8++;
					service_buffer_pu8[main_buffer_index_u8] = actual_status_u8;
					main_buffer_index_u8++;
				}
			}
		}
		response_length_s16 = main_buffer_index_u8;
	}
#endif
	return response_length_s16;
}

/**
 *    @brief  This function we will send data buffer address which have DTC value.
 *               fault manager has to write the values to the buffer as following
 *               DTCStatusAvailabilityMask        1 byte
 *               DTCSeverity                      1 byte
 *               DTCFunctionalUnit                1 byte
 *               DTC                              3 bytes
 *               status of DTC                    1 byte
 *
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t SeverityInformationOfDTC(uint8_t* service_buffer_pu8)
{                    //0x09
	uint32_t DTC_u32 = 0x00u;
	uint16_t response_length_s16 = 0x00u;
//    uint8_t status_of_DTC_u8        = 0x00u ;
	uint8_t actual_status_u8 = 0x00u;
	uint8_t actual_severity_mask_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	DTC_u32 = service_buffer_pu8[1];
	DTC_u32 = (DTC_u32 << 8) | service_buffer_pu8[2];
	DTC_u32 = (DTC_u32 << 8) | service_buffer_pu8[3];
	service_buffer_pu8[1] = FM_GetStatusAvailabiltyMask();
	//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32);
	//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
	FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
	actual_status_u8 = FM_DTCStatus_Un.Status_u8;

	actual_severity_mask_u8 = FM_GetSeverityMaskByDTC(DTC_u32, FM_PROTO_UDS_E);
	service_buffer_pu8[2] = actual_severity_mask_u8;
	service_buffer_pu8[3] = FM_GetFunctionalUnitOfDTCByDTC(DTC_u32,
			FM_PROTO_UDS_E);
	service_buffer_pu8[4] = (DTC_u32 >> 16);
	service_buffer_pu8[5] = (DTC_u32 >> 8);
	service_buffer_pu8[6] = DTC_u32;
	service_buffer_pu8[7] = actual_status_u8;
	response_length_s16 = 0x08;
#endif
	return response_length_s16;
}
/**
 *    @brief  This function will pass the data buffer address.
 *               fault manager has to write all DTCs that it support into the buffer.
 *
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t SupportedDTCs(uint8_t* service_buffer_pu8)
{                               //0x0A
	uint32_t DTC_u32 = 0x00;
	uint16_t response_length_s16 = 0x00u;
	uint16_t i = 0x00u;
//    uint8_t j                       = 0x00u ;
	uint8_t number_of_DTCs_u8 = 0x00u;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
//    uint8_t status_mask_u8          = 0x00u ;
//    uint8_t status_of_DTC_u8        = 0x00u ;
	uint8_t actual_status_u8 = 0x00u;
	uint8_t main_buffer_index_u8 = 0x02u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

//    status_mask_u8      = service_buffer_pu8[1];
	number_of_DTCs_u8 = FM_GetAllSupportedDTCs(DTCbuffer_au32, FM_PROTO_UDS_E);
	service_buffer_pu8[0] = SERV19_SUPPORTED_DTC_E;
	service_buffer_pu8[1] = FM_GetStatusAvailabiltyMask(); ///////////////////////////////////
	for (i = 0; i < number_of_DTCs_u8; i++)
	{
		DTC_u32 = DTCbuffer_au32[i];
		//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32) ;
		//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
		FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
		actual_status_u8 = FM_DTCStatus_Un.Status_u8;

		service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 16);
		main_buffer_index_u8++;
		service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 8);
		main_buffer_index_u8++;
		service_buffer_pu8[main_buffer_index_u8] = DTC_u32;
		main_buffer_index_u8++;
		service_buffer_pu8[main_buffer_index_u8] = actual_status_u8;
		main_buffer_index_u8++;
	}
	response_length_s16 = main_buffer_index_u8;
#endif
	return (response_length_s16);
}

/**
 *    @brief  this function will pass the data buffer address.
 *               fault manager has to write only one DTC which was failed first into the buffer.
 *               if No failed DTCs have occurred since the last ClearDiagnosticInformation don't write anything
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetFirstTestFailedDTC(uint8_t* service_buffer_pu8)
{                       //0x0B
	uint16_t response_length_s16 = 0x00u;
	uint32_t DTC_u32 = 0x00u;
//    uint8_t status_of_DTC_u8          = 0x00u ;
	uint8_t actual_status_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	DTC_u32 = FM_GetFirstTestFailedDTC();
	service_buffer_pu8[0] = SERV19_FIRST_TEST_FAILED_E;
	service_buffer_pu8[1] = FM_GetStatusAvailabiltyMask(); /////////////////////////////////////////////////////
	if (DTC_u32)
	{
		//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32) ;
		//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;

		FM_DTCStatus_Un_t FM_DTCStatus_Un;
		FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
		actual_status_u8 = FM_DTCStatus_Un.Status_u8;

		service_buffer_pu8[2] = (DTC_u32 >> 16);
		service_buffer_pu8[3] = (DTC_u32 >> 8);
		service_buffer_pu8[4] = DTC_u32;
		service_buffer_pu8[5] = actual_status_u8;
		response_length_s16 = 0x06;
	}
	else
	{
		response_length_s16 = 0x02;
	}
#endif
	return (response_length_s16);
}

/**
 *    @brief  This function will pass the data buffer address.
 *               fault manager has to write only one DTC which was confirmed first into the buffer.
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 */
uint16_t GetFirstConfirmedDTC(uint8_t* service_buffer_pu8)
{                        //0x0C
	uint16_t response_length_s16 = 0x00u;
	uint32_t DTC_u32 = 0x00u;
//    uint8_t status_of_DTC_u8          = 0x00u ;
	uint8_t actual_status_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	DTC_u32 = FM_GetFirstConfirmedDTC();
	service_buffer_pu8[0] = SERV19_FIRST_CONFIRMED_E;
	service_buffer_pu8[1] = FM_GetStatusAvailabiltyMask(); /////////////////////////////////////////////////////
	if (DTC_u32)
	{
		//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32) ;
		//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;
		FM_DTCStatus_Un_t FM_DTCStatus_Un;
		FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
		actual_status_u8 = FM_DTCStatus_Un.Status_u8;

		service_buffer_pu8[2] = (DTC_u32 >> 16);
		service_buffer_pu8[3] = (DTC_u32 >> 8);
		service_buffer_pu8[4] = DTC_u32;
		service_buffer_pu8[5] = actual_status_u8;
		response_length_s16 = 0x06;
	}
	else
	{
		response_length_s16 = 0x02;
	}
#endif
	return (response_length_s16);
}

/**
 *    @brief  This function will pass the data buffer which is empty.
 *               fault manager has to write DTC and its respective fault detection counter value.
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 *
 **/
uint16_t GetDTCFaultDetectionCounter(uint8_t* service_buffer_pu8)
{                    //0x14
	uint32_t DTC_u32 = 0x00u;
	uint16_t response_length_s16 = 0x00u;
	uint16_t fault_detection_counter_u16 = 0x00u;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
	uint8_t number_of_DTCs_u8 = 0x00u;
	uint8_t main_buffer_index_u8 = 0x01;
	uint8_t i = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	number_of_DTCs_u8 = FM_GetAllSupportedDTCs(DTCbuffer_au32, FM_PROTO_UDS_E);
	service_buffer_pu8[0] = SERV19_DTC_FAULT_DETECTION_COUNTER_E;
	for (i = 0; i < number_of_DTCs_u8; i++)
	{
		DTC_u32 = DTCbuffer_au32[i];
		service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 16);
		main_buffer_index_u8++;
		service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 8);
		main_buffer_index_u8++;
		service_buffer_pu8[main_buffer_index_u8] = DTC_u32;
		main_buffer_index_u8++;
		fault_detection_counter_u16 = FM_GetDTCFaultDetectionCounterByDTC(
				DTC_u32, FM_PROTO_UDS_E);
		service_buffer_pu8[main_buffer_index_u8] = fault_detection_counter_u16;
		main_buffer_index_u8++;
	}
	response_length_s16 = main_buffer_index_u8;
#endif
	return (response_length_s16);
}
/**
 *    @brief  This function will pass the data buffer which is empty.
 *                fault manager has to write DTC along with their statuses into the buffer which are permanent
 *
 *    @param  service_buffer_pu8 Holds the Tx data
 *    @return length of data
 *
 **/
uint16_t GetDTCWithPermanentStatus(uint8_t* service_buffer_pu8)
{                      //0x15
	uint32_t DTC_u32 = 0x00u;
	uint16_t response_length_s16 = 0x00u;
	uint8_t i = 0x00u;
//    uint8_t status_mask_u8          = 0x00u ;
	uint8_t main_buffer_index_u8 = 0x02u;
	uint8_t Length_u8 = main_buffer_index_u8;                       //New Added
	uint8_t actual_status_u8 = 0x00u;
//    uint8_t status_of_DTC_u8        = 0x00u ;
	uint32_t DTCbuffer_au32[100] =
	{ 0 };
	int8_t number_of_DTCs_u8 = 0x00u;
#if(TRUE == DIAG_CONF_FM_SUPPORTED)

	FM_DTCStatus_Un_t FM_DTCStatus_Un;

	number_of_DTCs_u8 = FM_GetDTCWithPermanentStatus(DTCbuffer_au32,
			FM_PROTO_UDS_E);

	if (ZERO_SER19 <= number_of_DTCs_u8)
	{
		service_buffer_pu8[0] = 0x15;    // SERV19_DTC_WITH_PERMANENT_STATUS_E ;
		service_buffer_pu8[1] = 0x7F;     //////////////////////////////////////
		for (i = 0; i < number_of_DTCs_u8; i++)
		{
			DTC_u32 = DTCbuffer_au32[i];         //           modified
			//status_of_DTC_u8 = FM_GetStatusOfDTCByDTCs(DTC_u32) ;
			//actual_status_u8 = StatusDecode(status_of_DTC_u8) ;     // modified
			FM_DTCStatus_Un = FM_GetStatusOfDTCByDTCs(DTC_u32, FM_PROTO_UDS_E);
			actual_status_u8 = FM_DTCStatus_Un.Status_u8;

			service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 16);
			main_buffer_index_u8++;
			service_buffer_pu8[main_buffer_index_u8] = (DTC_u32 >> 8);
			main_buffer_index_u8++;
			service_buffer_pu8[main_buffer_index_u8] = DTC_u32;
			main_buffer_index_u8++;
			service_buffer_pu8[main_buffer_index_u8] = actual_status_u8;
			main_buffer_index_u8++;
			Length_u8 = main_buffer_index_u8;      // New Added
		}
		response_length_s16 = Length_u8;                //Modified
	}
#endif
	return (response_length_s16);
}
/**
 *    @brief  This function will record the global and local snapshoot information.
 *
 *    @param  Did_u16 Holds the DID number
 *    @return void
 *
 **/

void FreezeFrame_DataLog(uint16_t Did_u16, uint8_t *Parameter_Address, uint32_t *BuffIndex_u32)
{
	uint16_t Parameter_Size = 0;
	uint16_t DID_Index = 0U;
	uint16_t ParamIndex = 0U;
	for(DID_Index = 0; DID_Index < TOTAL_DID; DID_Index++)
	{
		if((Did_u16 == DidConf_ast[DID_Index].DID_u16) && (FREEZE_FRAME_E == DidConf_ast[DID_Index].AccessType_u8))
		{
			Parameter_Size = DidConf_ast[DID_Index].FM_FrzFrm_UDS_St.ParamSize_u16;
			for(ParamIndex = 0; ParamIndex < Parameter_Size; ParamIndex++)
			{
				Parameter_Address[ParamIndex] = DidConf_ast[DID_Index].FM_FrzFrm_UDS_St.ParamBuffer_pu8[ParamIndex];
				(*BuffIndex_u32)++;
			}
			break;
		}
	}
}
/**
 *    @brief  This function will read the parameter size as per the DID.
 *
 *    @param  Did_u16 Holds the DID number
 *    @return void
 *
 **/
uint8_t Get_FreezeFrame_ParamSize(uint16_t Did_u16)
{
	uint8_t ParameterSize_u8 = 0;
	uint16_t DID_Index = 0U;
	for(DID_Index = 0; DID_Index < TOTAL_DID; DID_Index++)
	{
		if((Did_u16 == DidConf_ast[DID_Index].DID_u16) && (FREEZE_FRAME_E == DidConf_ast[DID_Index].AccessType_u8))
		{
			ParameterSize_u8 = (uint8_t)DidConf_ast[DID_Index].FM_FrzFrm_UDS_St.ParamSize_u16;
			break;
		}
	}
	return ParameterSize_u8;
}
