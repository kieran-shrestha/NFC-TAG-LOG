/*
 * ph.h
 *
 *  Created on: 2019. 5. 13.
 *      Author: kiran
 */

#ifndef PH_H_
#define PH_H_


int inline MAXPH(int MINMAX[]){
    return MINMAX[1];
}

int inline MINPH(int MINMAX[]){
    return MINMAX[0];
}
int getpH(int result);
void upDatepHLevels(int pHLevel[8]);


#endif /* PH_H_ */
