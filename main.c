#include <msp430.h> 
#include "myClock.h"
#include "myuart.h"
#include "rf430nfc.h"
#include "driverlib.h"
#include "rf430Process.h"
#include "myTimers.h"
#include "myadc.h"
//#include "ph.h"
#include "logger.h"
#include <stdio.h>
#include "datalog.h"
#include "TMP112.h"
#include "rtc.h"
#include "math.h"

const double  BALANCE_RESISTOR   = 200000.0;
const double  RESISTOR_ROOM_TEMP = 46611.0;
const double  MAX_ADC            = 4095.0;
const double  BETA               = 208.9;
const double  ROOM_TEMP          = 298.15;

#define DEV_BOARD 0                             //Disable if DEV board used; 1=> Dev board used

#if !DEV_BOARD  //Disable if DEV board used
    #define SUPPLY_VOLTAGE 2955                     //Battery voltage
    //#define SUPPLY_VOLTAGE 3280                   //If powered by debugger
    #define THERMISTOR_POWER_PORT GPIO_PORT_P3
    #define THERMISTOR_POWER_PIN GPIO_PIN1
#endif

#if DEV_BOARD   //Enable if DEV board used
    #define SUPPLY_VOLTAGE 3628
    #define THERMISTOR_POWER_PORT GPIO_PORT_P1
    #define THERMISTOR_POWER_PIN GPIO_PIN2
#endif

#define LED1_PORT GPIO_PORT_P4
#define LED1_PIN GPIO_PIN6
#define LED2_PORT GPIO_PORT_P4
#define LED2_PIN GPIO_PIN5

#define SLOPE -0.00792
#define INTERCEPT (284.73892-12-17-17)

char str[30];

extern float result,r;
extern int avghold[];

int adc_addlog = 1;
int i;
int is06Sec = 0;

void gpioInit();

unsigned char nfcFired = 0;
unsigned int flags = 0;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    clockInit();
    myuart_init();
    //initTimers();
    RTC_init();                 //initialize rtc
    datalog_Init();

#if !DEV_BOARD  //Disable if DEV board used
    TMP_Config_Init();          // configure to be in shutdown one shot mode
    RF430_Init();               //Disable if you want to test in development board with no RF430 ic
#endif
    myADCinit();
    myuart_tx_string("PROGRAM STARTED...\r");
//    datalog_cleanbuffer();

//    startTimer();
    GPIO_setOutputLowOnPin( LED1_PORT, LED1_PIN);

//    GPIO_setOutputHighOnPin( THERMISTOR_POWER_PORT, THERMISTOR_POWER_PIN);
    initTimers();
    gpioInit();
  //  myuart_init();
    RF430_Init();
    myADCinit();
  //  myuart_tx_string("PROGRAM STARTED...\r");

    startTimer();


    while(1){
        __bis_SR_register(LPM3_bits+GIE);
        __no_operation();
        if( nfcFired){
            flags = Read_Register(INT_FLAG_REG);
            GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN5);
            GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN5);

            do {
                if (flags) {
                    rf430Interrupt(flags);
                }
                flags = Read_Register(INT_FLAG_REG);
            } while (flags);

            GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN5);

            flags = 0;
            nfcFired = 0;
            P2IFG &= ~BIT2;	//clear the interrupt again
            P2IE |= BIT2;	//enable the interrupt

        }


        if(adc_addlog == 1){
            int l,m,n;
            adc_addlog = 0;
            GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);

            for(l = 0;l < SAMPLES ; l++){
                GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN6);
                ADCstartConv();
                while(!(ADC12IFGR0 & BIT0));
                result = ADC12MEM0;
                r = (result - 2048 - 0.5) *2500.0 /2048;
                r+= 0.610352;
                avghold[l] = (int)r;
              //  sprintf(str,"%d,",avghold[l]);
              //  myuart_tx_string(str);
                ADCstopConv();
                GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);
            }
        //    sprintf(str,"%d,", takeSamples());
        //    myuart_tx_string(str);
            GPIO_setOutputLowOnPin( THERMISTOR_POWER_PORT, THERMISTOR_POWER_PIN);           //turn off thermistor voltage source GIOP pin to power down the thermistor voltage divider circuit
            result = takeSamples();
            thermistor_voltage = ( result -1 )* (2500.0/4096.0);

            rThermistor = BALANCE_RESISTOR/((SUPPLY_VOLTAGE/thermistor_voltage)-1);

#if 1
            tCelsius = slope*rThermistor + intercept;
            thermistor_temperature = tCelsius*100.0;            //24.52312 degree => 2452
#endif
#if 0
            tKelvin = (BETA * ROOM_TEMP) /(BETA + (ROOM_TEMP * log(rThermistor / RESISTOR_ROOM_TEMP)));
            tCelsius = tKelvin - 273.15;  // convert kelvin to celsius
            thermistor_temperature = tCelsius*100.0;            //eg 24.52312 degree => 2452
#endif
#if 0   //enable if DEV board used
            //uart transmit the adc converted value
            sprintf(str,"%d,", Temperature);
            myuart_tx_byte(',');
            myuart_tx_string("Ref_temp,");
            myuart_tx_string(str);

            sprintf(str,"%.1f,", rThermistor);
            myuart_tx_byte(',');
            myuart_tx_string("rThermistor,");
            myuart_tx_string(str);

            sprintf(str,"%.1f,", tCelsius);
            myuart_tx_byte(',');
            myuart_tx_string("tCelsius,");
            myuart_tx_string(str);
            myuart_tx_byte('\r');
#endif

#if !DEV_BOARD  //Disable if DEV board used
            //log using datlog.c
            //data_buffer(thermistor_temperature, Temperature);
            data_buffer(Temperature);
#endif

//            n = takeSamples()%1000;
//            l = n/100;  //hundreds
//            n = n%100;
//            m = n/10;   //tenths
//            n = n%10;   //ones
         //   push_data(l,m,n);

//            n = takeSamples()%1000;
//            l = n/100;  //hundreds
//            n = n%100;
//            m = n/10;   //tenths
//            n = n%10;   //ones
//            push_data(l,m,n);


        }



    }
}

void gpioInit(){
    PM5CTL0 &= ~LOCKLPM5;
    P4DIR |= BIT5;	//SET AS OUTPUT
    P4OUT |= BIT5;	//set output as 1
    __delay_cycles(50000);
    P4OUT &= ~BIT5;	//SET to 0 again

    P1DIR |= 0b00000111;
    //#ifdef DEBUG
    P2DIR |= 0b10111000;
    //#else
    //	P2DIR |= 0b10111011;
    //#endif
    P3DIR |= 0b11001100;
    P4DIR |= 0xFF;
    PJDIR |= 0b11001111;

    P1OUT = 0x00;
    P2OUT = 0x00;
    P3OUT = 0x00;
    P4OUT = 0x00;
    PJOUT = 0X00;


    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6);		//for powering the IIC
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6);

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6);         // d1 led
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN5);         // d2 led
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN5);

    P1SEL1 |= BIT6;		//setting p1.6 as sda
    P1SEL1 |= BIT7;		//setting p1.7 as scl


}



#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    //INTO interrupt fired
    if (P2IFG & BIT2) {
        P2IE &= ~BIT2; //disable INTO
        P2IFG &= ~BIT2; //clear interrupt flag
        nfcFired = 1;
        __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
    }
}


//*****************************************************************************
// Interrupt Service Routine
//*****************************************************************************

//#pragma vector=TIMER1_A0_VECTOR
//__interrupt void ccr0_ISR (void)
//{
//    //**************************************************************************
//    // 4. Timer ISR and vector //
//    //timer set to interrupt every 500ms//
//    //**************************************************************************
//    timer_1sec_flag++;
//    if(timer_1sec_flag == 10){
//        //thermistorFired=1;
//        adc_addlog = 1;
//        timer_1sec_flag = 0;
//    }
//}
//
//#pragma vector=TIMER1_A1_VECTOR
//__interrupt void timer1_ISR(void) {
//
//    //**************************************************************************
//    // 4. Timer ISR and vector
//    //**************************************************************************
//    switch (__even_in_range(TA1IV, TA1IV_TAIFG)) {
//    case TA1IV_NONE:
//        break;                 // (0x00) None
//    case TA1IV_TACCR1:                      // (0x02) CCR1 IFG
//        _no_operation();
//        break;
//    case TA1IV_TACCR2:                      // (0x04) CCR2 IFG
//        _no_operation();
//        break;
//    case TA1IV_3:
//        break;                    // (0x06) Reserved
//    case TA1IV_4:
//        break;                    // (0x08) Reserved
//    case TA1IV_5:
//        break;                    // (0x0A) Reserved
//    case TA1IV_6:
//        break;                    // (0x0C) Reserved
//    case TA1IV_TAIFG:             // (0x0E) TA1IFG - TAR overflow
//        //        timer_1sec_flag++;
//        //        if(timer_1sec_flag == 1){
//        //            adc_addlog=1;
//        //            timer_1sec_flag = 0;
//        //        }
//        __bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle INTO
//        break;
//    default:
//        _never_executed();
//    }
//}


//
//#pragma vector = ADC12_VECTOR
//__interrupt void ADC12_ISR(void)
//{
//	switch(__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
//	{
//	case ADC12IV_NONE:        break;        // Vector  0:  No interrupt
//	case ADC12IV_ADC12OVIFG:  break;        // Vector  2:  ADC12MEMx Overflow
//	case ADC12IV_ADC12TOVIFG: break;        // Vector  4:  Conversion time overflow
//	case ADC12IV_ADC12HIIFG:  break;        // Vector  6:  ADC12BHI
//	case ADC12IV_ADC12LOIFG:  break;        // Vector  8:  ADC12BLO
//	case ADC12IV_ADC12INIFG:  break;        // Vector 10:  ADC12BIN
//	case ADC12IV_ADC12IFG0:
//		//ADCstopConv();
//		result = ADC12MEM0;
//		myuart_tx_byte('a');
//		adcfired = 1;
//		__bic_SR_register_on_exit(LPM0_bits + GIE); //wake up to handle INTO
//		break;        // Vector 12:  ADC12MEM0 Interrupt
//	case ADC12IV_ADC12IFG1:   break;        // Vector 14:  ADC12MEM1
//	case ADC12IV_ADC12IFG2:   break;        // Vector 16:  ADC12MEM2
//	case ADC12IV_ADC12IFG3:   break;        // Vector 18:  ADC12MEM3
//	case ADC12IV_ADC12IFG4:           // Vector 20:  ADC12MEM
//		break;
//	case ADC12IV_ADC12IFG5:   break;        // Vector 22:  ADC12MEM5
//	case ADC12IV_ADC12IFG6:   break;        // Vector 24:  ADC12MEM6
//	case ADC12IV_ADC12IFG7:   break;        // Vector 26:  ADC12MEM7
//	case ADC12IV_ADC12IFG8:   break;        // Vector 28:  ADC12MEM8
//	case ADC12IV_ADC12IFG9:   break;        // Vector 30:  ADC12MEM9
//	case ADC12IV_ADC12IFG10:  break;        // Vector 32:  ADC12MEM10
//	case ADC12IV_ADC12IFG11:  break;        // Vector 34:  ADC12MEM11
//	case ADC12IV_ADC12IFG12:  break;        // Vector 36:  ADC12MEM12
//	case ADC12IV_ADC12IFG13:  break;        // Vector 38:  ADC12MEM13
//	case ADC12IV_ADC12IFG14:  break;        // Vector 40:  ADC12MEM14
//	case ADC12IV_ADC12IFG15:  break;        // Vector 42:  ADC12MEM15
//	case ADC12IV_ADC12IFG16:  break;        // Vector 44:  ADC12MEM16
//	case ADC12IV_ADC12IFG17:  break;        // Vector 46:  ADC12MEM17
//	case ADC12IV_ADC12IFG18:  break;        // Vector 48:  ADC12MEM18
//	case ADC12IV_ADC12IFG19:  break;        // Vector 50:  ADC12MEM19
//	case ADC12IV_ADC12IFG20:  break;        // Vector 52:  ADC12MEM20
//	case ADC12IV_ADC12IFG21:  break;        // Vector 54:  ADC12MEM21
//	case ADC12IV_ADC12IFG22:  break;        // Vector 56:  ADC12MEM22
//	case ADC12IV_ADC12IFG23:  break;        // Vector 58:  ADC12MEM23
//	case ADC12IV_ADC12IFG24:  break;        // Vector 60:  ADC12MEM24
//	case ADC12IV_ADC12IFG25:  break;        // Vector 62:  ADC12MEM25
//	case ADC12IV_ADC12IFG26:  break;        // Vector 64:  ADC12MEM26
//	case ADC12IV_ADC12IFG27:  break;        // Vector 66:  ADC12MEM27
//	case ADC12IV_ADC12IFG28:  break;        // Vector 68:  ADC12MEM28
//	case ADC12IV_ADC12IFG29:  break;        // Vector 70:  ADC12MEM29
//	case ADC12IV_ADC12IFG30:  break;        // Vector 72:  ADC12MEM30
//	case ADC12IV_ADC12IFG31:  break;        // Vector 74:  ADC12MEM31
//	case ADC12IV_ADC12RDYIFG: break;        // Vector 76:  ADC12RDY
//	default: break;
//	}
//}
//
