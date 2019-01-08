#include "rf430nfc.h"
#include "iic.h"
#include <stdint.h>

unsigned char RxData[2] = { 0, 0 };
unsigned char TxData[2] = { 0, 0 };
unsigned char TxAddr[2] = { 0, 0 };

//BUG FIXES: LOOPING, INIT AND NO WAIT TIME EXTENSION REQUEST
uint8_t errata_fixes[] = { 0xB2, 0xF0, 0xFF, 0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF,
		0xFD, 0x00, 0x07, 0xA2, 0xC3, 0x00, 0x07, 0x08, 0x3C, 0xB2, 0xF0, 0xFF,
		0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF, 0xFD, 0x00, 0x07, 0xA2, 0xC3, 0x00,
		0x07, 0xB2, 0xB0, 0x04, 0x04, 0x00, 0x07, 0xF4, 0x23, 0x30, 0x41, 0x0F,
		0x12, 0x0E, 0x12, 0xB2, 0xB0, 0x00, 0x01, 0x00, 0x07, 0x08, 0x24, 0x3E,
		0x40, 0x1D, 0x00, 0x1F, 0x42, 0xB2, 0x2A, 0x6E, 0x9F, 0x02, 0x20, 0x92,
		0x43, 0x2E, 0x2A, 0x3E, 0x41, 0x3F, 0x41, 0x30, 0x41 };

struct NdefFile_Type NdefFiles[2];
uint16_t NumberOfFiles = 0;
const uint16_t E104_Length;

uint8_t CCFileText[15] = { 0x00, 0x0F, /* CCLEN */
		0x20, /* Mapping version 2.0 */
		0x00, 0xF9, /* MLe (249 bytes); Maximum R-APDU data size */
		0x00, 0xF6, /* MLc (246 bytes); Maximum C-APDU data size */
		0x04, /* Tag, File Control TLV (4 = NDEF file) */
		0x06, /* Length, File Control TLV (6 = 6 bytes of data for this tag) */
		0xE1, 0x04, /* File Identifier */
		0x2E, 0xF4, /* Max NDEF size (8990 bytes of useable memory) */
		0x00, /* NDEF file read access condition, read access without any security */
		0x00 /* NDEF file write access condition; write access without any security */
}; //CC file text

#pragma PERSISTENT (FileTextE104)
uint8_t FileTextE104[15000] = { 0x00, 0x3A+3, /* NLEN; NDEF length (3 byte long message) */
        0xC1, 0x01,/*lenghth of four bytes */0x00, 0x00, 0x00, 0x33+3, 0x54, /* T = text */
        0x02, 0x65, 0x6E,  /*'e', 'n', */

        '6','9','9','6','-','1','1','2','2',',',                //10
        '1','8','0','4','2','3',' ','1','3',':','5','1',',',    //13
        'S','E','O','U','L',' ','M','I','L','K',' ','1',' ','l','t',',',//16
        'L','o','w',' ','F','a','t',' ',' ',' ','\n',           //11

}; //Ndef file text

uint16_t SelectedFile;		//the file that is currently selected

/**********************************************************************************************/
void setupRF430GPIO(){
    //reset the rf430
    P4OUT |= BIT4;
    P4OUT &= ~BIT4; //RESET PIN
    _delay_cycles(1000);
    P4OUT |= BIT4;

    P2SEL0 &= ~BIT2;    //set as gpio
    P2SEL1 &= ~BIT2;
    P2DIR &= ~BIT2;     //setting as input ///by default as gpio
    P2OUT |= BIT2;      //pull up is selected
    P2REN |= BIT2;      //enable pullup
    P2IFG &= ~BIT2;     //clears the interrupt
    P2IES |= BIT2;      // int at htol transition

    _delay_cycles(1000000); // Leave time for the RF430CL33H to get itself initialized;
}

void errataFix(){
        Write_Register(0xFFE0, 0x004E);
        Write_Register(0xFFFE, 0x0080);
        Write_Continuous(0x2AD0, (uint8_t *) &errata_fixes, sizeof(errata_fixes));      //write the patch to memory
        Write_Register(0x2A90, 0x2AFC);                 // looping fix
        Write_Register(0x2AAE, 0x2AD0);               // wait time extension fix
        Write_Register(0x2A66, 0x0000);                 // fix by init
        Write_Register(0x2ABA, 0x0000); // 00 = 106kbps (DEFAULT SETTING) so this line is optional
        Write_Register(0x27B8, 0);
        Write_Register(0xFFE0, 0);
}


void rf430Init(void) {
    iicInit(RF430_I2C_ADDR);
    setupRF430GPIO();
    //wait for device to get ready
    while (!(Read_Register(STATUS_REG) & READY));
    errataFix();

    //only interrupt for general type 4 request
    Write_Register(INT_ENABLE_REG, /*EXTRA_DATA_IN_INT_ENABLE +*/DATA_TRANSACTION_INT_ENABLE + FIELD_REMOVED_INT_ENABLE);
    //Configure INTO pin for active low and enable RF
    Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE);
    // sets the command timeouts, generally should stay at these settings
    Write_Register(SWTX_INDEX, 0x3B); // maximum allowable wait time extension request

    P2IFG &= ~BIT2; //clear the interrupt again
    P2IE |= BIT2;   //enable the interrupt

    AppInit();

}


void AppInit() {
	// Init CC file info
	NdefFiles[0].FileID[0] = 0xE1;
	NdefFiles[0].FileID[1] = 0x03;
	NdefFiles[0].FilePointer = (uint8_t *) CCFileText;
	NdefFiles[0].FileLength = 0;		//?

	// Init Ndef file info
	NdefFiles[1].FileID[0] = 0xE1;
	NdefFiles[1].FileID[1] = 0x04;
	NdefFiles[1].FilePointer = (uint8_t *) FileTextE104;
	//NdefFiles[1].FileLength = 0;		//?

	NumberOfFiles = 2; 			//the number if NDEF files available
	SelectedFile = 0;			//default to CC file

}

unsigned int Read_Register(unsigned int reg_addr) {
	iicInit(RF430_I2C_ADDR);
	return iicReadRegister16bitW16bitAddress(reg_addr);
}

void Write_Register(unsigned int reg_addr, unsigned int value) {
    iicInit(RF430_I2C_ADDR);
    iicWriteRegister16bitW16bitAddress(value,reg_addr);
}

void Read_Continuous(unsigned int reg_addr, unsigned char* read_data,
		unsigned int data_length) {
	unsigned int i;

	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address

	UCB0CTL1 &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;	//start i2c write operation.  Sending Slave address

	while (!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	while (!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	while (!(UCB0IFG & UCTXIFG0));   	// Waiting for TX to finish on bus
	UCB0CTL1 &= ~UCTR; 				//i2c read operation
	UCB0CTL1 |= UCTXSTT; 			//repeated start
	while (!(UCB0IFG & UCRXIFG0));

	for (i = 0; i < data_length - 1; i++) {
		while (!(UCB0IFG & UCRXIFG0));
		read_data[i] = UCB0RXBUF;
	}

	UCB0CTL1 |= UCTXSTP; 			//send stop after next RX
	while (!(UCB0IFG & UCRXIFG0));
	read_data[i] = UCB0RXBUF;
	while ((UCB0STAT & UCBBUSY));    // Ensure stop condition got sent
	UCB0CTL1 |= UCSWRST;
}

void Write_Continuous(unsigned int reg_addr, unsigned char* write_data,	unsigned int data_length) {
	unsigned int i;

	iicInit(RF430_I2C_ADDR);

	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address

	UCB0CTL1 &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		//start i2c write operation
	//write the address
	while (!(UCB0IFG & UCTXIFG0));

	UCB0TXBUF = TxAddr[0];
	while (!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];

	for (i = 0; i < data_length; i++) {
		while (!(UCB0IFG & UCTXIFG0));
		UCB0TXBUF = write_data[i];
	}

	while (!(UCB0IFG & UCTXIFG0));
	UCB0CTL1 |= UCTXSTP;
	while ((UCB0STAT & UCBBUSY));    // Ensure stop condition got sent
	UCB0CTL1 |= UCSWRST;

}

enum FileExistType SearchForFile(uint8_t *fileId) {
	uint16_t i;
	enum FileExistType ret = FileNotFound; // 0 means not found, 1 mean found

	for (i = 0; i < NumberOfFiles; i++) {
		if (NdefFiles[i].FileID[0] == fileId[0] && NdefFiles[i].FileID[1] == fileId[1]) {
			ret = FileFound;
			SelectedFile = i;  // the index of the selected file in the array
			break;
		}
	}
	return ret;
}

uint16_t SendDataOnFile(uint16_t selectedFile, uint16_t buffer_start,uint16_t file_offset, uint16_t length) {
	uint16_t ret_length;
	uint16_t file_length;

	file_length = (((uint16_t) NdefFiles[selectedFile].FilePointer[0]) << 8)+ NdefFiles[selectedFile].FilePointer[1];
	// reads the NLEN of the file
	// make sure we are not sending above the length of the file (the send data early feature above does not do file length check)
	// if reader requests data we can expect that it is the correct length
	if (file_length < (file_offset + length)) {
		length = file_length - file_offset;
	}

	Write_Continuous(buffer_start,(uint8_t *) &NdefFiles[selectedFile].FilePointer[file_offset],length);
	ret_length = length;

	return ret_length;

	//Regs[NDEF_FILE_LENGTH_INDEX]  = Regs[NDEF_FILE_LENGTH_INDEX]
	//do not change right now, we are only sending as much as has been requested
	//if we wanted to send data than requested, we would update the Regs[NDEF_FILE_LENGTH_INDEX] register to a higher value
}

void ReadDataOnFile(uint16_t selectedFile, uint16_t buffer_start,uint16_t file_offset, uint16_t length) {
	uint16_t * e104_l = (uint16_t *) &E104_Length;

	Read_Continuous(buffer_start,(uint8_t *) &NdefFiles[selectedFile].FilePointer[file_offset],length);
	if (NdefFiles[selectedFile].FileLength < (file_offset + length)) {
		NdefFiles[selectedFile].FileLength = file_offset + length;
		*e104_l = file_offset + length;
	}
	//Regs[NDEF_FILE_LENGTH_INDEX]  = Regs[NDEF_FILE_LENGTH_INDEX]
	//do not change right now, we are only sending as much as has been requested
	//if we wanted to send more data than requested, we would update the Regs[NDEF_FILE_LENGTH_INDEX] register to a higher value
}

