/*
 * thermistor.c
 *
 *  Created on: Jan 9, 2019
 *      Author: kiran
 */
#include "msp430.h"
#include "adc.h"

#define THERMISTORON P3OUT |= BIT2
#define THERMISTOROFF P3OUT &= ~BIT2
#define REFRES 1000000

int thermistorTemp (float slope,int intercept){
    float thermistorVoltage;
    float rThermistor;
    float batVoltage;
    float tCelsius;

    unsigned int result;
    adcInit();
    THERMISTORON;
    setupADC13();
    adcStartConv();
    while(!(ADC12IFGR0 & BIT0));
    result = ADC12MEM0;
    thermistorVoltage = result*2500.0/4096 - 1250/4096.0;
    adcStopConv();
    THERMISTOROFF;

    setupBatMon();
    adcStartConv();
    while(!(ADC12IFGR0 & BIT0));
    result = ADC12MEM0;
    batVoltage = result*2500.0/4096 - 1250/4096.0;
    adcStopConv();
    offADC();

    rThermistor = thermistorVoltage*REFRES/(batVoltage*2.0 - thermistorVoltage);

    tCelsius = slope*rThermistor + intercept;

    return (int)(tCelsius*100);            //24.52312 degree => 2452
}
