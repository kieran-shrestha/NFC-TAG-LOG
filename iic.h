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

#endif /* IIC_H_ */
