/*-------------------------------------------------------------------------------For Display Libraries-------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, D2, D1);  //D2=SDK  D1=SCK  As per labeling on NodeMCU
/*-------------------------------------------------------------------------------Blockchain Info-------------------------------------------------------------------------------*/
String pubkey = ""; //Your public key
/*--------------------------------------------------------------------------------Setup----------------------------------------------------------------------------------- */
void displaysetup() {
  Serial.begin(115200);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void setup() {
  displaysetup();
  pinMode(D7,OUTPUT);
}
/*--------------------------------------------------------------------------------Main Program-------------------------------------------------------------------------------- */
void loop(){
    digitalWrite(D7,HIGH);
    delay(1000);
    digitalWrite(D7,LOW);
    delay(1000);
}