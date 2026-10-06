/******************************************************************************
 *    FILENAME    : dcan_tp_stack_version.h
 *    DESCRIPTION : This file contains version # of ISO 15765-2 stack software.
 *                  This is an internal version # to track the stack development.
 *                  This version # does not refer to ver # of ISO15765 standard. 
******************************************************************************
 * Revision history
 *  
 * Ver Author       Date               Description
 * 1   Sloki     18/01/2017		   Initial version
 ******************************************************************************
*/    

#ifndef	_DCAN_TP_STACK_VERSION_H_
#define _DCAN_TP_STACK_VERSION_H_

// The number to be incremented whenever a new release happens internally/to customer.
/**
Format
	Version numbering schemes: A.B.C.D
	A - incremented whenever a new module/file is added.
	B - incremented whenever a new API/function is added.
	C - incremented whenever a buf fixing with file/module/function/API
	D - incremented for any minor/cosmetics/comments/...etc changes.
*/
#define DCANTP_STACK_VERSION	"V1.0.0.0"

#endif
