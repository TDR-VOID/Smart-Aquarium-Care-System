#define F_CPU 8000000UL // Define the CPU clock frequency as 8 MHz

#include <avr/io.h> //Include AVR library for I/O and register definitions
#include <util/delay.h> //Include AVR library for delay functions
#include <avr/interrupt.h> //Include AVR library for interrupt handling

// Define UART baud rate pre-scale
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

// Initialize UART communication with the specified baud rate
void UART_init(long USART_BAUDRATE);

// Receive a character from UART
unsigned char UART_RxChar(void);

// Transmit a character through UART
void UART_TxChar(char ch);

// Send a string through UART
void UART_SendString(char *str);

int main(void) {
 while (1) {

 // Initialize UART communication with a baud rate of 9600
 UART_init(9600);

 // Send AT command to check the module's response
 UART_SendString("AT\r\n");
 _delay_ms(2000);

 // Disable command echo
 UART_SendString("ATE0\r\n");
 _delay_ms(2000);

 // Set SMS text mode
 UART_SendString("AT+CMGF=1\r\n");
 _delay_ms(2000);

 // Send an SMS to the specified number
 UART_SendString("AT+CMGS=\"+xxxxxxxxx\"\r\n");
 _delay_ms(2000);

 // Send the SMS message: "Food Level Low"
 UART_SendString("Food Level Low ");
 UART_TxChar(26);

 // Wait for 10 seconds before repeating the process
 _delay_ms(10000);
 }
 return 0;
}