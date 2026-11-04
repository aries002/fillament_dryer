#include <Arduino.h>

void setup(){
    Serial.begin(115200);
    delay(200);
    

}
void loop(){
    Serial.println(analogRead(3));
    delay(500);
}