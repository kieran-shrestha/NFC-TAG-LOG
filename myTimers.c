#include "myTimers.h"
#include "msp430.h"

void timerInit(void)   {
    TA0CTL = TASSEL_2 + TAIE + ID_3;   // timer source from sub and divider is 1
       //one interrupt every 4 seconds
//    TA0CTL = TASSEL_2  + ID_3;   // timer source from aux and divider is 1
//
//    TA0CCTL0 |= CCIE;
//    TA0CCR0 = 0xFFFF;
//    TA0EX0 = TAIDEX_2;
    TA0EX0 = TAIDEX_1;

}

void startTimer(){
    TA0CTL |= MC_2;
}

void stopTimer(){
    TA0CTL &= ~MC_2;

}
