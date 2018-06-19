#include "msp430.h"
#include "myuart.h"
#include <stdint.h>

void myuart_init(void)
{
  // Configure GPIO
  P2SEL1 |= UART_TX_PIN | UART_RX_PIN;                    // USCI_A0 UART operation
  P2SEL0 &= ~(UART_TX_PIN | UART_RX_PIN);

  // Configure USCI_A0 for UART mode
  UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK

  UCA0BR0 = 26;                             // 4000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW |= UCOS16 | UCBRF_1;
  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
}

void myuart_tx_byte(unsigned char data){
	while(!(UCA0IFG&UCTXIFG));
	UCA0TXBUF = data;
}

/*to transmit string characterwise untill null character is found*/
void myuart_tx_string(char *str){
	int i = 0;
	while(str[i]!='\0')	{
		myuart_tx_byte(str[i]);
		i++;
	}

}

#if 0
void myuart_rx()
{
	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
	__bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled
	__no_operation();                         // For debugger
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      while(!(UCA0IFG&UCTXIFG));
      UCA0TXBUF = UCA0RXBUF;		//echo rxdata
      __no_operation();
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
#endif
