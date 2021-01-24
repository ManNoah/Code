/************************************************************/
/* Basisfunktionen für die Steuerung der SPI Schnittstelle  */
/*															*/
/* Autor: ZKS												*/
/*															*/
/*															*/
/* Versionsinfos:											*/
/* 31.8.2020, Initial release V1							*/
/************************************************************/
#define ZKSLIBSPI 202105
#include <avr/io.h>

/************************************************************/
// HW-unabhängige Defines

// Anzahl möglicher SPI Geräte
#define SPI_MAX_DEVICES 4

// Rückgabe-Ergebnisse
#define SPI_OK 1
#define SPI_ERR 0

// Erro-Codes
#define SPI_ERR_OK 0
#define SPI_ERR_STAT 1
#define SPI_ERR_TMO 2
#define SPI_ERR_DATA 3
#define SPI_ERR_CHANNEL 4

// Mögliche Optionen für die Clock-Auswahl
#define SPI_CLKDIV_4	0
#define SPI_CLKDIV_16	1
#define SPI_CLKDIV_64	2
#define SPI_CLKDIV_128	3

// Mögliche Optionen fpr die Mode Auswahl
#define SPI_MODE_0	0
#define SPI_MODE_1	1
#define SPI_MODE_2	2
#define SPI_MODE_3	3

// Mögliche Optionen für die Bitreihenfolge
#define SPI_MSB_FIRST 1
#define SPI_LSB_FIRST 0

// Status-Codes
#define SPI_STAT_CLOSED 0
#define SPI_STAT_READY 1

// Mögliche Bit-Längen für die Übertragung
#define SPI_8BIT  0
#define SPI_16BIT 1
#define SPI_24BIT 2
#define SPI_32BIT 3

/************************************************************/



/************************************************************/
// HW-abhängige Defines
#ifdef DEVICE_ATMEGA16
	// Leitungszuordnung für die HW-SPI Schnittstelle
	#define USE_HW_SPI
	#ifdef USE_HW_SPI
		#define HW_SPI_SCK PORTB7
		#define HW_SPI_MISO PORTB6
		#define HW_SPI_MOSI PORTB5
		#define HW_SPI_PORT PORTB
		#define HW_SPI_DDR DDRB
	#endif

	// Leitungszuordnung für die SW-SPI Schnittstelle
	// Hinweis: für die SW-SPI hat der Clk-Parameter keine Wirkung, dies arbeitet immer mit dem maximalen Clock 
	#define USE_SW_SPI
	#ifdef USE_SW_SPI
		#define SW_SPI_SCK PORTC0
		#define SW_SPI_MISO PORTC2
		#define SW_SPI_MOSI PORTC1
		#define SW_SPI_PORT PORTC
		#define SW_SPI_PIN PINC
		#define SW_SPI_DDR DDRC
	#endif
	
	// Optionen für die Wahl der Schnittstelle
	#define SPI_HW 0
	#define SPI_SW 1
	
	// Einstellungen für die Chip Select Leitungen der  4 SPI Kanäle
	// Wenn der jeweilige Kanal benutzt wird, muss der Abschnitt per #define USE.. aktiviert werden
	// Diese Methode wird verwendet, um Code und Speicher zu sparen
	#define USE_SPI0
	#ifdef USE_SPI0
		#define SPI0_HWSW SPI_SW
		#define SPI0_CS_PORT PORTC
		#define SPI0_CS_DDR DDRC
		#define SPI0_CS_PIN 3
	#endif
	
	#define USE_SPI1
	#ifdef USE_SPI1
		#define SPI1_HWSW SPI_SW 
		#define SPI1_CS_PORT PORTC
		#define SPI1_CS_DDR DDRC
		#define SPI1_CS_PIN 3
	#endif
		
	#undef USE_SPI2
	#ifdef USE_SPI2
		#define SPI2_HWSW SPI_SW
		#define SPI2_CS_PORT PORTC
		#define SPI2_CS_DDR DDRC
		#define SPI2_CS_PIN 4
	#endif
	
	#undef USE_SPI3
	#ifdef USE_SPI3
		#define SPI3_HWSW SPI_SW
		#define SPI3_CS_PORT PORTC
		#define SPI3_CS_DDR DDRC
		#define SPI3_CS_PIN 5
	#endif
			
	
#endif

#ifdef DEVICE_ATMEGA328
// Leitungszuordnung für die HW-SPI Schnittstelle
#define USE_HW_SPI
	#ifdef USE_HW_SPI
		#define HW_SPI_SCK PORTB5
		#define HW_SPI_MISO PORTB4
		#define HW_SPI_MOSI PORTB3
		#define HW_SPI_PORT PORTB
		#define HW_SPI_DDR DDRB
	#endif

// Leitungszuordnung für die SW-SPI Schnittstelle
// Hinweis: für die SW-SPI hat der Clk-Parameter keine Wirkung, dies arbeitet immer mit dem maximalen Clock
#define USE_SW_SPI
	#ifdef USE_SW_SPI
		#define SW_SPI_SCK PORTD2
		#define SW_SPI_MISO PORTD4
		#define SW_SPI_MOSI PORTD3
		#define SW_SPI_PORT PORTD
		#define SW_SPI_PIN PIND
		#define SW_SPI_DDR DDRD
	#endif

// Optionen für die Wahl der Schnittstelle
	#define SPI_HW 0
	#define SPI_SW 1

// Einstellungen für die Chip Select Leitungen der  4 SPI Kanäle
// Wenn der jeweilige Kanal benutzt wird, muss der Abschnitt per #define USE.. aktiviert werden
// Diese Methode wird verwendet, um Code und Speicher zu sparen
	#define USE_SPI0
		#ifdef USE_SPI0
		#define SPI0_HWSW SPI_SW
		#define SPI0_CS_PORT PORTD
		#define SPI0_CS_DDR DDRD
		#define SPI0_CS_PIN PORTD5
	#endif

#define USE_SPI1
	#ifdef USE_SPI1
		#define SPI1_HWSW SPI_SW
		#define SPI1_CS_PORT PORTC
		#define SPI1_CS_DDR DDRC
		#define SPI1_CS_PIN 3
	#endif

#undef USE_SPI2
	#ifdef USE_SPI2
		#define SPI2_HWSW SPI_SW
		#define SPI2_CS_PORT PORTC
		#define SPI2_CS_DDR DDRC
		#define SPI2_CS_PIN 4
	#endif

#undef USE_SPI3
	#ifdef USE_SPI3
		#define SPI3_HWSW SPI_SW
		#define SPI3_CS_PORT PORTC
		#define SPI3_CS_DDR DDRC
		#define SPI3_CS_PIN 5
	#endif


#endif
/************************************************************/


/********************************************************************************/
// HW-unabhängige Funktionsprototypen

// Das SPI Modul für den gegebenen Parametersatz initialisieren
// Die Chip-Select Leitung muss vom Anwenderprogramm als Ausgang konfiguriert werden
// Beschreibung der Parameter:
// Der Rückgabewert ist die ID der Schnittstelle. (Für spätere Zwecke, derzeit nicht implementiert) 
// Ein Rückgabewert von 0x00 bedeutet einen Intitialisierungsfehler 
uint8_t spi_Init();

// führt einen Datentransfer aus und sendet die Daten in data über den Bus und liest die eingehenden Daten 
// SPiChannel: welcher Kanal soll verwendet werden
// SpiData: Zu übertragende Daten (max. 32 Bit)
// SpiNCycles: Anzahl der auszuführenden Clock-Cycles bzw. Bits
// SpiClk: Wahl der Taktrate für die HW SPI. Wird beid er SW SPI inoriert
// SpiMode: Legt die Phasen des Taktes fest laut SPI Spezifikation.
// SpiMsbFirst: Legt fest welches Bit zuerst übertragen wird. Wenn 0 beginnt das LSB
// SpiTerminate: wenn 1 wird am Ende des Transfer die Chip-Enable Leitung auf 1 gesetzt. Ansonsten bleibt CE auf 0
// Rückgabe: alle eingelesenen Bits der Übertragung
uint32_t spi_TransferWait(uint8_t SpiChannel, uint32_t SpiData, uint8_t SpiNCycles, uint8_t SpiClk, uint8_t SpiMode, uint8_t SpiMsbFirst, uint8_t SpiTerminate);

// Schliessen der Schnittstelle. Alle Devices werden gelöscht.
// Alle Leitungen gehen auf Eingang
// Nach einem Close müssen alle Devices neu initialisiert werden
void spi_Close();

// Abfrage des zuletzt aufgetretenen Fehlers
// Durch die Abfrage wird die Fehlermeldung gelöscht
uint8_t spi_GetLastError();

// Diese Funktionen werden in Zukunft nicht mehr unterstützt
uint8_t spi_SetSS(); 
uint8_t spi_ClearSS();
/************************************************************/
