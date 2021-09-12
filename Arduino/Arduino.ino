/**
 * Upload this Code on your Arduino UNO Board
 */


/******************************************************INCLUDE LIBRARIES********************************************************/
#define BLYNK_PRINT Serial
#include <SoftwareSerial.h>

//Humidity Sensor
#include "DHT.h"

/******************************************************DEFINE SENSORS PIN & VARIABLES********************************************/
#define PHOTO_REGISTER A0
#define SOIL A1
#define WATER_LEVEL A2

#define GREEN_LIGHT_PIN 5
#define BLUE_LIGHT_PIN 6
#define RED_LIGHT_PIN 8
#define WATER_PUMP_RELAY 9
#define HUMIDITY 10
#define BUZZER 11
#define FAN_RELAY 12
#define LED 13

SoftwareSerial nodemcu(2, 3);
#define HUMIDITY_SENSOR DHT11

int photocellReading;
int LEDbrightness;
float humi;
float tempC;
float tempF;
int soilSensorValue;
int motorbuzzer = 0;
int fanbuzzer = 0;
int water_level_value;
String cdata;
String myString;
char rdata;
String mode;
int app_water_pump_btn;
int app_led_btn;
int app_fan_btn;
float waterLevelPercentage=0;
float soilPercentage=0;
DHT HT(HUMIDITY, HUMIDITY_SENSOR);

void Manual(){
  Serial.println("Manual mode");
  digitalWrite(WATER_PUMP_RELAY,app_water_pump_btn); 
  digitalWrite(LED,app_led_btn);
  digitalWrite(FAN_RELAY,app_fan_btn);
}
void Auto() {
  Serial.println("Auto mode");
  //3 Color LED Module
  if (tempC < 30) {
    //Turn OFF FAN
    toggleFan("off");

    //Turn the GREEN LED
    analogWrite(GREEN_LIGHT_PIN, 255);
    analogWrite(BLUE_LIGHT_PIN, 0);
    analogWrite(RED_LIGHT_PIN, 0);
  } else if (tempC >= 30 && tempC < 40) {
    //Turn ON FAN
    toggleFan("on");

    //Turn the BLUE LED
    analogWrite(RED_LIGHT_PIN, 0);
    analogWrite(BLUE_LIGHT_PIN, 255);
    analogWrite(GREEN_LIGHT_PIN, 0);
  } else {
    //Turn ON FAN
    toggleFan("on");

    //Turn the RED LED
    analogWrite(RED_LIGHT_PIN, 255);
    analogWrite(BLUE_LIGHT_PIN, 0);
    analogWrite(GREEN_LIGHT_PIN, 0);
  }

  //Photo Register
  photocellReading = analogRead(PHOTO_REGISTER);
  photocellReading = 1023 - photocellReading;
  LEDbrightness = map(photocellReading, 0, 1023, 0, 255);
  analogWrite(LED, LEDbrightness);

  if (soilSensorValue > 700) {
    waterMotor("on");
  }else{
    waterMotor("off");
  }
}

void setup() {
  Serial.begin(9600);
  nodemcu.begin(9600);
  pinMode(WATER_PUMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  HT.begin();
  fanbuzzer=1;
}

void loop(void) {
  water_level_value = analogRead(WATER_LEVEL);
  waterLevelPercentage = float(water_level_value)*100/1024;
  soilSensorValue = analogRead(SOIL);
  soilPercentage = 100-(float(soilSensorValue)*100/1024);
  humi = HT.readHumidity();
  tempC = HT.readTemperature();
  tempF = HT.readTemperature(true);
  if (Serial.available() > 0 )
  {
    rdata = Serial.read();
    myString = myString + rdata;
    //Serial.print(rdata);
    if ( rdata == '\n')
    {
      mode = getValue(myString, ',', 0);
      app_water_pump_btn = getValue(myString, ',', 1).toInt();
      app_led_btn = getValue(myString, ',', 2).toInt();
      app_fan_btn = getValue(myString, ',', 3).toInt();
      myString = "";
    }
  }
  if (Serial.available() == 0 ) {
    cdata = cdata + humi + "," + tempC + "," + tempF+","+waterLevelPercentage+","+soilPercentage; //comma will be used a delimeter
    Serial.println(cdata);
    nodemcu.println(cdata);
    //Print the data on serial monitor
    //serialPrintData();
    if (mode=="0") {
      Manual();
    } else {
      Auto();
    }
    delay(1000); // 100 milli seconds
    cdata = "";
  }
}

//This function will turn ON / OFF the motor
void waterMotor(String cmd) {
  if (cmd == "on") {
    //start the motor
    digitalWrite(WATER_PUMP_RELAY, HIGH);
    if (motorbuzzer == 0) {
      tone(BUZZER, 1000);
      delay(1000);
      noTone(BUZZER);
      motorbuzzer = 1;
    }
  } else {
    //stop the motor
    digitalWrite(WATER_PUMP_RELAY, LOW);
    if (motorbuzzer == 1) {
      tone(BUZZER, 1000);
      delay(1000);
      noTone(BUZZER);
      motorbuzzer = 0;
    }
  }
}

//This function will turn ON / OFF the FAN
void toggleFan(String cmd) {
  if (cmd == "on") {
    digitalWrite(FAN_RELAY, HIGH);
    if (fanbuzzer == 0) {
      tone(BUZZER, 1000);
      delay(1000);
      noTone(BUZZER);
      fanbuzzer = 1;
    }
  } else {
    digitalWrite(FAN_RELAY, LOW);
    if (fanbuzzer == 1) {
      tone(BUZZER, 1000);
      delay(1000);
      noTone(BUZZER);
      fanbuzzer = 0;
    }
  }
}

//This function will display the data in serial monitor
void serialPrintData() {
  //Humidity Sensor Details
    Serial.print("Humidity:");
    Serial.print(humi, 0);
    Serial.print("%");
    Serial.print(" Temperature:");
    Serial.print(tempC, 1);
    Serial.print("C ~ ");
    Serial.print(tempF, 1);
    Serial.println("F");

  //Photo Register Sensor Details
    Serial.print("Analog reading = ");
    Serial.print(photocellReading);     // the raw analog reading
  
    if (photocellReading < 10) {
      Serial.println(" - Dark");
    } else if (photocellReading < 200) {
      Serial.println(" - Dim");
    } else if (photocellReading < 500) {
      Serial.println(" - Light");
    } else if (photocellReading < 800) {
      Serial.println(" - Bright");
    } else {
      Serial.println(" - Very bright");
    }
    Serial.println("Soil Sensor : ");
    Serial.print(soilSensorValue);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
