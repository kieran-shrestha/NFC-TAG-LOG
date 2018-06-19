/*
 * myadc.c
 *
 *  Created on: Apr 21, 2017
 *      Author: kiran
 */
#include "myadc.h"
#include "myuart.h"
#include <stdio.h>
#define MOVAVGSIZE 20
#define MOVARRSIZE SAMPLES-MOVAVGSIZE

int avghold[SAMPLES];
int movavghold[MOVARRSIZE];

void ADCstartConv(){
	ADC12CTL0 |= ADC12ENC | ADC12SC;
}

void ADCstopConv(){
	ADC12CTL0 &= ~(ADC12ENC | ADC12SC);
}

void setupADC(){
    P3SEL1 |= BIT1;                             // Configure ADC inputs A2
    P3SEL0 |= BIT1;

    ADC12MCTL0 &= 0xFFE0;
    ADC12MCTL0 |= ADC12INCH_13;                 // ADC A3 => ADCINCH_12
    ADC12MCTL0 &=  ~(ADC12DIF);                // NOT differential
}

void setupBatMon(){
    ADC12CTL3 |= ADC12BATMAP;

    ADC12MCTL0 &= 0xFFE0;
    ADC12MCTL0 |= ADC12INCH_31;
    ADC12MCTL0 &=  ~(ADC12DIF);   // NOT differential

}

void setupADC2(){
      P1SEL1 |= BIT4 | BIT5;                    // Configure ADC inputs A4 and A5
      P1SEL0 |= BIT4 | BIT5;

     ADC12MCTL0 &= 0xFFE0;
     ADC12MCTL0 |= ADC12INCH_4;
     ADC12MCTL0 |= ADC12DIF;     // VR+ = VREF buffered, VR- = AVSS ,differential
}

void myADCinit(){
    REFCTL0 = REFVSEL_2 | REFON;                //setting 2.5v as ref
    ADC12CTL0 = ADC12SHT0_2| ADC12ON;           // Sampling time, ADC12 on
    ADC12CTL1 = ADC12SHP | ADC12SSEL_2;         // Use sampling timer
    ADC12CTL2 |= ADC12RES_2;                    // 12-bit conversion results
    ADC12MCTL0 |=  ADC12VRSEL_1;                //VR+ = VREF buffered, VR- = AVSS , NOT differential
//  ADC12MCTL0 |=  ADC12VRSEL_1 | ADC12DIF;     // VR+ = VREF buffered, VR- = AVSS ,differential
//  ADC12IER0 |= ADC12IE0;                      // Enable ADC conv complete interrupt
    __delay_cycles(5000);
     while(REFCTL0 & REFGENBUSY);               // If ref generator busy, WAIT
//   while(!(REFCTL0 & REFGENRDY));             // Wait for reference generator  to settle
}

int takeSamples(){
	int x,y;
	long int sum=0;

	for( x = 0; x < MOVARRSIZE ;x++){
		for(y = x;y < x+MOVAVGSIZE;y++){
			sum+=avghold[y];
		}
		movavghold[x]=sum/(MOVAVGSIZE);
		sum = 0;
	}
	sum = 0;
	for( x =0 ; x < MOVARRSIZE ; x++){
		sum+= movavghold[x];

	}
	return sum/(MOVARRSIZE);

}

