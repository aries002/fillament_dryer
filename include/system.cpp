#include <Arduino.h>
#include <WiFi.h>

#include "system.h"

void flip_pin(int pin_number){
  if(digitalRead(pin_number) == HIGH){
    digitalWrite(pin_number, LOW);
  }else{
    digitalWrite(pin_number, HIGH);
  }
}

void wifi_scan(){
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");
}


// conect to wifi
bool wifi_connect(char *SSID, char *pass, char *hostname = "ESP-Dev", int time_out = 0){
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.setAutoReconnect(true);
  #ifdef LIMIT_WIFI_TX_POWER
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #endif
  WiFi.begin(SSID, pass);
  Serial.print("Connecting to ");
  Serial.print(SSID);
  while (WiFi.status() != WL_CONNECTED && time_out > 0)
  {
    Serial.print(".");
  }
  Serial.println("");
  return (WiFi.status() != WL_CONNECTED);
}

