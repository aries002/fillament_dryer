#include <Arduino.h>
#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include "Adafruit_SHTC3.h"

#include <WiFi.h>
#include "esp_bt.h"

#include "system.cpp"

#define LED_PIN                 8


void setup(){
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  wifi_scan();
  // esp_bt_controller_disable();
  // WiFi.begin("wireless@umsida.ac.id", "AhmadDahlan1868");
  // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   Serial.print("*");
  //   delay(1000);
  //   flip_pin(LED_PIN);
  // }
  // Serial.println("");
  // if(WiFi.status() == WL_CONNECTED){
  //   Serial.println("Connected..");
  //   Serial.println(WiFi.localIP());
  // }else{
  //   Serial.println("Not connected!");
  // }
  // digitalWrite(LED_PIN, HIGH);
  wifi_connect("wireless@umsida.ac.id", "AhmadDahlan1868");
  
}

unsigned long last_check = 0;

void loop(){
  if(millis() >= last_check + 1000){
    last_check = millis();
    Serial.print(last_check/1000);
    Serial.print(" ");
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Wifi not connected!");
      flip_pin(LED_PIN);
    }else{
      Serial.print("Wifi connceted, IP ");
      Serial.println(WiFi.localIP());
      digitalWrite(LED_PIN, LOW);
    }
  }
}