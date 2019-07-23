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

#define DIG13(x) if(x) {  P3OUT |= BIT3; } else {  P3OUT &= ~BIT3; }
#define DIG12(x) if(x) {  P4OUT |= BIT7; } else {  P4OUT &= ~BIT7; }
#define DIG11(x) if(x) {  P1OUT |= BIT3; } else {  P1OUT &= ~BIT3; }
#define DIG10(x) if(x) {  P1OUT |= BIT4; } else {  P1OUT &= ~BIT4; }
#define DIG9(x)  if(x) {  P1OUT |= BIT5; } else {  P1OUT &= ~BIT5; }
#define DIG8(x)  if(x) {  PJOUT |= BIT0; } else {  PJOUT &= ~BIT0; }
#define DIG7(x)  if(x) {  PJOUT |= BIT1; } else {  PJOUT &= ~BIT1; }

#define DIG6(x) if(x) {  PJOUT |= BIT2; } else {  PJOUT &= ~BIT2; }
#define DIG5(x) if(x) {  PJOUT |= BIT3; } else {  PJOUT &= ~BIT3; }
#define DIG4(x) if(x) {  P4OUT |= BIT0; } else {  P4OUT &= ~BIT0; }
#define DIG3(x) if(x) {  P4OUT |= BIT1; } else {  P4OUT &= ~BIT1; }
#define DIG2(x) if(x) {  P4OUT |= BIT2; } else {  P4OUT &= ~BIT2; }
#define DIG1(x) if(x) {  P4OUT |= BIT3; } else {  P4OUT &= ~BIT3; }
#define DIG0(x) if(x) {  P2OUT |= BIT5; } else {  P2OUT &= ~BIT5; }

#define CONTROLPINHIGH P1OUT |= BIT0
#define CONTROLPINLOW P1OUT &= ~BIT0

#define LED2HIGH P4OUT |= BIT5
#define LED2LOW P4OUT &= ~BIT5

#define ECDCONHIGH P1OUT |= BIT0
#define ECDCONLOW P1OUT &= ~BIT0

#define LED2TOG P4OUT ^= BIT5

#define LOGINTERVAL 1

char str[50];

extern float result,r;
extern int avghold[];

void gpioInit();
void writeNumber(unsigned int,int);
void writeNumber0(unsigned int,int);
void erase0();
void erase();

void gpioInit();

unsigned int intAddLog = 1;
#pragma PERSISTENT (logInterval)
const unsigned int logInterval = LOGINTERVAL;

int tempResult;
int displayCounter = 0;
int lastErase = 0;
int blinkCount = 0;
extern int doReset;

int isErase = 1;
int ecdNumber = 1;

extern unsigned int minuteAlert;

int waitCounter =5;

unsigned char numbers[] = {0x5F, 0x06, 0x3B, 0x2F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};

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

            ecdNumber = getpH(r);
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


void writeNumber(unsigned int num, int invert){
  unsigned char temp = numbers[num];
  if (invert == 1) {
      temp ^=temp;
  }
  ECDCONLOW;

  if(temp&0x01) {
      DIG13(1);
  } else
      DIG13(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG12(1);
  }else
      DIG12(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG11(1);
  }else
      DIG11(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG10(1);
  }else
      DIG10(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG9(1);
  }else
      DIG9(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG8(1);
  }else
      DIG8(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG7(1);
  }else
      DIG7(0);
  temp >>= 1;
}

void writeNumber0(unsigned int num, int invert){
  unsigned char temp = numbers[num];
  if (invert ==1 ) {
      temp ^=temp;
  }
  ECDCONLOW;

  if(temp&0x01) {
      DIG6(1);
  } else
      DIG6(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG5(1);
  }else
      DIG5(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG4(1);
  }else
      DIG4(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG3(1);
  }else
      DIG3(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG2(1);
  }else
      DIG2(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG1(1);
  }else
      DIG1(0);
  temp >>= 1;

  if(temp&0x01) {
      DIG0(1);
  }else
      DIG0(0);
  temp >>= 1;
}


void erase(){
    DIG13(0);
    DIG12(0);
    DIG11(0);
    DIG10(0);
    DIG9(0);
    DIG8(0);
    DIG7(0);
    ECDCONHIGH;
    lastErase = 1;
}

void erase0(){
    DIG6(0);
    DIG5(0);
    DIG4(0);
    DIG3(0);
    DIG2(0);
    DIG1(0);
    DIG0(0);
    ECDCONHIGH;
    lastErase = 1;
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
        displayCounter++;
        if(isErase == 1) {
//            intAddLog = 1;
             erase();
             erase0();
             if(displayCounter == waitCounter){
                 displayCounter = 0;
                 isErase = 0;
             }
        } else {
            if(ecdNumber/10 !=0)
               writeNumber0(ecdNumber/10,0);
            writeNumber(ecdNumber%10,0);
            if( displayCounter == waitCounter ){
                displayCounter = 0;
                isErase = 1;
                blinkCount++;
                if(blinkCount ==5){
                    blinkCount = 0;
                    stopTimer();
                }
              }
        }

      __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
      break;

    default: break;
}}
