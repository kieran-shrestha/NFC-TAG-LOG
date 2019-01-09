/*
 * rtc.h
 *
 *  Created on: Oct 31, 2016
 *      Author: kiran
 */

#ifndef RTC_H_
#define RTC_H_

#include "msp430.h"
#include "stdint.h"
#include "datalog.h"


typedef struct rtc_Type
{
	unsigned char hour[2];
	unsigned char minute[2];
	unsigned char year[2];
	unsigned char day[2];
	unsigned char month[2];

}rtcType;

void rtcInit();
rtcType getTimeStamp();


#endif /* RTC_H_ */
