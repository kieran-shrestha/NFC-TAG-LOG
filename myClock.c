#include <driverlib.h>
#include "myClock.h"

#define LF_CRYSTAL_FREQUENCY_IN_HZ     32768
#define HF_CRYSTAL_FREQUENCY_IN_HZ     0


void clockInit(void) {
    CS_setExternalClockSource(
            LF_CRYSTAL_FREQUENCY_IN_HZ,
            HF_CRYSTAL_FREQUENCY_IN_HZ
    );

     // Set FRAM Controller waitstates to 1 when MCLK > 8MHz (per datasheet)
	// Please refer to the "Non-Volatile Memory" chapter for more details
	//FRAMCtl_configureWaitStateControl( FRAMCTL_ACCESS_TIME_CYCLES_1 );

    // Set DCO to run at 4MHz
    CS_setDCOFreq(
            CS_DCORSEL_0,                                                       // Set Frequency range (DCOR)
            CS_DCOFSEL_3                                                        // Set Frequency (DCOF)
    );

    // Set ACLK to use VLO as its oscillator source (~32KHz)
    CS_initClockSignal(
            CS_ACLK,                                                            // Clock you're configuring
			CS_LFXTCLK_SELECT,                                                   // Clock source
            CS_CLOCK_DIVIDER_1                                                  // Divide down clock source by this much
    );

    // Set SMCLK to use DCO as its oscillator source (DCO was configured earlier in this function for 8MHz)
    CS_initClockSignal(
            CS_SMCLK,                                                           // Clock you're configuring
            CS_DCOCLK_SELECT,                                                   // Clock source
            CS_CLOCK_DIVIDER_1                                                  // Divide down clock source by this much
    );

    // Set MCLK to use DCO as its oscillator source (DCO was configured earlier in this function for 8MHz)
    CS_initClockSignal(
            CS_MCLK,                                                            // Clock you're configuring
            CS_DCOCLK_SELECT,                                                   // Clock source
            CS_CLOCK_DIVIDER_1                                                  // Divide down clock source by this much
    );
}


