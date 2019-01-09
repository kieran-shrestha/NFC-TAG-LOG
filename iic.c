/*
 * iic.c
 *
 *  Created on: Jan 7, 2019
 *      Author: kiran
 */
#include "msp430.h"

void iicInit(unsigned char address){
    UCB0CTL1 |= UCSWRST;                            //Software reset enabled
    UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR; //I2C mode, Master mode, sync, transmitter
    UCB0CTLW0 |= UCSSEL_2;                          // SMCLK = 0.125MHz

    UCB0BRW = 2;

    UCB0I2CSA  = address;                   // Set Slave Address
    UCB0IE = 0;
    UCB0CTL1 &= ~UCSWRST;
}

void iicWriteRegister16bit(unsigned int ui16data, unsigned char address){
    UCB0CTL1  |= UCSWRST;
    UCB0CTLW1 = UCASTP_2;  // generate STOP condition.
    UCB0TBCNT = 0x0001;
    UCB0CTL1  &= ~UCSWRST;

    UCB0CTL1 |= UCTXSTT + UCTR;     // Start i2c write operation
    while(!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = address & 0xFF;
    while(!(UCB0IFG & UCBCNTIFG));

    UCB0CTLW1 = UCASTP_2;           // generate STOP condition.
    UCB0TBCNT = 0x0002;
    UCB0CTL1 |= UCTXSTT;            // Repeated start

    UCB0TXBUF = ui16data & 0x00FF;
    while(!(UCB0IFG & UCBCNTIFG));

    UCB0TXBUF = ui16data>>8 & 0x00FF;
    while(!(UCB0IFG & UCBCNTIFG));

    UCB0CTL1  |= UCSWRST;
}

unsigned int iicReadRegister16bit(unsigned char address){   //reads corresponding 8 bit
    unsigned char ui8RxData[2];
    unsigned int pTempData;

    UCB0CTL1  |= UCSWRST;
    UCB0CTLW1 = UCASTP_2;  // generate STOP condition.
    UCB0TBCNT = 0x0001;
    UCB0CTL1  &= ~UCSWRST;

    UCB0CTL1 |= UCTXSTT + UCTR;     // Start i2c write operation
    while(!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = address & 0xFF;
    while(!(UCB0IFG & UCBCNTIFG));

    UCB0CTL1 &= ~UCTR;
    UCB0CTLW1 = UCASTP_2;           // generate STOP condition.
    UCB0TBCNT = 0x0002;
    UCB0CTL1 |= UCTXSTT;            // Repeated start

    while(!(UCB0IFG & UCRXIFG0));
    ui8RxData[1] = UCB0RXBUF;
    while(!(UCB0IFG & UCRXIFG0));
    ui8RxData[0] = UCB0RXBUF;
    while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
    UCB0CTL1  |= UCSWRST;

    pTempData = (0x0FF0 & (ui8RxData[1] << 4)) | (0x0F & (ui8RxData[0] >> 4 ));
    pTempData = 0x07FF & pTempData;
    return pTempData;

}

unsigned int iicReadRegister16bitW16bitAddress(unsigned int address){
    unsigned char RxData[2] = { 0, 0 };
    unsigned char TxAddr[2] = { 0, 0 };

    TxAddr[0] = address >> 8;      //MSB of address
    TxAddr[1] = address & 0xFF;    //LSB of address

    UCB0CTL1 |= UCSWRST;
    UCB0CTLW1 = UCASTP_0;  // generate STOP condition.
    UCB0CTL1 &= ~UCSWRST;

    UCB0CTL1 |= UCTXSTT + UCTR;     // Start i2c write operation
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = TxAddr[0] & 0xFF;
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = TxAddr[1] & 0xFF;
    while (!(UCB0IFG & UCTXIFG0));

    UCB0CTL1 &= ~UCTR;              //i2c read operation
    UCB0CTL1 |= UCTXSTT;            //repeated start

    while (!(UCB0IFG & UCRXIFG0));
    RxData[0] = UCB0RXBUF;
    UCB0CTL1 |= UCTXSTP;            //send stop after next RX

    while (!(UCB0IFG & UCRXIFG0));
    RxData[1] = UCB0RXBUF;
    while (!(UCB0IFG & UCSTPIFG));    // Ensure stop condition got sent
    UCB0CTL1 |= UCSWRST;

    return RxData[1] << 8 | RxData[0];

}

void iicWriteRegister16bitW16bitAddress(unsigned int ui16data, unsigned int address){
    unsigned char TxData[2] = { 0, 0 };
    unsigned char TxAddr[2] = { 0, 0 };

    TxAddr[0] = address >> 8;      //MSB of address
    TxAddr[1] = address & 0xFF;    //LSB of address

    TxData[0] = ui16data >> 8;
    TxData[1] = ui16data & 0xFF;

    UCB0CTL1 &= ~UCSWRST;
    UCB0CTL1 |= UCTXSTT + UCTR;     //start i2c write operation

    //write the address
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxAddr[0];
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxAddr[1];

    //write the data
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxData[1];
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxData[0];
    while (!(UCB0IFG & UCTXIFG0));

    UCB0CTL1 |= UCTXSTP;
    while ((UCB0STAT & UCBBUSY));     // Ensure stop condition got sent
    UCB0CTL1 |= UCSWRST;
}

void iicReadContinuous(unsigned int reg_addr, unsigned char* read_data,unsigned int data_length) {
    unsigned int i;
    unsigned char TxAddr[2] = { 0, 0 };

    TxAddr[0] = reg_addr >> 8;      //MSB of address
    TxAddr[1] = reg_addr & 0xFF;    //LSB of address

    UCB0CTL1 &= ~UCSWRST;
    UCB0CTL1 |= UCTXSTT + UCTR; //start i2c write operation.  Sending Slave address

    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxAddr[0];
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxAddr[1];
    while (!(UCB0IFG & UCTXIFG0));      // Waiting for TX to finish on bus
    UCB0CTL1 &= ~UCTR;              //i2c read operation
    UCB0CTL1 |= UCTXSTT;            //repeated start
    while (!(UCB0IFG & UCRXIFG0));

    for (i = 0; i < data_length - 1; i++) {
       while (!(UCB0IFG & UCRXIFG0));
       read_data[i] = UCB0RXBUF;
    }

    UCB0CTL1 |= UCTXSTP;            //send stop after next RX
    while (!(UCB0IFG & UCRXIFG0));
    read_data[i] = UCB0RXBUF;
    while ((UCB0STAT & UCBBUSY));    // Ensure stop condition got sent
    UCB0CTL1 |= UCSWRST;

}

void iicWriteContinuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length) {
    unsigned int i;
    unsigned char TxAddr[2] = { 0, 0 };

    TxAddr[0] = reg_addr >> 8;      //MSB of address
    TxAddr[1] = reg_addr & 0xFF;    //LSB of address

    UCB0CTL1 &= ~UCSWRST;
    UCB0CTL1 |= UCTXSTT + UCTR;     //start i2c write operation
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

