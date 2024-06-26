/*
 * CanNm.h
 *
 *  Created on: Jun 20, 2024
 *      Author: Abdelrahman
 */

#ifndef INC_CANNM_H_
#define INC_CANNM_H_

#include "stdint.h"
#include "CanIf.h"

// Enumeration for different modes of CanNm
typedef enum {
    NETWORK_MODE,               // Network mode
    BUS_SLEEP_MODE,             // Bus sleep mode
    PREPARE_BUS_SLEEP_MODE      // Prepare bus sleep mode
} CanNm_Mode_t;

// Enumeration for different network mode states
typedef enum {
    REPEAT_MESSAGE,             // Repeat message state
    READY_SLEEP,                // Ready sleep state
    NORMAL_OPERATION,           // Normal operation state
    NONE                        // No state
} CanNm_NetworkMode_State_t;

// Enumeration for different wake-up event states
typedef enum {
    PASSIVE_WAKEUP,             // Passive wake-up state
    ACTIVE_WAKEUP,              // Active wake-up state
    SLEEP                       // Sleep state
} CanNm_WakeUpEvent_State_t;

// Enumeration for different timer states
typedef enum {
    RUNNING,                    // Timer running state
    TIMEOUT,                    // Timer timeout state
    STOPPED                     // Timer stopped state
} CanNm_Timer_State_t;

// Enumeration for different network states
typedef enum {
    NETWORK_RELEASED,           // Network released state
    NETWORK_REQUESTED           // Network requested state
} CanNm_Network_State_t;

// Function to initialize CanNm module
void CanNm_Init(void);

// Function for CanNm main function
void CanNm_MainFunction();

// Function for CanNm receive indication
void CanNm_RxIndication();

// Function for CanNm timeout handling
void CanNm_TimeOut(uint8_t timerNum);

// Function for CanNm transmit confirmation
void CanNm_TxConfirmation(void);

// Function for CanNm Network Request
//void CanNm_NetworkRequest(void);
void CanNm_Transmit();

// Function for CanNm Network Release
void CanNm_NetworkRelease(void);


void CanNm_Sleep(void);

#endif /* INC_CANNM_H_ */
