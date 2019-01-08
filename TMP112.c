#include "msp430.h"
#include "tmp112.h"
#include "iic.h"

void tmpInit(){
    iicInit(TMP112_I2C_ADDR);   //int iic
	iicWriteRegister16bit(0xA078,TMP112_CONF_REG);  //setup device
}

unsigned int getTemperature(){
	iicWriteRegister16bit(0xA0F8,TMP112_CONF_REG);  //setting up device
	return iicReadRegister16bit(TMP112_TEMP_REG);  //reading temperature
}

void TMP_Get_Temp(int* ui16TempReturn, unsigned char* uc8NegFlagReturn){
	long pTempData;
	unsigned int g_TempDataCel;
	unsigned char g_TempNegFlagCel = 0;

	iicInit(TMP112_I2C_ADDR);
	pTempData = getTemperature();

	if(!(pTempData & 0x800)){ // Sign bit.  If +
		g_TempDataCel = (pTempData * 625)/100;
		g_TempNegFlagCel = 0;					//defined in main
	}
	else{
		pTempData = (~pTempData) & 0xFFF;
		g_TempDataCel = (pTempData * 625)/100;
		g_TempNegFlagCel = 1;
	}

	*ui16TempReturn = g_TempDataCel;
	*uc8NegFlagReturn = g_TempNegFlagCel;
}

