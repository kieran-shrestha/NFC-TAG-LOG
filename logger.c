/*
 * logger.c
 *
 *  Created on: Jun 10, 2017
 *      Author: kiera
 */


#include "rf430Process.h"
#include "rf430nfc.h"

extern uint8_t FileTextE104[];

#pragma PERSISTENT (log_counter)
uint16_t log_counter = 0;

void push_data(int l,int m,int n,int adcch){

    FileTextE104[12 + log_counter*11] = log_counter/100+48;
    FileTextE104[13 + log_counter*11] = (log_counter%100)/10 +48;
    FileTextE104[14 + log_counter*11] = log_counter%10 +48;
    FileTextE104[15 + log_counter*11] = ',';
    FileTextE104[16 + log_counter*11] = l+48;
    FileTextE104[17 + log_counter*11] = m+48;
    FileTextE104[18 + log_counter*11] = n+48;
    FileTextE104[19 + log_counter*11] = ',';
    if(adcch == 0)
        FileTextE104[20 + log_counter*11] = '1';
    else
        FileTextE104[20 + +log_counter*11] = '2';
    FileTextE104[ 21 + log_counter*11] = ',';
    FileTextE104[22 + log_counter*11] = 0x0A;

    log_counter++;
    FileTextE104[1] = log_counter*11 + 0x0A;
    FileTextE104[0] = (log_counter*11 + 0x0A)>>8;
    FileTextE104[7] = log_counter*11+3;
    FileTextE104[6] = (log_counter*11+3)>>8;
    if(log_counter== 999)
        log_counter = 0;

}

