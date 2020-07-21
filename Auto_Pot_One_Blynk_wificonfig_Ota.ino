
#include <FS.h>  
/* Comment this out to disable prints and save space */
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

char blynk_token[34] = "vqmXhqzPZVclHkQwJRUWHVJPQzZhDe8Z"; // sam_simsim@hotmail.com  tfr10sj1

bool shouldSaveConfig = false; //flag for saving data

int motorPin = 14;
int static timeflag = 0;
float static water_level = 2000;
int static newHour = 0;
int static newMinute = 0;
int static newSecond = 0;
int static newDay = day();
BlynkTimer timer;
WidgetRTC rtc;
WidgetLED led10(V10);

// numeric input Time and Amount
double amountV4 = 2 ;
long timeV0 = 6 ;
// Digital clock display of the time
void clockDisplay(){
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details
  String currentTime = "";
  String currentDate = "";
  if(newHour == 0 && newMinute == 0 && newSecond == 0 && newDay == 0){
    currentTime = "Next: "+ String(hour()) + ":" + String(minute()) + ":" + String(second());
    currentDate = String(day()) + " " + month() + " " + year();
  }
  else{
    currentTime = "Next: "+String(newHour) + ":" + newMinute + ":" + newSecond;
    currentDate = String(newDay) + " " + month() + " " + year();
  }
  // Send time to the App
  Blynk.virtualWrite(V1, currentTime);
  // Send Info to the App
  Blynk.virtualWrite(V3, currentDate);
}
void enable(int Delay){ // runs the pump depending on Delay time
  digitalWrite(motorPin, HIGH);
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
    //WateringOn(amountV4);
    if(amountV4/2 != 1){
     timer.enable((amountV4/2)*1000);
     Wateringoff();
      }
    else{
      //delay(1000);
      timer.enable(1000);
      Wateringoff();
      }
    if(water_level < 100){
      Blynk.notify("The water LEVEL is LOW. Fill your Watertank and Restart the Auto-Pot!");
    }
    Wateringoff();
    water_level -= amountV4;
    Blynk.virtualWrite(V6, water_level);
    timeflag = 1;
    if(newSecond+1 != second()){
      NextTime();
      Serial.println(String(newSecond) + ":"+String(second()));
    }
   // delay(2000);    //Send a request every 30 seconds
    
  }
  else {
    timeflag = 0; 
  }
  } 
   
BLYNK_WRITE(V0) {
  timeV0 = param[0];
}
BLYNK_WRITE(V4) {
  amountV4 = param[0].asDouble();
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
  timer.setInterval(2000L, runAuto);
    }
  }

BLYNK_WRITE(V5) {
  Serial.println("Plant 1 selected");
      water_level -= amountV4;
      Blynk.virtualWrite(V6, water_level);
}
BLYNK_CONNECTED() {
  // Synchronize time on connection
   rtc.begin();
}

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
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
  pinMode(BUILTIN_LED, OUTPUT);
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
  digitalWrite(BUILTIN_LED, HIGH);

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
  pinMode(motorPin, OUTPUT);
  led10.on();
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  
  timer.setInterval(5000L, clockDisplay);
  NextTime();
  clockDisplay();
  Blynk.virtualWrite(V6, water_level);
  Blynk.virtualWrite(V0, 6);
  Blynk.virtualWrite(V4, 4);
  
}

void loop()
{
  Blynk.run();
  timer.run();
  ArduinoOTA.handle();
}
