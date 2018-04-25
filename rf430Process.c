#include "myuart.h"
#include "rf430nfc.h"
#include "rf430Process.h"
#include "stdint.h"
#include <stdio.h>
#include <driverlib.h>

//#define DEBUG 1

unsigned char rstdata[] =  { 0x00, 0x3A+3, /* NLEN; NDEF length (3 byte long message) */
                             0xC1, 0x01,/*lenghth of four bytes */0x00, 0x00, 0x00, 0x33+3, 0x54, /* T = text */
                             0x02, 0x65, 0x6E,  /*'e', 'n', */

                             '6','9','9','6','-','1','1','2','2',',',                //10
                             '1','8','0','4','2','3',' ','1','3',':','5','1',',',    //13
                             'S','E','O','U','L',' ','M','I','L','K',' ','1',' ','l','t',',',//16
                             'L','o','w',' ','F','a','t',' ',' ',' ','\n',           //11
}; //Ndef file text

#pragma PERSISTENT (offset)
int offset = 0;

#pragma PERSISTENT (digitalSensor)
int digitalSensor = 0;

extern uint16_t SelectedFile;
extern uint8_t FileTextE104[];

extern unsigned char HOURS;
extern unsigned char MINUTES;
extern unsigned char MONTHS;
extern unsigned char DAYS;
extern unsigned int YEARS;


extern unsigned int numOfLogsInFram ;

extern unsigned int ui16nlenhold ;

extern unsigned int ui16plenhold;


void rf430Interrupt(uint16_t flags) {

	uint16_t interrupt_serviced = 0;//which interrupt is just served of nfc ic

	if (flags & FIELD_REMOVED_INT_ENABLE) {

#ifdef DEBUG
		myuart_tx_string("Removed from the RF field");
#endif
		interrupt_serviced |= FIELD_REMOVED_INT_FLAG; // clear this flag later
		Write_Register(INT_FLAG_REG, interrupt_serviced); //ACK the flags to clear
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	if (flags & DATA_TRANSACTION_INT_FLAG) {
		uint16_t status;
		uint16_t ret;
		status = Read_Register(STATUS_REG);	//read status register to determine the nature of interrupt

		switch (status & APP_STATUS_REGS) {

		// NDEF File Select Request is coming from the mobile/reader - response to the request is determined here
		// based on whether the file exists in our file database

		case FILE_SELECT_STATUS: {
			uint16_t file_id;


			file_id = Read_Register(NDEF_FILE_ID);//determine the file like e103 or e104
			ret = SearchForFile((uint8_t *) &file_id);//check if the file exist
			interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;// clear this flag later

			if (ret == FileFound) {
				Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
				Write_Register(HOST_RESPONSE,
						INT_SERVICED_FIELD + FILE_EXISTS_FIELD); // indicate to the RF430 that the file exist
			} else {
				Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
				Write_Register(HOST_RESPONSE,
						INT_SERVICED_FIELD + FILE_DOES_NOT_EXIST_FIELD); // the file does not exist
			}

			break;
		}

		// NDEF ReadBinary request has been sent by the mobile / reader

		case FILE_REQUEST_STATUS: {
			uint16_t buffer_start;
			uint16_t file_offset;
			uint16_t file_length;
#ifdef DEBUG
			char str[30];
			myuart_tx_string("entered reading section1\n\r");
#endif
			buffer_start = Read_Register(NDEF_BUFFER_START);
			// where to start writing the file info in the RF430 buffer (0-2999)
			file_offset = Read_Register(NDEF_FILE_OFFSET);
			// what part of the file to start sending
			file_length = Read_Register(NDEF_FILE_LENGTH);
			// how much of the file starting at offset to send
			// we can send more than requested, called caching
			// as long as we write back into the length register how
			// much we sent it



			interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;// clear this flag later
			// can have bounds check for the requested length

			file_length = SendDataOnFile(SelectedFile, buffer_start,
					file_offset, file_length);


#ifdef DEBUG
			sprintf(str, "\n\rbuff-%d off-%d len-%d self-%d\n\r",buffer_start,file_offset,file_length,SelectedFile);
			myuart_tx_string(str);
#endif
			Write_Register(NDEF_FILE_LENGTH, file_length); // how much was actually written
			Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
			Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD); // indicate that we have serviced the request

			break;
		}

		// NDEF UpdateBinary request
		case FILE_AVAILABLE_STATUS: {
			uint16_t buffer_start;
			uint16_t file_offset;
			uint16_t file_length;
			int i;
#ifdef DEBUG
			char str[30];
			myuart_tx_string("entered writing section1\n\r");
#endif
			interrupt_serviced |= DATA_TRANSACTION_INT_FLAG; // clear this flag later
			buffer_start = Read_Register(NDEF_BUFFER_START); // where to start in the RF430 buffer to read the file data (0-2999)
			file_offset = Read_Register(NDEF_FILE_OFFSET); // the file offset that the data begins at
			file_length = Read_Register(NDEF_FILE_LENGTH); // how much of the file is in the RF430 buffer

			//can have bounds check for the requested length
			ReadDataOnFile(SelectedFile, buffer_start, file_offset,
					file_length);

		//	for(i = 9 ;i<10+9;i++){

				if(FileTextE104[9] == 's' && FileTextE104[10] == 't'){
					HOURS = (FileTextE104[11]-48)<<4 |( FileTextE104[12]-48) ;
					MINUTES = (FileTextE104[13]-48)<<4 | FileTextE104[14]-48;
					YEARS = (FileTextE104[15]-48)<<4 | FileTextE104[16]-48;

					MONTHS = (FileTextE104[17]-48)<<4 | FileTextE104[18]-48;
					DAYS = (FileTextE104[19]-48)<<4 | FileTextE104[20]-48;

					for(i =0 ;i<sizeof(rstdata);i++){
						FileTextE104[i] = rstdata[i];

					}

					numOfLogsInFram = 0;

					ui16nlenhold = 0x3A+3;

					ui16plenhold = 0x33+3;

					WDTCTL = 0;
				} else if ( FileTextE104[9] == 'o'){
				    offset = (FileTextE104[11]-48)*10 + ( FileTextE104[12]-48) ;
				    if( FileTextE104[10] == '-'){
				        offset = -1*offset;
				    }
				    for(i =0 ;i<sizeof(rstdata);i++){
                        FileTextE104[i] = rstdata[i];

                    }

                    numOfLogsInFram = 0;

                    ui16nlenhold = 0x3A+3;

                    ui16plenhold = 0x33+3;

                    WDTCTL = 0;
				} else if ( FileTextE104[9] == 'D'){
				    digitalSensor = 1;
				    for(i =0 ;i<sizeof(rstdata);i++){
                        FileTextE104[i] = rstdata[i];

                    }

                    numOfLogsInFram = 0;

                    ui16nlenhold = 0x3A+3;

                    ui16plenhold = 0x33+3;

                    WDTCTL = 0;
				} else if (FileTextE104[9] == 'A'){
				    digitalSensor = 0;
				    for(i =0 ;i<sizeof(rstdata);i++){
                    FileTextE104[i] = rstdata[i];

                }

                numOfLogsInFram = 0;

                ui16nlenhold = 0x3A+3;

                ui16plenhold = 0x33+3;

                WDTCTL = 0;
				}
			//}

#ifdef DEBUG
			sprintf(str, "\n\rbuff-%d off-%d len-%d self-%d\n\r",buffer_start,file_offset,file_length,SelectedFile);
			myuart_tx_string(str);
#endif
			Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
			Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD); // the interrupt has been serviced

			break;
		}

		} //end of switch

	}

	else if (flags & EXTRA_DATA_IN_FLAG){
#define AMOUNT_DATA_TO_SENT_EARLY		255							// data to add to the buffer while transmit is happening
#define MAX_TAG_BUFFER_SPACE			2998						// actually 3000 but to avoid any possibility of an issue

		unsigned int buffer_start;
		unsigned int file_offset;
		unsigned int file_length = 0;

		interrupt_serviced |= EXTRA_DATA_IN_FLAG;
		if(SelectedFile == 1){
			buffer_start = Read_Register(NDEF_BUFFER_START);
			file_offset = Read_Register(NDEF_FILE_OFFSET);

			if((buffer_start + AMOUNT_DATA_TO_SENT_EARLY) >= MAX_TAG_BUFFER_SPACE){
				// can't fill the buffer anymore
				// do no fill.  New data request, the remaining data will be shifted to the beggining of the buffer, interrupt will come later.
				__no_operation();

			}else{
				file_length = SendDataOnFile(SelectedFile,buffer_start,file_offset,AMOUNT_DATA_TO_SENT_EARLY);

			}

		}
		Write_Register(NDEF_FILE_LENGTH, file_length);  						// how much was actually written
		Write_Register(INT_FLAG_REG, interrupt_serviced);						// ACK the flags to clear
		Write_Register(HOST_RESPONSE, EXTRA_DATA_IN_SENT_FIELD);				// interrupt was serviced

	}

}

