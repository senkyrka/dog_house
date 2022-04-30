
// pouzita deska NodeMCU-3S

// knihovna pro OLED displej
#include <GyverOLED.h>

// knihovny pro technologii LoRa
#include <LoRa.h>
#include <SPI.h>

// standard input output
#include <stdio.h>


#include <stdlib.h>
#include <Arduino.h>

// knihovna pro wifi
#include <WiFi.h>

//////// pro pripojeni na eduroam
//#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks  (pouze na EDUROAM)
//#define EAP_ANONYMOUS_IDENTITY "anonymous@upce.cz" //anonymous@example.com, or you can use also nickname@example.com
//#define EAP_IDENTITY "st60946@upce.cz" //nickname@example.com, at some organizations should work nickname only without realm, but it is not recommended
//#define EAP_PASSWORD "wio2PX4c" //password for eduroam account

//const char* ssid = "eduroam"; // eduroam SSID
///////

// nastaveni wifi site a hesla na danou sit
#define ssid "ASUS"
#define pass "10Albina."


#define ss 5
#define rst 14
#define dio0 2

// promenne pro senzory
int pes;
int venkuT;
int venkuV;
int boudaT;
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

// inicializace OLED dispeje
  oled.init(); 
  
  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  
  Serial.println("LoRa incializace OK!");

   WiFi.mode(WIFI_STA); // SETS TO STATION MODE!
   //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD); // bez certifikatu 
    WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Pokouším se připojit na SSID: ");
    Serial.println(ssid);
    
    // pockej 10 sekund pro pripojeni:
    delay(10000);
    minulyzapis=millis();
  }
  Serial.println("Připojeno na WiFi!");
  printWiFiStatus();


       oled.clear();
       oled.setScale(1);
       oled.setCursor(3, 3);
       oled.print("Zdravím!");
       oled.update();
}
 
void loop() 
{
  int packetSize = LoRa.parsePacket();    // pokus se vyparsrovat packet
  if (packetSize) 
  {
    Serial.print("Přišel packet'");
    while (LoRa.available())              // precti packet
    {
       LoRaData = LoRa.readString();
    }
    
        LoRaData.toCharArray(prijatytext,32);  //chci na porovnani použít c strncmp tak prevedu na char array
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
          else if(!strncmp(prijatytext,"boudaT",7))
          {
            sscanf(prijatytext,"boudaT%d",&boudaT);
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
          sprintf(textven,"%d %d %d %d %d %d %d", pes,venkuT, venkuV, boudaT,boudaV, boudaCO,EVOC);
          Serial.println(textven);

          // naformatovani hodnot ze senzoru
          sprintf(textven," Teplota=%d.%dC\n\r Vlhkost=%d%%\n\rV boude\n\r Teplota=%d.%dC\n\r Vlhkost=%d%%\n\r CO2=%dppm | EVOC=%d\n\r", venkuT/10,venkuT%10, venkuV, boudaT/10,boudaT%10,boudaV, boudaCO,EVOC);
          
          oled.clear();
          oled.setScale(1);
          oled.setCursor(0, 0);
          
          if(pes)oled.print("Pes v boude\n\r");
          else oled.print("Pes je venku\n\r");
          oled.print(textven);
          
          if(WiFi.status()!= WL_CONNECTED)oled.print("WiFi nepripojena");
          else oled.print("wifi OK");
          oled.update();

           ////////// Zapis na web jednou za cca 10 minut
           if((millis()-minulyzapis)>600000)
           {
            minulyzapis=millis();
           int jetovhaji=0;
           if(WiFi.status()!=WL_CONNECTED)WiFi.begin(ssid, pass);
              while (WiFi.status() != WL_CONNECTED) {
                  Serial.print("Pokouším se připojit na SSID: ");
                  Serial.println(ssid);
                  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
                  // wait 10 seconds for connection:
                  delay(10000);
                  jetovhaji++;
                  if(jetovhaji>10)break;  // nekdy se nepripoji, tak at neceka do nekonecna, tak casem breakni cyklus
                }
                
             if(WiFi.status()== WL_CONNECTED)  //kdyz se nepripojil, tak kasli na web
             {
             if (client.connect("chlupac.eparo.cz", 80)) 
             {
                Serial.println("Připojen na server");
                // Udělej HTTP request:
                sprintf(textven,"GET /vlozeni.php?pes=%d&venkuT=%d&venkuV=%d&boudaT=%d&boudaV=%d&boudaCO=%d&EVOC=%d HTTP/1.1", pes,venkuT, venkuV, boudaT,boudaV, boudaCO,EVOC);
                client.println(textven);
                client.println("Host: chlupac.eparo.cz");
                client.println("Spojení: ukončeno.");
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
  // vypis SSID site na kterou jsi pripojen:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // vypis WiFi shield's IP adresu :
  IPAddress ip = WiFi.localIP();
  Serial.print("IP adresa: ");
  Serial.println(ip);

  // vypis silu prijmuteho signalu:
  long rssi = WiFi.RSSI();
  Serial.print("Síla signálu (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
