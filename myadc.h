/*
 * myadc.h
 *
 *  Created on: Apr 21, 2017
 *      Author: kiran
 */

#include <msp430.h>

#ifndef MYADC_H_
#define MYADC_H_

#define AVGDATA 20
#define SAMPLES 200


void myADCinit();
int takeSamples();

void  ADCstartConv();
void  ADCstopConv();





#endif /* MYADC_H_ */
