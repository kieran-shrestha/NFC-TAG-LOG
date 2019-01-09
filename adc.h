/*
 * adc.h
 *
 *  Created on: Jan 9, 2019
 *      Author: kiran
 */

#ifndef ADC_H_
#define ADC_H_

void adcStartConv();
void adcStopConv();
void adcInit();
void adcDiffInit();
void offADC();
void setupADC13();
void setupADC45();
void setupBatMon();



#endif /* ADC_H_ */
