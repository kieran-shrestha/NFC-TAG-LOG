/*
 * rtc.c
 *
 *  Created on: Oct 31, 2016
 *      Author: kiran
 */

#include "rtc.h"
#include "datalog.h"
#include "time.h"

unsigned int minCounter = 0;

extern unsigned int intAddLog;
extern const unsigned int logInterval;

#pragma PERSISTENT (FIRSTBOOT)
unsigned char FIRSTBOOT = 1;

#pragma PERSISTENT (HOURS)
extern unsigned char HOURS = 1;

#pragma PERSISTENT (MINUTES)
extern unsigned char MINUTES = 1;

#pragma PERSISTENT (SECONDS)
extern unsigned char SECONDS = 1;

#pragma PERSISTENT (MONTHS)
extern unsigned char MONTHS = 1;

#pragma PERSISTENT (DAYS)
extern unsigned char DAYS = 1;

#pragma PERSISTENT (YEARS)
extern unsigned int YEARS = 1;


inline uint8_t decToBcd(uint8_t val){
  return ( (val/10*16) + (val%10) );
}

void rtcInit(){
	time_t mytime =time(NULL);
	struct tm tm = *localtime(&mytime);

	RTCCTL01 |= RTCHOLD | RTCTEV_0 | RTCBCD | RTCCTL0 | RTCTEVIE;//| RTCAIE;

	if(FIRSTBOOT){
		FIRSTBOOT = 0;
		RTCHOUR = decToBcd(tm.tm_hour);
		RTCMIN = decToBcd(tm.tm_min);
		RTCSEC = 0x00;
		YEARS = tm.tm_year-100;
		RTCYEAR = 0x2000 | decToBcd((unsigned char)YEARS);
		RTCMON = decToBcd(tm.tm_mon) +1;
		RTCDAY = decToBcd(tm.tm_mday);

		HOURS = RTCHOUR;
		MINUTES = RTCMIN;
		SECONDS = RTCSEC;
		MONTHS = RTCMON;			//since month set to [0,11] in inbuilt time.c
		DAYS = RTCDAY;
		YEARS = RTCYEAR;
	}else{
		RTCHOUR = HOURS;
		RTCMIN = MINUTES;
		RTCSEC = SECONDS;
		RTCYEAR = YEARS;
		RTCMON = MONTHS;
		RTCDAY = DAYS;
	}

	RTCAMIN = 0x00;
	RTCAHOUR = 0x00;
	RTCADAY = 0x00;
	RTCADOW = 0x00;

	RTCCTL01 &= ~RTCHOLD;

}

rtcType getTimeStamp(){
	rtcType rtcData;
	uint8_t x;

	x = RTCHOUR;
	//myuart_tx_byte(x);
	rtcData.hour[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.hour[1] = (x & 0x0F ) + 48;

	x = RTCMIN;
	rtcData.minute[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.minute[1] = (x & 0x0F ) + 48;

	x = (char)RTCYEAR;
	rtcData.year[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.year[1] = (x & 0x0F ) + 48;

	x = RTCMON;
	rtcData.month[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.month[1] = (x & 0x0F ) + 48;

	x = RTCDAY;
	rtcData.day[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.day[1] = (x & 0x0F ) + 48;

	return rtcData;
}


#pragma vector=RTC_VECTOR
__interrupt void RTCISR(void){
  switch (__even_in_range(RTCIV, RTCIV_RTCOFIFG)){
    case RTCIV_NONE: break;
    case RTCIV_RTCRDYIFG: break;
    case RTCIV_RTCTEVIFG:		// Should fire and be here once ever minute

    	minCounter++;
		//UPDATING TIME
		HOURS = RTCHOUR;
		MINUTES = RTCMIN;
		SECONDS = RTCSEC;
		MONTHS = RTCMON;
		DAYS = RTCDAY;

    	if(minCounter == logInterval){
    		minCounter = 0;
    		intAddLog = 1;
    	}
		__bic_SR_register_on_exit(LPM4_bits + GIE); //wake up to handle INTO
		break;

    case RTCIV_RTCAIFG:		// Alarm Flag
    	break;
    case RTCIV_RT0PSIFG: break;
    case RTCIV_RT1PSIFG: break;
    case RTCIV_RTCOFIFG: break;
  }
}

