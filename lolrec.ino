
// pouzita deska NodeMCU-3S
//blbne zavadec potom co hlasi ze nahrava klikat na reset, jak zacne zapisovat prestat!

//tohle staci jednou pro jakykoliv ESP32
// spravce desek (soubor>vlastnosti> (dole spravce dalsich desek) dat https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
// potom doinstalovat (pripadne aktualizovat) menu Nastroje>Vyvojova deska>manager desek>  dat vyhledat esp32 a instalova/aktualizovat na nej verzi


#include <GyverOLED.h>
#include <LoRa.h>
#include <SPI.h>
#include <stdio.h>
#include <stdlib.h>
 #include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>

////////
//#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks  (pouze na EDUROAM)
//#define EAP_ANONYMOUS_IDENTITY "anonymous@upce.cz" //anonymous@example.com, or you can use also nickname@example.com
//#define EAP_IDENTITY "paro0715@upce.cz" //nickname@example.com, at some organizations should work nickname only without realm, but it is not recommended
//#define EAP_PASSWORD "hyrBbY4d" //password for eduroam account

//const char* ssid = "eduroam"; // eduroam SSID
///////


#define ssid "ASUS"
#define pass "10Albina."
#define ss 5
#define rst 14
#define dio0 2
int pes;
int venkuT;
int venkuV;
int boudatT;
int boudaV;
int boudaCO;
int EVOC; 

char prijatytext[32];
char textven[128];
String LoRaData;
unsigned long minulyzapis=0;
WiFiClient client;
int status = WL_IDLE_STATUS;

GyverOLED<SSH1106_128x64> oled;
void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  oled.init(); 
  
  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");

   WiFi.mode(WIFI_STA); // SETS TO STATION MODE!
   //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD); //without CERT
    WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // wait 10 seconds for connection:
    delay(10000);
    minulyzapis=millis();
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();


       oled.clear();
       oled.setScale(1);
       oled.setCursor(3, 3);
       oled.print("baf baf");
       oled.update();
}
 
void loop() 
{
  int packetSize = LoRa.parsePacket();    // try to parse packet
  if (packetSize) 
  {
    Serial.print("Received packet '");
    while (LoRa.available())              // read packet
    {
       LoRaData = LoRa.readString();
    }
    
        LoRaData.toCharArray(prijatytext,32);  //chci na porovnani požít c strncmp tak prevedu na char array
          if(!strncmp(prijatytext,"pes",3))  //porovna zadany pocet znaku (tady 3) 
          {
            sscanf(prijatytext,prijatytext,"pes%d",&pes);   //jestli text obsahoval pes, sscan čeka tvar pes1 vytahne z nej 1
          }
          else if(!strncmp(prijatytext,"venkuT",6))
          {
            sscanf(prijatytext,"venkuT%d",&venkuT); //sscan ceka venku231 vytahne 231 a vlozi do venkuT
          }
          else if(!strncmp(prijatytext,"venkuV",6))
          {
            sscanf(prijatytext,"venkuV%d",&venkuV);
          }
          else if(!strncmp(prijatytext,"boudatT",7))
          {
            sscanf(prijatytext,"boudatT%d",&boudatT);
          }
          else if(!strncmp(prijatytext,"boudaV",6))
          {
            sscanf(prijatytext,"boudaV%d",&boudaV);
          }
          else if(!strncmp(prijatytext,"boudaCO",7))
          {
            sscanf(prijatytext,"boudaCO%d",&boudaCO);
          }
          else if(!strncmp(prijatytext,"EVOC",4))
          {
            sscanf(prijatytext,prijatytext,"EVOC%d",&EVOC);
          }
          sprintf(textven,"%d %d %d %d %d %d %d", pes,venkuT, venkuV, boudatT,boudaV, boudaCO,EVOC);
          Serial.println(textven);

          sprintf(textven," Teplota=%d.%dC\n\r Vlhkost=%d%%\n\rV boude\n\r Teplota=%d.%dC\n\r Vlhkost=%d%%\n\r CO2=%dppm | EVOC=%d\n\r", venkuT/10,venkuT%10, venkuV, boudatT/10,boudatT%10,boudaV, boudaCO,EVOC);
          oled.clear();
          oled.setScale(1);
          oled.setCursor(0, 0);
          if(pes)oled.print("Pes v boude\n\r");
          else oled.print("Pes je Venku\n\r");
          oled.print(textven);
          if(WiFi.status()!= WL_CONNECTED)oled.print("wifi nepripojen");
          else oled.print("wifi OK");
          oled.update();

           ////////// Zapis na web jednou za cca 10 minut
           if((millis()-minulyzapis)>10000)
           {
            minulyzapis=millis();
           int jetovhaji=0;
           if(WiFi.status()!=WL_CONNECTED)WiFi.begin(ssid, pass);
              while (WiFi.status() != WL_CONNECTED) {
                  Serial.print("Attempting to connect to SSID: ");
                  Serial.println(ssid);
                  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
                  // wait 10 seconds for connection:
                  delay(10000);
                  jetovhaji++;
                  if(jetovhaji>10)break;  // nekdy se projoit nejde tak at neceka do nekonecna, tak casem breakni cyklus
                }
             if(WiFi.status()== WL_CONNECTED)  //kdyz se nepripojil, tak kasli na web
             {
              // if (client.connect("chlupac.eparo.cz, 80))
             if (client.connect("suninu.org", 80)) 
             {
                Serial.println("connected to server");
                // Make a HTTP request:
               // sprintf(textven,"GET /vlozeni.php?pes=%d&venkuT=%d&venkuV=%d&boudatT=%d&boudaV=%d&boudaCO=%d&EVOC=%d HTTP/1.1", pes,venkuT, venkuV, boudatT,boudaV, boudaCO,EVOC);
                sprintf(textven,"GET /katerina/thesis/doghouse? HTTP/1.1");
                client.println(textven);
                Serial.println(textven);
                client.println("Host: suninu.org");
                client.println("Connection: close");
                client.println();
                  while (client.available())
                  {
                     char c = client.read();
                     Serial.write(c);
                  }
             }
             }
           }
  }
}



void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
