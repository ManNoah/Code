/************************************************************/
/* Implementierung von zkslibdisplay.h						*/
/*															*/
/*															*/
/*	Version 202105														*/				
/************************************************************/

#include <AVR/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "zkslibdisplay.h"

#ifdef DISP_MC_NEU
	#include "zkslibspi.h"
	
	#define MCP23S08_DEVICE_ADRESS_READ 0x41
	#define MCP23S08_DEVICE_ADRESS_WRITE 0x40
	#define MCP23S08_REG_ADRESS_GPIO 0x09
	#define MCP23S08_REG_ADRESS_DDR 0x00
	#define MCP23S08_SPI_CHANNEL 0x00
	#define MCP23S08_SPI_NCYCLES 8
	
	
	#define DIPS_EN_BIT 4
	#define DIPS_NWR_BIT 5
	#define DIPS_NCD_BIT 6
	#define DIPS_LED_BIT 7
	
	
#endif

// Vorbereiten der fixen Wait-Zyklen 
#define WAIT_0	0.2
#define WAIT_1	0.2
#define WAIT_2	1
#define WAIT_3	15
#define WAIT_4	5

#define WAIT_LONG_MS 2
#define WAIT_SHORT_US 50

#define DISP_DATA 1
#define DISP_CMD 0

//#define MAX_CHARS 8

/****************************************************************************************/
/* Variablen für Anzeigespeicher und Cursor Positionen															*/
/****************************************************************************************/
static uint8_t _loc_DispData[DISP_LEN];
static uint8_t _loc_Ix = 0;
static uint8_t _loc_X = 0;
static uint8_t _loc_Y = 0;

int	_loc_put(char c, FILE * f);
int	_disp_put(char c, FILE * f);

#ifdef STDOUT_DISP
FILE disp_str = FDEV_SETUP_STREAM(_disp_put, NULL, _FDEV_SETUP_WRITE);
#endif



/****************************************************************************************/
/* Hilfsfunktionen HW-unabhängig													    */
/****************************************************************************************/

/*************************************************************************/
/* Font Data													         */
#define FONT_CHAR_FIRST 32
#define FONT_CHAR_LAST	129
#define FONT_CHARS 98
#define FONT_WIDTH 7
#define FONT_HEIGHT 8
#define FONT_SPACING 1

#ifdef LOAD_FONT_DATA
#define FONT_DATA_VALID
static const uint8_t fontData7x8[FONT_CHARS][FONT_WIDTH]= {
	0,    0,   0,   0,   0,   0,   0, // ' '  32
	0,    6,  95,  95,   6,   0,   0, // '!'  33
	0,    7,   7,   0,   7,   7,   0, // '"'  34
	20, 127, 127,  20, 127, 127,  20, // '#'  35
	36,  46, 107, 107,  58,  18,   0, // '$'  36
	70, 102,  48,  24,  12, 102,  98, // '%'  37
	48, 122,  79,  93,  55, 122,  72, // '&'  38
	4,    7,   3,   0,   0,   0,   0, // '''  39
	0,   28,  62,  99,  65,   0,   0, // '('  40
	0,   65,  99,  62,  28,   0,   0, // ')'  41
	8,   42,  62,  28,  28,  62,  42, // '*'  42
	8,    8,  62,  62,   8,   8,   0, // '+'  43
	0,  128, 224,  96,   0,   0,   0, // ','  44
	8,    8,   8,   8,   8,   8,   0, // '-'  45
	0,    0,  96,  96,   0,   0,   0, // '.'  46
	96,  48,  24,  12,   6,   3,   1, // '/'  47
	62, 127, 113,  89,  77, 127,  62, // '0'  48
	64,  66, 127, 127,  64,  64,   0, // '1'  49
	98, 115,  89,  73, 111, 102,   0, // '2'  50
	34,  99,  73,  73, 127,  54,   0, // '3'  51
	24,  28,  22,  83, 127, 127,  80, // '4'  52
	39, 103,  69,  69, 125,  57,   0, // '5'  53
	60, 126,  75,  73, 121,  48,   0, // '6'  54
	3,    3, 113, 121,  15,   7,   0, // '7'  55
	54, 127,  73,  73, 127,  54,   0, // '8'  56
	6,   79,  73, 105,  63,  30,   0, // '9'  57
	0,    0, 102, 102,   0,   0,   0, // ':'  58
	0,  128, 230, 102,   0,   0,   0, // ';'  59
	8,   28,  54,  99,  65,   0,   0, // '<'  60
	36,  36,  36,  36,  36,  36,   0, // '='  61
	0,   65,  99,  54,  28,   8,   0, // '>'  62
	2,    3,  81,  89,  15,   6,   0, // '?'  63
	62, 127,  65,  93,  93,  31,  30, // '@'  64
	124,126,  19,  19, 126, 124,   0, // 'A'  65
	65, 127, 127,  73,  73, 127,  54, // 'B'  66
	28,  62,  99,  65,  65,  99,  34, // 'C'  67
	65, 127, 127,  65,  99,  62,  28, // 'D'  68
	65, 127, 127,  73,  93,  65,  99, // 'E'  69
	65, 127, 127,  73,  29,   1,   3, // 'F'  70
	28,  62,  99,  65,  81, 115, 114, // 'G'  71
	127,127,   8,   8, 127, 127,   0, // 'H'  72
	0,   65, 127, 127,  65,   0,   0, // 'I'  73
	48, 112,  64,  65, 127,  63,   1, // 'J'  74
	65, 127, 127,   8,  28, 119,  99, // 'K'  75
	65, 127, 127,  65,  64,  96, 112, // 'L'  76
	127,127,  14,  28,  14, 127, 127, // 'M'  77
	127,127,   6,  12,  24, 127, 127, // 'N'  78
	28,  62,  99,  65,  99,  62,  28, // 'O'  79
	65, 127, 127,  73,   9,  15,   6, // 'P'  80
	30,  63,  33, 113, 127,  94,   0, // 'Q'  81
	65, 127, 127,   9,  25, 127, 102, // 'R'  82
	38, 111,  77,  89, 115,  50,   0, // 'S'  83
	3,   65, 127, 127,  65,   3,   0, // 'T'  84
	127,127,  64,  64, 127, 127,   0, // 'U'  85
	31,  63,  96,  96,  63,  31,   0, // 'V'  86
	127,127,  48,  24,  48, 127, 127, // 'W'  87
	67, 103,  60,  24,  60, 103,  67, // 'X'  88
	7,   79, 120, 120,  79,   7,   0, // 'Y'  89
	71,  99, 113,  89,  77, 103, 115, // 'Z'  90
	0,  127, 127,  65,  65,   0,   0, // '['  91
	1,    3,   6,  12,  24,  48,  96, // '\'  92
	0,   65,  65, 127, 127,   0,   0, // ']'  93
	8,   12,   6,   3,   6,  12,   8, // '^'  94
	128,128, 128, 128, 128, 128, 128, // '_'  95
	0,    0,   3,   7,   4,   0,   0, // '`'  96
	32, 116,  84,  84,  60, 120,  64, // 'a'  97
	65, 127,  63,  72,  72, 120,  48, // 'b'  98
	56, 124,  68,  68, 108,  40,   0, // 'c'  99
	48, 120,  72,  73,  63, 127,  64, // 'd' 100
	56, 124,  84,  84,  92,  24,   0, // 'e' 101
	72, 126, 127,  73,   3,   2,   0, // 'f' 102
	56, 188, 164, 164, 252, 120,   0, // 'g' 103
	65, 127, 127,   8,   4, 124, 120, // 'h' 104
	0,   68, 125, 125,  64,   0,   0, // 'i' 105
	96, 224, 128, 128, 253, 125,   0, // 'j' 106
	65, 127, 127,  16,  56, 108,  68, // 'k' 107
	0,   65, 127, 127,  64,   0,   0, // 'l' 108
	120,124,  28,  56,  28, 124, 120, // 'm' 109
	124,124,   4,   4, 124, 120,   0, // 'n' 110
	56, 124,  68,  68, 124,  56,   0, // 'o' 111
	0,  252, 252, 164,  36,  60,  24, // 'p' 112
	24,  60,  36, 164, 248, 252, 132, // 'q' 113
	68, 124, 120,  76,   4,  28,  24, // 'r' 114
	72,  92,  84,  84, 116,  36,   0, // 's' 115
	0,    4,  62, 127,  68,  36,   0, // 't' 116
	60, 124,  64,  64,  60, 124,  64, // 'u' 117
	28,  60,  96,  96,  60,  28,   0, // 'v' 118
	60, 124, 112,  56, 112, 124,  60, // 'w' 119
	68, 108,  56,  16,  56, 108,  68, // 'x' 120
	60, 188, 160, 160, 252, 124,   0, // 'y' 121
	76, 100, 116,  92,  76, 100,   0, // 'z' 122
	8,    8,  62, 119,  65,  65,   0, // '{' 123
		0,    0,   0, 119, 119,   0,   0, // '|' 124
	65,  65, 119,  62,   8,   8,   0, // '}' 125
	2,    3,   1,   3,   2,   3,   1, // '~' 126
	0b11011011,0b11011011,0b00011000,0b00011000,0,255,255, // HTL H 127
	3,3,255,255,0,3,3, // HTL T 128
	255,255,0,0b11000000,0b11000000,0b11000000,0b11000000 // HTL L 129
};
/*************************************************************************/
#endif

// Lighweight code for conversion from unsigned int to Text
void _loc_uint2txt(uint32_t  BinData, char * TextBuffer, char NDigit)
{
	uint32_t Divisor;
	uint8_t Result;
	uint8_t	DigitCnt;
	
	/* Init the Algorithm */
	Divisor=1000000000;
	
	for(DigitCnt=10;DigitCnt>0;DigitCnt--)
	{
		Result=(BinData/Divisor);
		BinData=BinData-Result*Divisor;
		Divisor=Divisor/10;
		if (DigitCnt<=NDigit)
		{
			TextBuffer[NDigit-DigitCnt]='0'+Result;
		}
	}
}
/****************************************************************************************/


/****************************************************************/
/* HW-Abhängige Funktionen                                      */
/* Greifen nur auf die HW zu ohne Veränderung der Speicherdaten */
/****************************************************************/

#ifdef DISP_MEGACARD

// interne Funktion: Ausgabe eines 4-Bit Halbbytes auf dem Bus _hw_zToLCD
// Für die Übertragung eines Bytes muss die Funktion 2x aufgerufen werden 
// Die zu übertragenen Informationen befinden sich in den 4 unteren Bits
// Bit 4 zeigt an ob Daten (=1) übertragen werden oder Befehle (=0)
// Für die Anzeige eines Zeichens muss die Funktion _hw_CharToDisplay verwendet werden.
// Die Funktion unterstützt zwei Modi: die Ausgabe über PORTAB oder die Ausgabe über ein
// Port Extender MCP23S08 am SPI Bus (via #define DISP_MC_NEU) 

// Change: dataD hält ein ganzes Byte anstelle eines Halb-Bytes
// Es werden im Falle eines 4-Bit interfaces immer 2 Aufrufe daraus gemacht.
void _hw_zToLCD(char dataD, uint8_t IsData)
{
		//uint8_t Data = 0;

	
#ifdef DISP_MC_NEU

		// use the new Megacard Display

		// Display Enable auf 1
		spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_DEVICE_ADRESS_WRITE,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_REG_ADRESS_GPIO,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,(1<<DIPS_EN_BIT)|(1<<DIPS_LED_BIT),MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,1);

		_delay_ms(WAIT_2);	
		
		// Datenübertragung setzen
		Data=(dataD&0x0F);
		if (dataD&0x10)
		{
			// Es werden Daten übertragen
			Data=Data|(1<<DIPS_NCD_BIT)|(1<<DIPS_EN_BIT)|(1<<DIPS_LED_BIT);
		}
		else
		{
			// Es werden Commandos übertragen
			Data=Data|(1<<DIPS_EN_BIT)|(1<<DIPS_LED_BIT);
		}
		spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_DEVICE_ADRESS_WRITE,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_REG_ADRESS_GPIO,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,Data,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,1);
		
		_delay_ms(WAIT_2);
		
		// Fallende Flanke an EN -> Befehl ausführen
		Data&=~(1<<DIPS_EN_BIT);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_DEVICE_ADRESS_WRITE,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_REG_ADRESS_GPIO,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
		spi_TransferWait(MCP23S08_SPI_CHANNEL,Data,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,1);
		
		_delay_ms(WAIT_2);
	
#else 	
	
	// Ansteuerung des Displays über die normalen Interface - Leitungen
	
	// Übertragen des ersten Halbbytes (Höherwertiges zuerst)

	// Zuerst werden die relevanten PORT-PINS gelöscht
	PORTA&=~ (1<<DISP_RS | 1<<DISP_EN);
	
	// Datenleitungen auf Ausgang setzen und löschen
	//DDRB|=(1<<DISP_DB4 | 1<<DISP_DB5 | 1<<DISP_DB6 | 1<<DISP_DB7);
	PORTB&=~(1<<DISP_DB4 | 1<<DISP_DB5 | 1<<DISP_DB6 | 1<<DISP_DB7);

	// und wieder gesetzt entsprechend den Daten
	if (dataD & 0x10) PORTB|=1<<DISP_DB4;
	if (dataD & 0x20) PORTB|=1<<DISP_DB5;
	if (dataD & 0x40) PORTB|=1<<DISP_DB6;
	if (dataD & 0x80) PORTB|=1<<DISP_DB7;
	
	// Setze das Daten/Command flag
	if (IsData)
	{
		PORTA|=1<<DISP_RS;
	}

	// EN High->Low ausführen des Befehls
	PORTA|=1<< DISP_EN;
	_delay_us(1);

	// Fallende Flanke an EN -> Ausführen des Befehls
	PORTA &= ~(1<< DISP_EN);
	_delay_us(1);
		

	// Jetzt folgt das 2. Halbbyte

	// Zuerst werden die relevanten PORT-PINS gelöscht
	PORTA&=~ (1<<DISP_RS | 1<<DISP_EN);
	PORTB&=~(1<<DISP_DB4 | 1<<DISP_DB5 | 1<<DISP_DB6 | 1<<DISP_DB7);

	// und wieder gesetzt entsprechend den Daten
	if (dataD & 0x01) PORTB|=1<<DISP_DB4;
	if (dataD & 0x02) PORTB|=1<<DISP_DB5;
	if (dataD & 0x04) PORTB|=1<<DISP_DB6;
	if (dataD & 0x08) PORTB|=1<<DISP_DB7;
	if (IsData)
	{
		PORTA|=1<<DISP_RS;
	}
	
	// EN High->Low ausführen des Befehls
	PORTA|=1<< DISP_EN;
	_delay_us(1);

	// Fallende Flanke an EN -> Ausführen des Befehls
	PORTA &= ~(1<< DISP_EN);
	_delay_us(1);
	
	// Die Datenleitungen zurück auf Eingang schalten wegen USB
	// DDRB&= ~(1<<DISP_DB7 | 1<<DISP_DB6 | 1<<DISP_DB5 | 1<<DISP_DB4);

	
	
#endif
}


// Cursor auf 0/0 setzen (DD-RAM)
// Keine Veränderung der SPeicherdaten
void _hw_Home(void)
{
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x00);      	// LCD-Return-Home 1
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x02);       // LCD-Return-Home 2
	//_delay_ms(WAIT_2);

	_hw_zToLCD(0b00000010,0);
	_delay_ms(2);
}

 // interne Funktion: die Pins und den Display Controller konfigurieren 
 // Die Funktion konfiguriert nur die HW. Keine Zugriffe auf das Memory
void _hw_Init(void)
{

#ifdef DISP_MC_NEU
	// Für das neue Display
	// Initialisieren des Port Expanders (Alle Bits sind Ausgänge -> Schreibe 0x00 an Reg. 0x00) 
	spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_DEVICE_ADRESS_WRITE,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
	spi_TransferWait(MCP23S08_SPI_CHANNEL,MCP23S08_REG_ADRESS_DDR,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,0);
	spi_TransferWait(MCP23S08_SPI_CHANNEL,0x00,MCP23S08_SPI_NCYCLES,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,1);
#else
	// Für das alte Display:
	
	// Beid er Initialisierung muss ein erster Befehl als 8-Bit Kommando ausgegeben werden.
	// Danach folgen nur noch 4-Bit Ausgaben, die über die _hw_zToLCD() abgewicjelt werden.

	// 50ms Warten (Datenblatt)
	_delay_ms(50);

	
	// Display Steuerleitungen als Ausgang konfigurieren
	// Die DB-Leitungen werden im High-Z gehalten und jeweils bei der Datenausgabe gesetzt
	// Damit bleibt die USB Schnittstelle aktiv
	DDRA|= 1<<DISP_RS | 1<<DISP_EN ;
	DDRB|= 1<<DISP_DB7 | 1<<DISP_DB6 | 1<<DISP_DB5 | 1<<DISP_DB4;
	
	
	// Umschalten auf 4-Bit Display
	// Datenleitungen löschen
	PORTB&=~(1<<DISP_DB7 | 1<<DISP_DB6 | 1<<DISP_DB5 | 1<<DISP_DB4);
	// RS=0 und EN =0 
	PORTA&=~(1<<DISP_RS | 1<<DISP_EN);
	
	// Datenleitungen vorbereiten (Datenblatt Controller)
	PORTB|=(1<<DISP_DB4) | (1<<DISP_DB5);
	
	// Den Schreib-Befehl ausführen
	PORTB|=(1<<DISP_EN);
	_delay_us(1);
	PORTB&=~(1<<DISP_EN);
	
	// Die Datenleitungen zurück auf Eingang schalten wegen USB 
	// DDRB&= ~(1<<DISP_DB7 | 1<<DISP_DB6 | 1<<DISP_DB5 | 1<<DISP_DB4);
	
	// Wait laut Datenblatt
	_delay_us(40);
	  
	
#endif

	// ab hier ist die initialisierung ident für beide Displays.

	// Register des Displays setzen
	//_delay_ms(WAIT_2);
	//Alte Version -------------------------------------------------------
	//_hw_zToLCD(0x03);       // Function Set A
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x43);       // Function Set B
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x03);       // Function Set C
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x02);       // Function Set D
	//_delay_ms(WAIT_2);
	//
	//_hw_zToLCD(0x02);       // Funktion Set High Byte
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x08);
	//_delay_ms(WAIT_2);       
	//
	//_hw_zToLCD(0x00);       // Display On 1 High Byte
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x0C);       // Display On 2 Low Byte
	//_delay_ms(WAIT_2);	
//
	//_hw_zToLCD(0x00);       // Display Clear High Byte
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x01);       // Display Clear Low Byte
	//_delay_ms(WAIT_2);	
//
	//_hw_zToLCD(0x00);       // Entry Mode Set High Byte
	//_delay_ms(WAIT_2);
	//_hw_zToLCD(0x06);       // Entry Mode Set Low Byte
	//_delay_ms(WAIT_2);
//
	//_hw_zToLCD(0x00);
	//_hw_zToLCD(0x0F);
	//Alte Version Ende -------------------------------------------------------

	// Neue Version (2021):
	// Function Set: N=1 (2-Zeilen DIsplay) F=0 (5x8 Pixel) DL=0 (4-Bit Display) 
	_hw_zToLCD(0b00101000,0);
	_delay_us(40);
	
	_hw_zToLCD(0b00101000,0);
	_delay_us(40);
	
	
	
	
	// On/Off
	// D=1 turn on, D=0 turn off
	// C=0 no cursor, C=1 cursor
	// B=1 blink on, B=0 blink off
	// 0b00001DCB
	_hw_zToLCD(0b00001100,0);
	_delay_us(40);
	 		
	// Cursor or Display shift
	// S/C R/L is set to 0
			 
	// Clear Display
	_hw_zToLCD(0b00000001,0);
	_delay_ms(2);
			 
	// ENtry Mode
	// I/D=1 cursor right & increase DDRAM, I/D=0 go left and decrease DDRAM
	// S=1 Perform shift S=0 no shift
	_hw_zToLCD(0b00000110,0);
	_delay_ms(2);

	_hw_Home();
	
}

// Setzt den Cursor an eine entsprechende X/Y Position         
void _hw_Pos(uint8_t x, uint8_t y)
{
	//unsigned char Zeichen;

	if ((x<8) & (y<2))
	{
		if (y!=0) x+=0x40;
		
		// ALte Version
		//Zeichen=x;			     //Ausgabe der DD-Ram-Adresse
		//Zeichen>>=4;
		//Zeichen|=0x08;
		//_hw_zToLCD(Zeichen);    //Zeichenausgabe High Bytes
		//_delay_ms(1);           //Wartezeit 1ms
		//Zeichen=x;
		//Zeichen&=0x0F;
		//_hw_zToLCD(Zeichen);    //Zeichenausgabe Low Byte
		//_delay_ms(1);			//Wartezeit 1ms
		// Neue Version
		_hw_zToLCD(x|0x80,0);
		_delay_us(50);
	}	
	
}

// Low-Level Ausgabe eines Zeichens auf dem Display-Bus 
void _hw_CharToDisplay(uint8_t c)
{
	unsigned char Zeichen;

	Zeichen = (unsigned char)c;
	_hw_zToLCD(Zeichen,1);  //Zeichenausgabe High Bytes
	_delay_us(50);          
}

#endif

#ifdef DISP_NOKIA

#include "zkslibspi.h"

#define NOKIA_SET_CD (NOKIA_CONTROL_PORT|=(1<<NOKIA_CD_BIT))
#define NOKIA_CLEAR_CD (NOKIA_CONTROL_PORT&=~(1<<NOKIA_CD_BIT))

// CS Kontrolle übernimmt in der neuen Version die Schnittstelle
//#define NOKIA_SET_CS (NOKIA_CONTROL_PORT|=(1<<NOKIA_CS_BIT))
//#define NOKIA_CLEAR_CS (NOKIA_CONTROL_PORT&=~(1<<NOKIA_CS_BIT))

#define NOKIA_SET_RST (NOKIA_CONTROL_PORT|=(1<<NOKIA_RST_BIT))
#define NOKIA_CLEAR_RST (NOKIA_CONTROL_PORT&=~(1<<NOKIA_RST_BIT))

#define NOKIA_ROWS DISP_LINES
#define NOKIA_COLS DISP_COLS
#define NOKIA_PIXEL_X 84
#define NOKIA_PIXEL_Y 48
#define NOKIA_FIRST_USER_LINE 1
#define NOKIA_USER_LINES 6
#define NOKIA_HEADLINE "HTL Rankweil"
#define NOKIA_HEADLINE_LEN 12
#define NOKIA_FOOTLINE "zksLibNokia2020"
#define NOKIA_FOOTLINE_LEN 14

#define NOKIA_X_BASE 0x80
#define NOKIA_Y_BASE 0x40




// internal function declarations
void _hw_nokia_clear_full(void);
void _hw_nokia_pos_full(unsigned char col, unsigned char row);

// Sendet eine Kommando an die SPI Schnittstelle
void _hw_NokiaCmdWrite(uint8_t Data)
{
	// Set to Data MOde
	NOKIA_CLEAR_CD;
	// Select Display
	//NOKIA_CLEAR_CS;
	//Send Data
	//spi_TransferWait(Data);
	spi_TransferWait(NOKIA_SPI_CHANNEL,Data,8,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,1);
	//unselect Display
	//NOKIA_SET_CS;
	// Set to Data Mode
	NOKIA_SET_CD;
}

void _hw_NokiaDataWrite(uint8_t Data)
{
	// Default is data mode so CD kept high
	NOKIA_SET_CD;
	// Select Display
	//NOKIA_CLEAR_CS;
	//Send Data
	//spi_TransferWait(Data);
	spi_TransferWait(NOKIA_SPI_CHANNEL,Data,8,SPI_CLKDIV_4,SPI_MODE_0,SPI_MSB_FIRST,1);
	//unselect Display
	//NOKIA_SET_CS;
}

// Clears the Display Memory and resets the Cursor to 0/0
void _hw_NokiaClearDisplay(void)
{
	uint16_t Cnt;
	
	// Set the Data Pointer to 0/0
	_hw_NokiaCmdWrite(NOKIA_X_BASE);
	_hw_NokiaCmdWrite(NOKIA_Y_BASE);
	
	// Write 504 Bytes 0x00 to Display Ram
	for(Cnt=0;Cnt<504;Cnt++)
	{
		_hw_NokiaDataWrite(0x00);
	}
	
	// Set the Data Pointer to 0/0
	_hw_NokiaCmdWrite(NOKIA_X_BASE);
	_hw_NokiaCmdWrite(NOKIA_Y_BASE);
	
};

/*********************************************************************/
void _hw_Init(void)
{
	// The SPI Interface is assumed to be initialized
	
	// init control port
	NOKIA_DDR_PORT|=(1<<NOKIA_RST_BIT)|(1<<NOKIA_CD_BIT);
	//NOKIA_CONTROL_PORT|=(1<<NOKIA_RST_BIT)|(1<<NOKIA_SET_CD)|(1<<NOKIA_SET_CS);
	
	
	// Default level is high for all control lines
	NOKIA_SET_RST;
	NOKIA_SET_CD;
	//NOKIA_SET_CS;
	_delay_ms(10);

	// Reset the device
	NOKIA_CLEAR_RST;
	_delay_ms(100);
	NOKIA_SET_RST;
	_delay_ms(100);
	
	// Aktivate the Display in Extended command mode
	
	_hw_NokiaCmdWrite(0x21);
	
	
	// Adjust the contrast
	_hw_NokiaCmdWrite(0x8f);
	
	// Switch back to normal command mode
	_hw_NokiaCmdWrite(0x20);
	
	// Set normal display mode
	_hw_NokiaCmdWrite(0x0c);
		
	// Clear the Display
	_hw_NokiaClearDisplay();
	
}

/*************************************************************/
/* Sets the next Output to position(col, row)	       		 */
void _hw_Pos(unsigned char col, unsigned char row)
{
	// catch invalid parameters
	if(row>=NOKIA_ROWS) row=(NOKIA_ROWS-1);
	if(col>=NOKIA_COLS) col=(NOKIA_COLS-1);
	
	// Set Display Address pointer accordingly
	_hw_NokiaCmdWrite(NOKIA_X_BASE+(col<<3));
	_hw_NokiaCmdWrite(NOKIA_Y_BASE+row);
}


/*************************************************************/
/* Sets the next Output to position(0,0) of user space					 */
void _hw_Home(void)
{
	_hw_Pos(0,0);
}


/*************************************************************/
// clear the Display by writing zeros to all pages and display
// Headline and Footline

// Clear the user space
void _hw_Clear()
{
	_hw_NokiaClearDisplay();
}


/*************************************************************/
/* Display character at current position in user space                    */
void _hw_CharToDisplay (uint8_t c)
{
	uint8_t cnt;
	
	// fix data out of range
	if (c<FONT_CHAR_FIRST) c=FONT_CHAR_FIRST;
	if (c>FONT_CHAR_LAST) c=FONT_CHAR_LAST;
	
	// Adjust data to become an index into fontData
	c=c-FONT_CHAR_FIRST;
	
	// Display character c at current position
	for (cnt=0;cnt<7;cnt++)
	{
		_hw_NokiaDataWrite (fontData7x8[c][cnt]);
	}
	
	// Add a vertical empty line for the spaces
	_hw_NokiaDataWrite (0);
	
}
#endif


/* Ende der HW-Abhängigen Funktionen						   */	
/***************************************************************/



/***************************************************************************/
/* HW-Unabhängige low level Funktionen, diese sind für alle Displays ident */


// Folgende Funktionen müssen HW-seitig zur Verfügung stehen:
// _hw_Home, _hw_Init, _hw_CharToDisplay, _hw_Pos 

// Wrapper für die callbackfunktion zur AUsgabe eines Zeichens auf dem Display
// Damit werden Compiler-Warnungen vermieden.
int	_disp_put(char c, FILE * f)
{
	display_CharToDisplay(c);
	return 0;
}

// Schreibt Spaces in die angebene Zeile im Datenspeicher
// Ändert sonst nichts, kein HW Zugriff.
// Änderungen werden erst beim nächsten Refresh sichtbar.
void _loc_ClearLine(uint8_t IxLine)
{
	uint8_t Cnt = 0;
	uint8_t Ix = 0;

	// Prüfe ob Linienindex zulässig
	if(IxLine<DISP_LINES)
	{
		// Überschreiben des Speichers
		Ix=IxLine*DISP_COLS;
		for(Cnt=0;Cnt<DISP_COLS;Cnt++)
		{
			_loc_DispData[Ix]=' ';
			Ix++;
		}
	}

}


// Refresh des Displays basierend auf dem IxHome Pointer auf die Erste Zeile im
// Display Memory. Der HW Cursor wird am Ende wieder auf die aktuelle Position gesetzt
void _loc_Refresh(void)
{
	//lokale Variablen 
	uint8_t CntLines;
	uint8_t CntCols;
	uint8_t Ix=0;
		
	// Ix Zeigt auf das erste zu übertragende Zeichen 
	
	for (CntLines=0;CntLines<DISP_LINES;CntLines++)
	{
		// Den Cursor auf den Beginn der nächsten zeile setzen
		_hw_Pos(0,CntLines);
		
		for(CntCols=0;CntCols<DISP_COLS;CntCols++)
		{
			// Zeichen ausgeben und Zähler erhöhen
			_hw_CharToDisplay(_loc_DispData[Ix]);
			Ix++;
		}
	}
	
	// Cursor wieder an die aktuelle stelle setzen
	_hw_Pos(_loc_X,_loc_Y);
}


// Das Display wird um eine Zeile nach oben verschoben und erneut angezeigt
// Der Cursor wird nicht verändert.
void _loc_ScrollUp(void)
{
	uint8_t kx;
	
	// Kopieren der Zeilen unteren N Zeilen um eine Zeile nach oben im Datenspeicher
	for(kx=0;kx<((DISP_LINES-1)*DISP_COLS);kx++)
	{
		_loc_DispData[kx]=_loc_DispData[kx+DISP_COLS];
	}
	
	// Löschen der untersten Zeile
	_loc_ClearLine(DISP_LINES-1);
	
	// Display Inhalt aktualisieren
	_loc_Refresh();
}

void _loc_PrintDispData(void)
{
	uint8_t Cnt1,Cnt2,ix;
	
	ix=0;
	for(Cnt1=0;Cnt1<DISP_LINES;Cnt1++)
	{
		printf("\nd(%d)=",Cnt1);
		for(Cnt2=0;Cnt2<DISP_COLS;Cnt2++)
		{
			printf(" %d ",_loc_DispData[ix]);
			ix++;
		}
	}
}

/* ENde der HW-unabhängigen Low Level Funktionen                 */
/*****************************************************************/



/**************************************************************************/
/* Hier folgen die Bibliotheksfunktionen                                  */
/* Diese greifen ausschliesslich auf Low Level Funktionen zu              */

// Init the display controller
void display_Init(void)
{
	uint8_t Cnt;

	// Intitialisieren des Controllers	
	_hw_Init();
	
	// Initialisieren der internen Variablen und des internen Memories
	for(Cnt=0;Cnt<DISP_LINES;Cnt++)
	{
		_loc_ClearLine(Cnt);
	}
	
	_loc_X=0;
	_loc_Y=0;
	_loc_Ix=0;
	
	
	// Konfigurieren des stdout Kanals
#ifdef STDOUT_DISP	
	display_Clear();
	stdout = &disp_str;	
#endif	
	
	//_loc_PrintDispData();
	
}

// Löscht das gesamt Display und setzt den Cursor oben links (Home)
// Schreibt spaces in das Memory
// Setzt den Cursor auf 0/0
// Updated das Display
void display_Clear(void)
{
	uint8_t Cnt;
	
	// Alle Zeilen mit Spaces füllen
	for(Cnt=0;Cnt<DISP_LINES;Cnt++)
	{
		_loc_ClearLine(Cnt);
	}

	// Den Memory Inhalt ausgeben
	_loc_Refresh();

	// zuletzt wird der Cursor auf Anfang gesetzt
	display_Home();
	
	
}

// Den Cursor an die Position (0,0) verschieben
void display_Home(void)
{
	// Aufruf der Low-Level Funktion
	_hw_Home();
	
	// Die Datenpointer richtig setzen
	_loc_X=0;
	_loc_Y=0;
	_loc_Ix=0;
}

// Den Cursor an eine beliebige Position im Display verschieben
// Bei Überschreiten der Limite bleibt der Cursor unverändert
// X-Position von links nach rechts, y-Position von oben nach unten
// Zähler beginnt bei 0
void display_Pos(unsigned char x, unsigned char y)
{
	if((x<DISP_COLS)&&(y<DISP_LINES))
	{
		// Aufruf der Low-Level Funktion
		_hw_Pos(x,y);
		
		// Update der Datenpointer
		_loc_Y=y;
		_loc_X=x;
		_loc_Ix=y*DISP_COLS+x;
		
	}
	
}

// Ein Zeichen an der aktuellen Position ausgeben. 
// Aufruf der entsprechenden internen Funktion
// Das Zeichen im Speicher eintragen
// Wenn nötig, scrollen
// Am Ende zeigen Pointer und HW-Pos auf das nächste freie Zeichen, d.h.
// Die Inkrementierung und Prüfung erfolgt am Ende
void display_CharToDisplay (uint8_t c)
{
	
	switch (c)
	{
		case ASCII_CR:
			// Den Cursor auf den aktuellen Zeilenanfang verschieben ohne
			// Zeilenvorschub
			_loc_Ix-=_loc_X;
			_loc_X=0;
			_hw_Pos(_loc_X,_loc_Y);
		break;
		
		case ASCII_LF:
			// Den Cursor an den Beginn der nächsten Zeile verschieben
			_loc_Y++;
			
			// Display-Ende erreicht ?
			if (_loc_Y>=DISP_LINES)
			{
				_loc_ScrollUp();
				_loc_X=0;
				_loc_Y=DISP_LINES-1;
				_loc_Ix=DISP_LEN-DISP_COLS;
			}
			else
			{
				_loc_Ix=(_loc_Ix+DISP_COLS)-_loc_X;
				_loc_X=0;
			}
			_hw_Pos(_loc_X,_loc_Y);
		break;
		default:
			// Ausgabe im Display und Eintrag im Datenspeicher
			// Merke: die Pointer zeigen zu Beginn  immer auf die nächste freie Position
			// Diese kann immer bedenkenlos geschrieben werden
			// Danach werden die Pointer aktualisiert und geprüft
			
			// Änderung in Subversion 5
			// Die Prüfung der Pointer und das Scrollen wird erst ausgeführt,
			// wenn das nächste Zeichen ausgegeben werden soll. Damit wird verhindert,
			// dass beim letzten Zeichen bereits die Zeile nach oben verschwindet.
			
			// Zeilenende erreicht ?
			if (_loc_X>=DISP_COLS)
			{
				_loc_Y++;
				_loc_X=0;
				
				// Display-Ende erreicht ?
				if (_loc_Y>=DISP_LINES)
				{
					_loc_ScrollUp();
					_loc_Y=DISP_LINES-1;
					_loc_Ix=DISP_LEN-DISP_COLS;
				}
				_hw_Pos(_loc_X,_loc_Y);
			}
			
			// Jetzt erfolgt die Ausgabe des Zeichens und die Speicherung im internen Mem
			_hw_CharToDisplay(c);
			_loc_DispData[_loc_Ix]=c;			
	
			// Inkremetieren der Pointer, danach werden die Werte geprüft
			_loc_X++;
			_loc_Ix++;

			
		break;
	}
	
	//_loc_PrintDispData();
	// AM Ende der Funktion sind die Pointer aktuell aber noch nicht geprüft
	
}

void display_Test(void)
{
	uint8_t Cnt;
	
	// Fill Data Memory with Test Data
	//for(Cnt=0;Cnt<DISP_LEN;Cnt++)
	//{
	//	_loc_DispData[Cnt]='B'+Cnt;
	//}
	
	
	for(Cnt=0;Cnt<20;Cnt++)
	{
		display_CharToDisplay('A'+Cnt);
		_delay_ms(500);	
	}
	
	
	//_loc_IxHome++;
	_loc_Refresh();
	_delay_ms(500);

	
	//_loc_ClearLine(1);
	//_loc_Refresh();
	//_delay_ms(500);
	
	
	//_loc_IxHome++;
	
	//_loc_Refresh();
	//_delay_ms(500);
	
	//_loc_IxHome=0;
	//_loc_Refresh();
	//_delay_ms(500);
	
	
	//_loc_ScrollUp();
	//display_CharToDisplay('A');
	
	
	//_hw_CharToDisplay('X');
	
}

// Ausgabe eines Strings auf dem Display. keine Prüfung des Speichers
void display_TxtToDisplay(char *txt, unsigned char len)
{
	unsigned char cnt;
	
	for(cnt=0;cnt<len;cnt++)
	{
		display_CharToDisplay(txt[cnt]);
	}
}

// Ausgabe einer Integer-Variable als Zeichenfolge auf dem Display an der aktuellen Position
// x: Variable, N: Anzahl Stellen inklusive Vorzeichen
// es wird immer eine Vorzeichen ausgegeben + oder -.
// N muss >= 2 sein und <=12 sein, sonst passiert gar nichts
void display_IntToDisplay(int32_t x, char N)
{
	char Text[13];
	if ((N>=2) && (N<=12))
	{
		if (x>=0)
		{
			_loc_uint2txt(x, Text, N-1);
			display_CharToDisplay('+');
			display_TxtToDisplay(Text,N-1);
		}
		else
		{
			_loc_uint2txt(-x, Text, N-1);
			display_CharToDisplay('-');
			display_TxtToDisplay(Text,N-1);
		}
	}
}

// AUsgabe einer Unsigned Integer-Variable als Zeichenfolge auf dem Display an der aktuellen Position
// x: Variable, N: Anzahl Stellen 
// N muss >=1 und <=12 sein
void display_UintToDisplay(uint32_t x, char N)
{
	char Text[12];
	if ((N>=2) && (N<=12))
	{
		_loc_uint2txt(x, Text, N);
		display_TxtToDisplay(Text,N);
	}
}


/* Ende der Bibliotheksfunktionen                                         */
/**************************************************************************/