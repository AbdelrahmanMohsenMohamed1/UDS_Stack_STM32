/*
 * CanNm.c
 *
 *  Created on: Jun 20, 2024
 *      Author: Abdelrahman
 */
#include "CanNm.h"
#include "cmsis_os.h"
#include "tim.h"
#include "usart.h"
#include "string.h"

CanNm_Mode_t CanNm_Mode = BUS_SLEEP_MODE;                  // Represents the current mode of the CAN network management.
CanNm_WakeUpEvent_State_t WakeUp_Event = SLEEP;            // Indicates the current state of the wake-up event for the CAN network.
CanNm_NetworkMode_State_t NetworkModeState = NONE;         // Indicates the current state of the network mode.

CanNm_Timer_State_t NM_Timer = STOPPED;                    // State of the network management timer.
CanNm_Timer_State_t Repeat_Message_Timer = STOPPED;        // State of the repeat message timer.
CanNm_Timer_State_t Bus_Sleep_Timer = STOPPED;             // State of the bus sleep timer.
CanNm_Timer_State_t Send_MSG_Timer = STOPPED;             // Message Periodicity

CanNm_Network_State_t Network_State = NETWORK_RELEASED;    // Current state of the CAN network.
PduInfoTRx GlobalNmPdu = {.Length = 1};
volatile uint8_t NM_MSG_RecFlag = 0;                                // Flag indicating whether a network management message has been received.
volatile uint8_t NM_MSG_TranFlag = 0;                               // Flag indicating whether a network management message has been transmitted.
volatile uint8_t Rep_MSG_Bit_State = 0;                             // State of the repeat message bit.
volatile uint8_t Rep_MSG_State_Req = 0;                             // Request state for the repeat message.
volatile uint8_t SEND_REPEAT_MESSAGE = 0;

PduInfoTRx NM_PDU={
		.Data={11,0,0,0,0,0,0,0},
		.Length=8
};

void (*GlobalTxPTF)() = NULL;


void CanNm_Sleep(void)
{
	// Set CAN network mode to bus sleep mode
	CanNm_Mode = BUS_SLEEP_MODE;

	// Set wake-up event state to sleep
	WakeUp_Event = SLEEP;

	// Set network mode state to none
	NetworkModeState = NONE;

	// Initialize all timers to stopped state
	NM_Timer = STOPPED;
	Repeat_Message_Timer = STOPPED;
	Bus_Sleep_Timer = STOPPED;
	Send_MSG_Timer = STOPPED;
	//reset all timers
	//reset Nm timer

	HAL_TIM_Base_Stop_IT(&htim7); // Stop Timer7
	TIM7->CNT = 0; // Reset Timer7 counter to 0
	//reset rep msg timer
	HAL_TIM_Base_Stop_IT(&htim4); // Stop Timer4
	TIM4->CNT = 0; // Reset Timer4 counter to 0
	//reset prepare bus sleep timer
	HAL_TIM_Base_Stop_IT(&htim3); // Stop Timer3
	TIM3->CNT = 0; // Reset Timer3 counter to 0
	//reset send msg timer
	HAL_TIM_Base_Stop_IT(&htim2); // Stop Timer12
	TIM2->CNT = 0; // Reset Timer12 counter to 0

	// Set network state to network released
	Network_State = NETWORK_RELEASED;

	// Initialize message flags and repeat message states to 0
	NM_MSG_RecFlag = 0;
	NM_MSG_TranFlag = 0;
	Rep_MSG_Bit_State = 0;
	Rep_MSG_State_Req = 0;

	// Enter Sleep Mode
	//	HAL_SuspendTick();
	//	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}
void CanNm_Init(void)
{
	// Initialize the CAN network management to sleep mode and all related variables
	CanIf_setNmTxCallback(CanNm_TxConfirmation);
	CanIf_setNmRxCallback(CanNm_RxIndication);

	CanNm_Sleep();
	HAL_TIM_Base_Start_IT(&htim7); // Start Timer7 again

	HAL_TIM_Base_Start_IT(&htim4); // Start Timer4 again

	HAL_TIM_Base_Start_IT(&htim3); // Start Timer3 again

	HAL_TIM_Base_Start_IT(&htim2); // Start Timer12 again


	// Enter Sleep Mode
	//	HAL_SuspendTick();
	//	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void CanNm_MainFunction()
{
	while(1){
		switch(CanNm_Mode)
		{
		case BUS_SLEEP_MODE:
			HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nBUS_SLEEP_MODE\r\n", strlen("\r\nBUS_SLEEP_MODE\r\n"), HAL_MAX_DELAY);


			if(WakeUp_Event != SLEEP)
			{
				// 1- Start NM TimeOut Timer
				NM_Timer = RUNNING;
				// Start NM Timer here
				HAL_TIM_Base_Start_IT(&htim7);
				// 2- Start REPEAT_MESSAGE Timer
				Repeat_Message_Timer = RUNNING; //timeout must be less than NM_Timer
				HAL_TIM_Base_Start_IT(&htim4);
				// Start REPEAT_MESSAGE Timer here

				// 3- Notify Nm_NetworkMode --> change mode to Networkmode
				CanNm_Mode = NETWORK_MODE;
				NetworkModeState = REPEAT_MESSAGE;
				SEND_REPEAT_MESSAGE = 1;
			}
			else
			{
				// Do nothing
			}
			break;
		case NETWORK_MODE:

			switch(NetworkModeState)
			{
			case REPEAT_MESSAGE:
				HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nREPEAT_MESSAGE\r\n", strlen("\r\nREPEAT_MESSAGE\r\n"), HAL_MAX_DELAY);
				if(SEND_REPEAT_MESSAGE)
				{
					SEND_REPEAT_MESSAGE =0;
					//transmit
					CanIf_Transmit(1, &NM_PDU);
				}
				if(NM_Timer == TIMEOUT || NM_MSG_RecFlag == 1 || NM_MSG_TranFlag == 1 )
				{

					// Restart NM timer
					HAL_TIM_Base_Stop_IT(&htim7); // Stop Timer7
					TIM7->CNT = 0; // Reset Timer7 counter to 0
					HAL_TIM_Base_Start_IT(&htim7); // Start Timer7 again
					NM_MSG_RecFlag = 0;
					//NM_MSG_TranFlag = 0;
				}
				// Send Repeated Msg to inform nodes that this ECU is awake
				//				CanIf_Transmit(RxPduId,PduInfoPtr); // we will not implement this
				/*
				 * To Do
				 * if there is a message corruption
				 * 1-Error Detection: ECU B (this ECU) detects that the critical message from ECU A is missing or corrupted.
				 * 2-Repeat Message Request (RMR): ECU B sends an RMR to CanNm, requesting ECU A to retransmit the critical message.
				 * 3-Transition to Repeat Message State: CanNm receives the RMR from ECU B and transitions into the Repeat Message State.
				 * 4-Retransmission: In the Repeat Message State, CanNm sends a request to ECU A, asking it to retransmit the critical message.
				 * 5-Message Retransmission: ECU A receives the request from CanNm and retransmits the critical message to ECU B.
				 * 6-Reception and Processing: ECU B receives the retransmitted critical message from ECU A. Now ECU B has the complete and accurate information it needs.
				 */

				if(Repeat_Message_Timer == TIMEOUT && NM_Timer == RUNNING)
				{
					//reset NM_Timer
					HAL_TIM_Base_Stop_IT(&htim7); // Stop Timer7
					TIM7->CNT = 0; // Reset Timer7 counter to 0
					HAL_TIM_Base_Start_IT(&htim7); // Start Timer7 again
					if(Network_State == NETWORK_RELEASED)
					{
						NetworkModeState = READY_SLEEP;
						HAL_TIM_Base_Start_IT(&htim3);
					}
					else if(Network_State == NETWORK_REQUESTED)
					{

						HAL_TIM_Base_Start_IT(&htim2);
						NetworkModeState = NORMAL_OPERATION;
					}
					else
					{
						// Do nothing
					}
				}
				break;

			case READY_SLEEP:
				HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nREADY_SLEEP\r\n", strlen("\r\nREADY_SLEEP\r\n"), HAL_MAX_DELAY);

				if(NM_MSG_RecFlag == 1)
				{
					NM_MSG_RecFlag = 0;
					// Restart NM timer
					HAL_TIM_Base_Stop_IT(&htim7); // Stop Timer7
					TIM7->CNT = 0; // Reset Timer7 counter to 0
					HAL_TIM_Base_Start_IT(&htim7); // Start Timer7 again
					//read the CBV
					if(Rep_MSG_Bit_State){
						Rep_MSG_State_Req = 1;
					}
				}
				else if(NM_MSG_TranFlag == 1){
					// Restart NM timer
					HAL_TIM_Base_Stop_IT(&htim7); // Stop Timer7
					TIM7->CNT = 0; // Reset Timer7 counter to 0
					HAL_TIM_Base_Start_IT(&htim7); // Start Timer7 again
					NetworkModeState = NORMAL_OPERATION;
				}
				else
				{
					// Do nothing
				}
				if(NM_Timer == TIMEOUT)
				{
					NetworkModeState = NONE;
					CanNm_Mode = PREPARE_BUS_SLEEP_MODE;
					Bus_Sleep_Timer = RUNNING; // Start Bus Sleep Timer
					NM_Timer = STOPPED;
					//reset prepare bus sleep timer
					HAL_TIM_Base_Stop_IT(&htim3); // Stop Timer3
					TIM3->CNT = 0; // Reset Timer3 counter to 0
					HAL_TIM_Base_Start_IT(&htim3); // Start Timer3 again
				}
				else
				{
					// Do nothing
				}
				if(Rep_MSG_State_Req){
					Rep_MSG_State_Req = 0;
					NetworkModeState = REPEAT_MESSAGE;
					SEND_REPEAT_MESSAGE = 1;
					Repeat_Message_Timer = RUNNING;
					//reset rep msg timer
					HAL_TIM_Base_Stop_IT(&htim4); // Stop Timer4
					TIM4->CNT = 0; // Reset Timer4 counter to 0
					HAL_TIM_Base_Start_IT(&htim4); // Start Timer4 again
				}
				else{
					// Do nothing
				}
				/*
				 * To Do
				 * Add if (Repeat MSG Bit Or Repeat MSG State Requested)
				 * {
				 *  1- Start REPEAT_MESSAGE Timer
				 *	 Repeat_Message_Timer = RUNNING;
				 *	2- Notify Nm_NetworkMode --> change mode to Networkmode
				 *	CanNm_Mode = NETWORK_MODE;
				 *	NetworkModeState = REPEAT_MESSAGE;
				 *						SEND_REPEAT_MESSAGE = 1;
				 *
				 * }
				 */
				break;

			case NORMAL_OPERATION:
				HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nNORMAL_OPERATION\r\n", strlen("\r\nNORMAL_OPERATION\r\n"), HAL_MAX_DELAY);
				CanIf_Transmit(1, &NM_PDU);

				if(NM_MSG_RecFlag == 1 || NM_MSG_TranFlag == 1)
				{
					// Restart NM timer
					HAL_TIM_Base_Stop_IT(&htim7); // Stop Timer7
					TIM7->CNT = 0; // Reset Timer7 counter to 0
					HAL_TIM_Base_Start_IT(&htim7); // Start Timer7 again
					NM_MSG_RecFlag = 0;
					if(Rep_MSG_Bit_State){
						Rep_MSG_State_Req = 1;
					}
				}

				if(Rep_MSG_State_Req){
					Rep_MSG_State_Req = 0;
					NetworkModeState = REPEAT_MESSAGE;
					SEND_REPEAT_MESSAGE = 1;

					Repeat_Message_Timer = RUNNING;
					//reset rep msg timer
					HAL_TIM_Base_Stop_IT(&htim4); // Stop Timer4
					TIM4->CNT = 0; // Reset Timer4 counter to 0
					HAL_TIM_Base_Start_IT(&htim4); // Start Timer4 again
				}
				else if(Network_State == NETWORK_RELEASED)
				{
					NetworkModeState = READY_SLEEP;
				}
				else if(NM_MSG_TranFlag == 1){
					NM_MSG_TranFlag = 0;
					//transmit
					GlobalNmPdu.Data[0] = 1;
					//					CanIf_Transmit(1, &GlobalNmPdu);
				}

				/*
				 * To Do
				 * Add if (Repeat MSG Bit Or Repeat MSG State Requested)
				 * {
				 *  1- Start REPEAT_MESSAGE Timer
				 *	 Repeat_Message_Timer = RUNNING;
				 *	2- Notify Nm_NetworkMode --> change mode to Networkmode
				 *	CanNm_Mode = NETWORK_MODE;
				 *	NetworkModeState = REPEAT_MESSAGE;
				 *						SEND_REPEAT_MESSAGE = 1;
				 *
				 * }
				 */

				break;

			default:

				break;
			}
			break;

			case PREPARE_BUS_SLEEP_MODE:
				HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nPREPARE_BUS_SLEEP_MODE\r\n", strlen("\r\nPREPARE_BUS_SLEEP_MODE\r\n"), HAL_MAX_DELAY);

				if(Bus_Sleep_Timer == TIMEOUT)
				{
					CanNm_Mode = BUS_SLEEP_MODE;
					Bus_Sleep_Timer = STOPPED;
					CanNm_Sleep(); //Enter sleep mode
				}
				if(NM_MSG_RecFlag == 1 || NM_MSG_TranFlag == 1)
				{
					CanNm_Mode = NETWORK_MODE;
					NM_Timer = RUNNING;
					Repeat_Message_Timer = RUNNING;
					NetworkModeState = REPEAT_MESSAGE;
					SEND_REPEAT_MESSAGE = 1;
					// 1- Start NM TimeOut Timer
					//					count4 = 0;
					HAL_TIM_Base_Start_IT(&htim4); // Start Timer4 again


				}
				break;
			default:

				break;
		}
		vTaskDelay(100);
	}
}
//This should be passed to CanIf_setNmRxCallback()
void CanNm_RxIndication()
{
	NM_MSG_RecFlag = 1;
	if(CanNm_Mode == BUS_SLEEP_MODE){
		WakeUp_Event = PASSIVE_WAKEUP;
	}
}

void CanNm_NetworkRelease(void)
{
	Network_State = NETWORK_RELEASED;
	Send_MSG_Timer = STOPPED;
	NM_MSG_TranFlag = 0;
	//reset send msg timer
	HAL_TIM_Base_Stop_IT(&htim2); // Stop Timer2
	TIM2->CNT = 0; // Reset Timer12 counter to 0
	//	HAL_TIM_Base_Start_IT(&htim2); // Start Timer12 again
}

void CanNm_TimeOut(uint8_t timerNum)
{
	if(timerNum == 4){
		Repeat_Message_Timer = TIMEOUT;
	}
	else if(timerNum == 7){
		NM_Timer = TIMEOUT;
	}
	else if(timerNum == 3){
		Bus_Sleep_Timer = TIMEOUT;
	}
	else if(timerNum == 2){
		Send_MSG_Timer = TIMEOUT;
	}
}
//This should be passed to CanIf_setNmTxCallback()
void CanNm_TxConfirmation(void)
{
	//send confirmation
	if(GlobalTxPTF != NULL){
		GlobalTxPTF();
	}
}

void CanNm_setTxCallback(void (*PTF)()){
	if(PTF != NULL){
		GlobalTxPTF = PTF;
	}
}

void CanNm_Transmit(){
	Network_State = NETWORK_REQUESTED;
	NM_MSG_TranFlag = 1;
	Send_MSG_Timer = RUNNING;

	if(CanNm_Mode == BUS_SLEEP_MODE){
		WakeUp_Event = ACTIVE_WAKEUP;
	}
}
