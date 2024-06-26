/*
 * UDS_APP_Diag.c
 *
 *  Created on: Jun 4, 2024
 *      Author: Omnia
 */

#include "APP_UDS_Diag.h"

// there are many ser. has sub,  this var indecate for which sub servise
#define sub_func_control  	1
#define Num_of_Services 	5

Std_ReturnType CanTp_Transmit(uint32_t TxPduId, PduInfoType* PduInfoPtr);


PduInfoType Read_Data_Client;
PduInfoType Write_Data_Client;
PduInfoType Control_Session_Default;
PduInfoType Send_Security_Seed;
PduInfoType Control_Session_Extended;
PduInfoType PduInfoTypePtr;
volatile uint8_t global_sec_flag = 0;
volatile uint8_t global_session = DefaultSession;

volatile uint8_t Security_Service_Availability_Flag = Not_Available;
//uint8_t seed =50 ; // for example
/*Security Variables */
volatile uint32_t Sec_u32SeedValue = 0 ;
Security_Access_State Sec_State = Un_Secure ;
volatile uint32_t Oil_Pressure_var = 0x778899AA;
volatile uint32_t Oil_Temp_var = 0x5566;
volatile uint8_t VIN_number_var[17]= { 0x11, 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
//volatile uint32_t VIN_number_var;
ServiceInfo pos_Response;

ServiceInfo Control;
PduInfoType msg;
extern TIM_HandleTypeDef htim6;
volatile uint8_t flag_sub_fun;
uint8_t UDS_Tx_Confirm = 0;
PduInfoType* UDS_Struct;
PduInfoType* PduDataPTR;

uint8_t* seed = NULL;
uint8_t* key = NULL;


uint8_t SupressedPosRes_CLient = 1;
uint8_t SupressedPosRes_Server = 1;

uint8_t Source_Address = Client_Address;
uint8_t Target_Address = Tempreture_Address;


/***************************************************************Init**********************************************************************/


void UDS_Init(void)
{

}



void UDS_Tester_Presenter_Client(void)
{
	PduInfoTypePtr.Data[PCI]= 5; 	// TESTER_PRESENT_PCI
	PduInfoTypePtr.Data[ADD_Source]= Client_Address; 	// ADD_Source

	if( Target_Address == Tempreture_Address)
	{
		PduInfoTypePtr.Data[ADD_Target]= Tempreture_Address; 	// ADD_Target
	}
	else if( Target_Address == Pressure_Address)
	{
		PduInfoTypePtr.Data[ADD_Target]= Pressure_Address; 	// ADD_Target
	}
	else if( Target_Address == Functional_Address)
	{
		PduInfoTypePtr.Data[ADD_Target]= Functional_Address; 	// ADD_Target
	}
	else
	{
		//Nothing
	}


	PduInfoTypePtr.Data[SID]= Tester_Representer_Service; 	// TESTER_PRESENT_SID
	PduInfoTypePtr.Data[Sub_F]= 0x00; 	// TESTER_PRESENT_Sub_Fun
	PduInfoTypePtr.Length = 5;		// TESTER_PRESENT_LENGTH

	//Check supressed Positive Responce
	PduInfoTypePtr.Data[Sub_F] |= 0b10000000;
	PduInfoTypePtr.Data[Sub_F] &= ((SupressedPosRes_CLient<<7)|0x7F);
	//Sending Frame to Can TP
	CanTp_Transmit(0, &PduInfoTypePtr);
}



/***************************************************************Read**********************************************************************/

//Send Read Frame
void UDS_Read_Data_Client(DID did)
{
	Read_Data_Client.Data[PCI] = 6;
	Read_Data_Client.Data[ADD_Source]= Client_Address; 	// ADD_Source


	if( Target_Address == Tempreture_Address)
	{
		Read_Data_Client.Data[ADD_Target]= Tempreture_Address; 	// ADD_Target
	}
	else if( Target_Address == Pressure_Address)
	{
		Read_Data_Client.Data[ADD_Target]= Pressure_Address; 	// ADD_Target
	}
	else if( Target_Address == Functional_Address)
	{
		Read_Data_Client.Data[ADD_Target]= Functional_Address; 	// ADD_Target
	}
	else
	{
		//Nothing
	}

	Read_Data_Client.Data[SID] = Read_Service; //SID of Read
	Read_Data_Client.Length = 6; //length of Read frame

	//check DID which Data
	if(did == Oil_Temp)
	{
		Read_Data_Client.Data[DID_1] = Oil_Temp_First_byte;
		Read_Data_Client.Data[DID_2] = Oil_Temp_Second_byte;
	}
	else if (did == Oil_Pressure)
	{
		Read_Data_Client.Data[DID_1] = Oil_Pressure_First_byte;
		Read_Data_Client.Data[DID_2] = Oil_Pressure_Second_byte;
	}
	else if (did == VIN_number)
	{
		Read_Data_Client.Data[DID_1] = VIN_number_First_byte;
		Read_Data_Client.Data[DID_2] = VIN_number_Second_byte;
	}
	else
	{
		//Another DID
	}

	//Check supressed Positive Responce
	Read_Data_Client.Data[DID_1] |= 0b10000000;
	Read_Data_Client.Data[DID_1] &= ((SupressedPosRes_CLient<<7)|0x7F);
	//Send Frame to Can TP
	CanTp_Transmit(0, &Read_Data_Client);

	//For Debugging
	//HAL_UART_Transmit(&huart2, "\r\nRead Frame Client:", 50, HAL_MAX_DELAY);
	//sendHexArrayAsASCII(Read_Data_Client.Data, Read_Data_Client.Length);
	//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
}


//Recieve Read Frame
//before in call back: SID is checked
//void UDS_Read_Data_Server( PduInfoType* PduInfoTypePtr )
//{
//	ServiceInfo Read_Data_Server ;
//	Read_Data_Server.SID = 0x22;
//	Read_Data_Server.SUB_FUNC = -1;
//	//if DID --> Oil_Temp
//	if((PduInfoTypeptr->Data[SID] == 0xF1) && (PduInfoTypeptr->Data[Sub_F] == 0x3D) )
//	{
//		Read_Data_Server.DID[0] = 0xF1;
//		Read_Data_Server.DID[1] = 0x3D;
//		Read_Data_Server.DID_Length = 2;
//		//For Debugging
//		//HAL_UART_Transmit(&huart2, "\r\nRead Frame Client DID:", 50, HAL_MAX_DELAY);
//		//sendHexArrayAsASCII(Read_Data_Server.DID, Read_Data_Server.DID_Length );
//		//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
//
//		Read_Data_Server.Data[SID] = 0x55; //value of Oil_Temp
//		Read_Data_Server.Data[Sub_F] = 0x66; //value of Oil_Temp
//		Read_Data_Server.Data_Length = 2;
//		//Send +ve responce
//		//UDS_Send_Pos_Res(Read_Data_Server);
//
//		//For Debugging
//		//HAL_UART_Transmit(&huart2, "\r\nRead Frame Server Data:", 50, HAL_MAX_DELAY);
//		//sendHexArrayAsASCII(Read_Data_Server.Data, Read_Data_Server.Data_Length);
//		//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
//
//	}//if DID --> Oil_Pressure
//	else if((PduInfoTypeptr->Data[SID] == 0xF5) && (PduInfoTypeptr->Data[Sub_F] == 0x3D) )
//	{
//		Read_Data_Server.DID[0] = 0xF5;
//		Read_Data_Server.DID[1] = 0x3D;
//		Read_Data_Server.DID_Length = 2;
//		//For Debugging
//		//HAL_UART_Transmit(&huart2, "\r\nRead Frame Client DID:", 50, HAL_MAX_DELAY);
//		//sendHexArrayAsASCII(Read_Data_Server.DID, Read_Data_Server.DID_Length );
//		//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
//		Read_Data_Server.Data[SID] = 0x77; //value of Oil_Pressure
//		Read_Data_Server.Data[Sub_F] = 0x88; //value of Oil_Pressure
//		Read_Data_Server.Data[2] = 0x99; //value of Oil_Pressure
//		Read_Data_Server.Data[3] = 0xAA; //value of Oil_Pressure
//		Read_Data_Server.Data_Length = 4;
//		//Send +ve responce
//		//UDS_Send_Pos_Res(Read_Data_Server);
//
//		//For Debugging
//		//HAL_UART_Transmit(&huart2, "\r\nRead Frame Client DID:", 50, HAL_MAX_DELAY);
//		//sendHexArrayAsASCII(Read_Data_Server.DID, Read_Data_Server.DID_Length );
//		//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
//	}
//	else
//	{
//		//otherwize: send -ve responce
//		//UDS_Send_Neg_Res(Read_Data_Server.SID, NRC);
//	}
//
//}
//


/***************************************************************Write**********************************************************************/

void UDS_Write_Data_Client(DID did, uint32_t data)
{

	//PduInfoType Write_Data_Client;
	Write_Data_Client.Data[SID] = Write_Service; //SID of WDID
	Write_Data_Client.Data[ADD_Source] = Client_Address;

	if( Target_Address == Tempreture_Address)
	{
		Write_Data_Client.Data[ADD_Target]= Tempreture_Address; 	// ADD_Target
	}
	else if( Target_Address == Pressure_Address)
	{
		Write_Data_Client.Data[ADD_Target]= Pressure_Address; 	// ADD_Target
	}
	else if( Target_Address == Functional_Address)
	{
		Write_Data_Client.Data[ADD_Target]= Functional_Address; 	// ADD_Target
	}
	else
	{
		//Nothing
	}

	//check DID which Data
	if(did == Oil_Temp)
	{
		Write_Data_Client.Data[DID_1] = Oil_Temp_First_byte;
		Write_Data_Client.Data[DID_2] = Oil_Temp_Second_byte;

		// Assuming data is 2 bytes
		UART_ReceiveAndConvert(4, &Write_Data_Client);
		/*
		Write_Data_Client.Data[Data_DID] = data >> 8; // Most significant byte of data
		Write_Data_Client.Data[Data_DID+1] = data & 0xFF;	   // Least significant byte of data
		Write_Data_Client.Data[Data_DID+2] = 0x00;
		Write_Data_Client.Data[Data_DID+3] = 0x00;		  // Least significant byte of data
		 */
		Write_Data_Client.Length = 8; // SID + DID + Data
		Write_Data_Client.Data[PCI] = 8;


	}
	else if (did == Oil_Pressure)
	{
		Write_Data_Client.Data[DID_1] = Oil_Pressure_First_byte;
		Write_Data_Client.Data[DID_2] = Oil_Pressure_Second_byte;

		// Assuming data is 4 bytes
		UART_ReceiveAndConvert(8, &Write_Data_Client);
		/*
		Write_Data_Client.Data[Data_DID] = (data >> 24) & 0xFF; // Most significant byte of data
		Write_Data_Client.Data[Data_DID+1] = (data >> 16) & 0xFF;
		Write_Data_Client.Data[Data_DID+2] = (data >> 8) & 0xFF;
		Write_Data_Client.Data[Data_DID+3] = data & 0xFF;		  // Least significant byte of data
		 */
		Write_Data_Client.Length = 10; // SID + DID + Data
		Write_Data_Client.Data[PCI] = 10;


	}
	else if (did == VIN_number)
	{
		Write_Data_Client.Data[DID_1] = VIN_number_First_byte;
		Write_Data_Client.Data[DID_2] = VIN_number_Second_byte;

		// Assuming data is 4 bytes for now until we test then handle it as 17 bytes ISA
		UART_ReceiveAndConvert(34, &Write_Data_Client);
		//Write_Data_Client.Data[Data_DID] = (data >> 24) & 0xFF; // Most significant byte of data
		//Write_Data_Client.Data[Data_DID+1] = (data >> 16) & 0xFF;
		//Write_Data_Client.Data[Data_DID+2] = (data >> 8) & 0xFF;
		//Write_Data_Client.Data[Data_DID+3] = data & 0xFF;		  // Least significant byte of data
		/*
		Write_Data_Client.Length = 10; // SID + DID + Data
		Write_Data_Client.Data[PCI] = 10;
		 */
		Write_Data_Client.Length = 23; // SID + DID + Data
		Write_Data_Client.Data[PCI] = 23;

		/* to handle 17 bytes of data */
		/*	for (uint8_t i=16; i>=0; i--)
			{
				Write_Data_Client.Data[(Data_DID + (16-i))] = (data >> (i*8)) & 0xFF;
			}

		 */

	}
	else
	{
		//Another DID
	}
	//Check supressed Positive Responce
	Write_Data_Client.Data[DID_1] |= 0b10000000;
	Write_Data_Client.Data[DID_1] &= ((SupressedPosRes_CLient<<7)|0x7F);
	CanTp_Transmit(0, &Write_Data_Client);

	//For Debugging
	//HAL_UART_Transmit(&huart2, "\r\nWrite Frame Client:", 50, HAL_MAX_DELAY);
	sendHexArrayAsASCII(Write_Data_Client.Data,  Write_Data_Client.Length);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 50, HAL_MAX_DELAY);
}





//void UDS_Write_Data_Server(PduInfoType* Received_data)
//{
//
//	uint16_t Oil_Temp;
//	uint32_t Oil_Pressure;
//	PduInfoType Write_Data_Server;
//
//
//	// did for Oil_Temp or we can check by the length of the received_data
//	if ((Received_data->Data[1] == 0xF1) && (Received_data->Data[2] == 0x3D))
//	{
//		// udate the value of oil temperature
//		Oil_Temp = ( (Received_data->Data[3] << 8) | (Received_data->Data[4]) );
//
//	}
//	// did for Oil_Pressure
//	else if (Received_data->Data[1] == 0xF5 && Received_data->Data[2] == 0x3D)
//	{
//		// udate the value of oil pressure
//		Oil_Pressure = ( (Received_data->Data[3] << 24) | (Received_data->Data[4] << 16) | (Received_data->Data[5] << 8) | (Received_data->Data[6]) );
//
//	}
//	else
//	{
//
//		// error in the did
//		// call the negative response function
//	}
//
//	// prepare positive response
//	Write_Data_Server.Data[SID] = 0x6E; //SID of WDID
//	Write_Data_Server.Data[Sub_F] = Received_data->Data[1];
//	Write_Data_Server.Data[2] = Received_data->Data[2];
//
//	// Calculating the data length
//	Write_Data_Server.Length = 3;
//
//
//	//Sending Frame to Can TP
//	//CanTp_Transmit(0, &Write_Data_Server);
//}
//

/***************************************************************Control Session**********************************************************************/
void UDS_Control_Session_Default(void)
{
	// init struct var  to use for send TP

	// fill the struct data
	Control_Session_Default.Data[PCI] = 5;
	Control_Session_Default.Data[SID] = Control_Service;

	Control_Session_Default.Data[ADD_Source] = Client_Address;

	if( Target_Address == Tempreture_Address)
	{
		Control_Session_Default.Data[ADD_Target]= Tempreture_Address; 	// ADD_Target
	}
	else if( Target_Address == Pressure_Address)
	{
		Control_Session_Default.Data[ADD_Target]= Pressure_Address; 	// ADD_Target
	}
	else if( Target_Address == Functional_Address)
	{
		Control_Session_Default.Data[ADD_Target]= Functional_Address; 	// ADD_Target
	}
	else
	{
		//Nothing
	}
	Control_Session_Default.Data[Sub_F] = DefaultSession;
	//Check supressed Positive Responce
	Control_Session_Default.Data[Sub_F] |= 0b10000000;
	Control_Session_Default.Data[Sub_F] &= ((SupressedPosRes_CLient<<7)|0x7F);

	Control_Session_Default.Length = 5;

	// send to can tp
	CanTp_Transmit(0, &Control_Session_Default);

	//For Debugging
	//HAL_UART_Transmit(&huart2, "\r\nControl_Session_Default:", 100, HAL_MAX_DELAY);
	//sendHexArrayAsASCII(Control_Session_Default.Data,  Control_Session_Default.Length);
	//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
}


void UDS_Control_Session_Extended(void)
{
	// init struct var  to use for send TP


	// fill the struct data
	Control_Session_Extended.Data[PCI] = 5;
	Control_Session_Extended.Data[SID] = Control_Service;

	Control_Session_Extended.Data[ADD_Source] = Client_Address;

	if( Target_Address == Tempreture_Address)
	{
		Control_Session_Extended.Data[ADD_Target]= Tempreture_Address; 	// ADD_Target
	}
	else if( Target_Address == Pressure_Address)
	{
		Control_Session_Extended.Data[ADD_Target]= Pressure_Address; 	// ADD_Target
	}
	else if( Target_Address == Functional_Address)
	{
		Control_Session_Extended.Data[ADD_Target]= Functional_Address; 	// ADD_Target
	}
	else
	{
		//Nothing
	}
	Control_Session_Extended.Data[Sub_F] = ExtendedSession;
	//Check supressed Positive Responce
	Control_Session_Extended.Data[Sub_F] |= 0b10000000;
	Control_Session_Extended.Data[Sub_F] &= ((SupressedPosRes_CLient<<7)|0x7F);

	Control_Session_Extended.Length = 5;

	// send to can tp
	CanTp_Transmit(0, &Control_Session_Extended);

	//For Debugging
	//HAL_UART_Transmit(&huart2, "\r\Control_Session_Extended:", 100, HAL_MAX_DELAY);
	//sendHexArrayAsASCII(Control_Session_Extended.Data,  Control_Session_Extended.Length);
	//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
}



/***************************************************************Security**********************************************************************/


/***************************************************************Security**********************************************************************/

void UDS_Send_Security_Client(Sub_Fun sub_fun)
{

	Send_Security_Seed.Data[SID] = Security_Service;//Security SID

	//Prepare the Key
	//Security_Key security_key;
	Send_Security_Seed.Data[ADD_Source] = Client_Address;

	if( Target_Address == Tempreture_Address)
	{
		Send_Security_Seed.Data[ADD_Target]= Tempreture_Address; 	// ADD_Target
	}
	else if( Target_Address == Pressure_Address)
	{
		Send_Security_Seed.Data[ADD_Target]= Pressure_Address; 	// ADD_Target
	}
	else if( Target_Address == Functional_Address)
	{
		Send_Security_Seed.Data[ADD_Target]= Functional_Address; 	// ADD_Target
	}
	else
	{
		//Nothing
	}
	if(sub_fun == Seed)
	{
		Send_Security_Seed.Data[Sub_F] = Seed;//Sub_Fun Seed
		Send_Security_Seed.Length = 5;
		Send_Security_Seed.Data[PCI] = 5;
	}
	else if(sub_fun == Key)
	{
		Send_Security_Seed.Data[Sub_F] = Key;//Sub_Fun Key
		Send_Security_Seed.Length = 5+Seed_Key_Lenght;
		Send_Security_Seed.Data[PCI] = 5+Seed_Key_Lenght;
		UART_ReceiveAndConvert(8,&Send_Security_Seed);

		//security_key.Seed = seed;
		//Prepare Key From Seed
		//uint8_t security_key_counter = Seed_Key_Lenght-1;
		//uint8_t Send_Security_Seed_counter = 2;
		/*
		while(security_key_counter >= 0 )
		{
			//sendHexArrayAsASCII(security_key.Key,4);
			//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
			security_key.Key[security_key_counter] = security_key.Seed[security_key_counter] + Key_Code;//+0x05
			security_key_counter--;

			//Put New Key Calculated into Frame
			Send_Security_Seed.Data[Send_Security_Seed_counter] = security_key.Key[security_key_counter];
			Send_Security_Seed_counter++;
		}
		 */
		//key = security_key.Key;

	}
	else
	{
		//Nothing
	}

	//Check supressed Positive Responce
	Send_Security_Seed.Data[Sub_F] |= 0b10000000;
	Send_Security_Seed.Data[Sub_F] &= ((SupressedPosRes_CLient<<7)|0x7F);

	//Send Frame to Can TP
	CanTp_Transmit(0, &Send_Security_Seed);

	//For Debugging
	//HAL_UART_Transmit(&huart2, "\r\nUDS_Send_Security_Client", 50, HAL_MAX_DELAY);
	//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
	//sendHexArrayAsASCII(Send_Security_Seed.Data, Send_Security_Seed.Length);
	//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);


}




/***************************************************************CallBack Function**********************************************************************/
//
//void UDS_Client_Callback(uint32_t RxPduId,PduInfoType *Ptr)
//{
//	/*************************************************************************************/
//
//					//recieve response frame from tp
//					//check First Byte
//					// if(7F == arr[0])  // -ive  // print NRC
//					// else +ive
//					// read SID--> if read --> print data
//					// if write --> print success
//					// if control --> print current session
//					// if security --> security state
//
//	/*************************************************************************************/
//	//PduInfoType Glgl;
//	//CanTp_RxIndication ( 0, &Glgl);
//	//PduInfoType *Ptr1 = &Glgl;
//	//ptr->Data[0] = ptr->Data[0] - 0x40;
//
//	if ( Ptr ->Data[0] == 0x7f)
//	{
//		HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Sorry Negative frame .\r\n",strlen("\r\n Sorry Negative frame .\r\n"), HAL_MAX_DELAY);
//	}
//
//	else {
//		/*  READ IDS Message */
//		if (Ptr->Data[0] == Read_Service)
//
//		{
//			if  ( ptr->Data[SID] == Oil_Temp_First_byte &&  ptr->Data[Sub_F] == Oil_Temp_Second_byte )
//			{
//
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Oil Temp .\r\n", strlen("\r\n Oil Temp .\r\n"), HAL_MAX_DELAY);
//				sendHexArrayAsASCII((const uint8_t*)&Ptr->Data[3],2);
//
//			}
//
//			else if  ( ptr->Data[SID] == Oil_Pressure_First_byte &&  ptr->Data[Sub_F] == Oil_Pressure_Second_byte  )
//			{
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Oil Pressure .\r\n", strlen("\r\n Oil Pressure .\r\n"), HAL_MAX_DELAY);
//				sendHexArrayAsASCII((const uint8_t*)&Ptr->Data[3],4);
//
//			}
//		}
//
//		/*  Write IDS Message */
//		else if (Ptr->Data[0] == Write_Service)
//
//		{
//
//			if ( ptr->Data[SID] == Oil_Temp_First_byte &&  ptr->Data[Sub_F] == Oil_Temp_Second_byte )
//			{
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Oil Temperature Written successfully .\r\n", strlen("\r\n Oil Temperature Written successfully .\r\n"), HAL_MAX_DELAY);
//			}
//
//			else if  ( ptr->Data[SID] == Oil_Pressure_First_byte &&  ptr->Data[Sub_F] == Oil_Pressure_Second_byte  )
//			{
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Oil Pressure Written successfully .\r\n", strlen("\r\n Oil Pressure Written successfully .\r\n"), HAL_MAX_DELAY);
//			}
//		}
//		/*  Control Service  Session */
//
//		else if (Ptr->Data[0] == Control_Service)
//
//		{
//
//			switch ( ptr->Data[SID] )
//			{
//
//			case Default_Session :
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n It's Default Session ! .\r\n", strlen("\r\n It's Default Session ! .\r\n"), HAL_MAX_DELAY);
//				//UDS_Read_Data_Client(Frame_Info.DID);
//				break ;
//
//			case Extended_Session :
//				HAL_UART_Transmit(&huart2,(const uint8_t*) "\r\n It's Extended Session ! .\r\n", strlen("\r\n It's Extended Session ! .\r\n"), HAL_MAX_DELAY);
//				//UDS_Read_Data_Client(Frame_Info.DID);
//				break ;
//			}
//		}
//		/*  Security Service  */
//
//		else if (Ptr->Data[0] == Security_Service)
//
//		{
//
//			switch ( ptr->Data[SID] )
//			{
//
//			case Key :
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Key is Compatible ! .\r\n", strlen ( "\r\n Key is Compatible ! .\r\n"), HAL_MAX_DELAY);
//				break ;
//
//			case Seed :
//				HAL_UART_Transmit(&huart2, (const uint8_t*) "\r\n Seed is \r\n", strlen ("\r\n Seed is \r\n"), HAL_MAX_DELAY);
//				sendHexArrayAsASCII((const uint8_t*)&ptr->Data[Sub_F],4);
//
//				break ;
//			}
//		}
//		/*  Tester_Representer_Service  */
//
//		else if (Ptr->Data[0] == Tester_Representer_Service)
//
//		{
//
//			HAL_UART_Transmit(&huart2,(const uint8_t*) "\r\n ECU Reseted ! .\r\n", strlen("\r\n ECU Reseted ! .\r\n"), HAL_MAX_DELAY);
//
//		}
//
//		else { /* no thing  */	}
//
//	}
//
//}
//

/***************************************************************Shared Functions**********************************************************************/

//Function to convert from Hex to ASCII and Displays Frame on UART
void sendHexArrayAsASCII(uint8_t* hexArray, uint16_t length)
{

	// Buffer to hold the ASCII representation (2 chars per byte + 1 for null terminator)
	char asciiBuffer[length * 2 + 1];
	char* pBuffer = asciiBuffer;

	// Convert each byte to its ASCII representation
	for (uint16_t i = 0; i < length; i++) {
		sprintf(pBuffer, "%02X", hexArray[i]);
		pBuffer += 2;
	}

	// Null-terminate the string
	*pBuffer = '\0';

	// Transmit the ASCII string over UART
	HAL_UART_Transmit(&huart2, (uint8_t*)asciiBuffer, strlen(asciiBuffer), HAL_MAX_DELAY);
}



void UART_ReceiveAndConvert(uint8_t RX_BUFFER_SIZE, PduInfoType* PduInfoType_Ptr)
{
	uint8_t rxBuffer[RX_BUFFER_SIZE];
	uint8_t hexValue;
	uint8_t hexOutput[RX_BUFFER_SIZE / 2];
	uint16_t length = 0;

	// Receive ASCII characters from UART
	if (HAL_UART_Receive(&huart2, rxBuffer, RX_BUFFER_SIZE, HAL_MAX_DELAY) == HAL_OK) {
		// Calculate the length of the received string
		length = RX_BUFFER_SIZE/*strlen((char*)rxBuffer)*/;

		// Ensure the length is even (each hex byte is represented by 2 ASCII characters)
		if (length % 2 != 0) {
			// Send an error message
			char errorMsg[] = "Error: Odd number of characters received.\n";
			HAL_UART_Transmit(&huart2, (uint8_t*)errorMsg, strlen(errorMsg), HAL_MAX_DELAY);
			return;
		}

		// Process each pair of ASCII characters
		for (uint16_t i = 0; i < length; i += 2) {
			hexValue = (charToHex(rxBuffer[i]) << 4) | charToHex(rxBuffer[i + 1]);
			//update hexvalue into frame
			if(PduInfoType_Ptr->Data[Sub_F] == Key)
			{
				PduInfoType_Ptr->Data[(i / 2)+5] = hexValue;
			}
			else if ( (PduInfoType_Ptr->Data[DID_1] == Oil_Temp_First_byte) || (PduInfoType_Ptr->Data[DID_1] == Oil_Pressure_First_byte) ||  (PduInfoType_Ptr->Data[DID_1] == VIN_number_First_byte) )
			{
				PduInfoType_Ptr->Data[(i / 2)+6] = hexValue;
			}

			hexOutput[i / 2] = hexValue;
			// Optionally, send the hexadecimal values back via UART
			//HAL_UART_Transmit(&huart2, hexOutput, 1/*length / 2*/, HAL_MAX_DELAY);
		}
		sendHexArrayAsASCII(hexOutput,  length / 2);

	}
}

uint8_t charToHex(uint8_t ascii)
{
	if (ascii >= '0' && ascii <= '9') {
		return ascii - '0';
	} else if (ascii >= 'A' && ascii <= 'F') {
		return ascii - 'A' + 10;
	} else if (ascii >= 'a' && ascii <= 'f') {
		return ascii - 'a' + 10;
	} else {
		return 0; // Invalid character
	}
}


/***************************************************************Server**********************************************************************/

void UDS_Write_Data_Server(uint8_t* received_data, uint16_t received_length)
{
	/*???????????????????????*/
	uint8_t received_data_l = sizeof(received_data);
	if (received_length != received_data_l - 1)
	{
		//UDS_Send_Neg_Res(Write_Service, incorrectMessageLengthOrInvalidFormat);
	}

	// Extract the DID from the received data
	//	uint16_t DID = (received_data[2] << 8) | received_data[3];

	// Extract the data from the received data
	//	uint32_t data;
	//	data = (received_data[4] << 24) | (received_data[5] << 16) | (received_data[6] << 8) | received_data[7];


	// update the data in the required DID
	//	uint8_t returnType = check_DID( DID, data);
	// according to the returnType if it is equal to 1 --> positive response - but if it is equal to 0 --> negative response

	/*********************************
		this is the logic of +ive resp ( we need edit )
	 *************************************/
	// Define the positive response array
	//	uint8_t arr[7]; // Adjust size as needed
	//	uint8_t  SID_response = 0x6E; // Positive response SID (0x2E + 0x40)

	// Fill the local array with SID_response, DID, and data
	//	arr[0] = SID_response;
	//	arr[1] = (DID >> 8) & 0xFF; // Most significant byte of DID
	//	arr[2] = DID & 0xFF;        // Least significant byte of DID
	//
	//	// Assuming Data is 4 bytes
	//	arr[3] = (data >> 24) & 0xFF; // Most significant byte of data
	//	arr[4] = (data >> 16) & 0xFF;
	//	arr[5] = (data >> 8) & 0xFF;
	//	arr[6] = data & 0xFF;		  // Least significant byte of data


	pos_Response.SID = Write_Service ;
	pos_Response.DID[0]=received_data[DID_1];
	pos_Response.DID[1]=received_data[DID_2];
	pos_Response.DID_Length=2;
	pos_Response.Data_Length = 0;
	pos_Response.SUB_FUNC = -1;

	pos_Response.DID[0] |= 0b10000000;//Original DID

	if(received_data[DID_1] == Oil_Temp_First_byte && received_data[DID_2] == Oil_Temp_Second_byte){
		Oil_Temp_var = received_data[Data_DID] << 8 | received_data[Data_DID+1];
	}
	else if(received_data[DID_1] == Oil_Pressure_First_byte && received_data[DID_2] == Oil_Pressure_Second_byte){
		Oil_Pressure_var = received_data[Data_DID] << 24 | received_data[Data_DID+1] << 16 | received_data[Data_DID+2] << 8 | received_data[Data_DID+3];
	}
	else if(received_data[DID_1] == VIN_number_First_byte && received_data[DID_2] == VIN_number_Second_byte){
		//VIN_number_var = received_data[Data_DID] << 24 | received_data[Data_DID+1] << 16 | received_data[Data_DID+2] << 8 | received_data[Data_DID+3];

		for (uint8_t i=0; i<=16; i++)
		{
			VIN_number_var[i] = received_data[Data_DID + i];
			//VIN_number_var |= received_data[(Data_DID + (16-i))] << (i*8);
		}
/*
		HAL_UART_Transmit(&huart2, (uint8_t*)"VIN_number_var: ", strlen("VIN_number_var: "), HAL_MAX_DELAY);
		sendHexArrayAsASCII(VIN_number_var,  17);
		HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", strlen("\r\n"), HAL_MAX_DELAY);
		sendHexArrayAsASCII(&received_data[Data_DID],  17);
		HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", strlen("\r\n"), HAL_MAX_DELAY);
	*/
	}
	else
	{
		//Another DID
	}
	if(SupressedPosRes_Server == 1)
	{
		UDS_Send_Pos_Res(&pos_Response);
		return;
	}

	return;

	/*
	PduInfoType hamada_write;

	// Prepare the TP structure
	//hamada_write.Data = arr;

	for(int i = 0 ; i< 8; i++)
	{
		hamada_write.Data[i] = arr[i];
	}
	hamada_write.Length = sizeof(arr);

	// Transmit the data through CAN_TP using this function
	CanTP_Transmit(0, &hamada_write);*/
}



/*****************************************************************************/

void UDS_Send_Pos_Res(ServiceInfo* Response)
{

	uint8_t PCI = 4 + Response->DID_Length + Response->Data_Length;
	msg.Data[SID] = Response->SID + 0x40;
	msg.Data[ADD_Source] = Response->ADD_Source;
	msg.Data[ADD_Target] = Response->ADD_Target;
	uint8_t currentIndex = Sub_F;
	if(Response->SUB_FUNC != -1)
	{
		PCI++;
		msg.Data[currentIndex++]= Response->SUB_FUNC;
	}
	else
	{
		for(currentIndex = DID_1; currentIndex < Response->DID_Length + 4; currentIndex++)
		{
			msg.Data[currentIndex] = Response->DID[currentIndex - 4];
		}
	}

	uint8_t temp = currentIndex;
	while(currentIndex < Response->Data_Length + temp)
	{
		msg.Data[currentIndex] = Response->Data[currentIndex - temp];
		currentIndex++;
	}
	msg.Data[0] = PCI;
	msg.Length = PCI;

	CanTp_Transmit(0, &msg);
}

void UDS_Send_Neg_Res(uint8_t SID, uint8_t NRC)
{
	msg.Data[PCI] = 6;
	msg.Data[Neg_Res_INDEX] = 0x7F;
	msg.Data[SID_NR_INDEX] = SID;
	msg.Data[NRC_INDEX] = NRC;
	msg.Length = 6;

	CanTp_Transmit(0, &msg);
}





void UDS_Tester_Presenter_Server(void)//Khaled Waleed
{
	//globally in server:  there is a timer >= tout { reset timer + return to default (global ssesion flag in server) ask nour}
	//	reset_timer();
	pos_Response.SID = Tester_Representer_Service ;
	pos_Response.DID_Length=0;
	pos_Response.Data_Length = 0;
	pos_Response.SUB_FUNC = 0x00;

	if(SupressedPosRes_Server == 1 )
	{
		UDS_Send_Pos_Res(&pos_Response);
	}

}

void reset_timer(void)
{
	HAL_TIM_Base_Stop_IT(&htim6); // Stop Timer6
	TIM6->CNT = 0; // Reset Timer6 counter to 0
	//    HAL_TIM_Base_Start_IT(&htim6); // Start Timer6 again
}

void stop_timer(TIM_HandleTypeDef* htim){
	HAL_TIM_Base_Stop_IT(htim); // Stop Timer
}

void start_timer(void)
{
	//	HAL_TIM_Base_Start(&htim6);
	HAL_TIM_Base_Start_IT(&htim6);
}

void UDS_Control_Session_Server(uint8_t *Received)
{



	if(Received[Sub_F] == DefaultSession || Received[Sub_F] == ExtendedSession)
	{
		global_session = Received[Sub_F];
		Control.SID = Received[SID];
		Control.SUB_FUNC = Received[Sub_F];
		Control.DID_Length = 0;
		Control.Data_Length = 0;


		if(Received[Sub_F] == ExtendedSession){
			reset_timer();
			start_timer();
		}
		if(SupressedPosRes_Server == 1)
		{
			UDS_Send_Pos_Res(&Control);
		}

	}
	else
	{
		UDS_Send_Neg_Res(Received[SID], subFunctionNotSupported);
	}
}

void Sec_u32GetSeed (void)
{
	Sec_u32SeedValue = HAL_GetTick();
	//	printf("%d",Sec_u32SeedValue) ;
}

uint32_t Sec_u32GetAlgorithm(void)
{
	return 5 ;
}

uint32_t Sec_u32GetKey (void)
{
	uint32_t Local_u32KeyValue = 0 ;
	Local_u32KeyValue = Sec_u32SeedValue + Sec_u32GetAlgorithm() ;
	//	Local_u32KeyValue = 0x01020304 + Sec_u32GetAlgorithm() ;
	return Local_u32KeyValue;
}


uint8_t Sec_uint32SecurityAccess (PduInfoType * Ptr)
{
	uint8_t Local_u8ErrorStates = E_OK ;
	Ptr->Data[Sub_F] &= 0x7F;

	if (Ptr->Data[Sub_F] == Seed)
	{
		/*Generate Seed */
		Sec_u32GetSeed();

		/*Send Frame with Positive Response */
		//		Frame_Info Response ;
		pos_Response.SID 		= Security_Service ;
		pos_Response.SUB_FUNC	= Seed ;
		pos_Response.DID_Length=0;
		pos_Response.Data_Length=4;
		//		for(int i = 1; i < pos_Response.Data_Length + 1; i++){
		//			pos_Response.Data[i-1] = i;
		//		}
		for(int i =0 ; i< 4; i++)
		{
			pos_Response.Data[i]=(uint8_t) (Sec_u32SeedValue>>(24-(i*8))) ;
		}



	}
	else if (Ptr->Data[Sub_F] == Key)
	{
		uint32_t user_key= Ptr->Data[Data_Sub_Fun]<<24 | Ptr->Data[Data_Sub_Fun+1]<<16 | Ptr->Data[Data_Sub_Fun+2]<<8 |Ptr->Data[Data_Sub_Fun+3];
		/*Check if Key sent is correct or Not */
		if (user_key == Sec_u32GetKey())
		{
			/*Change the state of security */
			Sec_State = Secure ;
			global_sec_flag = Secure;
			/*Send Positive Response */
			pos_Response.SID= Ptr->Data[SID];
			pos_Response.SUB_FUNC = Key;
			pos_Response.DID_Length = 0;
			pos_Response.Data_Length = 0;

		}
		else
		{
			Sec_State = Un_Secure ;
			global_sec_flag = Un_Secure;
			Local_u8ErrorStates = E_NOK ;
			UDS_Send_Neg_Res(Ptr->Data[SID] , invalidKey) ;
			return Local_u8ErrorStates;
		}
	}
	else
	{
		Local_u8ErrorStates = E_NOK ;
		UDS_Send_Neg_Res(Ptr->Data[SID] , subFunctionNotSupported) ;
	}


	UDS_Send_Pos_Res(&pos_Response) ;

	return Local_u8ErrorStates ;
}


/***************************************************************************************************/
void UDS_Read_Data_Server(uint8_t* data)
{
	pos_Response.SUB_FUNC = -1;

	//Send +ve responce
	pos_Response.SID = Read_Service ;
	pos_Response.DID_Length=2;
	data[DID_1] |= 0b10000000; //Original DID

	//if DID --> Oil_Temp
	if((data[DID_1] == Oil_Temp_First_byte) && (data[DID_2] == Oil_Temp_Second_byte) )
	{

		pos_Response.DID[0]=Oil_Temp_First_byte;
		pos_Response.DID[1]=Oil_Temp_Second_byte;
		pos_Response.Data[0]=Oil_Temp_var>>8;
		pos_Response.Data[1]=Oil_Temp_var & 0xFF;
		pos_Response.Data_Length = 2;



		//	UDS_Send_Pos_Res(Read_Data_Server);
	}//if DID --> Oil_Pressure
	else if((data[DID_1] == Oil_Pressure_First_byte) && (data[DID_2] == Oil_Pressure_Second_byte) )
	{
		pos_Response.DID[0]=Oil_Pressure_First_byte;
		pos_Response.DID[1]=Oil_Pressure_Second_byte;
		pos_Response.Data[0]=Oil_Pressure_var>>24;
		pos_Response.Data[1]=Oil_Pressure_var>>16;
		pos_Response.Data[2]=Oil_Pressure_var>>8;
		pos_Response.Data[3]=Oil_Pressure_var & 0xFF;

		pos_Response.Data_Length = 4;



		//For Debugging
		//HAL_UART_Transmit(&huart2, "\r\nRead Frame Client DID:", 50, HAL_MAX_DELAY);
		//sendHexArrayAsASCII(Read_Data_Server.DID, Read_Data_Server.DID_Length );
		//HAL_UART_Transmit(&huart2, "\r\n", 50, HAL_MAX_DELAY);
	}
	else if( (data[DID_1] == VIN_number_First_byte) && (data[DID_2] == VIN_number_Second_byte) ){
		pos_Response.DID[0] = VIN_number_First_byte;
		pos_Response.DID[1] = VIN_number_Second_byte;
		/*
		pos_Response.Data[0] = (VIN_number_var>>24) & 0xFF;
		pos_Response.Data[1] = (VIN_number_var>>16) & 0xFF;
		pos_Response.Data[2] = (VIN_number_var>>8) & 0xFF;
		pos_Response.Data[3] = VIN_number_var & 0xFF;

		pos_Response.Data_Length = 4;
		 */
		for (uint8_t i=0; i<=16; i++)
		{
			pos_Response.Data[/*16-*/i] = VIN_number_var[i] /*(VIN_number_var >> (i*8)) & 0xFF*/ ;

		}
		pos_Response.Data_Length = 17;

	}
	else
	{
		//otherwize: send -ve responce
		UDS_Send_Neg_Res(Read_Service, RequestOutofRange);
	}


	UDS_Send_Pos_Res(&pos_Response);

}

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

/*************************************************
 void server_call_back()

 Description	:
 *	- The function initializes a flag to check the validity of the target.
 *	- The function then initializes a flag to check the validity of the received SID.
 *  - The function then validates the received SID against a set of predefined services.
 *  - Based on the validated SID, it executes corresponding service actions, such as Control Service,
 *    Read Service, Security Service, Write Service, and Tester Representative Service.
 *  - If the SID is valid and matches Control Service, it further checks the session type and changes
 *    the session state accordingly.
 Synch/Asynch	:	Synchronouse performs operations sequentially and returns once all actions are completed.
 Reentrance		:	non reeentrant because global variable
 parameter in	:	null	(void)
 parameter out	:	null	(void)
 return value	:	void

 ***************************************************/

void server_call_back(uint32_t TxPduId, PduInfoType* ptr)
{
	PduDataPTR = ptr;
	// create flag for check SID this is local bec . every frame i need to check the sid
	uint8_t local_sid_flag = 0;
	uint8_t local_target_accept = 0;
	SupressedPosRes_Server = ((ptr->Data[Sub_F] & 0b10000000)>>7) ;
	pos_Response.ADD_Target = Client_Address;
	Control.ADD_Target = Client_Address;
	// this for test only
	//uint8_t ptr->Data[20] = {2 ,Control_Service , 5 };
	// we need to check for address target (ADD_Target) if true this message is for me if not rejected it

	#if (Global_Target == Tempreture_Add )
	if(ptr->Data[ADD_Target] == Tempreture_Address )
	{
		local_target_accept=1;
		pos_Response.ADD_Source = Tempreture_Address;
		Control.ADD_Source = Tempreture_Address;
		msg.Data[ADD_NR_Source] = Tempreture_Address;
	}
#elif (Global_Target ==Pressure_Add )
	if(ptr->Data[ADD_Target] == Pressure_Address )
	{
		local_target_accept=1;
		pos_Response.ADD_Source = Pressure_Address;
		Control.ADD_Source = Pressure_Address;
		msg.Data[ADD_NR_Source] = Pressure_Address;

	}
#elif (Global_Target ==Functional_Add )
	if(ptr->Data[ADD_Target] == Functional_Address )
	{
		local_target_accept=1;
		pos_Response.ADD_Source = Functional_Address;
		Control.ADD_Source = Functional_Address;
		msg.Data[ADD_NR_Source] = Functional_Address;

	}
#endif

	if(local_target_accept)
	{
		// for SID validation
		if (ptr->Data[SID] == Control_Service || ptr->Data[SID]== Read_Service || ptr->Data[SID] == Write_Service || ptr->Data[SID] == Security_Service || ptr->Data[SID] == Tester_Representer_Service)
		{
			// tmam
			reset_timer();
			if(global_session != DefaultSession){
				start_timer();
			}
			local_sid_flag = 1;


		}
		else
		{
			// let error code of NRC =0 ;

			// for test only
			//HAL_UART_Transmit(&huart2, (const uint8_t*)" -ive there is no SID has this name \r\n", 50, HAL_MAX_DELAY );

			// this mean the SID not supported
			UDS_Send_Neg_Res(ptr->Data[SID],  serviceNotSupported);
			// go out of isr
			return;
		}
		if (local_sid_flag)
		{
			//Check Suppressed Positive Response


			if (ptr->Data[SID] == Control_Service)
			{
				flag_sub_fun = 1;
			}
			else if (ptr->Data[SID] == Read_Service)
			{
				//  for test
				//	printf("u are in Read_Service\n");
				// send read function (rad resp as the actual ptr->Data of temp or pressure)
				//	HAL_UART_Transmit(&huart2, (const uint8_t*)" UDS_Read_Data_Server() \r\n", 50, HAL_MAX_DELAY ); // delete ---> after write your func

				//UDS_Read_Data_Server();

				UDS_Read_Data_Server(ptr->Data);

			}
			else if (ptr->Data[SID] == Security_Service )
			{

				//	printf("send_ser_sec() +ive resp \n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)" send_ser_sec() +ive resp change the flag \r\n", 50, 100 ); // delete this after you put your func

				//		send_ser_sec() ; // send seed

				// (write here +ive resp for security) ------------------------> here

				Sec_uint32SecurityAccess(PduDataPTR);
				//			UDS_Send_Pos_Res(&pos_Response) ;
			}


			else if (ptr->Data[SID] == Write_Service && global_sec_flag ==1 && global_session == ExtendedSession)
			{
				//printf("u are in Write_Service\n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)" u are in Write_Service \r\n", 50, 100 );
				//			// send write response
				//			//	printf("UDS_Write_Data_Server() \n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)" UDS_Write_Data_Server() \r\n", 50, 100 ); // delete it after put your func

				// ptr->Data write with +ive resp


				UDS_Write_Data_Server(ptr->Data,  ptr->Data[0]);
			}
			else if (ptr->Data[SID] == Write_Service && global_sec_flag == 0 && global_session == ExtendedSession )
			{
				//printf("u are not in Write_Service\n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)" u are not in Write_Service \r\n", 50, 100 );
				//			// send -ive response
				//			//printf("UDS_Write_Data_Server() \n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)"UDS_Write_Data_Server() \r\n", 50, 100 ); // delete this after put your func
				// (write here -ive resp for write security ) ------------------------> here

				UDS_Send_Neg_Res(ptr->Data[SID], securityAccessDenied);

			}
			else if (ptr->Data[SID] == Write_Service  && global_session == DefaultSession)
			{
				// (write here -ive resp for write session (NRC ) ------------------------> here

				UDS_Send_Neg_Res(ptr->Data[SID], serviceNotSupportedInActiveSession);

			}
			else if (ptr->Data[SID] == Tester_Representer_Service)
			{
				//printf("u are in Tester_Representer_Service\n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)" u are in Tester_Representer_Service \r\n", 50, 100 );
				// call the fun of tester Representer
				UDS_Tester_Presenter_Server();
				//printf("void UDS_Tester_Present(void) \n");
				//			HAL_UART_Transmit(&huart2, (const uint8_t*)" void UDS_Tester_Present(void) \r\n", 50, 100 ); // delete this func after put your func

				// (write here +ive resp for  Tester_Representer_Service) ------------------------> here
			}
		}
		// check sub fun
		if (flag_sub_fun== sub_func_control)
		{
			ptr->Data[Sub_F] &= 0x7F; //Original Sub_Func
			// true sub fun
			if (ptr->Data[SID]== Control_Service && ptr->Data[Sub_F] == DefaultSession)
			{
				// change the state to default
				//	printf(" UDS_Process_Session(void); \n ");
				//HAL_UART_Transmit(&huart2, (const uint8_t*)" UDS_Process_Session(void) \r\n", 50, 100 );

				//HAL_UART_Transmit(&huart2, (const uint8_t*)" changed to DefaultSession \r\n", 50, 100 );
				// (write here +ive resp for  change to def- session ) ------------------------> here

				//global_session = DefaultSession;
				UDS_Control_Session_Server(ptr->Data);

			}
			else if (ptr->Data[SID] == Control_Service && ptr->Data[Sub_F] == ExtendedSession)
			{
				//printf(" UDS_Process_Session(void); \n ");
				// change to extended
				//	printf(" changed to ExtendedSession ");
				//	HAL_UART_Transmit(&huart2, (const uint8_t*)" changed to ExtendedSession \r\n", 50, 100 );

				// (write here +ive resp for  change to ext session ) ------------------------> here
				//global_session = ExtendedSession;
				UDS_Control_Session_Server(ptr->Data);
			}
			else
			{
				//printf(" not supported ");
				//HAL_UART_Transmit(&huart2, (const uint8_t*)" not supported \r\n", 50, 100 );

				// (write here -ive resp for sub servise ) ------------------------> here
			}
		}
		else
		{
			UDS_Send_Neg_Res(ptr->Data[SID], subFunctionNotSupported);
			// error in sub func

		}
	}
	else
	{
		// not the target
	}
}

void UDS_MainFunction()
{
	while(1)
	{
		if(UDS_Tx_Confirm)
		{
			UDS_Tx_Confirm = 0;
			if ( UDS_Struct ->Data[Neg_Res_INDEX] == 0x7f)
			{
				switch(UDS_Struct ->Data[NRC_INDEX])
				{
				case serviceNotSupported :
					HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nThis Service is not Available.\r\n",strlen("\r\nThis Service is not Available.\r\n"), HAL_MAX_DELAY);
					break ;

				case securityAccessDenied :
					HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nNot Secured, Please Enter a Key.\r\n",strlen("\r\nNot Secured, Please Enter a Key.\r\n"), HAL_MAX_DELAY);

					break ;

				case subFunctionNotSupported :
					HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nWrong Session, Please Enter Extended Session.\r\n",strlen("\r\nWrong Session, Please Enter Extended Session.\r\n"), HAL_MAX_DELAY);

					break ;

				case serviceNotSupportedInActiveSession :
					HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nWrong Session, Please Enter Extended Session.\r\n",strlen("\r\nWrong Session, Please Enter Extended Session.\r\n"), HAL_MAX_DELAY);

					break ;

				case invalidKey :
					HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nWrong Key.\r\n",strlen("\r\nWrong Key.\r\n"), HAL_MAX_DELAY);

					break ;

				default:
					//Nothing
				}
			}

			else {
				UDS_Struct->Data[SID] = UDS_Struct->Data[SID] - 0x40;
				if (UDS_Struct->Data[SID] == Read_Service)

				{
					if  ( UDS_Struct->Data[DID_1] == Oil_Temp_First_byte &&  UDS_Struct->Data[DID_2] == Oil_Temp_Second_byte )
					{

						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nOil Temp:\r\n", strlen("\r\nOil Temp:\r\n"), HAL_MAX_DELAY);
						sendHexArrayAsASCII((uint8_t*)&UDS_Struct->Data[Data_DID],2);

					}

					else if  ( UDS_Struct->Data[DID_1] == Oil_Pressure_First_byte &&  UDS_Struct->Data[DID_2] == Oil_Pressure_Second_byte  )
					{
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nOil Pressure:\r\n", strlen("\r\nOil Pressure:\r\n"), HAL_MAX_DELAY);
						sendHexArrayAsASCII((uint8_t*)&UDS_Struct->Data[Data_DID],4);

					}

					else if  ( UDS_Struct->Data[DID_1] == VIN_number_First_byte &&  UDS_Struct->Data[DID_2] == VIN_number_Second_byte  )
					{
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nVIN Number:\r\n", strlen("\r\nVIN Number:\r\n"), HAL_MAX_DELAY);
						sendHexArrayAsASCII((uint8_t*)&UDS_Struct->Data[Data_DID],17);
						//sendHexArrayAsASCII((uint8_t*)&UDS_Struct->Data[Data_DID],17);

					}
				}

				/*  Write IDS Message */
				else if (UDS_Struct->Data[SID] == Write_Service)

				{

					if ( UDS_Struct->Data[DID_1] == Oil_Temp_First_byte &&  UDS_Struct->Data[DID_2] == Oil_Temp_Second_byte )
					{
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nOil Temperature Written successfully.\r\n", strlen("\r\nOil Temperature Written successfully .\r\n"), HAL_MAX_DELAY);
					}

					else if  ( UDS_Struct->Data[DID_1] == Oil_Pressure_First_byte &&  UDS_Struct->Data[DID_2] == Oil_Pressure_Second_byte  )
					{
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nOil Pressure Written successfully.\r\n", strlen("\r\nOil Pressure Written successfully .\r\n"), HAL_MAX_DELAY);
					}

					else if  ( UDS_Struct->Data[DID_1] == VIN_number_First_byte &&  UDS_Struct->Data[DID_2] == VIN_number_Second_byte   )
					{
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nVIN Number Written successfully.\r\n", strlen("\r\nVIN Number Written successfully .\r\n"), HAL_MAX_DELAY);
					}
				}
				/*  Control Service  Session */

				else if (UDS_Struct->Data[SID] == Control_Service)

				{

					switch ( UDS_Struct->Data[Sub_F] )
					{

					case DefaultSession :
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nIt's Default Session!\r\n", strlen("\r\nIt's Default Session!\r\n"), HAL_MAX_DELAY);
						//UDS_Read_Data_Client(Frame_Info.DID);
						break ;

					case ExtendedSession :
						HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nIt's Extended Session!\r\n", strlen("\r\nIt's Extended Session!\r\n"), HAL_MAX_DELAY);
						//UDS_Read_Data_Client(Frame_Info.DID);
						break ;
					}
				}
				/*  Security Service  */

				else if (UDS_Struct->Data[SID] == Security_Service)

				{

					switch ( UDS_Struct->Data[Sub_F] )
					{

					case Key :
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nKey is Compatible!\r\n", strlen ( "\r\nKey is Compatible!\r\n"), HAL_MAX_DELAY);
						break ;

					case Seed :
						HAL_UART_Transmit(&huart2, (uint8_t*) "\r\nSeed is:\r\n", strlen ("\r\nSeed is:\r\n"), HAL_MAX_DELAY);
						sendHexArrayAsASCII((uint8_t*)&UDS_Struct->Data[Data_Sub_Fun],4);

						break ;
					}
				}
				/*  Tester_Representer_Service  */

				else if (UDS_Struct->Data[SID] == Tester_Representer_Service)

				{

					HAL_UART_Transmit(&huart2,(uint8_t*) "\r\nECU Reseted!\r\n", strlen("\r\n ECU Reseted!\r\n"), HAL_MAX_DELAY);

				}

				else { /* no thing  */	}

			}

		}

		vTaskDelay(50);
	}
}

void UDS_Client_Callback(uint32_t TxPduId,PduInfoType *PduInfoPtr)
{
	UDS_Tx_Confirm = 1;
	UDS_Struct = PduInfoPtr;

}
