#include <Arduino.h>
#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include "Adafruit_SHTC3.h"

// put function declarations here:
#define NAMA_PROYEK "Dryer Fillament"
#define FAN_PIN                 2
#define HEATER_PIN              1
#define LED_PIN                 4
#define SENSOR_PIN              3
#define HEATER_ACTIVE_LOW       false
#define FAN_ACTIVE_LOW          false
// #define HEATER_MIN_TIME         1000
#define HEATER_LIMIT            70
#define MIN_HEATER_RAISE        1
#define NEAR_TARGET_MARGIN      2
#define HEATER_RAISE_TIME       20000

#define REFERENCE_RESISTANCE    20000
#define NOMINAL_RESISTANCE      10000
#define NOMINAL_TEMPERATURE     25
#define B_VALUE                 3950
// #define B_VALUE                 3380
#define ESP32_ANALOG_RESOLUTION 4095
#define ESP32_ADC_VREF_MV       3300

#define ON HIGH
#define OFF LOW

#define MONITOR_SPEED 115200
#define STATUS_INTERVAL 500
#define UPDATE_INTERVAL 1000

unsigned long last_status = 0;
unsigned long last_update = 0;
unsigned long last_heater_chage = 0;

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();
Thermistor* thermistor;

double chamber_temp = 0;
double chamber_humid = 0;
double heater_temp = 0;
// double last_heater_temp = 0;
double start_temp = 0;
double set_temp = 0;
double toleransi = 2;
bool heater_on = false;
bool fan_on = false;
bool heater_check = false;
bool heater_check_enabled = false;

void flip_pin(int pin_number){
  if(digitalRead(pin_number) == HIGH){
    digitalWrite(pin_number, LOW);
  }else{
    digitalWrite(pin_number, HIGH);
  }
}

void fan_controll(bool on, bool force = false){
  if(on == fan_on && !force) return;
  fan_on = on;
  digitalWrite(FAN_PIN, (fan_on ^ FAN_ACTIVE_LOW)? ON : OFF);
}

void heater_controll(bool on, bool force = false){
  if(on == heater_on && !force) return;
  heater_on = on;
  // if(heater_on && !fan_on)fan_controll(true);
  digitalWrite(HEATER_PIN, (heater_on ^ HEATER_ACTIVE_LOW)? ON : OFF);
  last_heater_chage = millis();
  start_temp = heater_temp;
  heater_check = (on)? false:true;
}

void failsafe(){
  set_temp = 0;
  heater_on = false;
  fan_on = false;
  digitalWrite(HEATER_PIN, (false ^ HEATER_ACTIVE_LOW)? ON : OFF);
  digitalWrite(FAN_PIN, (false ^ FAN_ACTIVE_LOW)? ON : OFF);
  Serial.println(F("FAILSAVE TRIGGERD!!"));
}

// pengecekean keamanan pemanas
void safety_check(){
  // matikan pemanas jika lebih dari
  if (heater_temp > HEATER_LIMIT && (set_temp > 0 || heater_on)){
    Serial.println("Heater over temp!");
    failsafe();
  }

  // cek apakah pemanas berhasil memanas ketika menyala
  if(heater_check_enabled && !heater_check && heater_on && last_heater_chage + (HEATER_RAISE_TIME) <= millis()){
    heater_check = true;
    bool near_target = heater_temp >= (set_temp - NEAR_TARGET_MARGIN);
    // matikan jika gagal memanaskan pemanas kurang dari MIN_HEATER_RAISE
    if(!near_target && heater_temp <= (start_temp + MIN_HEATER_RAISE)){
      Serial.println(F("Heater failed!!"));
      failsafe();
    }
    // failsafe jika pemanas masih meningkat tapi heater mati
    // if(!heater_on && start_temp + MIN_HEATER_RAISE < heater_temp){
    //   Serial.println(F("Pemanas tetap menyala"));
    //   failsafe();
    //   fan_controll(true,true); // tetap nyalakan kipas
      
    // }
  }
}

void temp_controll(double temp){
  // logika penyalaan pemanas
  // nyalakan kipas ketika suhu lebih dari 40
  if(temp > 40.0 && !fan_on) fan_controll(true);
  if(temp <= 39.0 && fan_on) fan_controll(false);

  if(set_temp <= 0 && heater_on){
    heater_controll(false);
    return;
  }else if (set_temp <= 0)
  {
    return;
  }

  // bool elpased = (millis() - (HEATER_MIN_TIME+last_heater_chage));
  // logika pemanas utama
  double high = set_temp + toleransi / 2.0;
  double low = set_temp - toleransi / 2.0;
  if(heater_on && temp >= high) heater_controll(false);
  if(!heater_on && temp <= low) heater_controll(true);
  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(MONITOR_SPEED);
  delay(200);
  Serial.println("Starting!!");


  // setup IO
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  delay(200);
  fan_controll(false, true);
  heater_controll(false, true);
  digitalWrite(LED_PIN,HIGH);

  shtc3.begin();
  thermistor = new NTC_Thermistor_ESP32(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE,
    ESP32_ADC_VREF_MV,
    ESP32_ANALOG_RESOLUTION
  );

  Serial.println("Started!!");

}

void handle_serial(){
  if (Serial.available() && Serial.read() == 's') {
    float v = Serial.parseFloat();
    if (v < HEATER_LIMIT) {
      set_temp = v;
      Serial.print(F("Setpoint -> ")); Serial.println(set_temp, 1);
    }
  }
}

void print_data(){
  // digunakan untuk menampilkan data untuk debugig
  if(last_status + STATUS_INTERVAL <= millis()){
    Serial.print("SET TEMP = ");
    Serial.print(set_temp);
    Serial.print("|");
    Serial.print("HEATER_TEMP = ");
    Serial.print(heater_temp);
    Serial.print("/");
    Serial.print(start_temp + MIN_HEATER_RAISE);
    Serial.print("|");
    Serial.print("CHAMBER = ");
    Serial.print(chamber_temp);
    Serial.print("C/");
    Serial.print(chamber_humid);
    Serial.print("%");
    Serial.print("|");
    Serial.print("FAN ");
    Serial.print((fan_on)? "ON" : "OFF");
    Serial.print("|");
    Serial.print("HEATER ");
    Serial.print((heater_on)? "ON" : "OFF");
    Serial.println("");
    last_status = millis();
  }
}
double read_heater_temp(){
  // membaca suhu pemanas menggunakan rata rata
  double sum = 0;
  for(int i = 0; i < 16; i++) sum += thermistor->readCelsius();
  return sum / 16.0;
}
void loop() {
  handle_serial();
  // last_heater_temp = heater_temp;
  heater_temp = read_heater_temp();
  safety_check();
  temp_controll(heater_temp);
  print_data();
  

  if(last_update + UPDATE_INTERVAL <= millis()){
    sensors_event_t humidity, temp;
    shtc3.getEvent(&humidity, &temp);
    chamber_humid = humidity.relative_humidity;
    chamber_temp = temp.temperature;
    flip_pin(LED_PIN);

    last_update = millis();
  }
}
