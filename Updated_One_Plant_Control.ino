/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how to use the Menu Widget.

  App project setup:
    
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_SSL.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

int motorPin = 5;

int static timeflag = 0;
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7OglfGHWHXIPi-y-rHGDcjMANgIsncR1"; //"BHPhkqfZPTQpY23SPG-7hfiDiAEH4-o6";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ComHemF17542";
char pass[] = "tfr10sj1";

float static water_level = 2000;
int static newHour = 0;
int static newMinute = 0;
int static newSecond = 0;
int static newDay = day();
BlynkTimer timer;
WidgetRTC rtc;


// numeric input Time and Amount
long amountV4 = 1000 ;
long timeV0 = 0 ;

// Digital clock display of the time
void clockDisplay(){
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details
  String currentTime = "Next: "+String(newHour) + ":" + newMinute + ":" + newSecond;
  String currentDate = String(newDay) + " " + month() + " " + year();
  // Send time to the App
  Blynk.virtualWrite(V1, currentTime);
  // Send Info to the App
  Blynk.virtualWrite(V3, currentDate);
}
void WateringOn(int Delay){
  digitalWrite(motorPin, HIGH);
  delay(Delay);
}

void RunWateringOn(){
  WateringOn(amountV4);
}

void Wateringoff(){
  digitalWrite(motorPin, LOW);
}

void NextTime(){
  int tot = timeV0 + hour();
  if (tot <= 24)
  {
    newHour = tot;
    if (tot == 24) {
      newHour = 0;
      if(minute() > 0){
        newDay = day() + 1;
      }
      newMinute = minute();
      newSecond = second();
    }
    else{
    newMinute = minute();
    newSecond = second();
    newDay = day();
    }
  }
  else
  {
    newHour = (tot - 24);
    if (newHour != 24) {
      newHour = (newHour);
    }
    newMinute = minute();
    newSecond = second();
    newDay = day() + 1; 
  } 
}

void runAuto(){
  if (newHour == hour() && newMinute == minute() && newSecond == second() && timeflag == 0){
    int angelcounter = 0;
    WateringOn(amountV4);
    delay(2000);
    Wateringoff();
    water_level -= 5.7;
    Blynk.virtualWrite(V6, water_level);
    timeflag = 1;
    if(newSecond+1 != second()){
      NextTime();
      Serial.println(String(newSecond) + ":"+String(second()));
    }
    delay(2000);    //Send a request every 30 seconds
  }
  else {
    timeflag = 0; 
  }
  }
   
BLYNK_WRITE(V0) {
  timeV0 = param[0].asLong();
}
BLYNK_WRITE(V4) {
  amountV4 = param[0].asLong();
}

BLYNK_WRITE(V2) {
  if(timeV0 != 0 || amountV4 != 0){
  NextTime();
  String AutoTime = "Autotime: " + String(timeV0);
  String AutoAmount = "AutoAmuont: " + String(amountV4);
  // Send time to the App
  Blynk.virtualWrite(V1, AutoTime);
  // Send Info to the App
  Blynk.virtualWrite(V3, AutoAmount);
  runAuto();
    }
  }

BLYNK_WRITE(V5) {
  Serial.println("Plant 1 selected");
      water_level -= 10;
      Blynk.virtualWrite(V6, water_level);
}
BLYNK_CONNECTED() {
  // Synchronize time on connection
   rtc.begin();
}

void setup()
{
  pinMode(motorPin, OUTPUT);
  
  // Debug console
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  timer.setInterval(5000L, clockDisplay);
  Blynk.virtualWrite(V6, water_level);
  
}

void loop()
{
  Blynk.run();
  timer.run();
  runAuto();
}
