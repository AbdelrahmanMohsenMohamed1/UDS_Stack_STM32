/*
 * CanIf.h
 *
 *  Created on: Jun 11, 2024
 *      Author: DELL
 */

#ifndef INC_CANIF_H_
#define INC_CANIF_H_

#include "stdint.h"
#include "can.h"
#include "cmsis_os.h"


typedef struct
{
	uint8_t Data[8];
	uint32_t Length;
}PduInfoTRx;

typedef enum
{
	E_OK=0,
	E_NOK
}Std_ReturnType;

void CanIf_Transmit(uint32_t RxPduId, PduInfoTRx* PduInfoPtr);
void CanIf_Receive();
//use this setcallback in the init so that the canIf calls our  CanTp_RxIndication
void CanIf_setCallback(Std_ReturnType (*IF_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr));
void CanIf_setNmTxCallback(void (*PTF)());
void CanIf_setNmRxCallback(void (*PTF)());
#endif /* INC_CANIF_H_ */
