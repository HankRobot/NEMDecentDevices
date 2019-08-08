/*-------------------------------------------------------------------------------For Display Libraries-------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, D2, D1);  //D2=SDK  D1=SCK  As per labeling on NodeMCU
/*-------------------------------------------------------------------------------For REST API-------------------------------------------------------------------------------*/
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "NBC_Tenant";
const char* password = "nbctenant1234!";
/*-------------------------------------------------------------------------------Blockchain Info-------------------------------------------------------------------------------*/
String pubkey = "DA71E422A3B22BE6CC691EC3A569C73137DA921F339CD1B5DDD5A39A2BD3F281"; //Your public key
/*--------------------------------------------------------------------------------Setup----------------------------------------------------------------------------------- */
void displaysetup() {
    Serial.begin(115200);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
}

void WifiSetup(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(1000);
        Serial.println("Connecting...");
    }
}

void setup() {
    displaysetup();
    WifiSetup();
    pinMode(D7,OUTPUT);
}
/*--------------------------------------------------------------------------------Main Program-------------------------------------------------------------------------------- */
int ledstatus;
void getblockchaininfo(){
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http; //Object of class HTTPClient
        http.begin("http://3.1.202.148:3000/account/DA71E422A3B22BE6CC691EC3A569C73137DA921F339CD1B5DDD5A39A2BD3F281/transactions");
        int httpCode = http.GET();

        if (httpCode > 0) {
            const size_t bufferSize = JSON_OBJECT_SIZE(7)+ 370;
            DynamicJsonBuffer jsonBuffer(bufferSize);
            JsonArray& rootarray = jsonBuffer.parseArray(http.getString());
            JsonObject& root = rootarray[0];

            const char* message = root["transaction"]["transactions"][0]["transaction"]["message"]["payload"];

            Serial.print("Message:");
            Serial.print(message);

            const char* ptr = strstr(message, "30");

            if(message == ptr){
                ledstatus = 0;
            }
            else
            {
                ledstatus = 1;
            }
        }
        http.end(); //Close connection
    }
}

void loop(){
    digitalWrite(D7,ledstatus);
    getblockchaininfo();
    delay(4000);
}
