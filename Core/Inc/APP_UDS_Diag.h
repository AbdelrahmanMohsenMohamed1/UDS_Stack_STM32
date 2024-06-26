
#ifndef INC_APP_UDS_DIAG_H_
#define INC_APP_UDS_DIAG_H_



#include "APP_UDS_Diag_CFG.h"
#include "stdint.h"
#include "can.h"
#include "usart.h"
#include "gpio.h"
#include "string.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "CanIf.h"
#include "CanTp.h"
//#define NULL_Value 0
//For Security
//extern uint8_t* seed;
//extern uint8_t* key;

/*******************************************************************/

// select one target for MC

#define Tempreture_Add  	1
#define Pressure_Add    	2
#define Functional_Add  	3

#define	Global_Target 		Tempreture_Add


// select CAN_MODE for MC :
//Client
//Global_Target
#define Client 				0
#define CAN_MODE 			Global_Target

//Can Tp Structs & functions




//Std_ReturnType CanTp_Transmit(uint32_t TxPduId, PduInfoType* PduInfoPtr);
//Std_ReturnType CanTp_RxIndication (uint32_t RxPduId, PduInfoTRx* PduInfoPtr);


void UDS_Init(void);
void UDS_Read_Data_Client(DID did);
//void UDS_Read_Data_Server( PduInfoType* PduInfoTypePtr );

void UDS_Write_Data_Client(DID did, uint32_t data);
//void UDS_Write_Data_Server(PduInfoType* Received_data);

void UDS_Control_Session_Default(void);
void UDS_Control_Session_Extended(void);

void UDS_Send_Security_Client(Sub_Fun sub_fun);
//void UDS_Send_Security_Server(void);

void UDS_Tester_Presenter_Client(void);
//void UDS_Client_Callback(uint32_t RxPduId,PduInfoType *Ptr);

void sendHexArrayAsASCII(uint8_t* hexArray, uint16_t length);

void UART_ReceiveAndConvert(uint8_t RX_BUFFER_SIZE, PduInfoType* PduInfoType_Ptr);
uint8_t charToHex(uint8_t ascii);

void UDS_Write_Data_Server(uint8_t* received_data, uint16_t received_length);
void UDS_Send_Pos_Res(ServiceInfo* Response);
void UDS_Send_Neg_Res(uint8_t SID, uint8_t NRC);
void Sec_u32GetSeed (void);
uint32_t Sec_u32GetAlgorithm(void);
uint32_t Sec_u32GetKey (void);
uint8_t Sec_uint32SecurityAccess (PduInfoType * Ptr);
void UDS_Read_Data_Server(uint8_t* data);
void UDS_Control_Session_Server(uint8_t *Received);
void reset_timer(void);
void stop_timer(TIM_HandleTypeDef* htim);
void start_timer(void);
void UDS_Tester_Presenter_Server(void);
void UDS_MainFunction();
void server_call_back(uint32_t TxPduId, PduInfoType* ptr);
void UDS_Client_Callback(uint32_t TxPduId,PduInfoType *PduInfoPtr);

#endif /* INC_APP_UDS_DIAG_H_ */
