/******************************************************INCLUDE LIBRARIES********************************************************/
//Humidity Sensor
#include "DHT.h"

//OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//IR Receiver
//Removed IR Sensor because of some issues
#include <IRremote.h>

/******************************************************DEFINE SENSORS PIN & VARIABLES********************************************/
#define PHOTO_REGISTER A0
#define RED_LIGHT_PIN 3
#define GREEN_LIGHT_PIN 5
#define BLUE_LIGHT_PIN 6
#define IR_RECEIVER 8
#define FAN_RELAY 12
#define WATER_PUMP_RELAY 9
#define HUMIDITY 10
#define BUZZER 11
#define LED 13

//OLED define
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define HUMIDITY_SENSOR DHT11
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int photocellReading;
int LEDbrightness;
float humi;
float tempC;
float tempF;
IRrecv irrecv(IR_RECEIVER);
DHT HT(HUMIDITY, HUMIDITY_SENSOR);

void setup() {
  Serial.begin(9600);

  pinMode(IR_RECEIVER, INPUT);
  pinMode(WATER_PUMP_RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  irrecv.enableIRIn();
  HT.begin();
  //For OLED I2C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display(); //Display logo
  delay(1000);
  display.clearDisplay();
}

void loop(void) {
  if (Serial.available()) {
    String cmd = Serial.readString();
    if (cmd == "on") {
      startMotor();
    }
  }

  //IR Receiver
    if (irrecv.decode() ) {
      Serial.println(irrecv.decodedIRData.command);
      if (irrecv.decodedIRData.command == 15) {
        startMotor();
        irrecv.resume(); // Receive the next value
      }
    }
  //Humidity Sensor
  humi = HT.readHumidity();
  tempC = HT.readTemperature();
  tempF = HT.readTemperature(true);
  
  oledDisplayHeader();
  oledDisplay(3, 5, 28, humi, "%");
  oledDisplay(2, 70, 16, tempC, "C");
  oledDisplay(2, 70, 44, tempF, "F");
  display.display();

  //3 Color LED Module
  if (tempC < 30) {
    //Turn OFF FAN
    digitalWrite(FAN_RELAY,LOW);
    
    //Turn the GREEN LED
    analogWrite(GREEN_LIGHT_PIN, 255);
    analogWrite(BLUE_LIGHT_PIN, 0);
    analogWrite(RED_LIGHT_PIN, 0);
  } else if (tempC >= 30 && tempC < 40) {
    //Turn OFF FAN
    digitalWrite(FAN_RELAY,LOW);
    
    //Turn the BLUE LED
    analogWrite(RED_LIGHT_PIN, 0);
    analogWrite(BLUE_LIGHT_PIN, 255);
    analogWrite(GREEN_LIGHT_PIN, 0);
  } else {
    //Turn ON FAN
    digitalWrite(FAN_RELAY,HIGH);
    
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

  //Print the data on serial monitor
  serialPrintData();
  delay(1000);
}


void oledDisplayHeader() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Humidity");
  display.setCursor(60, 0);
  display.print("Temperature");
}
void oledDisplay(int size, int x, int y, float value, String unit) {
  int charLen = 12;
  int xo = x + charLen * 3.2;
  int xunit = x + charLen * 3.6;
  int xval = x;
  display.setTextSize(size);
  display.setTextColor(WHITE);

  if (unit == "%") {
    display.setCursor(x, y);
    display.print(value, 0);
    display.print(unit);
  } else {
    if (value > 99) {
      xval = x;
    } else {
      xval = x + charLen;
    }
    display.setCursor(xval, y);
    display.print(value, 0);
    display.drawCircle(xo, y + 2, 2, WHITE); // print degree symbols (  )
    display.setCursor(xunit, y);
    display.print(unit);
  }

}

//This function will starts the motor for 3 seconds
void startMotor() {
  //start the motor
  digitalWrite(WATER_PUMP_RELAY, HIGH);
  tone(BUZZER, 1000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(BUZZER);     // Stop sound...
  
  delay(3000);

  //stop the motor after 3 secons
  digitalWrite(WATER_PUMP_RELAY, LOW);
  tone(BUZZER, 1000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(BUZZER);     // Stop sound...
}

//This function will display the data in serial monitor
void serialPrintData(){
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
}
