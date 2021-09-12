/**
 * Upload this Code on your NodeMCU Board
 */

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
//#include <SimpleTimer.h>
SoftwareSerial arduino(D1, D2);
char auth[] = "Paste your Blynk App Auth code here";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Your Wifi name";
char pass[] = "Your Wifi Password";

BlynkTimer timer;

String myString; // complete message from arduino, which consistors of snesors data
char rdata; // received charactors
int humidity,soil;
float tempC, tempF,waterLevel,soilPercentage; // sensors

int app_led_btn_state=0;
int app_water_pump_btn_state=0;
int app_fan_btn_state=0;

int mode=0;
void sendDataToBlynk()
{
  int local_humidity = humidity;
  float local_tempC = tempC;
  float local_tempF = tempF;
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V10, local_humidity);
  Blynk.virtualWrite(V2, local_tempC);
  Blynk.virtualWrite(V3, local_tempF);
  Blynk.virtualWrite(V1, soilPercentage);
  Blynk.virtualWrite(V4, waterLevel);
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
BLYNK_WRITE(V0) {
  mode = param.asInt();
}

BLYNK_WRITE(V9) {
  app_water_pump_btn_state = param.asInt();
  Serial.println(app_water_pump_btn_state);
}
BLYNK_WRITE(V13) {
  app_led_btn_state = param.asInt();
}

BLYNK_WRITE(V12) {
  app_fan_btn_state = param.asInt();
}
void setup()
{
  // Debug console
  Serial.begin(9600);
  arduino.begin(9600);
  Serial.println("Setup function called");
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendDataToBlynk); 
}

String cdata;
void loop()
{
  if (Serial.available() == 0 )
  {
    cdata = String(mode)+","+(String)app_water_pump_btn_state+","+(String)app_led_btn_state+","+(String)app_fan_btn_state;
//    Serial.println(cdata);
    arduino.println(cdata);
    Blynk.run();
    timer.run(); // Initiates BlynkTimer
    cdata = "";
  }

  if (Serial.available() > 0 )
  {
    rdata = Serial.read();
    myString = myString + rdata;
    //Serial.print(rdata);
    if ( rdata == '\n')
    {
      humidity = getValue(myString, ',', 0).toInt();
      tempC = getValue(myString, ',', 1).toFloat();
      tempF = getValue(myString, ',', 2).toFloat();
      waterLevel = getValue(myString, ',', 3).toFloat();
      soilPercentage = getValue(myString, ',', 4).toFloat();
      myString = "";
    }
  }
}
