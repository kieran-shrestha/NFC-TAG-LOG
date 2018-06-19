#define DATA_WIDTH 27

#define NDEFSTART 63

#define MAX_LOGS 500
#define CHECK_INTERVAL 1

#include <stdio.h>
#include "rtc.h"
#include "myuart.h"
#include "datalog.h"

//#define DEBUG 1

#pragma PERSISTENT (numOfLogsInFram)
extern unsigned int numOfLogsInFram = 0;

#pragma PERSISTENT (ui16nlenhold)
extern unsigned int ui16nlenhold =  0x3A+3;

#pragma PERSISTENT (ui16plenhold)
extern unsigned int ui16plenhold = 0x33+3;

datalog_interval_type interval;

extern uint8_t FileTextE104[];
extern rtcType timestamp;

extern unsigned char check;

unsigned char bufferHold[DATA_WIDTH];

void datalog_Init(){
    interval.temp_interval_minute = CHECK_INTERVAL;
    bufferHold[0] = ' ';
    bufferHold[3] = '.';
    bufferHold[6] = ',';
    bufferHold[9] = ',';
    bufferHold[12] = ':';
    bufferHold[15] = ',';
    bufferHold[20] = '/';
    bufferHold[23] = '/';
    bufferHold[26] = 0x0A;
}

void data_buffer(unsigned int Temperature,unsigned char pHValue){
    unsigned int temp;
    temp = Temperature;

#ifdef DEBUG
            char str[30];
            sprintf(str, "\n\rTemperature: %d ", Temperature);
            myuart_tx_string(str);
            myuart_tx_byte(0xB0);
            myuart_tx_byte('C');
            myuart_tx_byte(0x0D);
#endif

    temp /= 1000;
    bufferHold[1] = (char) temp + 48;
    temp = Temperature % 1000;

    temp /= 100;
    bufferHold[2] = (char) temp + 48;
    temp = Temperature % 100;

    temp /= 10;
    bufferHold[4] = (char) temp + 48;
    temp = Temperature % 10;

    bufferHold[5] = (char) temp + 48;
//////////////////////////////////////////////////////////////

    temp = pHValue;
    temp /=10;
    bufferHold[7] = (char) temp + 48;
    temp = pHValue % 10;
    bufferHold[8] = (char) temp + 48;

/////////////////////////////////////////////////////////////
    timestamp = getTimeStamp();

    bufferHold[10] = timestamp.hour[1];
    bufferHold[11] = timestamp.hour[0];

    bufferHold[13] = timestamp.minute[1];
    bufferHold[14] = timestamp.minute[0];

    bufferHold[16] = '2';
    bufferHold[17] = '0';
    bufferHold[18] = timestamp.year[1];
    bufferHold[19] = timestamp.year[0];

    bufferHold[21] = timestamp.month[1];
    bufferHold[22] = timestamp.month[0];

    bufferHold[24] = timestamp.day[1];
    bufferHold[25] = timestamp.day[0];


#ifdef DEBUG
    myuart_tx_byte(timestamp.hour[1]);
    myuart_tx_byte(timestamp.hour[0]);

    myuart_tx_byte(':');

    myuart_tx_byte(timestamp.minute[1]);
    myuart_tx_byte(timestamp.minute[0]);

    myuart_tx_byte(0x20);

    myuart_tx_byte(timestamp.year[1]);
    myuart_tx_byte(timestamp.year[0]);
    myuart_tx_byte('/');
    myuart_tx_byte(timestamp.month[1]);
    myuart_tx_byte(timestamp.month[0]);
    myuart_tx_byte('/');
    myuart_tx_byte(timestamp.day[1]);
    myuart_tx_byte(timestamp.day[0]);
    myuart_tx_byte(0x0D);
#endif

    if(ui16nlenhold <  DATA_WIDTH*MAX_LOGS){    //maximum data it can hold

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

        for( temp = 0 ; temp < DATA_WIDTH ; temp++){
            FileTextE104[NDEFSTART + temp + numOfLogsInFram*DATA_WIDTH] = bufferHold[temp];
        }

        numOfLogsInFram+=1;

    } else {
#ifdef DEBUG
        myuart_tx_string("\n\r.............Memory full..............\n\r");
#endif
        for(temp = NDEFSTART ;temp < MAX_LOGS*DATA_WIDTH - DATA_WIDTH ; temp++){
            FileTextE104[temp] = FileTextE104[temp + DATA_WIDTH];
        }

        for( temp = 0 ; temp < DATA_WIDTH ; temp++){
            FileTextE104[NDEFSTART + temp + (numOfLogsInFram-1)*DATA_WIDTH] = bufferHold[temp];
        }

    }





#ifdef DEBUG

        sprintf(str,"\n\rTL=%d dumping all\n\r",numOfLogsInFram);
        myuart_tx_string(str);
//      for(temp = 0 ;temp < numOfLogsInFram*DATA_WIDTH;temp++){
//              myuart_tx_byte(FileTextE104[12+temp]);
//      }

#endif


}
