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
    // more options below
    // I currently left the factory feature where it shows an incrementing number count 
    // just so we still have an easy way to see if they are connected while testing
    case STATE_TX:
      delay(1000);
      txNumber++;
      sprintf(txpacket, "Device1 Temp: 25.3C, Hum: 60%%");
      Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
      state=LOWPOWER;
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

      /* What do we want the packet to be
       in this case we store the data as a test packet and send it using Radio.send()
       I'll edit this based on the form that we obatin the sensor data, but some options
       are:
       We can send binary data:

       uint8_t dataToSend[5] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5}; // Example binary data
       Radio.Send(dataToSend, sizeof(dataToSend));

       We can send integers and directly by
       converting them into byte arrays:

       int sensorValue = 1234;
       Radio.Send((uint8_t *)&sensorValue, sizeof(sensorValue));

       With each thing we send, we would need to modify the receiving end so the data
       can be parsed correctly. Receiving ints:

       int receivedValue;
       memcpy(&receivedValue, payload, sizeof(receivedValue));
       Serial.printf("Received Integer: %d\n", receivedValue);

       We can also send all sensor values together as a struct:

       struct SensorData {
       float temperature;
       int humidity;
       uint16_t battery;
        };
       SensorData data = {25.4, 60, 3700}; // Temp = 25.4°C, Humidity = 60%, Battery = 3.7V
       Radio.Send((uint8_t *)&data, sizeof(data));


       Receiving a struct: SensorData receivedData;
       memcpy(&receivedData, payload, sizeof(receivedData));
       Serial.printf("Temp: %.1f C, Hum: %d%%, Battery: %.2fV\n", 
              receivedData.temperature, receivedData.humidity, receivedData.battery / 1000.0);

      
      */