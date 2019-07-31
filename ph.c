/*
 * ph.c
 *
 *  Created on: 2019. 5. 13.
 *      Author: kiran
 */


#include "ph.h"

//#pragma PERSISTENT (PH1)
//int PH1[2] = {999,999};
//#pragma PERSISTENT (PH2)
//int PH2[2] = {999,999};
//#pragma PERSISTENT (PH3)
//int PH3[2] = {999,999};
#pragma PERSISTENT (PH4)
int PH4[2] = {999,999};
#pragma PERSISTENT (PH5)
int PH5[2] = {999,999};
#pragma PERSISTENT (PH6)
int PH6[2] = {999,999};
#pragma PERSISTENT (PH7)
int PH7[2] = {999,999};
//#pragma PERSISTENT (PH8)
//int PH8[2] = {999,999};
//#pragma PERSISTENT (PH9)
//int PH9[2] = {999,999};
//#pragma PERSISTENT (PH10)
//int PH10[2] = {999,999};
//#pragma PERSISTENT (PH11)
//int PH11[2] = {999,999};
//#pragma PERSISTENT (PH12)
//int PH12[2] = {999,999};
//#pragma PERSISTENT (PH13)
//int PH13[2] = {999,999};
//#pragma PERSISTENT (PH14)
//int PH14[2] = {999,999};

void upDatepHLevels(int pHLevel[8]){
    PH4[0] = pHLevel[0];
    PH4[1] = pHLevel[1];

    PH5[0] = pHLevel[2];
    PH5[1] = pHLevel[3];

    PH6[0] = pHLevel[4];
    PH6[1] = pHLevel[5];

    PH7[0] = pHLevel[6];
    PH7[1] = pHLevel[7];
}

int getpH(int result){
//    if(result >= MINPH(PH1) && result <= MAXPH(PH1)){
//        return  (int)(10*( (float)(result-MAXPH(PH1))/( MINPH(PH1) - MAXPH(PH1) ) + 0 ));
//    } else
//    if(result >= MINPH(PH2) && result <= MAXPH(PH2)){
//        return  (int)(10*( (float)(result-MAXPH(PH2))/( MINPH(PH2) - MAXPH(PH2) ) + 1 ));
//    }else
//    if(result >= MINPH(PH3) && result <= MAXPH(PH3)){
//        return  (int)(10*( (float)(result-MAXPH(PH3))/( MINPH(PH3) - MAXPH(PH3) ) + 2 ));
//    }else
    if(result >= MINPH(PH4) && result <= MAXPH(PH4)){
        return 40;
    }else
    if(result >= MINPH(PH5) && result <= MAXPH(PH5)){
        return 50;
        //
    }else
    if(result >= MINPH(PH6) && result <= MAXPH(PH6)){
        return 60;
    }else
    if(result >= MINPH(PH7) && result <= MAXPH(PH7)){
        return 70;
    }
    /************************************************************************************/
    else if (result > MAXPH(PH5) && result < MINPH(PH4)){
        return  (int)(10*( (float)(result-MINPH(PH4))/( MAXPH(PH5) - MINPH(PH4) ) + 4 ));

    }
    else if (result > MAXPH(PH6) && result < MINPH(PH5)){
        return  (int)(10*( (float)(result-MINPH(PH5))/( MAXPH(PH6) - MINPH(PH5) ) + 5 ));

    }
    else if (result > MAXPH(PH7) && result < MINPH(PH6)){
        return (int)(10*( (float)(result-MINPH(PH6))/( MAXPH(PH7) - MINPH(PH6) ) + 6 ));
    }

    else if ( result < MINPH(PH7)){
        return 80;

    }

    else if ( result > MAXPH(PH4)){
        return 30;
    }

    //else
//    if(result >= MINPH(PH8) && result <= MAXPH(PH8)){
//        return  (int)(10*( (float)(result-MAXPH(PH8))/( MINPH(PH8) - MAXPH(PH8) ) + 7 ));
//    }else

//    if(result >= MINPH(PH9) && result <= MAXPH(PH9)){
//        return  (int)(10*( (float)(result-MAXPH(PH9))/( MINPH(PH9) - MAXPH(PH9) ) + 8 ));
//    }else
//    if(result >= MINPH(PH10) && result <= MAXPH(PH10)){
//        return  (int)(10*( (float)(result-MAXPH(PH10))/( MINPH(PH10) - MAXPH(PH10) ) + 9 ));
//    }else
//    if(result >= MINPH(PH11) && result <= MAXPH(PH11)){
//        return  (int) ((float)(result-MAXPH(PH11))/( MINPH(PH11) - MAXPH(PH11) ) + 10 );
//    }else
//        if(result >= MINPH(PH12) && result <= MAXPH(PH12)){
//            return  (int)( (float)(result-MAXPH(PH12))/( MINPH(PH12) - MAXPH(PH12) ) + 11 );
//    }else
//    if(result >= MINPH(PH13) && result <= MAXPH(PH13)){
//        return  (int)( (float)(result-MAXPH(PH13))/( MINPH(PH13) - MAXPH(PH13) ) + 12 );
//    }else
//        if(result >= MINPH(PH14) && result <= MAXPH(PH14)){
//            return  (int)( (float)(result-MAXPH(PH14))/( MINPH(PH14) - MAXPH(PH14) ) + 13 );
//    }else{
//        return 0;
//    }
}

