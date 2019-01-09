#include "myTimers.h"
#include "msp430.h"

void timerInit(void)   {
    TA0CTL = TASSEL_1 + TAIE + ID_1;   // timer source from aclk =32.768lxft and divider is 2
    //one interrupt every 4 seconds

}

void startTimer(){
    TA0CTL |= MC_2;
}

void stopTimer(){
    TA0CTL &= ~MC_2;

}
