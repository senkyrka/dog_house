
// pouzita deska MH ET LIve ESP32 MiniKit
// spravce desek (soubor>vlastnosti> (dole spravce dalsich desek) dat https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
// potom doinstalovat (pripadne aktualizovat) menu Nastroje>Vyvojova deska>manager desek>  dat vyhledat esp32 a instalova/aktualizovat na nej verzi


#include <ccs811.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <DHT_U.h>
#include <stdio.h>
#include <LoRa.h>
#include <SPI.h>

#include <Adafruit_Si7021.h>

#define ss 5
#define rst 14
#define dio0 2
#define WAKEUP 26

#define DHTPIN_22 16
#define DHTTYPE_22 DHT22

//dht
DHT dht_22 (DHTPIN_22, DHTTYPE_22);

//veci k fialove desce cjmcu2128
CCS811 ccs811;
Adafruit_BMP280 bmp280;                // I2C
Adafruit_Si7021 SI702x = Adafruit_Si7021();


int pes;
int venkuT;
int venkuV;
int boudatT;
int boudaV;
uint16_t boudaCO;
uint16_t EVOC;
int kterouposlem=0;
uint16_t  errstat, raw;
char textven[100];
void setup() 
{
  Serial.begin(115200); 
  while (!Serial);
  Serial.println("LoRa bouda");

  Wire.begin();      //nahod I2C
  Serial.println("BMP280 test");     /* --- SETUP BMP on 0x76 ------ */
  if (!bmp280.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (true);
  }
  delay(10);
  Serial.println("CCS811 test");      /* --- SETUP CCS811 on 0x5A ------ */
  if (!ccs811.begin()) {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while (true);
  }
  bool ok = ccs811.start(CCS811_MODE_1SEC);
  if ( !ok ) Serial.println("setup: CCS811 start FAILED");

 delay(10);
  Serial.println("Si7021 test!");     /* ---- SETUP SI702x ----- */
  if (!SI702x.begin()) {
    Serial.println("Did not find Si702x sensor!");
    while (true);
  }

   dht_22.begin();

  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
  while (!LoRa.begin(433E6))     
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa  OK!");
}
 
void loop() 
{
   //vyctu hodnoty
   boudatT=(int)(bmp280.readTemperature()*10);
  //Serial.print(bmp280.readPressure() / 100;
   boudaV=SI702x.readHumidity();
   ccs811.set_envdata(bmp280.readTemperature(), SI702x.readHumidity());
    ccs811.read(&boudaCO, &EVOC, &errstat, &raw);
    venkuV = (int)(dht_22.readHumidity());
    venkuT = (int)(dht_22.readTemperature()*10);

    sprintf(textven,"T=%d V=%d boud t=%d V=%d CO2=%d EVOC=%d",venkuT,venkuV,boudatT,boudaV,boudaCO,EVOC);
    Serial.println(textven);
  
  switch(kterouposlem)
  {
    case 0:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("pes");
        LoRa.print(pes);
        LoRa.endPacket();
    break;
    case 1:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("venkuT");
        LoRa.print(venkuT);
        LoRa.endPacket();
    break;
    case 2:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("venkuV");
        LoRa.print(venkuV);
        LoRa.endPacket();
    break;
    case 3:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("boudatT");
        LoRa.print(boudatT);
        LoRa.endPacket();
    break;
    case 4:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("boudaCO");
        LoRa.print(boudaCO);
        LoRa.endPacket();
    break;
    case 5:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("boudaV");
        LoRa.print(boudaV);
        LoRa.endPacket();
    break;
    case 6:
        LoRa.beginPacket();   //Send LoRa packet to receiver
        LoRa.print("EVOC");
        LoRa.print(EVOC);
        LoRa.endPacket();
    break;
  }

 
  

 kterouposlem++;
 if(kterouposlem>6)kterouposlem=0;
 delay(10000);
}
