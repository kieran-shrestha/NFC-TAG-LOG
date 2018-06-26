/* This code is for dual ph and temp paper
 */

#include <msp430.h> 
#include "myClock.h"
//#include "myuart.h"
#include "rf430nfc.h"
#include "driverlib.h"
#include "rf430Process.h"
#include "myTimers.h"
#include "myadc.h"
#include <stdio.h>
#include "datalog.h"
#include "rtc.h"
#include "ph.h"

#define BALANCE_RESISTOR  100000

#define THERMISTOR_POWER_PORT GPIO_PORT_P3
#define THERMISTOR_POWER_PIN GPIO_PIN2

#define LED1_PORT GPIO_PORT_P4
#define LED1_PIN GPIO_PIN6
#define LED2_PORT GPIO_PORT_P4
#define LED2_PIN GPIO_PIN5

#define slope -0.00912
#define intercept 388.19

//char str[60];

extern int avghold[];

extern int offset;
extern float mSlope;

int adc_addlog = 1;
int i;

void gpioInit();

unsigned char nfcFired = 0;
unsigned int flags = 0;

int main(void) {
    unsigned int intTemp,intpH;        //to hold Thermistor temperature
    float tCelsius;

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    gpioInit();
    GPIO_setOutputHighOnPin( LED1_PORT, LED1_PIN);
    clockInit();
  //  myuart_init();
    initTimers();
    RTC_init();
    datalog_Init();
    RF430_Init();
    myADCinit();

   // myuart_tx_string("PROGRAM STARTED...\r");

    startTimer();
    GPIO_setOutputLowOnPin( LED1_PORT, LED1_PIN);

    while(1){
        __bis_SR_register(LPM3_bits+GIE);
        __no_operation();

        if( nfcFired){
            flags = Read_Register(INT_FLAG_REG);
            GPIO_setOutputHighOnPin(  LED1_PORT, LED1_PIN);
            do {
                if (flags) {
                    rf430Interrupt(flags);
                }
                flags = Read_Register(INT_FLAG_REG);
            } while (flags);

            GPIO_setOutputLowOnPin(  LED1_PORT, LED1_PIN);

            flags = 0;
            nfcFired = 0;
            P2IFG &= ~BIT2;	//clear the interrupt again
            P2IE |= BIT2;	//enable the interrupt
        }

        if(adc_addlog == 1){
            int l;
            float thermistor_voltage;
            float ph_voltage;
            float battery_voltage;
            float rThermistor;
            adc_addlog = 0;

            GPIO_setOutputHighOnPin( THERMISTOR_POWER_PORT, THERMISTOR_POWER_PIN);          //turn on thermistor voltage source GIOP pin to power up the thermistor voltage divider circuit
            setupADC(); //adc for temp sensor
            for(l = 0;l < SAMPLES ; l++){   //takes SAMPLES amount of data
                GPIO_setOutputHighOnPin(  LED2_PORT, LED2_PIN);
                ADCstartConv();
                while(!(ADC12IFGR0 & BIT0));
                thermistor_voltage = ADC12MEM0;
                avghold[l] = (int)thermistor_voltage;
                ADCstopConv();
                GPIO_setOutputLowOnPin( LED2_PORT, LED2_PIN);           //turn off led (1.0) to indicate ADC conversion completed
            }
            GPIO_setOutputLowOnPin( THERMISTOR_POWER_PORT, THERMISTOR_POWER_PIN);           //turn off thermistor voltage source GIOP pin to power down the thermistor voltage divider circuit

            thermistor_voltage = takeSamples();
            thermistor_voltage = thermistor_voltage*2500.0/4096 - 1250/4096.0;

            setupBatMon();  //checks internal battery volt
            ADCstartConv();
            while(!(ADC12IFGR0 & BIT0));
            battery_voltage = ADC12MEM0;
            battery_voltage = battery_voltage*2500.0/4096 - 1250/4096.0;
            ADCstopConv();

            rThermistor = thermistor_voltage*BALANCE_RESISTOR/(battery_voltage*2 - thermistor_voltage);
            tCelsius = mSlope*rThermistor + intercept + offset;

            intTemp = tCelsius*100;
            if(intTemp > 9999)
                intTemp = 9999;

            setupADC2(); //differential ADC for pH
            for(l = 0;l < SAMPLES ; l++){   //takes SAMPLES amount of data
                GPIO_setOutputHighOnPin(  LED2_PORT, LED2_PIN);
                ADCstartConv();
                while(!(ADC12IFGR0 & BIT0));
                ph_voltage = ADC12MEM0;
                avghold[l] = (int)ph_voltage;
                ADCstopConv();
                GPIO_setOutputLowOnPin( LED2_PORT, LED2_PIN);           //turn off led (1.0) to indicate ADC conversion completed
            }

            ph_voltage = takeSamples();
            ph_voltage = (ph_voltage-2048-0.5)*2500.0/2048.0;

            intpH = getpH((int)ph_voltage);

          //  sprintf(str, "batV: %.2f thV: %0.2f pHV: %0.2f\n",2*battery_voltage,thermistor_voltage,ph_voltage);
          //  myuart_tx_string(str);

            data_buffer(intTemp,intpH);
        }
    }
}

void gpioInit(){
    PM5CTL0 &= ~LOCKLPM5;
    P4DIR |= BIT5;	//SET AS OUTPUT
    P4OUT |= BIT5;	//set output as 1
    __delay_cycles(50000);
    P4OUT &= ~BIT5;	//SET to 0 again

    P1DIR |= 0b00000111;
    P2DIR |= 0b10111000;

    P3DIR |= 0b11001100;
    P4DIR |= 0xFF;
    PJDIR |= 0b11001111;

    P1OUT = 0x00;
    P2OUT = 0x00;
    P3OUT = 0x00;
    P4OUT = 0x00;
    PJOUT = 0X00;


    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6);		//for powering the IIC
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6);

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6);         // d1 led
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN5);         // d2 led
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN5);

    GPIO_setAsOutputPin( THERMISTOR_POWER_PORT, THERMISTOR_POWER_PIN);
    GPIO_setOutputHighOnPin( THERMISTOR_POWER_PORT, THERMISTOR_POWER_PIN);

    P1SEL1 |= BIT6;		//setting p1.6 as sda
    P1SEL1 |= BIT7;		//setting p1.7 as scl
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    if (P2IFG & BIT2) {
        P2IE &= ~BIT2; //disable INTO
        P2IFG &= ~BIT2; //clear interrupt flag
        nfcFired = 1;
        __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void ccr0_ISR (void){
        GPIO_setOutputHighOnPin( LED1_PORT, LED1_PIN);
        __delay_cycles(4000);
        GPIO_setOutputLowOnPin( LED1_PORT, LED1_PIN);
    __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
}
