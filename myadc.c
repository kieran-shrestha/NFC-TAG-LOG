/*
 * myadc.c
 *
 *  Created on: Apr 21, 2017
 *      Author: kiran
 */
#include "myadc.h"
#include "myuart.h"
#include <stdio.h>
#define SAMPLES 200
#define MOVAVGSIZE 20
#define MOVARRSIZE SAMPLES-MOVAVGSIZE

char str[30];

float r , result;
int avghold[SAMPLES];
int movavghold[MOVARRSIZE];

void ADCstartConv(){
	ADC12CTL0 |= ADC12ENC | ADC12SC;
}

void ADCstopConv(){
	ADC12CTL0 &= ~(ADC12ENC | ADC12SC);
}


void myADCinit(){
//	P1SEL1 |= BIT4 | BIT5;                    // Configure ADC inputs A2 and A3
//	P1SEL0 |= BIT4 | BIT5;

	P3SEL1 |= BIT0 | BIT1;                    // Configure ADC inputs A2 and A3
	P3SEL0 |= BIT0 | BIT1;

	REFCTL0 = REFVSEL_1 | REFOUT;			//setting 2.0v as ref
	REFCTL0 |= REFON;
	ADC12CTL0 = ADC12SHT0_3| ADC12ON;     // Sampling time, ADC12 on
	ADC12CTL1 = ADC12SHP | ADC12SSEL_2 | ADC12DIV_7;     // Use sampling timer
	ADC12CTL2 |= ADC12RES_2;                // 12-bit conversion results
	ADC12MCTL0 |=  ADC12VRSEL_1 | ADC12INCH_12| ADC12DIF;   // Channel2 ADC input select; Vref=AVCC
//	ADC12IER0 |= ADC12IE0;                  // Enable ADC conv complete interrupt
	//__bis_SR_register( GIE);     // LPM0, ADC12_ISR will force exit
	__delay_cycles(5000);

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

