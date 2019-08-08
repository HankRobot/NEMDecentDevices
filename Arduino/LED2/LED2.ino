#include "ESP8266.h"

const char *SSID     = "NBC_Tenant";
const char *PASSWORD = "nbctenant1234!";

SoftwareSerial mySerial(10, 11); //SoftwareSerial pins for MEGA/Uno. For other boards see: https://www.arduino.cc/en/Reference/SoftwareSerial

ESP8266 wifi(mySerial); 

void setup(void)
{
  //Start Serial Monitor at any BaudRate
  Serial.begin(9600);
  Serial.println("Begin");

  if (!wifi.init(SSID, PASSWORD))
  {
    Serial.println("Wifi Init failed. Check configuration.");
    while (true) ; // loop eternally
  }
}

char* request = "POST /fcm/send HTTP/1.1\r\nHost: fcm.googleapis.com\r\ncontent-type: application/json\r\ncontent-length: 200\r\nauthorization: key=AIzaSyAFRHNLIRXjVkEHHhzkEYk3_cycj2yVkv0\r\nConnection: close\r\n\r\n{\r\n\t\"to\": \"/topics/general\",\r\n\t\"notification\":{\r\n\t\t\"title\": \"Motion detected!\",\r\n\t\t\"body\": \"An activity was registered by sensor.\",\r\n\t\t\"sound\": \"default\"\r\n\t},\r\n\t\"data\": {\r\n\t\t\"sensorValue\": \"01\"\r\n\t}\r\n}";
void loop(void)
{   
    Serial.println("Sending Request to www.google.com");
    wifi.httpGet(request);

    delay(4000);
}