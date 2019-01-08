#include "myClock.h"
#include "msp430.h"

//dco = 1MHz
//master = 1MHz div = 1
//submaster = 1 MHz div / 8
//alck = 32.768(LFXT) div /1

void clockInit(void) {
    CSCTL0_H = 0xA5;
    CSCTL1 = DCOFSEL_0;
    //all the above setting is default except master should be divided by 1
    CSCTL3 = DIVS_3 + DIVM_0;
    CSCTL4 = LFXTDRIVE_0;
    CSCTL4 &= ~(LFXTOFF);

  }


