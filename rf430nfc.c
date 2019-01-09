//this code section is only for physical interface of chip

#include "rf430nfc.h"
#include "iic.h"
#include "NFCType4.h"

unsigned char RxData[2] = { 0, 0 };
unsigned char TxData[2] = { 0, 0 };
unsigned char TxAddr[2] = { 0, 0 };

uint8_t errata_fixes[] = { 0xB2, 0xF0, 0xFF, 0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF,
		0xFD, 0x00, 0x07, 0xA2, 0xC3, 0x00, 0x07, 0x08, 0x3C, 0xB2, 0xF0, 0xFF,
		0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF, 0xFD, 0x00, 0x07, 0xA2, 0xC3, 0x00,
		0x07, 0xB2, 0xB0, 0x04, 0x04, 0x00, 0x07, 0xF4, 0x23, 0x30, 0x41, 0x0F,
		0x12, 0x0E, 0x12, 0xB2, 0xB0, 0x00, 0x01, 0x00, 0x07, 0x08, 0x24, 0x3E,
		0x40, 0x1D, 0x00, 0x1F, 0x42, 0xB2, 0x2A, 0x6E, 0x9F, 0x02, 0x20, 0x92,
		0x43, 0x2E, 0x2A, 0x3E, 0x41, 0x3F, 0x41, 0x30, 0x41 };

void setupRF430GPIO(){
    //reset the rf430
    P4OUT |= BIT4;
    P4OUT &= ~BIT4; //RESET PIN
    _delay_cycles(1000);
    P4OUT |= BIT4;

    P2SEL0 &= ~BIT2;    //set as gpio
    P2SEL1 &= ~BIT2;
    P2DIR &= ~BIT2;     //setting as input ///by default as gpio
    P2OUT |= BIT2;      //pull up is selected
    P2REN |= BIT2;      //enable pullup
    P2IFG &= ~BIT2;     //clears the interrupt
    P2IES |= BIT2;      // int at htol transition
    _delay_cycles(1000000); // Leave time for the RF430CL33H to get itself initialized;
}

void errataFix(){
    NFCWrite_Register(0xFFE0, 0x004E);
    NFCWrite_Register(0xFFFE, 0x0080);
    iicInit(RF430_I2C_ADDR);
    iicWriteContinuous(0x2AD0, (uint8_t *) &errata_fixes, sizeof(errata_fixes));      //write the patch to memory
    NFCWrite_Register(0x2A90, 0x2AFC);                 // looping fix
    NFCWrite_Register(0x2AAE, 0x2AD0);               // wait time extension fix
    NFCWrite_Register(0x2A66, 0x0000);                 // fix by init
    NFCWrite_Register(0x2ABA, 0x0000); // 00 = 106kbps (DEFAULT SETTING) so this line is optional
    NFCWrite_Register(0x27B8, 0);
    NFCWrite_Register(0xFFE0, 0);
}


void rf430Init(void) {
    iicInit(RF430_I2C_ADDR);
    setupRF430GPIO();
    //wait for device to get ready
    while (!(NFCRead_Register(STATUS_REG) & READY));
    errataFix();

    //only interrupt for general type 4 request
    NFCWrite_Register(INT_ENABLE_REG, /*EXTRA_DATA_IN_INT_ENABLE +*/DATA_TRANSACTION_INT_ENABLE + FIELD_REMOVED_INT_ENABLE);
    //Configure INTO pin for active low and enable RF
    NFCWrite_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE);
    // sets the command timeouts, generally should stay at these settings
    NFCWrite_Register(SWTX_INDEX, 0x3B); // maximum allowable wait time extension request

    P2IFG &= ~BIT2; //clear the interrupt again
    P2IE |= BIT2;   //enable the interrupt

    AppInit();
}


unsigned int NFCRead_Register(unsigned int reg_addr) {
	iicInit(RF430_I2C_ADDR);
	return iicReadRegister16bitW16bitAddress(reg_addr);
}

void NFCWrite_Register(unsigned int reg_addr, unsigned int value) {
    iicInit(RF430_I2C_ADDR);
    iicWriteRegister16bitW16bitAddress(value,reg_addr);
}

void NFCWrite_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length){
    iicInit(RF430_I2C_ADDR);
    iicWriteContinuous(reg_addr,write_data,data_length);
}
