/*
 * HelTec Automation(TM) WIFI_LoRa_32 factory test code, witch includ
 * follow functions:
 * 
 * - Basic OLED function test;
 * 
 * - Basic serial port test(in baud rate 115200);
 * 
 * - LED blink test;
 * 
 * - WIFI connect and scan test;
 * 
 * - LoRa Ping-Pong test (DIO0 -- GPIO26 interrup check the new incoming messages);
 * 
 * - Timer test and some other Arduino basic functions.
 *
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/

#include "Arduino.h"
#include "WiFi.h"
#include "images.h"
#include "LoRaWan_APP.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include <CAN.h>
/********************************* lora  *********************************************/
#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             10        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

typedef enum
{
    LOWPOWER,
    STATE_RX,
    STATE_TX
}States_t;

int16_t txNumber;
int16_t rxNumber;
States_t state;
bool sleepMode = false;
int16_t Rssi,rxSize;

String rssi = "RSSI --";
String packSize = "--";
String packet;
String send_num;
String show_lora = "lora data show";

unsigned int counter = 0;
bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends
uint64_t chipid;
int16_t RssiDetection = 0;



// ***********************************************
// CAN Definitions and Helper Functions
// ***********************************************
// Define CAN message IDs (from your Python script)
#define POT_DATA_A_ID      0x100  // Front two linear pots
#define DR_Lat_Long_ID     0x311  // GPS Latitude/Longitude (decimal degrees)
#define DR_AltSpeedCOG_ID  0x312  // Altitude, speed, course over ground, etc.

// Constants for ADC processing (from your Python Constant enum)
#define ESP32_ADC_PRECISION      12
#define MAX_POT_DISPLACEMENT     3.937
#define WORD_SIZE                32
#define WORD_SELECT              0xFFFFFFFF

// Assume CAN_message_t is defined by the CAN library with:
//   uint32_t id; uint8_t len; uint8_t buf[8];

void extractESPData(const CAN_message_t &msg, uint32_t &word1, uint32_t &word2) {
  word1 = (uint32_t)msg.buf[0] | ((uint32_t)msg.buf[1] << 8) | ((uint32_t)msg.buf[2] << 16) | ((uint32_t)msg.buf[3] << 24);
  word2 = (uint32_t)msg.buf[4] | ((uint32_t)msg.buf[5] << 8) | ((uint32_t)msg.buf[6] << 16) | ((uint32_t)msg.buf[7] << 24);
}

float rescale(uint32_t value) {
  return (value * MAX_POT_DISPLACEMENT) / (float)(1 << ESP32_ADC_PRECISION);  // 2^12 = 4096
}

// Parse DR_Lat_Long message: 4 bytes for latitude, 4 bytes for longitude.
void parseLatLong(const CAN_message_t &msg, float &latitude, float &longitude) {
  int32_t rawLat = (int32_t)(msg.buf[0] | (msg.buf[1] << 8) | (msg.buf[2] << 16) | (msg.buf[3] << 24));
  int32_t rawLon = (int32_t)(msg.buf[4] | (msg.buf[5] << 8) | (msg.buf[6] << 16) | (msg.buf[7] << 24));
  latitude  = rawLat / 10000000.0;
  longitude = rawLon / 10000000.0;
}

// Parse DR_AltSpeedCOG message: altitude (bytes 0-1), COG (2-3), speed (4-5),
// DR mode & satellite count in byte 6, PDOP in byte 7.
void parseAltSpeedCOG(const CAN_message_t &msg, float &altitude, float &cog, float &speed, char *dr_mode, int &satellite_count, int &pdop) {
  int16_t rawAlt = (int16_t)(msg.buf[0] | (msg.buf[1] << 8));
  altitude = rawAlt * 0.1;  // 0.1m precision
  
  uint16_t rawCOG = (uint16_t)(msg.buf[2] | (msg.buf[3] << 8));
  cog = rawCOG * 0.1;  // 0.1 degree precision
  if (cog > 360) {
    cog = fmod(cog, 360.0);
  }
  
  uint16_t rawSpeed = (uint16_t)(msg.buf[4] | (msg.buf[5] << 8));
  speed = rawSpeed * 0.1;  // 0.1 m/s precision
  
  uint8_t mode_byte = msg.buf[6];
  uint8_t mode = mode_byte & 0x07;  // lower 3 bits
  switch (mode) {
    case 0: strcpy(dr_mode, "No fix"); break;
    case 1: strcpy(dr_mode, "Dead reckoning"); break;
    case 2: strcpy(dr_mode, "2D fix"); break;
    case 3: strcpy(dr_mode, "3D fix"); break;
    case 4: strcpy(dr_mode, "GNSS+DR"); break;
    default: strcpy(dr_mode, "Unknown"); break;
  }
  satellite_count = (mode_byte >> 3) & 0x1F;  // upper 5 bits
  pdop = msg.buf[7];
}

// ***********************************************
// Binary Packet Structures
// ***********************************************

// We would change these based on what we are actually sending
struct LatLongPacket {
  uint8_t type;       // 1 for DR_Lat_Long
  float latitude;
  float longitude;
};

struct PotPacket {
  uint8_t type;       // 2 for POT_DATA_A
  float pot1;
  float pot2;
};

struct AltSpeedCOGPacket {
  uint8_t type;       // 3 for DR_AltSpeedCOG
  float altitude;
  float cog;
  float speed;
  uint8_t dr_mode;       // Numeric code for DR mode
  uint8_t satellite_count;
  uint8_t pdop;
};




// ***********************************************
// LoRa Event Functions
// ***********************************************

void OnTxDone( void )
{
	Serial.print("TX done......");
	state=STATE_RX;

}

void OnTxTimeout( void )
{
  Radio.Sleep( );
  Serial.print("TX Timeout......");
	state=STATE_TX;
}

// This is where we edit the receiver end of the code
// More options on this below where we send the data packets, we will modify
// the receiving end based on the kind of data that we receive from the sender

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	rxNumber++;
  Rssi=rssi;
  rxSize=size;
  memcpy(rxpacket, payload, size );
  rxpacket[size]='\0';
  Radio.Sleep( );
  Serial.printf("\nReceived data: %s\n", payload);
  if (strstr((char*)payload, "Temp:") != NULL) {
    Serial.println("Temperature data received!");
  }
  //Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,Rssi,rxSize);
  //Serial.println("wait to send next packet");
	receiveflag = true;
  state=STATE_TX;
}


void lora_init(void)
{
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  txNumber=0;
  Rssi=0;
  rxNumber = 0;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                 LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                 LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
	state=STATE_TX;
}


/********************************* lora  *********************************************/

SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst


void logo(){
	factory_display.clear();
	factory_display.drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
	factory_display.display();
}

void WIFISetUp(void)
{
	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.disconnect(true);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin("Your WiFi SSID","Your Password");//fill in "Your WiFi SSID","Your Password"
	delay(100);

	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
		factory_display.drawString(0, 0, "Connecting...");
		factory_display.display();
	}

	factory_display.clear();
	if(WiFi.status() == WL_CONNECTED)
	{
		factory_display.drawString(0, 0, "Connecting...OK.");
		factory_display.display();
//		delay(500);
	}
	else
	{
		factory_display.clear();
		factory_display.drawString(0, 0, "Connecting...Failed");
		factory_display.display();
		//while(1);
	}
	factory_display.drawString(0, 10, "WIFI Setup done");
	factory_display.display();
	delay(500);
}

void WIFIScan(unsigned int value)
{
	unsigned int i;
    WiFi.mode(WIFI_STA);

	for(i=0;i<value;i++)
	{
		factory_display.drawString(0, 20, "Scan start...");
		factory_display.display();

		int n = WiFi.scanNetworks();
		factory_display.drawString(0, 30, "Scan done");
		factory_display.display();
		delay(500);
		factory_display.clear();

		if (n == 0)
		{
			factory_display.clear();
			factory_display.drawString(0, 0, "no network found");
			factory_display.display();
			//while(1);
		}
		else
		{
			factory_display.drawString(0, 0, (String)n);
			factory_display.drawString(14, 0, "networks found:");
			factory_display.display();
			delay(500);

			for (int i = 0; i < n; ++i) {
			// Print SSID and RSSI for each network found
				factory_display.drawString(0, (i+1)*9,(String)(i + 1));
				factory_display.drawString(6, (i+1)*9, ":");
				factory_display.drawString(12,(i+1)*9, (String)(WiFi.SSID(i)));
				factory_display.drawString(90,(i+1)*9, " (");
				factory_display.drawString(98,(i+1)*9, (String)(WiFi.RSSI(i)));
				factory_display.drawString(114,(i+1)*9, ")");
				//factory_display.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
				delay(10);
			}
		}

		factory_display.display();
		delay(800);
		factory_display.clear();
	}
}

bool resendflag=false;
bool deepsleepflag=false;
bool interrupt_flag = false;
void interrupt_GPIO0()
{
	interrupt_flag = true;
}
void interrupt_handle(void)
{
	if(interrupt_flag)
	{
		interrupt_flag = false;
		if(digitalRead(0)==0)
		{
			if(rxNumber <=2)
			{
				resendflag=true;
			}
			else
			{
				deepsleepflag=true;
			}
		}
	}

}
void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
  
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}


// ***********************************************
// Setup Function
// ***********************************************

void setup()
{
	Serial.begin(115200);
	VextON();
	delay(100);
	factory_display.init();
	factory_display.clear();
	factory_display.display();
	logo();
	delay(300);
	factory_display.clear();

	WIFISetUp();
	WiFi.disconnect(); //
	WiFi.mode(WIFI_STA);
	delay(100);

	WIFIScan(1);

	chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
	Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

	attachInterrupt(0,interrupt_GPIO0,FALLING);
	lora_init();
	packet ="waiting lora data!";
  factory_display.drawString(0, 10, packet);
  factory_display.display();
  delay(100);
  factory_display.clear();
	pinMode(LED ,OUTPUT);
	digitalWrite(LED, LOW);  
}

// ***********************************************
// Main Loop
// ***********************************************
void loop()
{
interrupt_handle();
 if(deepsleepflag)
 {
	VextOFF();
	Radio.Sleep();
	SPI.end();
	pinMode(RADIO_DIO_1,ANALOG);
	pinMode(RADIO_NSS,ANALOG);
	pinMode(RADIO_RESET,ANALOG);
	pinMode(RADIO_BUSY,ANALOG);
	pinMode(LORA_CLK,ANALOG);
	pinMode(LORA_MISO,ANALOG);
	pinMode(LORA_MOSI,ANALOG);
	esp_sleep_enable_timer_wakeup(600*1000*(uint64_t)1000);
	esp_deep_sleep_start();
 }

 if(resendflag)
 {
	state = STATE_TX;
	resendflag = false;
 }

if(receiveflag && (state==LOWPOWER) )
{
	receiveflag = false;
	packet ="R_data:";
	int i = 0;
	while(i < rxSize)
	{
		packet += rxpacket[i];
		i++;
	}
	packSize = "R_Size: ";
	packSize += String(rxSize,DEC);
	packSize += " R_rssi: ";
	packSize += String(Rssi,DEC);
	send_num = "send num: ";
	send_num += String(txNumber,DEC);
	factory_display.drawString(0, 0, show_lora);
  factory_display.drawString(0, 10, packet);
  factory_display.drawString(0, 20, packSize);
  factory_display.drawString(0, 50, send_num);
  factory_display.display();
  delay(10);
  factory_display.clear();

  if((rxNumber%2)==0)
  {
   digitalWrite(LED, HIGH);  
  }
}
switch(state)
  {
    // This is where we edit the code being sent
    // We would have to edit 
    case STATE_TX:
      delay(1000);
      txNumber++;
      // Check if a CAN message is available
      if (CAN.parsePacket())
      {
        CAN_message_t msg;
        if (CAN.read(msg))
        {
          Serial.print("Received CAN message with ID: 0x");
          Serial.println(msg.id, HEX);
          if (msg.id == POT_DATA_A_ID && msg.len >= 8)
          {
            uint32_t word1, word2;
            extractESPData(msg, word1, word2);
            float pot1 = rescale(word1);     // potentiometer 1
            float pot2 = rescale(word2);
            sprintf(txpacket, "Pot A1: %.3f, A2: %.3f", pot1, pot2);

            // Alternatively, we could make the data we want to send into a struct
            // and send it as binary data, but this would also require
            // parsing it when we receive

            // PotPacket p;
            // p.type = 2; // POT_DATA_A
            // p.pot1 = pot1;
            // p.pot2 = pot2;
            // Serial.println("Sending binary POT_DATA_A packet");
            // Radio.Send((uint8_t *)&p, sizeof(p));
          }
          else if (msg.id == DR_Lat_Long_ID && msg.len >= 8)
          {
            float lat, lon;
            parseLatLong(msg, lat, lon);
            sprintf(txpacket, "Lat: %.7f, Lon: %.7f", lat, lon);
          }
          else if (msg.id == DR_AltSpeedCOG_ID && msg.len >= 8)
          {
            float altitude, cog, speed;
            char dr_mode[20];
            int satellite_count, pdop;
            parseAltSpeedCOG(msg, altitude, cog, speed, dr_mode, satellite_count, pdop);
            sprintf(txpacket, "Alt: %.1fm, COG: %.1f°, Spd: %.1f m/s, Mode: %s, Sat: %d, PDOP: %d",
                    altitude, cog, speed, dr_mode, satellite_count, pdop);
          }
          else
          {
            sprintf(txpacket, "Unhandled CAN msg ID: 0x%X", msg.id);
          }
        }
      }
      else
      {
        // No CAN message available; optionally send a default test message
        sprintf(txpacket, "No CAN data");
      }
      
      Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));
      Radio.Send((uint8_t *)txpacket, strlen(txpacket));
      state = LOWPOWER;
      break;

    case STATE_RX:
      Serial.println("into RX mode");
      Radio.Rx( 0 );
      state=LOWPOWER;
      break;

    case LOWPOWER:
      Radio.IrqProcess( );
      break;
    default:
      break;
  }
}

