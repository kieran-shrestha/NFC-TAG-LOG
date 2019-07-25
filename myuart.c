#include "msp430.h"
#include "myuart.h"
#include <stdint.h>

void myuart_init(void){
  // Configure GPIO
  P2SEL1 |= UART_TX_PIN | UART_RX_PIN;                    // USCI_A0 UART operation
  P2SEL0 &= ~(UART_TX_PIN | UART_RX_PIN);
  // Configure USCI_A0 for UART mode
  UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK

  UCA0BR0 = 8;                             // 4000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW |= UCOS16 | UCBRF_10;
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
