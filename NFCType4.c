#include "msp430.h"
#include "NFCType4.h"
#include "rf430nfc.h"
#include "iic.h"
#include "stdint.h"
#include "ph.h"
#include "rtc.h"

extern unsigned char HOURS;
extern unsigned char MINUTES;
extern unsigned char MONTHS;
extern unsigned char DAYS;
extern unsigned int YEARS;

struct NdefFile_Type NdefFiles[8];

extern unsigned int logInterval;

unsigned int NumberOfFiles = 0;
const unsigned int E104_Length;

int pHLevels[8];

unsigned int SelectedFile;		//the file that is currently selected

const uint8_t CCFileText[50] = { 0x00, 0x18,//0x2F, /* CCLEN */
		0x20,               /* Mapping version 2.0 */
		0x00, 0xF9,         /* MLe (249 bytes); Maximum R-APDU data size */
		0x00, 0xF6,         /* MLc (246 bytes); Maximum C-APDU data size */
		0x04,               /* Tag, File Control TLV (4 = NDEF file) */
		0x06,               /* Length, File Control TLV (6 = 6 bytes of data for this tag) */
		0xE1, 0x04,         /* File Identifier */
		0xB4, 0x00,         /* Max NDEF size (45K bytes of useable memory) */
		0x00, /* NDEF file read access condition, read access without any security */
		0x00, /* NDEF file write access condition; write access without any security */

		//proprietary file
		0x05,               /* Tag, File Control TLV (4 = NDEF file) */
        0x06,               /* Length, File Control TLV (6 = 6 bytes of data for this tag) */
        0xE1, 0x05,         /* File Identifier */
        0x28, 0x00,         /* Max NDEF size (10K bytes of useable memory) */
        0x00,               /* Proprietary file read access condition, 1 byte:*/
        0x00,               /* Proprietary file write access condition, 1 byte:*/
}; //CC file text

/* This is a NDEF file. It contains NLEN and NDEF message*/
#pragma PERSISTENT(FileTextE104)
uint8_t FileTextE104[20000] = {
          0x00, DEFNLEN, /* NLEN; exclude itself */
          0xC1, 0x01,/*length of four bytes nlen-7 */0x00, 0x00, 0x00, DEFPLEN, 0x54, /* T = text */
          //pay load length starts from this line
          0x02, 0x65, 0x6E,  /*'e', 'n', */

                 '6','9','9','6',              '-',          '1','1','2','2',        ',',                //10
                 '1','8','0','4','2','3',      ' ',          '1','3',':','5','1',    ',',               //13
                 'S','E','O','U','L',' ','M','I','L','K',' ','1',' ','l','t',',',                       //16
                 'L','o','w',' ','F','a','t','\n',0x03                                                 //9
};

//#pragma PERSISTENT(settingFile)
unsigned char settingFile[100]= {0};

/* COMMAND FILE */

#pragma NOINIT(FileTextE105)
uint8_t FileTextE105[13];

int doReset = 0;

void AppInit(){
	//The NFC Forum Device SHALL accept NDEF Tag Applications having a CC file with a file identifier equal to E103h.
	// Init CC file info
	NdefFiles[0].FileID[0] = 0xE1;
	NdefFiles[0].FileID[1] = 0x03;
	NdefFiles[0].FilePointer = (unsigned char *)CCFileText;
	NdefFiles[0].FileLength = 0;		//?

	// Init Ndef file info
	NdefFiles[1].FileID[0] = 0xE1;
	NdefFiles[1].FileID[1] = 0x04;
	NdefFiles[1].FilePointer = (unsigned char *)FileTextE104;
	//NdefFiles[1].FileLength = 0;		//?

	// Init Ndef file info
	NdefFiles[2].FileID[0] = 0xE1;
	NdefFiles[2].FileID[1] = 0x05;
	NdefFiles[2].FilePointer = (unsigned char *)FileTextE105;
	//NdefFiles[1].FileLength = 0;		//?

	NumberOfFiles = 3; 			//the number if NDEF files available
	SelectedFile = 0;			//default to CC file

	/* PLEN */
	FileTextE105[0] = 0x00; 	/*PLEN Low Byte*/
	FileTextE105[1] = 0x0B; 	/*PLEN High Byte*/
	/* Proprietary Message */
	FileTextE105[2] = 0x00;      //Busy?
	FileTextE105[3] = 0x00; 	//START_DATA_HEADER
	FileTextE105[4] = 0x00; 	//COMMAND
	FileTextE105[5] = 0x00; 	//Command parameter #1
	FileTextE105[6] = 0x00; 	//Command parameter #2
	FileTextE105[7] = 0x00; 	//Command parameter #3
	FileTextE105[8] = 0x00; 	//Command parameter #4
	FileTextE105[9] = 0x00; 	//END_DATA_HEADER
	FileTextE105[10] = 0x00; 	//END_DATA_HEADER
	FileTextE105[11] = 0x00; 	//'\n';
	FileTextE105[12] = 0x00; 	//Smart Patch ACK response.

}


void rf430Interrupt(unsigned int flags){
    unsigned int interrupt_serviced = 0;

    if (flags & FIELD_REMOVED_INT_ENABLE) {
            interrupt_serviced |= FIELD_REMOVED_INT_FLAG; // clear this flag later
            NFCWrite_Register(INT_FLAG_REG, interrupt_serviced); //ACK the flags to clear
    }

    if(flags & DATA_TRANSACTION_INT_FLAG){ //check if the tag was read
        unsigned int status;
        unsigned int ret;

        status = NFCRead_Register(STATUS_REG);   //read status register to determine the nature of interrupt

        switch (status & APP_STATUS_REGS) {
            case FILE_SELECT_STATUS:
            {
                unsigned int file_id;
                file_id = NFCRead_Register(NDEF_FILE_ID);           //determine the file like e103 or e104
                ret = SearchForFile((unsigned char *)&file_id);     // determine if the file exists on the host controller
                interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;    //clear this flag later

                if (ret == FileFound){
                    NFCWrite_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
                    NFCWrite_Register(HOST_RESPONSE, INT_SERVICED_FIELD + FILE_EXISTS_FIELD); // indicate to the RF430 that the file exist
                } else {
                   NFCWrite_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
                   NFCWrite_Register(HOST_RESPONSE, INT_SERVICED_FIELD + FILE_DOES_NOT_EXIST_FIELD); // the file does not exist
                }
                break;
            }

            //NDEF ReadBinary request has been sent by the mobile / reader
            case FILE_REQUEST_STATUS:
            {
                unsigned int buffer_start;
                unsigned int file_offset;
                unsigned int file_length;

                buffer_start = NFCRead_Register(NDEF_BUFFER_START);
                file_offset = NFCRead_Register(NDEF_FILE_OFFSET);
                file_length = NFCRead_Register(NDEF_FILE_LENGTH);

                interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;

                file_length = SendDataOnFile(SelectedFile, buffer_start, file_offset, file_length);//THis is a write data to FR430 buff command.                //13, 29, 39

                NFCWrite_Register(NDEF_FILE_LENGTH, file_length); // how much was actually written
                NFCWrite_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
                NFCWrite_Register(HOST_RESPONSE, INT_SERVICED_FIELD); // indicate that we have serviced the request

                break;
            }

            // NDEF UpdateBinary request
            case FILE_AVAILABLE_STATUS:
            {
                unsigned int buffer_start;
                unsigned int file_offset;
                unsigned int file_length;

                interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;            // clear this flag later
                buffer_start = NFCRead_Register(NDEF_BUFFER_START); // where to start in the RF430 buffer to read the file data (0-2999)
                file_offset = NFCRead_Register(NDEF_FILE_OFFSET); // the file offset that the data begins at
                file_length = NFCRead_Register(NDEF_FILE_LENGTH); // how much of the file is in the RF430 buffer

                ReadDataOnFile(99, buffer_start, file_offset, file_length);

                if(settingFile[9] == 's' && settingFile[10] == 't'){
                    HOURS = (settingFile[11]-48)<<4 |( settingFile[12]-48) ;
                    MINUTES = (settingFile[13]-48)<<4 | settingFile[14]-48;
                    YEARS = (settingFile[15]-48)<<4 | settingFile[16]-48;

                    MONTHS = (settingFile[17]-48)<<4 | settingFile[18]-48;
                    DAYS = (settingFile[19]-48)<<4 | settingFile[20]-48;

                    rtcInit();
                } else if (settingFile[9] == 'I' && settingFile[10] == 'i'){
                    logInterval = (settingFile[11]-48)*10 + ( settingFile[12]-48) ;

                } else if (settingFile[9] == 't' && settingFile[10] == 'S' && settingFile[11] == 'r'){
                     doReset = 1;
                } else if (settingFile[9] == 'X'){
                    uint8_t c;
                    int tempVar=0 ;
                    uint8_t decimalPlace = 1;
                    uint8_t index = 0;
                    for(c = 0;settingFile[c+10] != ';';c++){
                        if( settingFile[c+10] != ','){
                            if(settingFile[c+10] == '-'){
                                tempVar *= -1;
                                continue;
                            }
                            tempVar += (settingFile[c+10] - 48)*decimalPlace;
                            decimalPlace *=10;
                        } else {
                            pHLevels[index] = tempVar;
                            tempVar = 0;
                            index++;
                            decimalPlace = 1;
                        }
                    }
                    upDatepHLevels(pHLevels);

                }
                NFCWrite_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
                NFCWrite_Register(HOST_RESPONSE, INT_SERVICED_FIELD); // the interrupt has been serviced
                break;
            }
        }//end of switch
    }//end of transaction flag

    else if(flags & EXTRA_DATA_IN_FLAG){ //check if the tag was read
        #define AMOUNT_DATA_TO_SENT_EARLY       255         // data to add to the buffer while transmit is happening
        #define MAX_TAG_BUFFER_SPACE            2998        // actually 3000 but to avoid any possibility of an issue

        unsigned int buffer_start;
        unsigned int file_offset;
        unsigned int file_length = 0;

        interrupt_serviced |= EXTRA_DATA_IN_FLAG;         // clear the interrupt flag
        if (SelectedFile == 1)  {                         // allows only prefetch on NDEF files, no Capability container ones
            buffer_start = NFCRead_Register(NDEF_BUFFER_START);
            file_offset = NFCRead_Register(NDEF_FILE_OFFSET);

            if ((buffer_start + AMOUNT_DATA_TO_SENT_EARLY) >= MAX_TAG_BUFFER_SPACE){
                __no_operation();
            } else {
                file_length = SendDataOnFile(SelectedFile, buffer_start, file_offset, AMOUNT_DATA_TO_SENT_EARLY); //255 is enough for atleast one packet
            }
        }
        NFCWrite_Register(NDEF_FILE_LENGTH, file_length);                          // how much was actually written
        NFCWrite_Register(INT_FLAG_REG, interrupt_serviced);                       // ACK the flags to clear
        NFCWrite_Register(HOST_RESPONSE, EXTRA_DATA_IN_SENT_FIELD);                // interrupt was serviced
    }
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

	if (file_length < (file_offset + length)) {
		length = file_length - file_offset;
	}
	iicInit(RF430_I2C_ADDR);
	iicWriteContinuous(buffer_start,(uint8_t *) &NdefFiles[selectedFile].FilePointer[file_offset],length);
	ret_length = length;

	return ret_length;
}

void ReadDataOnFile(uint16_t selectedFile, uint16_t buffer_start,uint16_t file_offset, uint16_t length) {
	uint16_t * e104_l = (uint16_t *) &E104_Length;
	iicInit(RF430_I2C_ADDR);
	if( selectedFile != 99){
        iicReadContinuous(buffer_start,(uint8_t *)&NdefFiles[selectedFile].FilePointer[file_offset] , length);
        if (NdefFiles[selectedFile].FileLength < (file_offset + length)) {
            NdefFiles[selectedFile].FileLength = file_offset + length;
            *e104_l = file_offset + length;
        }
	} else {
	    iicReadContinuous(buffer_start,(unsigned char *)&settingFile+2, length);

	}
}
