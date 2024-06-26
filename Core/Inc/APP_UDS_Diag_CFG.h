#ifndef INC_APP_UDS_DIAG_CFG_H_
#define INC_APP_UDS_DIAG_CFG_H_

#include "stdint.h"



typedef enum {
	Main_Menu = 0,
	Control_Session_Menu,
	Read_Data_Menu,
	Write_Data_Menu,
	Security_Access_Menu,
	Response_Permission_Menu,
	Target_Selection_Menu,

} ClientMenu;


//typedef enum {
//	Default_Session = 1,
//	Extended_Session=3,
//
//} SessionType;


typedef enum {
	Control_Service = 0x10,
	Read_Service = 0x22,
	Security_Service = 0x27,
	Write_Service = 0x2E,
	Tester_Representer_Service = 0x3E,

} Services;


typedef enum {
	Not_Available = 0x00,
	Available = 0x01,

} Availability;


typedef enum {
	Control_Service_Availability = Available,
	Read_Service_Availability = Available,
	Security_Service_Availability = Available,
	Write_Service_Availability = Available,
	Tester_Representer_Service_Availability = Available,

} Services_Availability;


typedef enum {
	Oil_Temp = 0xF13D,
	Oil_Pressure = 0xF53D,
	VIN_number = 0xF23D,
	Oil_Temp_First_byte=0xF1,
	Oil_Temp_Second_byte=0x3D,
	Oil_Pressure_First_byte=0xF5,
	Oil_Pressure_Second_byte=0x3D,
	VIN_number_First_byte=0xF2,
	VIN_number_Second_byte=0x3D,


}DID;




typedef enum {
	Secure = 0x01,
	Un_Secure = 0x00,

}Security_Access_State;


typedef enum {
	DefaultSession = 0x01,
	ExtendedSession = 0x03,
	Seed = 0x01,
	Key = 0x02,

}Sub_Fun;



// Struct For:
//void UDS_Send_Pos_Res(void);
//void UDS_Send_Neg_Res(void);

typedef struct {
	Services SID;
	DID did;
	Sub_Fun sub_fun;
	int8_t* data;

}Frame_Info;


typedef struct
{
	uint8_t ADD_Source;
	uint8_t ADD_Target;
	uint8_t SID;
	int8_t SUB_FUNC;
	int8_t DID[2];
	uint8_t DID_Length;
	int8_t Data[4096];
	uint8_t Data_Length;
}ServiceInfo;

/*

typedef enum
{
	E_OK = 0x00,
	E_NOK
}Std_ReturnType;
 */

typedef enum
{
	Key_Code = 0x05,
	Seed_Key_Lenght = 4
}Security_Info;


typedef struct
{
	uint8_t* Seed;
	uint8_t* Key;
	Security_Access_State state;//enum
}Security_Key;

/*
typedef enum {
	PCI = 0,
	SID = 1,
	DID_1 = 2,
	DID_2 = 3,
	Sub_F = 2,
	Data_DID = 4 ,
	Data_Sub_Fun = 3 ,

	Neg_Res_INDEX=1,
	SID_NR_INDEX=2,
	NRC_INDEX=3,

} Indecis;

 */


typedef enum {
	// PCI, ADD_Source ,ADD_Target, SID ,(DID1,DID2) / Sub_F , Data.
	// PCI, ADD_Source ,ADD_Target, SID ,(DID1,DID2) / Sub_F , Data.

	Addition_length=2,
	PCI = 0,
	ADD_Source= 1,
	ADD_Target= 2,
	SID = 3,
	DID_1 = 4 ,
	DID_2 = 5 ,
	Sub_F = 4,
	Data_DID = 6 ,
	Data_Sub_Fun = 5 ,

	Neg_Res_INDEX=1,
	ADD_NR_Source=2,
	ADD_NR_Target=3,
	SID_NR_INDEX=4,
	NRC_INDEX=5,

}Indices;

typedef enum {
	Client_Address = 0xF1 ,
	Tempreture_Address= 0x3D ,
	Pressure_Address= 0x4D,
	Functional_Address=0xFF
}Addresses;


typedef enum
{
	NRC_WRITE_secuirty = 10,
	NRC_WRITE_defualt	=15,
	NRC_SID = 20,
	NRC_sub_fun = 30,
	NRC_sec_key_seed = 40


}NRC_VAR;


typedef enum
{
	serviceNotSupported	 = 0x11,
	subFunctionNotSupported	 = 0x12,
	serviceNotSupportedInActiveSession = 0x7F,
	invalidKey = 0x35,
	securityAccessDenied = 0x33,

	incorrectMessageLengthOrInvalidFormat = 0x13,
	RequestOutofRange = 0x31,  // when we want to read var didn't exist

}NRC_222;

#endif
