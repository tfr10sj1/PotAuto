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
#include <Servo.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "BHPhkqfZPTQpY23SPG-7hfiDiAEH4-o6";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ComHemF17542";
char pass[] = "tfr10sj1";

float static water_level = 1000;


BlynkTimer timer;
WidgetRTC rtc;
Servo servo;

// numeric input Time and Amount
long amountV4 = 10 ;
long timeV0 = 6 ;
// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = "Auto time: " + String(timeV0);//String(hour()) + ":" + minute() + ":" + second();
  //String currentDate = String(day()) + " " + month() + " " + year();
  String currentInfo = "Auto Amuont: " + String(amountV4);
  // Send time to the App
  Blynk.virtualWrite(V1, currentTime);
  // Send Info to the App
  Blynk.virtualWrite(V3, currentInfo);
}


BLYNK_WRITE(V0) {
  timeV0 = param[0].asLong();
}
BLYNK_WRITE(V4) {
  amountV4 = param[0].asLong();
}

BLYNK_WRITE(V2) {
  if(timeV0 != 0 || amountV4 != 0){
    // Send time to the App
    Blynk.virtualWrite(V1, timeV0);
    // Send date to the App
    Blynk.virtualWrite(V3, amountV4);
    }
  }

BLYNK_WRITE(V5) {
  switch (param.asInt())
  {
    case 1: // Item 1
      Serial.println("Plant 1 selected");
      water_level -= 10;
      Blynk.virtualWrite(V6, water_level);
      servo.write(0);
      break;
    case 2: // Item 2
      Serial.println("Plant 2 selected");
      water_level -= 10;
      Blynk.virtualWrite(V6, water_level);
      servo.write(60);
      break;
    case 3: // Item 3
      Serial.println("Plant 3 selected");
      water_level -= 10;
      Blynk.virtualWrite(V6, water_level);
      servo.write(120);
      break;
    default: // Item 4
      Serial.println("Plant 4 selected");
      water_level -= 10;
      Blynk.virtualWrite(V6, water_level);
      servo.write(180);
  }
}
BLYNK_CONNECTED() {
  // Synchronize time on connection
   rtc.begin();
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  // Other Time library functions can be used, like:
  //   timeStatus(), setSyncInterval(interval)...
  // Read more: http://www.pjrc.com/teensy/td_libs_Time.html


  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  timer.setInterval(10000L, clockDisplay);
  servo.attach(2);
  Blynk.virtualWrite(V6, water_level);
}

void loop()
{
  
  Blynk.run();
  timer.run();
}
