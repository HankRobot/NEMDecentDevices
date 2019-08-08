/*-------------------------------------------------------------------------------For WIFI Libraries-------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>
/*-------------------------------------------------------------------------------For REST API-------------------------------------------------------------------------------*/
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "NBC_Tenant";
const char* password = "nbctenant1234!";
/*--------------------------------------------------------------------------------Setup----------------------------------------------------------------------------------- */
void WifiSetup(){
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(1000);
        Serial.println("Connecting...");
    }
}

void setup() {
    WifiSetup();
    pinMode(15,OUTPUT);
}
/*--------------------------------------------------------------------------------Main Program-------------------------------------------------------------------------------- */
int ledstatus;
void getblockchaininfo(){
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http; //Object of class HTTPClient
        http.begin("http://3.1.202.148:3000/account/CAB20564F345507D1C75FE0BEE8EA36720AE5FC6DCDDE7F04BFECE44E1EC45F1/transactions");
        int httpCode = http.GET();

        if (httpCode > 0) {
            const size_t bufferSize = JSON_OBJECT_SIZE(7)+ 370;
            DynamicJsonBuffer jsonBuffer(bufferSize);
            JsonArray& rootarray = jsonBuffer.parseArray(http.getString());
            JsonObject& root = rootarray[0];

            const char* message = root["transaction"]["transactions"][1]["transaction"]["message"]["payload"];

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
    digitalWrite(15,ledstatus);
    getblockchaininfo();
    delay(4000);
}
