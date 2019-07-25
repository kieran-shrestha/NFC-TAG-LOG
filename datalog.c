#define DATA_WIDTH 22U
#define NDEFSTART 59
#define MAX_LOGS 500

#include "rtc.h"
#include "NFCType4.h"
#include "myTimers.h"
#include "ph.h"

#pragma PERSISTENT (numOfLogsInFram)
extern unsigned int numOfLogsInFram = 0;

#pragma PERSISTENT (ui16nlenhold)
unsigned int ui16nlenhold =  DEFNLEN;

#pragma PERSISTENT (ui16plenhold)
unsigned int ui16plenhold = DEFPLEN;

extern uint8_t FileTextE104[];

rtcType timestamp;

unsigned char bufferHold[DATA_WIDTH];

void resetLog(){
    numOfLogsInFram = 0;
    ui16nlenhold = DEFNLEN;
    ui16plenhold = DEFPLEN;
}

void datalogInit(){
    bufferHold[4] = ',';
    bufferHold[7] = ':';
    bufferHold[10] = ',';
    bufferHold[15] = '/';
    bufferHold[18] = '/';
    bufferHold[21] = '\n';
}


void data_buffer(int sign, int l,int m, int n){
    unsigned int temp;
/////////////filling the temperature/////////////////////////
   bufferHold[1] = l+48;
   bufferHold[2] = m+48;
   bufferHold[3] = n+48;

   if( sign == 1)
       bufferHold[0] = '-';
   else
       bufferHold[0] = '+';
//////////////////////////////////////////////////////////////

    timestamp = getTimeStamp();

    bufferHold[5] = timestamp.hour[1];
    bufferHold[6] = timestamp.hour[0];

    bufferHold[8] = timestamp.minute[1];
    bufferHold[9] = timestamp.minute[0];

    bufferHold[11] = '2';
    bufferHold[12] = '0';
    bufferHold[13] = timestamp.year[1];
    bufferHold[14] = timestamp.year[0];

    bufferHold[16] = timestamp.month[1];
    bufferHold[17] = timestamp.month[0];

    bufferHold[19]  = timestamp.day[1];
    bufferHold[20] = timestamp.day[0];

    if(ui16nlenhold  < (unsigned int ) DATA_WIDTH*MAX_LOGS  ){    //maximum data it can hold
        ui16nlenhold += DATA_WIDTH;

        temp = ui16nlenhold;
    /////setting up the length of the ndef record
        FileTextE104[1] = (unsigned char) ui16nlenhold;
        temp >>=8;
        FileTextE104[0] = (unsigned char) temp;
    /////setting up the length of the ndef payload
        ui16plenhold += DATA_WIDTH;
        temp = ui16plenhold;

        FileTextE104[7] = (unsigned char) ui16plenhold;
        temp >>= 8;
        FileTextE104[6] = (unsigned char) temp;

    //appending data
        for( temp = 0 ; temp < DATA_WIDTH ; temp++){
            FileTextE104[NDEFSTART + temp + numOfLogsInFram*DATA_WIDTH] = bufferHold[temp];
        }

        numOfLogsInFram += 1;

    } else {
        stopTimer();
        for(temp = NDEFSTART ;temp <  MAX_LOGS*DATA_WIDTH - DATA_WIDTH ; temp++){
            FileTextE104[temp] = FileTextE104[temp + DATA_WIDTH];
        }

        for( temp = 0 ; temp < DATA_WIDTH ; temp++){
            FileTextE104[NDEFSTART + temp + (numOfLogsInFram-1)*DATA_WIDTH] = bufferHold[temp];
        }
        startTimer();
    }
}
