#include <FS.h>
/* Comment this out to disable prints and save space */
// Wemos D1 R1 
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266_SSL.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//for LED status
#include <Ticker.h>
Ticker ticker;

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

char blynk_token[34] = "vqmXhqzPZVclHkQwJRUWHVJPQzZhDe8Z"; 

bool shouldSaveConfig = false; //flag for saving data
int enA = 15; //enable pin in l298n
int motorPin = 14; // for mini D1 = 5, Nodemcu = 14, ESP_01 = 0;

int static timeflag = 0;
float static water_level = 1000;
int static newHour = 0;
int static newMinute = 0;
int static newSecond = 0;
int static newDay = day();

// numeric input Time and Amount
double static  amountV4 = 4.0 ;
double static Runtime = 0.6481; // amountV4 = 4.0

int static SwitchV5 = 0;
long static hourV0 = 24 ;
long static minV7 = 0;

BlynkTimer timer;
WidgetRTC rtc;
int timerIdrunAuto = 0;
int timerIdDisplay = 0;

void clockDisplay() {
  if(SwitchV5 == 1){
    String newhour = "";
    String newminute = "";
    String newsecund = "";
    String newday = "";
    String newmonth = "";
    String currentTime = "";
    String currentDate = "";
  
    newhour = String(newHour);
    newminute = String(newMinute);
    newsecund = String(newSecond);
    newday = String(newDay);
    newmonth = String(month());
  
    if (newHour < 10) {
      newhour = '0' + newhour;
    }
    if (newMinute < 10) {
      newminute = '0' + newminute;
    }
    if (newSecond < 10) {
      newsecund = '0' + newsecund;
    }
  
    if (newDay < 10) {
      newday = '0' + newday;
    }
    if (month() < 10) {
      newmonth = '0' + newmonth;
    }
    currentTime = "Next: " + newhour + ":" + newminute + ":" + newsecund;
    currentDate = "Date: " + newday + "/" + newmonth + "/" + String(year());
  
    // Send time to the App
    Blynk.virtualWrite(V1, currentTime);
    // Send Info to the App
    Blynk.virtualWrite(V3, currentDate);
  }
}
void WateringOn() {
  digitalWrite(enA, HIGH); // Send PWM signal to L298N Enable pin
  digitalWrite(motorPin, HIGH);
}

void Wateringoff() {
  digitalWrite(enA, LOW); // Send PWM signal to L298N Enable pin
  digitalWrite(motorPin, LOW);
}

void NextTime() {
  long totm = minV7 + minute();
  long extraHour = 0;
  if (totm <= 60) {
    newMinute = totm;
    if (totm == 60) {
      newMinute = 0;
      extraHour = 1;
    }
    else {
      newSecond = second();
    }
  }
  else
  {
    newMinute = (totm - 60);
    if (newMinute != 60) {
    }
    newSecond = second();
    extraHour = 1;
  }

  long toth = hourV0 + hour() + extraHour;
  if (toth <= 24)
  {
    newHour = toth;
    if (toth == 24) {
      newHour = 0;
      if (newMinute > 0) {
        newDay = day() + 1;
      }
      newSecond = second();
    }
    else {
      newSecond = second();
      newDay = day();
    }
  }
  else if(toth > 24){
    newHour = toth % 24;
    int days = toth / 24;
    if (newMinute > 0) {
      newDay = day() + days;
    }
  }
  else
  {
    newHour = (toth - 24);
    if (newHour != 24) {
      newHour = (newHour);
    }
    newSecond = second();
    newDay = day() + 1;
  }
}

void runAuto() {
  if (newHour == hour() && newMinute == minute() && newDay ==day() &&  timeflag == 0) {
    WateringOn();
    Serial.println("Runtime1 = " + String(Runtime));
    timer.setTimeout(Runtime * 1000, Wateringoff);
    Serial.println("Runtime2 = " + String(Runtime));
    water_level -= amountV4;
    if (water_level < 0) {
      if (SwitchV5 == 1) {
        Blynk.notify("The water LEVEL is to LOW. Fill your Watertank and Restart the Auto-Pot!");
        Blynk.virtualWrite(V6, 0);
        // Send time to the App
        Blynk.virtualWrite(V1, "No Water!");
        timer.disable(timerIdDisplay);
        timer.disable(timerIdrunAuto);
      }
    }
    else {
      if (SwitchV5 == 1) {
        Blynk.virtualWrite(V6, water_level);
      }
    }
    timeflag = 1;
    if (newSecond + 1 != second()) {
      NextTime();
    }

  }
  else {
    timeflag = 0;
  }
}

BLYNK_WRITE(V0) {
  if(SwitchV5 == 1){
    hourV0 = param[0];
  }
}
BLYNK_WRITE(V7) {
  if(SwitchV5 == 1){
    minV7 = param[0];
  }
}
BLYNK_WRITE(V4) {
  if(SwitchV5 == 1){
    amountV4 = param[0].asDouble();
    Runtime = double((1.1067 + amountV4) / 7.8797);
    Serial.println("BLYNK_WRITE(V4): " + String(Runtime));
  }
}
BLYNK_WRITE(V5) {
  SwitchV5 = param.asInt();
  if (SwitchV5 == 1) {
      if (water_level < 0) {
      Blynk.notify("The water LEVEL is to LOW. Fill your Watertank and Restart the Auto-Pot!");
      Blynk.virtualWrite(V6, 0);
      // Send time to the App
      Blynk.virtualWrite(V1, "No Water!");
      timer.disable(timerIdDisplay);
      timer.disable(timerIdrunAuto);
    }
    else{
      clockDisplay();
      Blynk.virtualWrite(V6, water_level);
    }
    Blynk.virtualWrite(V0, hourV0);
    Blynk.virtualWrite(V4, amountV4);
    
  }
}

BLYNK_WRITE(V2) {
  if(SwitchV5 == 1){
    if (hourV0 != 0 || minV7 != 0 || amountV4 != 0) {
      String AutoTime = "Autotime: " + String(hourV0) + ":" + String(minV7);
      String AutoAmount = "AutoAmuont: " + String(amountV4);
      // Send time to the App
      Blynk.virtualWrite(V1, AutoTime);
      // Send Info to the App
      Blynk.virtualWrite(V3, AutoAmount);
      NextTime();
      runAuto();
      //timer.setInterval(2000L, runAuto);
    }
  }
}
BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

void tick()
{
  //toggle state
  int state = digitalRead(2);  // get the current state of GPIO1 pin  // for ESP-01 the led input is 1
  digitalWrite(2, !state);     // set pin to the opposite state // for ESP-01 the led input is 1
}

void saveConfigCallback () {  //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
  ticker.attach(0.2, tick);  // led toggle faster
}

void setup()
{
  Serial.begin(9600);
  Serial.println();
  /****************************WIFI Configuration*******************************************/
  //set led pin as output
  pinMode(2, OUTPUT); // for ESP-01 the led input is 1
  pinMode(enA, OUTPUT);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  Wateringoff();
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //SPIFFS.format();    //clean FS, for testing
  Serial.println("Mounting FS...");    //read configuration from FS json

  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("Failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 33);   // was 32 length

  Serial.println(blynk_token);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);   //set config save notify callback

  //set static ip
  // this is for connecting to Office router not GargoyleTest but it can be changed in AP mode at 192.168.4.1
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,10,111), IPAddress(192,168,10,90), IPAddress(255,255,255,0));

  wifiManager.addParameter(&custom_blynk_token);   //add all your parameters here

  //wifiManager.resetSettings();  //reset settings - for testing

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep, in seconds
  wifiManager.setTimeout(600);   // 10 minutes to enter data and then Wemos resets to try again.

  //fetches ssid and pass and tries to connect, if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AUTO-Pot", "7754")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.println("Connected Auto-Pot Watering System :)");   //if you get here you have connected to the WiFi
  ticker.detach();
  //turn LED off
  digitalWrite(2, HIGH);// for ESP-01 the led input is 1

  strcpy(blynk_token, custom_blynk_token.getValue());    //read updated parameters

  if (shouldSaveConfig) {      //save the custom parameters to FS
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  Blynk.config(blynk_token);
  Blynk.connect();
  /****************************WIFI Configuration*******************************************/
  /**************************************************OTA*****************************************/
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /**************************************************OTA*****************************************/
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds

  timerIdDisplay = timer.setInterval(5000L, clockDisplay);
  NextTime();
  clockDisplay();
  Blynk.virtualWrite(V6, water_level);
  Blynk.virtualWrite(V0, 6);
  Blynk.virtualWrite(V4, 4);
  Blynk.virtualWrite(V7, 0);
  timerIdrunAuto = timer.setInterval(2000L, runAuto);
}

void loop()
{
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();

}
