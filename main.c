/* This if final code for mass production
 * with TMP sensor
 */

#include <msp430.h>
#include "myClock.h"
#include "myTimers.h"
#include "rtc.h"
#include "TMP112.h"
#include "rf430nfc.h"
#include "NFCType4.h"
#include "thermistor.h"

#define LOGINTERVAL 1

#define LED1HIGH P4OUT |= BIT6
#define LED1LOW P4OUT &= ~BIT6
#define LED2HIGH P4OUT |= BIT5
#define LED2LOW P4OUT &= ~BIT5

#define LED1TOG P4OUT ^= BIT6
#define LED2TOG P4OUT ^= BIT5

void gpioInit();

unsigned int intAddLog = 0;

#pragma PERSISTENT (logInterval)
unsigned int logInterval = LOGINTERVAL;

#pragma PERSISTENT (mSlope)
float mSlope = 0.12345;

#pragma PERSISTENT (mIntercept)
float mIntercept = -123.12;

#pragma PERSISTENT (digitalSensor)
unsigned char digitalSensor = 1;

unsigned char nfcFired = 0;
unsigned int flags = 0;

int main(void) {
    int digitalTemperature;       //to hold the temperature
    unsigned char tempNegFlag = 0;
    gpioInit();
    LED2HIGH;
    LED1HIGH;
    clockInit();
    timerInit();
    rtcInit();
    tmpInit();
    datalogInit();
    LED1LOW;
    rf430Init();
    LED2LOW;
    startTimer();

    while(1){
        __bis_SR_register(LPM4_bits+GIE);
        __no_operation();

        if( nfcFired){
            stopTimer();
            flags = NFCRead_Register(INT_FLAG_REG);
            LED2HIGH;
            do {
                if (flags) {
                    rf430Interrupt(flags);
                }
                flags = NFCRead_Register(INT_FLAG_REG);
            } while (flags);

            LED2LOW;
            flags = 0;
            nfcFired = 0;
            P2IFG &= ~BIT2;	//clear the interrupt again
            P2IE |= BIT2;	//enable the interrupt
            startTimer();
        }

        if(intAddLog == 1){
            LED1HIGH;
            intAddLog = 0;
            if(digitalSensor == 1){
                TMP_Get_Temp(&digitalTemperature, &tempNegFlag);
                if (tempNegFlag)
                    digitalTemperature = -1*digitalTemperature;
               data_buffer(digitalTemperature);
            } else {
               data_buffer(thermistorTemp(mSlope,mIntercept));
            }
           LED1LOW;
        }
    }
}

void gpioInit(){
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= 0b11000000;
   	P2DIR |= 0b11111011;
    P3DIR |= 0b11001111;
    P4DIR |= 0xFF;
    PJDIR |= 0b11001111;

    P1OUT = 0x00;
    P2OUT = 0x00;
    P3OUT = 0x00;
    P4OUT = 0x00;
    PJOUT = 0X00;

    P2OUT |= BIT6;

    P1SEL1 |= BIT6;		//setting p1.6 as sda
    P1SEL1 |= BIT7;		//setting p1.7 as scl
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    if (P2IFG & BIT2) {
        P2IE &= ~BIT2; //disable INTO
        P2IFG &= ~BIT2; //clear interrupt flag
        nfcFired = 1;
        __bic_SR_register_on_exit(LPM4_bits + GIE); //wake up to handle INTO
    }
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
  switch(__even_in_range(TA0IV, TA0IV_TAIFG)) {
    case TA0IV_NONE:   break;               // No interrupt
    case TA0IV_TACCR1: break;               // CCR1 not used
    case TA0IV_TACCR2: break;               // CCR2 not used
    case TA0IV_3:      break;               // reserved
    case TA0IV_4:      break;               // reserved
    case TA0IV_5:      break;               // reserved
    case TA0IV_6:      break;               // reserved
    case TA0IV_TAIFG:                       // overflow
        LED2HIGH;
        __delay_cycles(500);
        intAddLog = 1;
        LED2LOW;
      __bic_SR_register_on_exit(LPM4_bits + GIE); //wake up to handle INTO
      break;
    default: break;
  }
}
