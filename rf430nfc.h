#ifndef RF430NFC_H_
#define RF430NFC_H_

#include "msp430.h"

void NFCWrite_Register(unsigned int reg_addr, unsigned int value);
unsigned int NFCRead_Register(unsigned int reg_addr);
void NFCWrite_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length);
void rf430Init(void);

#define RF430_I2C_ADDR 	0x0018

//define the values for Granite's registers we want to access
#define CONTROL_REG 			0xFFFE
#define STATUS_REG				0xFFFC
#define INT_ENABLE_REG			0xFFFA
#define INT_FLAG_REG			0xFFF8
#define CRC_RESULT_REG			0xFFF6
#define CRC_LENGTH_REG			0xFFF4
#define CRC_START_ADDR_REG		0xFFF2
#define COMM_WD_CTRL_REG		0xFFF0
#define VERSION_REG				0xFFEE //contains the software version of the ROM
#define NDEF_FILE_ID			0xFFEC
#define HOST_RESPONSE			0xFFEA
#define NDEF_FILE_LENGTH		0xFFE8
#define NDEF_FILE_OFFSET		0xFFE6
#define NDEF_BUFFER_START		0xFFE4
#define TEST_FUNCTION_REG   	0xFFE2
#define SWTX_INDEX				0xFFDE
#define CUSTOM_RESPONSE_REG		0xFFDA

//define the different virtual register bits
//CONTROL_REG bits
#define SW_RESET			BIT0
#define RF_ENABLE			BIT1
#define INT_ENABLE			BIT2
#define INTO_HIGH			BIT3
#define INTO_DRIVE			BIT4
#define BIP8_ENABLE			BIT5
#define STANDBY_ENABLE		BIT6
#define AUTO_ACK_ON_WRITE	BIT8

//STATUS_REG bits
#define READY					BIT0
#define CRC_ACTIVE				BIT1
#define RF_BUSY					BIT2
#define APP_STATUS_REGS			BIT4 + BIT5 + BIT6
#define FILE_SELECT_STATUS		BIT4
#define FILE_REQUEST_STATUS		BIT5
#define FILE_AVAILABLE_STATUS	BIT5 + BIT4


//INT_ENABLE_REG bits
#define EOR_INT_ENABLE				BIT1
#define EOW_INT_ENABLE				BIT2
#define CRC_INT_ENABLE				BIT3
#define BIP8_ERROR_INT_ENABLE		BIT4
#define DATA_TRANSACTION_INT_ENABLE	BIT5
#define FIELD_REMOVED_INT_ENABLE 	BIT6
#define GENERIC_ERROR_INT_ENABLE	BIT7
#define EXTRA_DATA_IN_INT_ENABLE	BIT8

//INT_FLAG_REG bits
#define EOR_INT_FLAG				BIT1
#define EOW_INT_FLAG				BIT2
#define CRC_INT_FLAG				BIT3
#define BIP8_ERROR_INT_FLAG			BIT4
#define DATA_TRANSACTION_INT_FLAG	BIT5
#define FIELD_REMOVED_INT_FLAG	 	BIT6
#define GENERIC_ERROR_INT_FLAG		BIT7
#define EXTRA_DATA_IN_FLAG			BIT8

//COMM_WD_CTRL_REG bits
#define WD_ENABLE	BIT0
#define TIMEOUT_PERIOD_2_SEC	0
#define TIMEOUT_PERIOD_32_SEC	BIT1
#define TIMEOUT_PERIOD_8_5_MIN	BIT2
#define TIMEOUT_PERIOD_MASK		BIT1 + BIT2 + BIT3


//Host response index
#define INT_SERVICED_FIELD 			BIT0
#define FILE_EXISTS_FIELD			BIT1
#define CUSTOM_RESPONSE_FIELD		BIT2
#define EXTRA_DATA_IN_SENT_FIELD	BIT3
#define FILE_DOES_NOT_EXIST_FIELD	0

#endif
