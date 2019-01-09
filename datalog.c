#define DATA_WIDTH 24
#define NDEFSTART 60
#define MAX_LOGS 1920

#include "rtc.h"
#include "NFCType4.h"

#pragma PERSISTENT (numOfLogsInFram)
extern unsigned int numOfLogsInFram = 0;

#pragma PERSISTENT (ui16nlenhold)
unsigned int ui16nlenhold =  DEFNLEN;

#pragma PERSISTENT (ui16plenhold)
unsigned int ui16plenhold = DEFPLEN;

extern uint8_t FileTextE104[];

rtcType timestamp;

unsigned char bufferHold[DATA_WIDTH];

void datalogInit(){
    bufferHold[0] = ' ';
    bufferHold[3] = '.';
    bufferHold[6] = ',';
    bufferHold[9] = ':';
    bufferHold[12] = ',';
    bufferHold[17] = '/';
    bufferHold[20] = '/';
    bufferHold[23] = '\n';
}


void data_buffer(int Temperature){
/////////////filling the temperature/////////////////////////
    unsigned int temp;
    if(Temperature < 0 ){
        bufferHold[0] = '-';
        Temperature *= -1;
    }
    temp = Temperature;

    temp = temp / 1000;

    bufferHold[1] = (char) temp + 48;
    temp = Temperature % 1000;

    temp = temp / 100;

    bufferHold[2] = (char) temp + 48;
    temp = Temperature % 100;

    temp = temp / 10;

    bufferHold[4] = (char) temp + 48;
    temp = Temperature % 10;


    bufferHold[5] = (char) temp + 48;
//////////////////////////////////////////////////////////////

    timestamp = getTimeStamp();

    bufferHold[7] = timestamp.hour[1];
    bufferHold[8] = timestamp.hour[0];

    bufferHold[10] = timestamp.minute[1];
    bufferHold[11] = timestamp.minute[0];

    bufferHold[13] = '2';
    bufferHold[14] = '0';
    bufferHold[15] = timestamp.year[1];
    bufferHold[16] = timestamp.year[0];

    bufferHold[18] = timestamp.month[1];
    bufferHold[19] = timestamp.month[0];

    bufferHold[21]  = timestamp.day[1];
    bufferHold[22] = timestamp.day[0];

    if(ui16nlenhold  < (unsigned int ) DATA_WIDTH*MAX_LOGS  ){    //maximum data it can hold
        ui16nlenhold += DATA_WIDTH;

        temp = ui16nlenhold;
    /////setting up the length of the ndef record
        FileTextE104[1] = ( char) ui16nlenhold;
        temp >>=8;
        FileTextE104[0] = ( char) temp;
    /////setting up the length of the ndef payload
        ui16plenhold += DATA_WIDTH;
        temp = ui16plenhold;

        FileTextE104[7] = ( char) ui16plenhold;
        temp >>= 8;
        FileTextE104[6] = ( char) temp;

    //appending data
        for( temp = 0 ; temp < DATA_WIDTH ; temp++){
            FileTextE104[NDEFSTART + temp + numOfLogsInFram*DATA_WIDTH] = bufferHold[temp];
        }

        numOfLogsInFram += 1;

    } else {
        for(temp = NDEFSTART ;temp < (unsigned int) MAX_LOGS*DATA_WIDTH - DATA_WIDTH ; temp++){
            FileTextE104[temp] = FileTextE104[temp + DATA_WIDTH];
        }

        for( temp = 0 ; temp < DATA_WIDTH ; temp++){
            FileTextE104[NDEFSTART + temp + (numOfLogsInFram-1)*DATA_WIDTH] = bufferHold[temp];
        }
    }
}
