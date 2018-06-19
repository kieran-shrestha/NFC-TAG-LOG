// ----------------------------------------------------------------------------
// myTimers.c  (for lab_06b_upTimer project) ('FR5969 Launchpad)
// ----------------------------------------------------------------------------

//***** Header Files **********************************************************
#include <driverlib.h>
#include "myTimers.h"


void initTimers(void){
    Timer_A_initUpModeParam initUpParam = 		{ 0 };
        initUpParam.clockSource = 				TIMER_A_CLOCKSOURCE_ACLK;                       // Use ACLK (slower clock)
        initUpParam.clockSourceDivider = 		TIMER_A_CLOCKSOURCE_DIVIDER_4;           		// Input clock = ACLK / 1 = 32KHz
//        initUpParam.timerPeriod = 				0xFFFF / 2;                                  	// Period (0xFFFF/2):  8000 / 32Khz = 1 second
//        initUpParam.timerPeriod = 				0x9248;                                     	// Period (0xFFFF/2): => 32Khz = 1 second (correction so calculated value 0x9248 cycles ~ 1sec
        initUpParam.timerPeriod = 				0x9248;                                     	//ideally, 32kHz = 1 second => 32768 cycles = 1 second and 32768/2=16384=0x4000 = 500ms)
     //   initUpParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;    				// Enable TAR -> 0 interrupt
        initUpParam.captureCompareInterruptEnable_CCR0_CCIE =
                								TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE;             // Enable CCR0 compare interrupt
        initUpParam.timerClear = 				TIMER_A_DO_CLEAR;                               // Clear TAR & clock divider
        initUpParam.startTimer = 				false;                                          // Don't start the timer, yet
    Timer_A_initUpMode( TIMER_A1_BASE, &initUpParam );                            				// Set up Timer A1

    Timer_A_clearTimerInterrupt( TIMER_A1_BASE );                                 // Clear TA1IFG
    Timer_A_clearCaptureCompareInterrupt( TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0                                         // Clear CCR0IFG
    );
}

void startTimer(){
    Timer_A_startCounter(
        TIMER_A1_BASE,
        TIMER_A_UP_MODE
    );
}
