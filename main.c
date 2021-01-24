/*
 * Mangeng_Konzett-Lichtschranke.c
 *
 * Created: 11.12.2020 11:59:07
 * Author : NoahM
 */ 




#include <avr/io.h>

#include "zkslibdisplay.h"
#include <avr/interrupt.h>
#include <util/delay.h>


int count;
int pwmtest;
int lichtschranke;
int save;
int dauer;
int savezwei;
int umdrehung;
int drehzahl;
int MotorOCR;






void pwmsignal()
{
	TCNT0 = 0;
	TCCR0 |= (1<<WGM01) | (1<<WGM00) | (1<<COM01)| (1<<CS02);
	DDRB|=(1<<PB3);
	MotorOCR = 23437;
	OCR0 = MotorOCR;
	 
}


void timer1_init()
{
	
	TCNT1 = 0;
	TCCR1A |= (1<<WGM12);
	TCCR1B |=(1<<CS11);
	TIMSK|=(1<<OCIE1B);
	
	OCR1B = 150-1; //0.1ms abtastfrequenz 
		
}


//Polling ISR
ISR(TIMER1_COMPB_vect)
{
	
	
	
	count++;
	
	
	
	TCNT1 = 0;
}

//MotorPWM
ISR(TIMER0_COMP_vect)
{
			
	if (pwmtest == 0)
	{
		PB3;
	}
	if (pwmtest ==1)
	{
		!PB3;
		pwmtest = 0;
	}
	pwmtest++;
	
}
void display_Init(void);
void display_Clear();
void display_TxtToDisplay(char *txt, unsigned char len);
void display_UintToDisplay(uint32_t x, char N);
void display_Home(void);





int main(void)
{
	
	int soll;
	soll = round(12000.0 * (MotorOCR/46874.0));	//12000 = max rpm, MotorOCR/46874 um den Dutycycle zu berechnen
	//berechnung soll funktioniert nicht, Grund unbekannt
	//MotorOCR wird deswegen verwendet, da OCR0 nicht als die eingegebene Zahl abgespeichert wird, sondern als restwert von "(Eingegebener Wert/256)"
	//also wird OCR0 bei 23437 als 141 abgespeichert (91*256+141 = 23437)
	
	
	DDRD &= ~(1 << PD7);
	PORTD = 0x00;
	DDRC = 0xFF;
	PORTC = 0x00;
	
	cli();
	
	timer1_init();	
	display_Init();
	display_Clear();
	
	int last = 0;
	int ausgabe;
	pwmsignal();
	
	sei();
	
	while (1)
	{
		
	
		if((PIND&(1<<7)))
		{
			
			if(last == 0)
			{
				save++;
				lichtschranke++;
				if(lichtschranke >=100)
				{
					lichtschranke  =0; 
					drehzahl = round((25.0/count)*10000*60);
					//ausgabe = (int)(drehzahl + 0.5d);
					
					display_Home();
					display_TxtToDisplay("Ist", 3);
					display_UintToDisplay(drehzahl, 5);
					
					display_TxtToDisplay("Soll", 2);
					display_UintToDisplay(soll, 6);
					count = 0;
				}
			}
			
			last = 1;
			
			
			//PORTC |= (1<<PC5) ;
		}
		else
		{
			last = 0;
			//PORTC &= ~(1<<PC5);
		}
	}
		
}
	

