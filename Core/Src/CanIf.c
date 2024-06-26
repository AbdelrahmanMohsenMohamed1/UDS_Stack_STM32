/*
 * CanIf.c
 *
 *  Created on: Jun 11, 2024
 *      Author: DELL
 */


#include "CanIf.h"


// Define CAN Rx message structure
CAN_RxHeaderTypeDef rxHeader;
PduInfoTRx CanIfPduInfo;
volatile int8_t CanIf_Rx;
Std_ReturnType (*CanTp_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr) = NULL;
void (*CanNm_TxCallback)(void) = NULL;
void (*CanNm_RxCallback)(void) = NULL;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1){
	if (HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, CanIfPduInfo.Data) != HAL_OK) {
		// Reception error
		Error_Handler();
	}
	CanIf_Rx = 1;
}

/**
 *  @brief CAN interface transmit function
 *  @param  PduInfoTRx*		: Pointer to message structure contain (Data, Length)
 *  @param  TxPduId	: PDU ID
 *  @return None
 */
void CanIf_Transmit(uint32_t TxPduId, PduInfoTRx* PduInfoPtr){
	CAN_TxHeaderTypeDef txHeader;
	uint32_t txMailbox;
	if(TxPduId == 0){
		txHeader.StdId = 0x100;
	}
	else if(TxPduId == 1){
		txHeader.StdId = 0x200;
	}

	txHeader.ExtId = 0x00;
	txHeader.IDE = CAN_ID_STD;
	txHeader.RTR = CAN_RTR_DATA;
	txHeader.DLC = 8;
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0);
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, PduInfoPtr->Data, &txMailbox) != HAL_OK) {
		// Transmission error
		Error_Handler();

	}
	else if(CanNm_TxCallback != NULL && TxPduId == 1){
		CanNm_TxCallback();
	}
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
}

/**
 *  @brief CAN interface receive data
 *  @param  None
 *  @return None
 */
void CanIf_Receive(){
	uint32_t PDU_ID;
	while(1){
		if(CanIf_Rx){
			CanIf_Rx = 0;
			CanIfPduInfo.Length = rxHeader.DLC;
			switch(rxHeader.StdId)
			{
			case 0x100 :PDU_ID = 0;
			break;
			case 0x200 :PDU_ID = 1;
			break;
			case 0x300 :PDU_ID = 2;
			break;
			case 0x400 :PDU_ID = 3;
			break;
			case 0x500 :PDU_ID = 4;
			break;
			case 0x600 :PDU_ID = 5;
			break;
			case 0x700 :PDU_ID = 6;
			break;
			case 0x800 :PDU_ID = 7;
			break;
			}

			if(CanTp_Callback != NULL && PDU_ID == 0)
			{
				CanTp_Callback(PDU_ID, &CanIfPduInfo);
			}
			else if(CanNm_RxCallback != NULL && PDU_ID == 1){
				CanNm_RxCallback();
			}
		}
		vTaskDelay(10);
	}
}

void CanIf_setCallback(Std_ReturnType (*IF_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr)){
	if(IF_Callback != NULL)
	{
		CanTp_Callback = IF_Callback ;
	}
}

void CanIf_setNmTxCallback(void (*PTF)()){
	if(PTF!=NULL){
		CanNm_TxCallback= PTF;
	}
}

void CanIf_setNmRxCallback(void (*PTF)()){
	if(PTF!=NULL){
		CanNm_RxCallback= PTF;
	}
}
