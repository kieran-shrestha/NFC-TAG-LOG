/*
 * NFCType4.h
 *
 *  Created on: Jan 8, 2019
 *      Author: kiran
 */

#ifndef NFCTYPE4_H_
#define NFCTYPE4_H_
#include <stdint.h>

#define DEFNLEN 0x3A
#define DEFPLEN 0x33

enum FileExistType{
    FileFound       = 1,
    FileNotFound    = 0
};

typedef struct NdefFile_Type{
    unsigned char FileID[2];
    unsigned char * FilePointer;
    unsigned int FileLength;
}NdefFileType;

void AppInit();
void rf430Interrupt(unsigned int flags);
enum FileExistType SearchForFile(uint8_t *fileId);

uint16_t SendDataOnFile(uint16_t selectedFile, uint16_t buffer_start, uint16_t file_offset, uint16_t length);
void ReadDataOnFile(uint16_t selectedFile, uint16_t buffer_start, uint16_t file_offset, uint16_t length);


#endif /* NFCTYPE4_H_ */
