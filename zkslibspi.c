/************************************************************/
/* Basisfunktionen für die Steuerung der SPI Schnittstelle  */
/*															*/
/* Autor: ZKS												*/
/*															*/
/*															*/
/*                                                          */        
/* Versionsinfos:											*/
/* 31.8.2020, Initial release V1							*/
/* 6.9.2020, Added SW SPI support                           */
/************************************************************/
#include <avr/io.h>
#include "zkslibspi.h"


// Speicher für die Geräte-Daten
static uint8_t _loc_SpiStatus=SPI_STAT_CLOSED; 
static uint8_t _loc_LastError=SPI_ERR_OK;
static uint8_t _loc_MsbFirst_Cpol_Cpha = 0;


// Standardwerte für die Implementierung
#define SPI_CNT_TMO 10
#define SPI_CNT_DELAY_US 10

/************************************************************/
// HW-unabhängige (interne) Funktionen
void _loc_RecError(uint8_t Err)
{
	#ifdef ERROR_LOG
		printf("SPIERR %d\n",Err);
	#endif
	_loc_LastError=Err;	
}
/************************************************************/


/************************************************************/
// HW-abhängige (interne) Funktionen
#ifdef DEVICE_ATMEGA16

// Makros für die Steuerung der CS Leitung
#ifdef USE_SPI0
#define SPI0_CS_DDR_ON (SPI0_CS_DDR|=(1<<SPI0_CS_PIN))
#define SPI0_CS_DDR_OFF (SPI0_CS_DDR&=~(1<<SPI0_CS_PIN))
#define SPI0_CS_ACTIVE (SPI0_CS_PORT&=~(1<<SPI0_CS_PIN))
#define SPI0_CS_INACTIVE (SPI0_CS_PORT|=(1<<SPI0_CS_PIN))
#endif

#ifdef USE_SPI1
#define SPI1_CS_DDR_ON (SPI1_CS_DDR|=(1<<SPI1_CS_PIN))
#define SPI1_CS_DDR_OFF (SPI1_CS_DDR&=~(1<<SPI1_CS_PIN))
#define SPI1_CS_ACTIVE (SPI1_CS_PORT&=~(1<<SPI1_CS_PIN))
#define SPI1_CS_INACTIVE (SPI1_CS_PORT|=(1<<SPI1_CS_PIN))
#endif

#ifdef USE_SPI2
#define SPI2_CS_DDR_ON (SPI2_CS_DDR|=(1<<SPI2_CS_PIN))
#define SPI2_CS_DDR_OFF (SPI2_CS_DDR&=~(1<<SPI2_CS_PIN))
#define SPI2_CS_ACTIVE (SPI2_CS_PORT&=~(1<<SPI2_CS_PIN))
#define SPI2_CS_INACTIVE (SPI2_CS_PORT|=(1<<SPI2_CS_PIN))
#endif

#ifdef USE_SPI3
#define SPI3_CS_DDR_ON (SPI3_CS_DDR|=(1<<SPI3_CS_PIN))
#define SPI3_CS_DDR_OFF (SPI3_CS_DDR&=~(1<<SPI3_CS_PIN))
#define SPI3_CS_ACTIVE (SPI3_CS_PORT&=~(1<<SPI3_CS_PIN))
#define SPI3_CS_INACTIVE (SPI3_CS_PORT|=(1<<SPI3_CS_PIN))
#endif


// Konfiguration der HW Leitungen für SW und HW SPI
// Die Konfiguration ist Kanalunabhängig
// Konfiguraion der CS-Leitungen erfolgt im HW-unabhängigen Teil
void _loc_spi_Init()
{
		//Set the pin directions for the HW SPI connections 
		#ifdef USE_HW_SPI
			HW_SPI_PORT&=~((1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK));
			HW_SPI_DDR&=~((1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK) | (1<<HW_SPI_MISO));
			HW_SPI_DDR |=  (1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK);
		#endif
		
		//Set the pin directions for the  SW SPI connections
		#ifdef USE_SW_SPI
			SW_SPI_PORT&=~((1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK));
			SW_SPI_DDR&=~((1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK) | (1<<SW_SPI_MISO));
			SW_SPI_DDR |=  (1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK);
		#endif
							
		// all other inits are enabled during the Transfer call	
}

// Die interne SetupTransfer Funktion setzt die Schnitttsellenparameter 
// Das Management der CD Lietungen übernimmt die übergeordnete Funktion
void _loc_spi_SetupTransfer(uint8_t SpiClk, uint8_t SpiMode, uint8_t SpiMsbFirst, uint8_t SpiHwSwSelect)
{

	if(SpiHwSwSelect==SPI_HW)
	{
	#ifdef USE_HW_SPI	
		// Konfigurieren der HW-Schnittstelle
		// Clear the SPCR Register
		SPCR=0x00;
		if (SpiMsbFirst)
		{
			SPCR |= (1<<SPE) | (1<<MSTR); //DORD(MSB), CPOL(0), CPHA(0)
		}
		else
		{
			SPCR |= (1<<SPE) | (1<<MSTR) | (1<<DORD); //DORD(LSB), CPOL(0), CPHA(0)
		}
	
		// Configure the Spi Op Mode
		SPCR |= (SpiMode & 0x03)<<2;
	
		// Configure the clock divider
		SPCR |= (SpiClk & 0x03);
	#endif	
	}
	else 
	{
	#ifdef USE_SW_SPI
		// Konfigurieren der SW-SPI Schnittstelle
		
		// SPeichern der Parameter für den nächsten Datentransfer
		_loc_MsbFirst_Cpol_Cpha=(SpiMsbFirst<<2)+SpiMode;
		
		// Setzen der Leitungen
		if(SpiMode<2)
		{
			// Set Clock line to Low
			SW_SPI_PORT&=~(1<<SW_SPI_SCK);
		}
		else
		{
			// Set Clock line to Hi
			SW_SPI_PORT|=(1<<SW_SPI_SCK);
		}
	#endif	
	}
}

// Die interne TransferWait Funktion 
// startet den Transfer und wartet bis die Daten hinausgeschoben sind.
// Damit sind mehrere aufeinanderfolgende Transfers möglich ohne die Übertragung
// zu unterbrechen
uint8_t _loc_spi_TransferWait(uint8_t SpiData, uint8_t SpiHwSwSelect)
{
	uint8_t SpiDataOut=0;


	if (SpiHwSwSelect==SPI_HW)
	{
		#ifdef USE_HW_SPI	
		/*********************************************************/
		// Execute the Transfer via HW Interface
		// Start the transfer by writing  Data Out
		SPDR=SpiData;
		// Wait until transfer completed.
		// Because the transfer is not HW dependant, no Timeout control is necessary
		while(!(SPSR & (1<<SPIF))); //wait for transimission to complete
		#endif		
	}	
	else
	{
		#ifdef USE_SW_SPI	
		/*********************************************************/
		// Execute the Transfer via SW Interface
		
		// Schleife für die 8 Bits 
		// Clock befindet sich in der korrekten Ruhestellung

	uint8_t BitCnt =0;
	uint8_t NextOutBit=0;
	uint8_t NextInBit=0;

		
		for(BitCnt=0;BitCnt<8;BitCnt++)
		{
			// Extrahiere, welches Bit übertragen wird
			if (_loc_MsbFirst_Cpol_Cpha&0x04)
			{
				//MSB First
				NextOutBit=((SpiData&(1<<(7-BitCnt)))!=0);
			}
			else
			{
				//LSB First
				NextOutBit=((SpiData&(1<<BitCnt))!=0);
			}
			

			
			// Nun wird ein Clock-Cyclus erzeugt
			// Je nach Mode wird dabei die MOSI Leitung gesetzt bzw. die MISO Leitung gelesen
			switch(_loc_MsbFirst_Cpol_Cpha&0x03)
			{
				case 0:
				{
					// Mode 0: MOSI, 0-->1, MISO, 1-->0
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT|=(1<<SW_SPI_SCK);					
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
				}
				break;
				case 1:
				{
					// Mode 1: MOSI, 0--> 1, NOP, 1-->0 MISO  
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
					asm volatile ("nop");
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
				}
				break;
				case 2:
				{
					// Mode 2: MOSI 1-->0 MISO 0-->1 
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
				}
				break;
				case 3:
				{
					// Mode 3: MOSI 1-->0, NOP, 0-->1 MISO
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
					asm volatile ("nop");
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
				}
				break;
				default:
				{
					
				}
			} // of switch Mode 

			// Empfangene Bits zurückschreiben
			if (_loc_MsbFirst_Cpol_Cpha&0x04)
			{
				//MSB First
				SpiDataOut+=(NextInBit<<(7-BitCnt));
			}
			else
			{
				//LSB First
				SpiDataOut+=(NextInBit<<BitCnt);
			}

		} // of Bitloop
		#endif
	} // end of if SW SPI
	
	return SpiDataOut;
}

void _loc_spi_Close()
{
		
	// Set all pins to inputs
	#ifdef USE_HW_SPI
		HW_SPI_DDR&=~( (1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK) | (1<<HW_SPI_MISO));
	#endif
	#ifdef USE_SW_SPI
		SW_SPI_DDR&=~( (1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK) | (1<<SW_SPI_MISO));
	#endif

	// Disable SPI Interface
	SPCR=0x00;
	
}

#endif

#ifdef DEVICE_ATMEGA328

// Makros für die Steuerung der CS Leitung
#ifdef USE_SPI0
#define SPI0_CS_DDR_ON (SPI0_CS_DDR|=(1<<SPI0_CS_PIN))
#define SPI0_CS_DDR_OFF (SPI0_CS_DDR&=~(1<<SPI0_CS_PIN))
#define SPI0_CS_ACTIVE (SPI0_CS_PORT&=~(1<<SPI0_CS_PIN))
#define SPI0_CS_INACTIVE (SPI0_CS_PORT|=(1<<SPI0_CS_PIN))
#endif

#ifdef USE_SPI1
#define SPI1_CS_DDR_ON (SPI1_CS_DDR|=(1<<SPI1_CS_PIN))
#define SPI1_CS_DDR_OFF (SPI1_CS_DDR&=~(1<<SPI1_CS_PIN))
#define SPI1_CS_ACTIVE (SPI1_CS_PORT&=~(1<<SPI1_CS_PIN))
#define SPI1_CS_INACTIVE (SPI1_CS_PORT|=(1<<SPI1_CS_PIN))
#endif

#ifdef USE_SPI2
#define SPI2_CS_DDR_ON (SPI2_CS_DDR|=(1<<SPI2_CS_PIN))
#define SPI2_CS_DDR_OFF (SPI2_CS_DDR&=~(1<<SPI2_CS_PIN))
#define SPI2_CS_ACTIVE (SPI2_CS_PORT&=~(1<<SPI2_CS_PIN))
#define SPI2_CS_INACTIVE (SPI2_CS_PORT|=(1<<SPI2_CS_PIN))
#endif

#ifdef USE_SPI3
#define SPI3_CS_DDR_ON (SPI3_CS_DDR|=(1<<SPI3_CS_PIN))
#define SPI3_CS_DDR_OFF (SPI3_CS_DDR&=~(1<<SPI3_CS_PIN))
#define SPI3_CS_ACTIVE (SPI3_CS_PORT&=~(1<<SPI3_CS_PIN))
#define SPI3_CS_INACTIVE (SPI3_CS_PORT|=(1<<SPI3_CS_PIN))
#endif


// Konfiguration der HW Leitungen für SW und HW SPI
// Die Konfiguration ist Kanalunabhängig
// Konfiguraion der CS-Leitungen erfolgt im HW-unabhängigen Teil
void _loc_spi_Init()
{
	//Set the pin directions for the HW SPI connections
	#ifdef USE_HW_SPI
		HW_SPI_PORT&=~((1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK));
		HW_SPI_DDR&=~((1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK) | (1<<HW_SPI_MISO));
		HW_SPI_DDR |=  (1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK);
	#endif
	
	//Set the pin directions for the  SW SPI connections
	#ifdef USE_SW_SPI
		SW_SPI_PORT&=~((1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK));
		SW_SPI_DDR&=~((1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK) | (1<<SW_SPI_MISO));
		SW_SPI_DDR |=  (1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK);
	#endif
	
	// all other inits are enabled during the Transfer call
}

// Die interne SetupTransfer Funktion setzt die Schnitttsellenparameter
// Das Management der CD Lietungen übernimmt die übergeordnete Funktion
void _loc_spi_SetupTransfer(uint8_t SpiClk, uint8_t SpiMode, uint8_t SpiMsbFirst, uint8_t SpiHwSwSelect)
{

	if(SpiHwSwSelect==SPI_HW)
	{
		#ifdef USE_HW_SPI
		// Konfigurieren der HW-Schnittstelle
		// Clear the SPCR Register
		SPCR=0x00;
		if (SpiMsbFirst)
		{
			SPCR |= (1<<SPE) | (1<<MSTR); //DORD(MSB), CPOL(0), CPHA(0)
		}
		else
		{
			SPCR |= (1<<SPE) | (1<<MSTR) | (1<<DORD); //DORD(LSB), CPOL(0), CPHA(0)
		}
		
		// Configure the Spi Op Mode
		SPCR |= (SpiMode & 0x03)<<2;
		
		// Configure the clock divider
		SPCR |= (SpiClk & 0x03);
		#endif
	}
	else
	{
		#ifdef USE_SW_SPI
		// Konfigurieren der SW-SPI Schnittstelle
		
		// SPeichern der Parameter für den nächsten Datentransfer
		_loc_MsbFirst_Cpol_Cpha=(SpiMsbFirst<<2)+SpiMode;
		
		// Setzen der Leitungen
		if(SpiMode<2)
		{
			// Set Clock line to Low
			SW_SPI_PORT&=~(1<<SW_SPI_SCK);
		}
		else
		{
			// Set Clock line to Hi
			SW_SPI_PORT|=(1<<SW_SPI_SCK);
		}
		#endif
	}
}

// Die interne TransferWait Funktion
// startet den Transfer und wartet bis die Daten hinausgeschoben sind.
// Damit sind mehrere aufeinanderfolgende Transfers möglich ohne die Übertragung
// zu unterbrechen
uint8_t _loc_spi_TransferWait(uint8_t SpiData, uint8_t SpiHwSwSelect)
{
	uint8_t SpiDataOut=0;


	if (SpiHwSwSelect==SPI_HW)
	{
		#ifdef USE_HW_SPI
		/*********************************************************/
		// Execute the Transfer via HW Interface
		// Start the transfer by writing  Data Out
		SPDR=SpiData;
		// Wait until transfer completed.
		// Because the transfer is not HW dependant, no Timeout control is necessary
		while(!(SPSR & (1<<SPIF))); //wait for transimission to complete
		#endif
	}
	else
	{
		#ifdef USE_SW_SPI
		/*********************************************************/
		// Execute the Transfer via SW Interface
		
		// Schleife für die 8 Bits
		// Clock befindet sich in der korrekten Ruhestellung

		uint8_t BitCnt =0;
		uint8_t NextOutBit=0;
		uint8_t NextInBit=0;

		
		for(BitCnt=0;BitCnt<8;BitCnt++)
		{
			// Extrahiere, welches Bit übertragen wird
			if (_loc_MsbFirst_Cpol_Cpha&0x04)
			{
				//MSB First
				NextOutBit=((SpiData&(1<<(7-BitCnt)))!=0);
			}
			else
			{
				//LSB First
				NextOutBit=((SpiData&(1<<BitCnt))!=0);
			}
			

			
			// Nun wird ein Clock-Cyclus erzeugt
			// Je nach Mode wird dabei die MOSI Leitung gesetzt bzw. die MISO Leitung gelesen
			switch(_loc_MsbFirst_Cpol_Cpha&0x03)
			{
				case 0:
				{
					// Mode 0: MOSI, 0-->1, MISO, 1-->0
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
				}
				break;
				case 1:
				{
					// Mode 1: MOSI, 0--> 1, NOP, 1-->0 MISO
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
					asm volatile ("nop");
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
				}
				break;
				case 2:
				{
					// Mode 2: MOSI 1-->0 MISO 0-->1
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
				}
				break;
				case 3:
				{
					// Mode 3: MOSI 1-->0, NOP, 0-->1 MISO
					if (NextOutBit) SW_SPI_PORT|=(1<<SW_SPI_MOSI);
					else SW_SPI_PORT&=~(1<<SW_SPI_MOSI);
					SW_SPI_PORT&=~(1<<SW_SPI_SCK);
					asm volatile ("nop");
					SW_SPI_PORT|=(1<<SW_SPI_SCK);
					NextInBit=((SW_SPI_PIN&(1<<SW_SPI_MISO))!=0);
				}
				break;
				default:
				{
					
				}
			} // of switch Mode

			// Empfangene Bits zurückschreiben
			if (_loc_MsbFirst_Cpol_Cpha&0x04)
			{
				//MSB First
				SpiDataOut+=(NextInBit<<(7-BitCnt));
			}
			else
			{
				//LSB First
				SpiDataOut+=(NextInBit<<BitCnt);
			}

		} // of Bitloop
		#endif
	} // end of if SW SPI
	
	return SpiDataOut;
}

void _loc_spi_Close()
{
	
	// Set all pins to inputs
	#ifdef USE_HW_SPI
	HW_SPI_DDR&=~( (1<<HW_SPI_MOSI) | (1<<HW_SPI_SCK) | (1<<HW_SPI_MISO));
	#endif
	#ifdef USE_SW_SPI
	SW_SPI_DDR&=~( (1<<SW_SPI_MOSI) | (1<<SW_SPI_SCK) | (1<<SW_SPI_MISO));
	#endif

	// Disable SPI Interface
	SPCR=0x00;
	
}

#endif
/************************************************************/


/*************************************************************************************/
// HW-unabhängige Funktionen


// Die Init Funktion registriert die Kommunikationsparameter für den jeweiligen Kanal
// und konfiguriert die Hardware Leitungen. 
// Die Einstellungen für die Übertragung werden registriert, werden dann aber erst beim
// Beginn einer Übertragung konfiguriert.
uint8_t spi_Init()
{

		if(_loc_SpiStatus!=SPI_STAT_CLOSED)
		{
			_loc_RecError(SPI_ERR_STAT);
			return SPI_ERR;
		}
		else
		{
			// Konfigurieren der Kommunikationsleitungen
			_loc_spi_Init();
			
			// Konfigurieren der CS Leitungen
			#ifdef USE_SPI0
				SPI0_CS_DDR_ON;
				SPI0_CS_INACTIVE;
			#endif

			#ifdef USE_SPI1
				SPI1_CS_DDR_ON;
				SPI1_CS_INACTIVE;
			#endif

			#ifdef USE_SPI2
				SPI2_CS_DDR_ON;
				SPI2_CS_INACTIVE;
			#endif

			#ifdef USE_SPI3
				SPI3_CS_DDR_ON;
				SPI3_CS_INACTIVE;
			#endif
						
			_loc_SpiStatus=SPI_STAT_READY;
			return SPI_OK;
			
		}
	
}

// Die Transfer Funktion übernimmt das Senden und Empfange von Daten über einen beliebigen SPI Kanal
// Zuerst werden die Schnittstellenparameter konfiguriert, diese können bei jedem Transfer ändern
// Dann wird die CS Leitung auf Aktiv gesetzt. 
// Dann beginnt der Daten-Transfer
// Danach wird in Abhänigkeit des SpiTerminate Parameters das Datenpaket abgeschlossen oder 
// Die Übertragung geht beim nächsten Aufruf weiter (CS bleibt aktiv)
uint32_t spi_TransferWait(uint8_t SpiChannel, uint32_t SpiData, uint8_t SpiNCycles, uint8_t SpiClk, uint8_t SpiMode, uint8_t SpiMsbFirst, uint8_t SpiTerminate)
{
	uint8_t SpiDataIn = 0;
	
	// Da die Chip Select Steuerung auf Basis von Macros erfolgt, muss für jeden Kanal ein
	// anderer Code ausgeführt werden
	switch(SpiChannel)
	{
		case 0:
		{
			// Parameter setzen, Chip Select aktivieren, Daten ausgeben,
			// Terminate, dann den Chiip Select für diesen Kanal deselektieren
			#ifdef USE_SPI0
				_loc_spi_SetupTransfer(SpiClk, SpiMode, SpiMsbFirst,SPI0_HWSW);
				SPI0_CS_ACTIVE;
				SpiDataIn=_loc_spi_TransferWait(SpiData,SPI0_HWSW);
				if(SpiTerminate)
				{
					SPI0_CS_INACTIVE;
				}
			#endif
		}
		break;
		
		case 1:
		{
			// Parameter setzen, Chip Select aktivieren, Daten ausgeben, 
			// Terminate, dann den Chiip Select für diesen Kanal deselektieren
			#ifdef USE_SPI1
				_loc_spi_SetupTransfer(SpiClk, SpiMode, SpiMsbFirst,SPI1_HWSW);
				SPI1_CS_ACTIVE;
				SpiDataIn=_loc_spi_TransferWait(SpiData,SPI1_HWSW);
				if(SpiTerminate)
				{
					SPI1_CS_INACTIVE;
				}
			#endif
		}
		break;

		case 2:
		{
			// Parameter setzen, Chip Select aktivieren, Daten ausgeben,
			// Terminate, dann den Chiip Select für diesen Kanal deselektieren
			#ifdef USE_SPI2
				_loc_spi_SetupTransfer(SpiClk, SpiMode, SpiMsbFirst,SPI2_HWSW);
				SPI2_CS_ACTIVE;
				SpiDataIn=_loc_spi_TransferWait(SpiData,SPI2_HWSW);
				if(SpiTerminate)
				{
					SPI2_CS_INACTIVE;
				}
			#endif
		}
		break;

		case 3:
		{
			// Parameter setzen, Chip Select aktivieren, Daten ausgeben,
			// Terminate, dann den Chiip Select für diesen Kanal deselektieren
			#ifdef USE_SPI3
				_loc_spi_SetupTransfer(SpiClk, SpiMode, SpiMsbFirst,SPI3_HWSW);
				SPI3_CS_ACTIVE;
				SpiDataIn=_loc_spi_TransferWait(SpiData,SPI3_HWSW);
				if(SpiTerminate)
				{
					SPI3_CS_INACTIVE;
				}
			#endif
		}
		break;

		default:
		{
			_loc_RecError(SPI_ERR_CHANNEL);
			return SPI_ERR;
		}
	}
	return SpiDataIn;
}

// Abfrage des zuletzt aufgetretenen Fehlers
// Durch die Abfrage wird die Fehlermeldung gelöscht
uint8_t spi_GetLastError()
{
	uint8_t LastError;
	LastError=_loc_LastError;
	_loc_LastError=SPI_ERR_OK;
	return LastError;	
}

// Historische Funktionen
uint8_t spi_SetSS()
{
	//SET_SS;
	return 0;
}

uint8_t spi_ClearSS()
{
	//CLR_SS;
	return 0;
}

// Freigabe der Pins und resetten der Memories
void spi_Close(void)
{
	
	// Release all data pins
	_loc_spi_Close();
	
	// Reset the status
	_loc_SpiStatus=SPI_STAT_CLOSED;
	
	// Loop over all Devices and close
	// Leave the CS Pin inactive (nothing to do as this is the default state)
	// It does not make sense to leave the CS line uncontrolled, even in closed mode	
	#ifdef USE_SPI0
		SPI0_CS_INACTIVE;
	#endif

	#ifdef USE_SPI1
		SPI1_CS_INACTIVE;
	#endif

	#ifdef USE_SPI2
		SPI2_CS_INACTIVE;
	#endif

	#ifdef USE_SPI3
		SPI3_CS_INACTIVE;
	#endif
	
	
}
/*************************************************************************************/

