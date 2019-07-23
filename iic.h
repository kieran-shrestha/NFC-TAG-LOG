/*
 * iic.h
 *
 *  Created on: Jan 7, 2019
 *      Author: kiran
 */

#ifndef IIC_H_
#define IIC_H_



void iicInit(unsigned char address);
void iicWriteRegister16bit(unsigned int ui16data, unsigned char address);
unsigned int iicReadRegister16bit(unsigned char address);

unsigned int iicReadRegister16bitW16bitAddress(unsigned int address);
void iicWriteRegister16bitW16bitAddress(unsigned int ui16data, unsigned int address);

void iicWriteContinuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length);
void iicReadContinuous(unsigned int reg_addr, unsigned char* read_data,unsigned int data_length);

#endif /* IIC_H_ */
