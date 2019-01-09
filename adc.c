/*
 * adc.c
 *
 *  Created on: Jan 9, 2019
 *      Author: kiran
 */

#include "msp430.h"

void adcStartConv(){
    ADC12CTL0 |= ADC12ENC | ADC12SC;

}

void adcStopConv(){
    ADC12CTL0 &= ~(ADC12ENC | ADC12SC);

}

void adcInit(){
    REFCTL0 = REFVSEL_2 | REFON;                //setting 2.5v as ref
    ADC12CTL0 = ADC12SHT0_2| ADC12ON;           // Sampling time 16 ADC12ckl, ADC12 on
    ADC12CTL1 = ADC12SHP | ADC12SSEL_2;         // Use sampling timer and master clock
    ADC12CTL2 |= ADC12RES_2;                    // 12-bit conversion results(14 clk)
    ADC12MCTL0 |=  ADC12VRSEL_1;                //VR+ = VREF buffered, VR- = AVSS , NOT differential
//  ADC12MCTL0 |=  ADC12VRSEL_1 | ADC12DIF;     // VR+ = VREF buffered, VR- = AVSS ,differential
//   ADC12IER0 |= ADC12IE0;                      // Enable ADC conv complete interrupt
     while(REFCTL0 & REFGENBUSY);               // If ref generator busy, WAIT
}

void adcDiffInit(){
    REFCTL0 = REFVSEL_2 | REFON;                //setting 2.5v as ref
    ADC12CTL0 = ADC12SHT0_2| ADC12ON;           // Sampling time 16 ADC12ckl, ADC12 on
    ADC12CTL1 = ADC12SHP | ADC12SSEL_2;         // Use sampling timer and master clock
    ADC12CTL2 |= ADC12RES_2;                    // 12-bit conversion results(14 clk)
//    ADC12MCTL0 |=  ADC12VRSEL_1;                //VR+ = VREF buffered, VR- = AVSS , NOT differential
    ADC12MCTL0 |=  ADC12VRSEL_1 | ADC12DIF;     // VR+ = VREF buffered, VR- = AVSS ,differential
//   ADC12IER0 |= ADC12IE0;                      // Enable ADC conv complete interrupt
     while(REFCTL0 & REFGENBUSY);               // If ref generator busy, WAIT
}

void offADC(){
    ADC12CTL0 &= ~ADC12ON;           // Sampling time 16 ADC12ckl, ADC12 on
}

void setupADC13(){
    P3SEL1 |= BIT1;                             // Configure ADC inputs A13
    P3SEL0 |= BIT1;

    ADC12MCTL0 &= 0xFFE0;
    ADC12MCTL0 |= ADC12INCH_13;                 // AADCINCH_12
}

void setupADC45(){
    P1SEL1 |= BIT4 | BIT5;                    // Configure ADC inputs A4 and A5
    P1SEL0 |= BIT4 | BIT5;

    ADC12MCTL0 &= 0xFFE0;
    ADC12MCTL0 |= ADC12INCH_4;                 // AADCINCH_12

}

void setupBatMon(){
    ADC12CTL3 |= ADC12BATMAP;

    ADC12MCTL0 &= 0xFFE0;
    ADC12MCTL0 |= ADC12INCH_31;
}
