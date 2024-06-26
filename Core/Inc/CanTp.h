/*
 * CanTp.h
 *
 *  Created on: Jun 11, 2024
 *      Author: DELL
 */

#ifndef INC_CANTP_H_
#define INC_CANTP_H_

#include "stdint.h"
#include "CanIf.h"
#include "cmsis_os.h"

typedef enum
{
	Single_Frame,
	First_Frame,
	Consecutive_Frame,
	FlowControl_Frame,
	None
}Frame_Type;

typedef enum
{
	Any_State,
	Consecutive_Frame_State,
	FlowControl_Frame_State
}Frame_States;

typedef struct
{
	uint8_t Data[4096];
	uint32_t Length;
}PduInfoType;

void CanTp_Init();
void CanTp_MainFunction();
Std_ReturnType CanTp_Transmit(uint32_t TxPduId, PduInfoType* PduInfoPtr);
Std_ReturnType CanTp_RxIndication (uint32_t RxPduId, PduInfoTRx* PduInfoPtr);
Frame_Type CanTp_GetFrameType(uint8_t PCI);
void CanTp_setCallback(void (*PTF)(uint32_t TxPduId, PduInfoType* PduInfoPtr));
void CanTp_encodeSingleFrame(uint32_t TxPduId,PduInfoType* PduInfoPtr);
void CanTp_encodeFirstFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr);
void CanTp_encodeConsecutiveFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr);
void CanTp_encodeFlowControlFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr);
void CanTp_decodeSingleFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr);
void CanTp_decodeFirstFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr);
void CanTp_decodeConsecutiveFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr);
void CanTp_decodeFlowControlFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr);
void CanTp_ConnectData(PduInfoTRx* PduInfoPtr);

#endif /* INC_CANTP_H_ */
