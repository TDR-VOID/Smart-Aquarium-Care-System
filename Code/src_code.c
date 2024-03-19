#define F_CPU 8000000UL			/* # Define CPU Frequency (CPU clock frequency for the AVR microcontroller)*/
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) /* GSM_USART_BAUDRATE: This is the desired baud rate for UART communication */
#include <string.h> 			/* # Include the standard C string library for string manipulation (strlen)*/
#include <avr/io.h>				/* # Include the AVR I/O (Input/Output) header file for accessing hardware registers. */
#include <util/delay.h>			/* # Include delay header file  (_dealy_ms)*/
#include <avr/interrupt.h>		/* #Include the AVR interrupt header file for enabling and managing interrupts.*/


/* 
   Smart Aquarium Care System Project
   LCD initialization and functions adapted from "ElectronicWings"
   https://www.electronicwings.com/avr-atmega/interfacing-lcd-16x2-in-4-bit-mode-with-atmega-16-32-
*/

#define LCD_Dir  DDRC			/* Define LCD data port data direction registry */
#define LCD_Port PORTC			/* Define LCD data port */
#define RS PC0					/* Define Register Select pin */
#define EN PC1 					/* Define Enable signal pin */


volatile unsigned int e =1, q=0, k=0; /* e - Variable assign for 'Gate Openings' , */
volatile unsigned int l=0,s=0,m=0,h=0; /*l - create the ms, s - seconds, m - minutes, h - hours*/
volatile _Bool active=1; /*For the first screen of the LCD*/
volatile unsigned char u=4;
int i = 0;


/* Declare a character variable int_str1,int_str2,int_str3,int_str4 */
char int_str1; 
char int_str2;
char int_str3;
char int_str4;




// Function prototypes
void UART_init(long USART_BAUDRATE);
unsigned char UART_RxChar(void);
void UART_TxChar(char ch);
void UART_SendString(char *str);
void LCD_Clear();
void LCD_Command( unsigned char cmnd );
void LCD_Char( unsigned char data );
void LCD_Init (void);
void LCD_String (char *str);
void initTimer2CTC(void);
void LCD_String_xy (char row, char pos, char *str);
void externalInt(void);
void ServoTime();




void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		/* RS=0, command reg. */
	LCD_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);		/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			/* Make LCD port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	LCD_Command(0x02);		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/
	_delay_ms(2);
}

void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}

void externalInt(void){
    DDRD = DDRD & ~(1<<PD3); /*Setting up DDRD PIN3 as input (set 0 wihtout changing other pins)*/
    DDRB = DDRB & ~(1<<PB2); /*Setting up DDRB PIN2 as input*/
    GICR |= (1<<INT1) | (1<<INT2); /*enable external interrupt 1 & 0 ; INT1 = PD3 , INT2 = PB2 (General Interrupt
Control Register)*/
    MCUCR |= (1<<ISC10) | (1<<ISC11);  /*The rising edge of INT1 generates an interrupt request.*/
    MCUCSR |= (1<<ISC2); /*falling edge on
INT2 activates the interrupt*/
}
 
 
void initTimer2CTC(void){   //CTC mode; Clock ps/8; OC0 disconnected
    TCCR2 = 0X00; /*sets the Timer/Counter Control Register 2 (TCCR2) to zero*/
    TCCR2 |= 1<<CS22|1<<WGM21; /*CS22-clkT2S/64 (From prescaler) , CTC Mode - Clear Timer on Compare Match */
    TIMSK |= 1<<OCIE2;   /*Output Compare Match Interrupt for Timer 2*/
    OCR2=124; /* Output Compare Register 2  to 124*/
	/*DesiredTime = F_CPU / (Prescale * (OCR2 + 1))*/
} 



void ServoTime()
{
	//int i;
	for(i=0;i<5;i++)
	{
		_delay_ms(200);

	}

}


/*Initialize UART with the specified baud rate*/ 
void UART_init(long USART_BAUDRATE){
    UCSRB |= (1 << RXEN) | (1 << TXEN); 	/*Enabling RXEN allows the UART to receive data, and enabling TXEN allows it to transmit data (Turn on transmission and reception)*/
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes
    UBRRL = BAUD_PRESCALE; 			// Load lower 8-bits of the baud rate value
    UBRRH = (BAUD_PRESCALE >> 8); 		// Load upper 8-bits
}

// Receive a character from UART
unsigned char UART_RxChar(void){		
    while ((UCSRA & (1 << RXC)) == 0); 		// Wait till data is received
    return(UDR); 				// Return the received byte
}

// Transmit a character through UART
void UART_TxChar(char ch){
    while (!(UCSRA & (1<<UDRE))); 		// Wait for empty transmit buffer
    UDR = ch;
}

// Send a string through UART
void UART_SendString(char *str){
    unsigned char j=0;
    while (str[j] != 0){ 			// Send string till null terminator
        UART_TxChar(str[j]);
        j++;
        
    
        
    }
}


int main()
{	
	sei(); /*enable global interrupts in an AVR */
	
	externalInt();

	LCD_Init();
	LCD_Clear();
	LCD_String_xy(0,4,"Welcome");
	_delay_ms(2000);
	
	
	//Configure TIMER1
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);		 	//NON Inverted PWM
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10);	//PRESCALER=64 MODE 14(FAST PWM)
	ICR1=4999;	//fPWM=50Hz (Period = 20ms Standard).
	DDRD|=(1<<PD4)|(1<<PD5);	//PWM Pins as Out


	char int_str1[20]; /* Declare an array to store a string of up to 20 characters*/
	char int_str2[20];
	char int_str3[20];
	char int_str4[20];

	while(active){
	sprintf(int_str4, "%02d", u);
	LCD_String_xy(0,0,"Feeding Interval :");	/* Initialization ofLCD*/
	LCD_Command(0xC0);
	LCD_String_xy(1,7,int_str4);		/* Go to 2nd line*/
	_delay_ms(10);

	
	}
	
	initTimer2CTC();
	
	_delay_ms(100);

	while(1){
	
	if (k==6){
	UART_init(9600);		// Initialize UART communication with a baud rate of 9600
        UART_SendString("AT\r\n");	// Send AT command to the GSM module
        _delay_ms(200);
        UART_SendString("ATE0\r\n");	// Disable command echo
        _delay_ms(200);
        UART_SendString("AT+CMGF=1\r\n");	// Set SMS text mode
        _delay_ms(200);
        UART_SendString("AT+CMGS=\"xxxxxxxxx\"\r\n");	// Set recipient phone number
        _delay_ms(200);
        UART_SendString("Food Level Low");// Send SMS text
        UART_TxChar(26);		// Send Ctrl+Z to indicate end of SMS
    	k=0;
  
	
	}
	
	if(s==u){
	for(int n=0;n<e;n++){
	k=k+1;
	
	OCR1A=175;
	ServoTime();

	OCR1A=220;
	ServoTime();
	s=0;

	}
	}
	

	sprintf(int_str1, "%02d", e); /*%d give only one digit*/
	sprintf(int_str2, "%02d", s);
	sprintf(int_str3, "%02d", m);
	
	
	LCD_String_xy(0,0,"Gate Openings:");/* Initialization ofLCD*/
	LCD_String_xy(0,14,int_str1);	/* Write string on 1st line of LCD*/
	LCD_Command(0xC0);
	LCD_String_xy(1,0,"Time: ");		/* Go to 2nd line*/
	LCD_String_xy(1,7,int_str3);
	LCD_String_xy(1,9,":");
	LCD_String_xy(1,10,int_str2);	/* Write string on 2nd line*/
	_delay_ms(10);

	
	}	
	
		
	
}


ISR(TIMER2_COMP_vect ){ 
	l=l+1;
		if(l == 1000){
			s+=1 ;
			l=0;
			if(s==60){
				m=m+1;
				s=0;
				if (m==60){
					h=h+1;
					m=0;
					}
				}
				 
		}
		
	}




ISR(INT1_vect){
   
   if(active){
	   u+=2;
	   if(u>13){
		u=4;
   }
   
   }
   else if(active==0){ e=e+1;
    if(e==9){
    e=0;
    }
       
}
}
ISR(INT2_vect){
  
  if(active){
	active=0;
  
 
  
  }
  if(active==0){
  
   TIMSK ^= 1<<OCIE2; /*TIMSK (Timer/Counter Interrupt Mask Register)*/
}

}
    