#include <WiFi.h>
#include "Modbus.h"
#include "PubSubClient.h"

#define TXPIN 17
#define RXPIN 16
#define MODE  5

WiFiClient client;
PubSubClient mqtt(client);
Modbus modbus(Serial1);

char mac[16];
char INVREQ[] = "INVREQ/000000000000";
char INVRES[] = "INVRES/000000000000";
char INVRAW[] = "INVRAW/000000000000";



void callback(char *topic, uint8_t *payload, unsigned int len){


}


void mqttInit(){
 mqtt.setBufferSize(2048);
 mqtt.setServer("test.com", 1883);
 mqtt.setCallback(callback);
 mqtt.connect("","username","password");
 mqtt.subscribe(INVREQ);
}

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1,RXPIN, TXPIN);
  modbus.init(MODE);
  Serial.println("\n\nConnecting to IoT");
  WiFi.begin("IoT","iot@2022");
  while (WiFi.status() != WL_CONNECTED) {
   Serial.print(".");
   delay(500);
  }
  Serial.println("\n\nWiFi Connected");
  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  uint64_t chipid =  ESP.getEfuseMac();
  sprintf(mac, "%04X%08X", (uint16_t)(chipid>>32),(uint32_t)chipid);
  for(int i = 0;i < 12;i++){
    INVREQ[i+7] = mac[i];
    INVRES[i+7] = mac[i];
    INVRAW[i+7] = mac[i];
  }

  Serial.println(INVREQ);
  Serial.println(INVRES);
  Serial.println(INVRAW);
  mqttInit();

  
  
}

long timer = 0;


void loop() {
  // put your main code here, to run repeatedly:

  if (millis() - timer > 2000) {
   
 
  if(modbus.requestFrom(0x01, 0x04, 0x00, 79) > 0) {


    byte  raw[200];
    uint8_t len;
    
    modbus.RxRaw(raw,  len);
    long  PV_V    = modbus.uint16(1) / 10;
    long  PV_P    = modbus.uint32(3);
    long  Load_P  = modbus.uint32(9) / 10;
    float Load    = modbus.uint16(27)/10.0;
    int   SOC     = modbus.uint16(18);
    float GridV   = modbus.uint16(20)/10.0;
    float OutV    = modbus.uint16(22)/10.0;
    float OutA    = modbus.uint16(34)/10.0;
    float BAT_D   = modbus.uint32(60) / 10.0;
    float BAT_T   = modbus.uint32(62) / 10.0;
    long  GRID_IN  = modbus.uint32(36) / 10;
    long  BAT_DIS = modbus.uint32(73) / 10;
    long  BATCD   = (long)modbus.uint32(77) / 10;
    String rawStr = "";
    for (int i =0; i < len; i++) {
      char txt[3];
      sprintf(txt, "%02X",raw[i]);
      rawStr += String(txt);
      // Serial.printf("%02X", raw[i]);
    }
    Serial.println(rawStr);
    Serial.printf("PV_V: %4d   ", PV_V);
    Serial.printf("PV_P: %4d   ", PV_P);
    Serial.printf("LOAD: %3.1fV   %3.1fA %4dW  %4.1f%%   ",OutV,OutA, Load_P,Load);
 
    Serial.printf("SOC: %2d%%   ", SOC);
    Serial.printf("GRID_V: %3.1f   ", GridV);
    Serial.printf("BAT TODAY: %2.1f/%0.1f Kwh   ", BAT_D,BAT_T);
    Serial.printf("GRID_IN: %4d W   BAT_DIS: %4d W   ", GRID_IN, BAT_DIS);
    Serial.printf("BAT %S %4d W", (BATCD > 0? "DIS": (BATCD < 0? "CH ": "NA")),  BATCD);
     Serial.println();
    if(mqtt.connected()){

      //Serial.println("Publish message");
      mqtt.publish(INVRES,rawStr.c_str());
      mqtt.publish(INVRAW,raw,len);
    }else {
      Serial.println("Not Connect");
    }
  }
  timer = millis();
 }
  mqtt.loop();
}
