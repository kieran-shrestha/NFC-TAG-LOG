#include <msp430.h> 
#include "myClock.h"
#include "myuart.h"
#include "driverlib.h"
#include "rf430nfc.h"
#include "NFCType4.h"
#include "myTimers.h"
#include "myadc.h"
#include "rtc.h"
#include "ph.h"
#include "datalog.h"
#include <stdio.h>

#define LOGINTERVAL 1

#define LED2HIGH P4OUT |= BIT5
#define LED2LOW P4OUT &= ~BIT5
#define LED2TOG P4OUT ^= BIT5

//char str[25];

extern float result,r;
extern int avghold[];

void gpioInit();

unsigned int intAddLog = 1;

#pragma PERSISTENT (logInterval)
const unsigned int logInterval = LOGINTERVAL;

int tempResult;

extern int doReset;

extern unsigned int minuteAlert;

unsigned char nfcFired = 0;
unsigned int flags = 0;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    clockInit();
    gpioInit();
    LED2HIGH;
    timerInit();
//    myuart_init();
    rf430Init();
    myADCinit();
    datalogInit();
    rtcInit();
//    myuart_tx_string("PROGRAM STARTED...\r");
    LED2LOW;
    startTimer();

    while(1){
        if (doReset ==1 ){
            resetLog();
            doReset = 0;
            WDTCTL = 0;
        }

        __bis_SR_register(LPM3_bits+GIE);
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
             P2IFG &= ~BIT2; //clear the interrupt again
             P2IE |= BIT2;   //enable the interrupt
             startTimer();
          }


        if(intAddLog == 1){
            int l,m,n,sign;
            intAddLog = 0;
            ADCstartConv();

            while(!(ADC12IFGR0 & BIT0));
            result = ADC12MEM0;
            r = (result - 2048 - 0.5) *2000.0 /2048;
//            sprintf(str,"MilliVolts , %.2f\n\r",r);
//            myuart_tx_string(str);
            ADCstopConv();

            if (r < 0){
                sign = 1;
                r=r*-1;
            }

            tempResult = (int)r%1000;
            l = tempResult/100;  //hundreds
            tempResult = tempResult%100;
            m = tempResult/10;   //tenths
            tempResult = tempResult%10;
            n = tempResult;   //ones
            data_buffer(sign, l,m,n);
            sign = 0;

        }
    }
}

void gpioInit(){
    PM5CTL0 &= ~LOCKLPM5;
    P4DIR |= BIT5;	//SET AS OUTPUT
    P1DIR |= 0b00000111;
    //#ifdef DEBUG
    P2DIR |= 0b10111000;
    //#else
    //	P2DIR |= 0b10111011;
    //#endif
    P3DIR |= 0b11001100;
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

    P1SEL1 |= BIT1;
    P1SEL0 |= BIT1;

}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    //INTO interrupt fired
    if (P2IFG & BIT2) {
        P2IE &= ~BIT2; //disable INTO
        P2IFG &= ~BIT2; //clear interrupt flag
        nfcFired = 1;
        __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
    }
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
  switch(__even_in_range(TA0IV, TA0IV_TAIFG)) {
    case TA0IV_NONE:   break;               // No interrupt
    case TA0IV_TACCR1:                      //wake up to handle INTO
        break;               // CCR1 not used
    case TA0IV_TACCR2: break;               // CCR2 not used
    case TA0IV_3:      break;               // reserved
    case TA0IV_4:      break;               // reserved
    case TA0IV_5:      break;               // reserved
    case TA0IV_6:      break;               // reserved
    case TA0IV_TAIFG:                       // overflow

      __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
      break;

    default: break;
}}
