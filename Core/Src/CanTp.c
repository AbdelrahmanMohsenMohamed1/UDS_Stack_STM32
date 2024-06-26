/*
 * CanTp.c
 *
 *  Created on: Jun 11, 2024
 *      Author: DELL
 */

#include "CanTp.h"

#define RX_BUFFER_SIZE 100
#define CAN_FRAME_LENGTH 8
#define CAN_MAX_PAYLOAD_LENGTH 7

volatile uint32_t numberOfConsecutiveFramesToSend = 0;
volatile uint32_t currentConsecutiveFrames = 0;
volatile uint32_t numberOfConsecutiveFramesToReceive = 0;
volatile uint32_t numberOfRemainingBytesToSend = 0;
volatile uint32_t numberOfRemainingBytesToReceive = 0;
volatile uint32_t availableBuffers = 10;
void (*App_Callback)(uint32_t RxPduId, PduInfoType* PduInfoPtr) = NULL;


Frame_States expectedFrameState = Any_State;
PduInfoTRx EncodedPduInfo;
PduInfoTRx DecodedPduInfo;
PduInfoType CompletePduInfo;
PduInfoTRx* GlobalRxPduInfoPtr;
PduInfoType* GlobalTxPduInfoPtr;

volatile uint8_t ConsecSN;
volatile uint16_t currentIndex;
volatile int32_t currentOffset = -1;
volatile int32_t startOffset;

volatile int8_t CanTp_Rx;
volatile int8_t CanTp_Tx;
volatile uint32_t GlobalRxPduId;
volatile uint32_t GlobalTxPduId;


uint8_t RxData[8]; // Buffer to store received data
//uint8_t TxData[8]; // Data to be transmitted

uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile int32_t rxBufferIndex = 0;
volatile int32_t rxCurrentMaxIndex = 0;

volatile uint8_t rxData = 0;
volatile uint8_t rxComplete = 0;

void CanTp_Init(){
	CanIf_setCallback(CanTp_RxIndication);
}

void CanTp_MainFunction(){
	while(1){
		if(CanTp_Rx){
			CanTp_Rx = 0;
			//Stop the program if the PduID doesn't equal 0 :)
			if(GlobalRxPduId != 0){
				while(1);
			}
			//Get the frame type from the
			Frame_Type frame_type = CanTp_GetFrameType(GlobalRxPduInfoPtr->Data[0]);
			//	Frame_Type frame_type = First_Frame

			//Call the correct decoder depending on the frame type
			//extract the length and save it in numberOfRemainingBytesToReceive and connect the data
			switch(frame_type){
			case Single_Frame:
				CanTp_decodeSingleFrame(GlobalRxPduId, GlobalRxPduInfoPtr);
				break;
			case First_Frame:
				CanTp_decodeFirstFrame(GlobalRxPduId, GlobalRxPduInfoPtr);
				expectedFrameState = FlowControl_Frame_State;
				CanTp_Transmit(GlobalRxPduId, (PduInfoType*) GlobalRxPduInfoPtr);
				break;
			case Consecutive_Frame:
				CanTp_decodeConsecutiveFrame(GlobalRxPduId, GlobalRxPduInfoPtr);
				if(numberOfConsecutiveFramesToReceive == 0 && numberOfRemainingBytesToReceive > 0){
					expectedFrameState = FlowControl_Frame_State;
					CanTp_Transmit(GlobalRxPduId, (PduInfoType*) GlobalRxPduInfoPtr);
				}
				break;
			case FlowControl_Frame:
				//adjust the numberOfConsecutiveFramesToSend variable inside a function
				//based on the number of empty buffers available in the other node
				//as indicated in the BS (block size) byte of the flow control frame
				CanTp_decodeFlowControlFrame(GlobalRxPduId, GlobalRxPduInfoPtr);
				break;
			default:
				break;
			}

			if(frame_type == FlowControl_Frame){

			}
			else if(numberOfRemainingBytesToReceive == 0){
				if(App_Callback != NULL){
					currentIndex = 0;
					App_Callback(GlobalRxPduId, &CompletePduInfo);
				}
			}
		}
		else if(CanTp_Tx){
			//Stop the program if the PduID doesn't equal 0 :)
			if(GlobalTxPduId != 0){
				while(1);
			}

			Frame_Type frame_type = None;
			if(numberOfRemainingBytesToSend == 0 && expectedFrameState == Any_State){
				numberOfRemainingBytesToSend = GlobalTxPduInfoPtr->Length;
				CompletePduInfo.Length = numberOfRemainingBytesToSend;
				if(GlobalTxPduInfoPtr->Length < 8){
					frame_type = Single_Frame;
				}
				else{
					frame_type = First_Frame;
				}
			}


			if(numberOfRemainingBytesToSend > 0 || expectedFrameState == FlowControl_Frame_State){

				if(expectedFrameState == Consecutive_Frame_State){
					frame_type = Consecutive_Frame;
				}
				else if(expectedFrameState == FlowControl_Frame_State){
					frame_type = FlowControl_Frame;
				}
				else
				{

				}

				//Call the right encoder function according to the frame type
				//Make sure to adjust the numberOfRemainingBytesToSend variable to know if all the data has been sent
				//Also make sure to call the CanIf_Transmit method at the end of these functions.
				switch(frame_type){
				case Single_Frame:
					CanTp_encodeSingleFrame(GlobalTxPduId, GlobalTxPduInfoPtr);
					break;
				case First_Frame:
					CanTp_encodeFirstFrame(GlobalTxPduId, GlobalTxPduInfoPtr);
					frame_type = None;
					break;
				case Consecutive_Frame:
					if(numberOfConsecutiveFramesToSend > 0){
						numberOfConsecutiveFramesToSend--;
						CanTp_encodeConsecutiveFrame(GlobalTxPduId, GlobalTxPduInfoPtr);
					}
					else{
						frame_type = None;
						//wait for flow control to reach CanTp_RxIndication in order to change numberOfConsecutiveFramesToSend variable
					}
					break;
				case FlowControl_Frame:
					//Check the availableBuffers variable (in our case it's the size of the receive array)
					CanTp_encodeFlowControlFrame(GlobalTxPduId, GlobalTxPduInfoPtr);
					break;
				default:
					break;
				}
			}

			if(numberOfRemainingBytesToSend == 0){
				//Reset the expected frame
				expectedFrameState = Any_State;
				currentOffset = -1;
				CanTp_Tx = 0;
			}
		}
		vTaskDelay(100);
	}
}

Std_ReturnType CanTp_Transmit(uint32_t TxPduId, PduInfoType* PduInfoPtr){
	GlobalTxPduInfoPtr = PduInfoPtr;
	GlobalTxPduId = TxPduId;
	CanTp_Tx = 1;
	return E_OK;
}

Std_ReturnType CanTp_RxIndication (uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
	GlobalRxPduInfoPtr = PduInfoPtr;
	GlobalRxPduId = RxPduId;
	CanTp_Rx = 1;
	return E_OK;
}

Frame_Type CanTp_GetFrameType(uint8_t PCI){
	//Switch case on the PCI to determine the frame type
	PCI >>= 4;
	if(PCI < 4){
		return (Frame_Type) PCI;
	}
	else{
		return None;
	}
}

void CanTp_setCallback(void (*PTF)(uint32_t TxPduId, PduInfoType* PduInfoPtr)){
	if(PTF != NULL){
		App_Callback = PTF;
	}
}


void CanTp_encodeSingleFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
	//	// Check for NULL pointers
	//	if (PduInfoPtr == NULL ) {
	//		return E_NOK; // Return E_NOK for NULL pointer
	//	}

	// Ensure the data length does not exceed the maximum payload length
	uint32_t dataLength = PduInfoPtr->Length;
	EncodedPduInfo.Length = PduInfoPtr->Length;
	numberOfRemainingBytesToSend -= dataLength;
	//	if (dataLength > CAN_MAX_PAYLOAD_LENGTH) {
	//		return E_NOK; // Return E_NOK for data length exceeding CAN payload length
	//	}

	// The first byte of the CAN frame is reserved for PCI (Protocol Control Information)
	EncodedPduInfo.Data[0] = 0x00 | (dataLength & 0x0F); // PCI is 0x0N where N is the length of the data
	uint32_t i;
	// Copy the data from PduInfoType to the CAN frame manually, starting from the second byte
	for ( i = 0; i < dataLength; i++) {
		EncodedPduInfo.Data[i + 1] = PduInfoPtr->Data[i];
	}

	// Fill the rest of the frame with zeros if necessary
	for (i = dataLength + 1; i < CAN_FRAME_LENGTH; i++) {
		EncodedPduInfo.Data[i] = 0;
	}

	CanIf_Transmit(TxPduId, &EncodedPduInfo);
}
void CanTp_encodeFirstFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
	/*** Local Variables ****/
	uint8_t Counter=0;
	PduInfoTRx EncodedPduInfo ;
	/************/

	// assume that data is [0x1 0x2 0x3 0x4 0x5 0x6 0x7 0x8 0x9 0xA]
	EncodedPduInfo.Data[0] = (0x01 <<4 ) | ((PduInfoPtr->Length)>>8 & 0x0F); // First Frame Should be 10 A 0x1 0x2 0x3 0x4 0x5 0x6
	EncodedPduInfo.Data[1] = (PduInfoPtr->Length)& 0xFF;

	// Form First Frame
	for(Counter=2;Counter<8;Counter++)
	{
		EncodedPduInfo.Data[Counter]=PduInfoPtr->Data[Counter - 2];
	}

	/** Call CanIF_Transmit Function**/
	numberOfRemainingBytesToSend = (PduInfoPtr->Length - 6);
	CanIf_Transmit(TxPduId, &EncodedPduInfo);
}
void CanTp_encodeConsecutiveFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
	uint8_t i = 0;
	EncodedPduInfo.Length = numberOfRemainingBytesToSend > 7 ? 7 : numberOfRemainingBytesToSend;
	EncodedPduInfo.Data[0]=(0x02 << 4) | ConsecSN;

	currentOffset = startOffset + ConsecSN * 7;

	for(i=0 ; i < EncodedPduInfo.Length ; i++)
	{
		EncodedPduInfo.Data[i+1] = PduInfoPtr->Data[i + currentOffset];
	}

	ConsecSN++;
	if(ConsecSN > 0xF){
		startOffset = currentOffset;
		ConsecSN = 0;
	}
	numberOfRemainingBytesToSend -= EncodedPduInfo.Length;
	CanIf_Transmit(TxPduId, &EncodedPduInfo);
}
void CanTp_encodeFlowControlFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
	// Initialize the flow control frame parameters
	// Byte 0: Flow Status (0x30 for continue to send, 0x31 for wait, 0x32 for overflow/abort)
	// Byte 1: Block Size (0 for continuous sending without waiting for flow control)
	// Byte 2: Separation Time (ST, in milliseconds, 0-127, 241-249 are valid values)

	EncodedPduInfo.Data[0] = 0x30;  // Flow Status: Continue to send (CTS)
	EncodedPduInfo.Data[1] = availableBuffers;  // Block Size: 0 (no blocks)
	EncodedPduInfo.Data[2] = 0x00;  // Separation Time: 0 ms (no delay)
	numberOfConsecutiveFramesToReceive = availableBuffers;
	// The remaining bytes can be set to 0
	for (uint8_t i = 3; i < 8; i++) {
		EncodedPduInfo.Data[i] = 0x00;
	}

	// Set the length of the flow control frame
	//    EncodedPduInfo.Length = 3;
	ConsecSN = 1;
	// Use CanIf_Transmit to send the flow control frame
	CanIf_Transmit(TxPduId, &EncodedPduInfo);
}

void CanTp_decodeSingleFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
	// Extract the data length from the first byte of the CAN frame
	uint32_t dataLength = PduInfoPtr->Data[0] & 0x0F;
	numberOfRemainingBytesToReceive = dataLength;
	CompletePduInfo.Length = numberOfRemainingBytesToReceive;
	int i;
	// Allocate memory for the data in the PduInfoTRx struct
	for ( i = 0; i < dataLength; i++) {
		DecodedPduInfo.Data[i] = PduInfoPtr->Data[i+1];
	}

	// Check for memory allocation failure
	//	if (PduInfoPtr->Data == NULL) {
	//		DecodedPduInfo.Length = 0;
	//		return DecodedPduInfo;
	//	}

	// Set the length in the PduInfoType struct
	DecodedPduInfo.Length = dataLength;

	CanTp_ConnectData(&DecodedPduInfo);
}
void CanTp_decodeFirstFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
	numberOfRemainingBytesToReceive = ((PduInfoPtr->Data[0] & 0x0F) << 8) | PduInfoPtr->Data[1];
	CompletePduInfo.Length = numberOfRemainingBytesToReceive;
	DecodedPduInfo.Length=6;
	uint8_t Counter=0;

	for(Counter=0;Counter<8;Counter++)
	{
		DecodedPduInfo.Data[Counter]=PduInfoPtr->Data[Counter+2];
	}
	CanTp_ConnectData(&DecodedPduInfo);
}
void CanTp_decodeConsecutiveFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
	numberOfConsecutiveFramesToReceive--;
	uint8_t i = 0;
	DecodedPduInfo.Length = numberOfRemainingBytesToReceive > 7 ? 7 : numberOfRemainingBytesToReceive;
	if(ConsecSN == (PduInfoPtr->Data[0] & 0x0F)){
		for(i=0 ; i < DecodedPduInfo.Length ; i++)
		{
			DecodedPduInfo.Data[i] = PduInfoPtr->Data[i+1];
		}
		ConsecSN = ConsecSN + 1 > 0xF ? 0 : ConsecSN + 1;
		CanTp_ConnectData(&DecodedPduInfo);
	}
}
void CanTp_decodeFlowControlFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
	// Extract the Flow Status, Block Size, and Separation Time from the PDU
	uint8_t flowStatus = PduInfoPtr->Data[0];
	uint8_t blockSize = PduInfoPtr->Data[1];
	//	uint8_t separationTime = PduInfoPtr->Data[2];

	// Update the number of consecutive frames to send based on the Block Size
	//	if (blockSize == 0) {
	// Continuous sending without waiting for further flow control
	numberOfConsecutiveFramesToSend = blockSize;

	//	} else {
	//		numberOfConsecutiveFramesToSend = blockSize;
	//	}

	// Handle different flow statuses
	switch (flowStatus) {
	case 0x30:  // Continue to send (CTS)
		// Update expected frame state to send consecutive frames
		expectedFrameState = Consecutive_Frame_State;
		startOffset = currentOffset;
		ConsecSN = 1;
		break;

	case 0x31:  // Wait (WT)
		// Flow control indicates to wait
		expectedFrameState = FlowControl_Frame_State;
		break;

	case 0x32:  // Overflow/Abort (OVFLW/ABORT)
		// Handle overflow or abort condition
		//		expectedFrameState = Any_State;
		// Perform any additional actions needed for abort, such as notifying upper layers
		break;

	default:
		// Invalid flow status, handle as needed (e.g., set an error state)
		//		expectedFrameState = Any_State;
		break;
	}

}

void CanTp_ConnectData(PduInfoTRx* PduInfoPtr){
	//use CompletePduInfo struct to connect the data received from PduInfoTRx
	uint16_t tempCurrentIndex = currentIndex;
	while(currentIndex < PduInfoPtr->Length + tempCurrentIndex){
		CompletePduInfo.Data[currentIndex] = PduInfoPtr->Data[currentIndex - tempCurrentIndex];
		currentIndex++;
	}
	numberOfRemainingBytesToReceive -= PduInfoPtr->Length;
}
