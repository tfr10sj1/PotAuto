/*
Authur: Sam Jabbar
Date: 8/8/2019

Pin  function DS3231 pin
1  GND          PINS 5,6,7,8
2  N/C
3  SCL          16
4  SDA          15
5  VCC          2
*/

#include <Servo.h>
#include <ESP8266WiFi.h>;
#include <ESP8266HTTPClient.h>;
#include <ESP8266WebServer.h>
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
//https://iotguider.in/esp8266-nodemcu/learn-interfacing-ds3231-rtc-module-with-nodemcu/
RTC_DS3231 rtc;
// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

String cDay = ""; 
String oldYear = "";
String oldMonth = "";
String oldDay = "";
String oldHour = "";
String oldMinute = "";

String Hour = "06";
String Minute = "30";

String Direction = "vi";
String Delayv = "0";
String Delayh = "0";
String newHour = "06";
String newMinute = "30";
//int angle = 0;
//int MAX_angle = 105;
//int MIN_angle = 9;
//int MID_angle = 32;
//int a = 0;
//int b = 0;
//int limit = 0;
int timeflag = 0;
String History[32];
static int  vin = 0;
ESP8266WebServer server(80); // Create a webserver object listens HTTP request on port 80

static String rtcYear;
static String rtcMonth;
static String rtcDay;
static String rtcHour;
static String rtcMinute;
static String rtcSecund;

const char* ssid = "ComHemF17542";
const char* password = "ytkwsqqu";

Servo servo;
int count = 6; // repeat every 6 hours
int ENA = 10;
int IN1 = 0;





void setup() {
  // pump
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  
  // servo 
  servo.attach(2); //D4
  servo.write(0);
 // delay(100);
 
  
  #ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
  #endif

  Serial.begin(115200);

  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  if (rtc.lostPower()) 
  {
    //Serial.println("RTC lost power, lets set the time!"); 
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2019, 8, 26, 9, 37, 0));
  }
  
  WiFi.begin(ssid, password);
  // set all the motor control pins to outputs
  while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.println("Connecting..");
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //Print the local IP
 
    server.on("/", HTTP_GET, handleRoot);        // Call 'handleRoot' function
    server.on("/login", HTTP_POST, handleLogin); // Call 'handleLogin' function when a POST request made to "/login"
    server.onNotFound(handleNotFound);           // call function "handleNotFound" when unknown URI requested
  
    server.begin();                            // start the server
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://worldclockapi.com/api/jsonp/cet/now?callback=mycallback");  //Specify request destination
    int httpCode = http.GET();  //Send the request
    if (httpCode > 0) //Check the returning code
    { 
      String payload = http.getString();   //Get the request response payload
      for(int i = 41; i< 60; i++)
      { 
        if(i > 40 && i < 45)
          {
            oldYear = oldYear + payload[i];
          }
        if(i > 45 && i < 48 )
          {
            oldMonth  = oldMonth + payload[i];
          }
        if(i > 48 && i < 51 )
          {
            oldDay  = oldDay + payload[i];
          }
        if(i > 51 && i < 54)
          {
            oldHour = oldHour + payload[i];
          }
        if (i> 54 && i < 57)
          {
            oldMinute = oldMinute + payload[i];
          } 
       }
    }
    http.end();   //Close connection
}
void loop() 
{   
  DateTime now = rtc.now(); // rtc DS3231 timenow
  rtcYear = String(now.year());
  rtcMonth = String(now.month());    
  rtcDay = String(now.day());
  rtcHour = String(now.hour());
  rtcMinute = String(now.minute()); 
  
  server.handleClient();                     // Listen for HTTP requests
  handleserver();
  String currentYear = "";
  String currentMonth = "";
  String currentDay = "";
  String currentHour = "";
  String currentMinute = "";
  
  if (WiFi.status() == WL_CONNECTED) 
  { //Check WiFi connection status
    static String payload;
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://worldclockapi.com/api/jsonp/cet/now?callback=mycallback");//"http://worldclockapi.com/api/jsonp/cet/now?callback=mycallback");  //Specify request destination
    int httpCode = http.GET();  //Send the request
    if (httpCode > 0) //Check the returning code
    { 
      payload = http.getString();   //Get the request response payload
      for(int i = 41; i< 60; i++)
      { 
        if(i > 40 && i < 45)
          {
            currentYear = currentYear + payload[i];
          }
        if(i > 45 && i < 48 )
          {
            currentMonth  = currentMonth + payload[i];
          }
        if(i > 48 && i < 51 )
          {
            currentDay  = currentDay + payload[i];
          }
        if(i > 51 && i < 54)
          {
            currentHour = currentHour + payload[i];
          }
        if (i> 54 && i < 57)
          {
            currentMinute = currentMinute + payload[i];
          } 
       }
    }
    http.end();   //Close connection
    if(payload.length()< 270)
    {
      currentYear = rtcYear;
      currentMonth = rtcMonth;    
      currentDay = rtcDay;
      currentHour = rtcHour;
      currentMinute = rtcMinute;
    }
    if(currentYear != oldYear)
    {
    oldYear = currentYear;
    }
    if(currentMonth != oldMonth)
    {
    oldMonth = currentMonth;
    }
    if(currentDay != oldDay)
    {
    oldDay = currentDay;
    cDay = currentDay;
    }
    if(currentHour != oldHour)
    {
    oldHour = currentHour;
    }
    if(currentMinute != oldMinute)
    {
    oldMinute = currentMinute;
    }  
    
  }
  if(newHour == currentHour && newMinute == currentMinute && timeflag == 0)
  {
    //MovingOn(0);
    //WateringOn(Delayh);
    //MovingOn(80);
    WateringOn(Delayv);
    NextTime();
    timeflag = 1;
    //Serial.println("History: ");
    ShowHistory();
    delay(600);    //Send a request every 30 seconds
  } 
  else if(newHour != currentHour || newMinute != currentMinute && timeflag == 1)
  {
    timeflag = 0;
  }
  servo.detach();
}
void handleRoot() 
{
  server.send(200, "text/html", 
"<!DOCTYPE html>"
"<html lang='en'>"
"<head>"
"<title>Watering System</title>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style> *{'box-sizing: border-box;'}"
"body {'font-family: Arial, Helvetica, sans-serif;'}"
/* Style the header */
"header {"
  "background-color: #e3eaa7;"
  "padding: 30px;"
  "text-align: center;"
  "font-size: 40px;"
  "color: white;"
"}"

/* Create two columns/boxes that floats next to each other */
"nav {"
  "float: left;"
  "width: 15%;"
  "height: 410px;" /* only for demonstration, should be removed */
  "background:#b5e7a0;"
  "padding: 20px;"
"}"
/* Style the list inside the menu */
"nav ul {"
  "list-style-type: none;"
  "padding: 0;"
"}"
"article {"
  "float: right;"
  "padding: 10px;"
  "width: 98.6%;"
  "background-color: #d5e1d0;"
  "height: 70px;" /* only for demonstration, should be removed */
"}"
/* Clear floats after the columns */
"section:after {"
  "content: "";"
  "display: table;"
  "clear: both;"
"}"
/* Style the footer */
"footer {"
  "background-color: #86af90;"
  "padding: 10px;"
  //"text-align: center;"
  "height: 430px;"
  "color: white;"
"}"
/* Responsive layout - makes the two columns/boxes stack on top of each other instead of next to each other, on small screens */
"@media (max-width: 400px) {"
  "nav, article {"
    "width: 98.6%;"
    "height: 70px;"
  "}"
"}"
"* {"
  "margin: 0;"
  "padding: 0;"
  "box-sizing: border-box;"
"}"
"</head>"
"<body>"
"<header>"
  "<h2>Login to Wateringsystem</h2>"
"</header>"
"<section>"
  "<nav>"
    "<ul>"
      "<li><a href='#'><form action=\"/login\" method=\"POST\"><input type=\"text\" name=\"uname\" placeholder=\"Username\"></br><input type=\"password\" name=\"pass\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Login\"></form>"
    "</a></li>"
    "</ul>"
  "</nav>"
  "<article>"
    "<a>&#169 Sam Jabbar</a>"
  "</article>"
"</section>"
"<footer>"

"<style>"
"table {"
  "border-collapse: collapse;"
  "width: 100%;"
"}"
 "td {"
  "text-align: left;"
  "padding: 12px;"
"}"
"</style>"

"<div style=\"overflow-x:auto;\">"
"<table id=\"myTable\">"
  "<tr>"
    "<td>"+History[1]+"</td>"
    "<td>"+History[11]+"</td>"
    "<td>"+History[21]+"</td>"
    "<td>"+History[31]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[2]+"</td>"
    "<td>"+History[12]+"</td>"
    "<td>"+History[22]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[3]+"</td>"
    "<td>"+History[13]+"</td>"
    "<td>"+History[23]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[4]+"</td>"
    "<td>"+History[14]+"</td>"
    "<td>"+History[24]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[5]+"</td>"
    "<td>"+History[15]+"</td>"
    "<td>"+History[25]+"</td>"
  "</tr>"
    "<tr>"
    "<td>"+History[6]+"</td>"
    "<td>"+History[16]+"</td>"
    "<td>"+History[26]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[7]+"</td>"
    "<td>"+History[17]+"</td>"
    "<td>"+History[27]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[8]+"</td>"
    "<td>"+History[18]+"</td>"
    "<td>"+History[28]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[9]+"</td>"
    "<td>"+History[19]+"</td>"
    "<td>"+History[29]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[10]+"</td>"
    "<td>"+History[20]+"</td>"
    "<td>"+History[30]+"</td>"
  "</tr>"
"</table>"
"</div>"
"</footer>"
"</body>"
"</html>"
);
}

void handleLogin() 
{                         //Handle POST Request
  if( ! server.hasArg("uname") || ! server.hasArg("pass") || server.arg("uname") == NULL || server.arg("pass") == NULL) // Request without data
  { 
    server.send(400, "text/plain", "400: Invalid Request");// Print Data on screen
    return;
  }
  if(server.arg("uname") == "sam_simsim"||server.arg("uname") == "Sam_simsim" && server.arg("pass") == "7754") 
  { // If username and the password are correct
    handleserver();
  } 
  else { // Username and password don't match
    server.send(401, "text/plain", "401: Invalid Credentials");
  }
}
void handleNotFound(){
  if(server.arg("action")== "x" || server.arg("action")== "X" )
    {
      WateringOn(Delayv);
    }
  if(server.arg("action") == "B" || server.arg("action") == "b" )
    {
      servo.attach(2); 
      servo.write(servo.read()+45);                 //command to rotate the servo to the specified angle
      delay(500);
      Serial.println(servo.read());
      servo.detach();
      
    }
  if(server.arg("action") == "-B" || server.arg("action") == "-b" )
    {   
        servo.attach(2); 
        servo.write(servo.read()-50);                 //command to rotate the servo to the specified angle
        delay(500);
        Serial.println(servo.read());
        servo.detach();
        
    }
   if(!server.hasArg("AutomaticWatering")&& server.arg("hour") != "" && server.arg("minute")!= "")
    {
      Hour = server.arg("hour");
      Minute = server.arg("minute");
    }
   if(!server.hasArg("Counter") && server.arg("counter")!= "")
    {
      count = server.arg("counter").toInt();
      newHour = oldHour;
      newMinute = oldMinute;
      NextTime();
    }
    if(!server.hasArg("Counter") && server.arg("Delayv")!= "")
    {
      Delayv = server.arg("Delayv");
    }
    if(!server.hasArg("Counter") && server.arg("Delayh")!= "")
    {
      Delayh = server.arg("Delayh");
    }
    handleserver();
   }
void handleAction()
{
  server.send(200, "text/html", "<h3>Action = " +server.arg("action")+"</h3>");
}
void handleserver()
{
  HTTPClient http;  //Declare an object of class HTTPClient
  int httpCode = http.GET();  //Send the request
  server.send(200, "text/html",
  "<!DOCTYPE html>"
"<html lang='en'>"
"<head>"
"<title>Watering System</title>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style> *{'box-sizing: border-box;'}"
"body {'font-family: Arial, Helvetica, sans-serif;'}"
/* Style the header */
"header {"
  "background-color: #e3eaa7;"
  "padding: 30px;"
  "text-align: center;"
  "font-size: 40px;"
  "color: white;"
"}"

/* Create two columns/boxes that floats next to each other */
"nav {"
  "float: left;"
  "width: 20%;"
  "height: 410px;" /* only for demonstration, should be removed */
  "background:#b5e7a0;"
  "padding: 20px;"
"}"
/* Style the list inside the menu */
"nav ul {"
  "list-style-type: none;"
  "padding: 0;"
"}"
"article {"
  "float: right;"
  "padding: 10px;"
  "width: 98.6%;"
  "background-color: #d5e1d0;"
  "height: 70px;" /* only for demonstration, should be removed */
"}"
/* Clear floats after the columns */
"section:after {"
  "content: "";"
  "display: table;"
  "clear: both;"
"}"
/* Style the footer */
"footer {"
  "background-color: #86af90;"
  "padding: 10px;"
  //"text-align: center;"
  "height: 430px;"
  "color: white;"
"}"
/* Responsive layout - makes the two columns/boxes stack on top of each other instead of next to each other, on small screens */
"@media (max-width: 400px) {"
  "nav, article {"
    "width: 98.6%;"
    "height: 70px;"
  "}"
"}"
"</style>"
"</head>"
"<body>"
"<header>"
  "<h2>Welcome to WateringSystem home page</h2>"
"</header>"
"<section>"
  "<nav>"
    "<ul>"
      "<li><h2><form action=\"/Action\" method=\"POST\"><input type=\"text\" name=\"action\" placeholder=\"Action\"><input type=\"submit\" value=\"Send Command\"></form></h2></li>"
    "</ul>"
    "<ul>"
      "<li><h2> <form action=\"/Counter\" method=\"POST\"><input type=\"text\" name=\"counter\" placeholder=\"Start every N hours \"><input type=\"text\" name=\"Delayv\" placeholder=\"VDir_Defualt = 1500 ms \"><input type=\"text\" name=\"Delayh\" placeholder=\"HDir_Defualt = 1500 ms \"><input type=\"submit\" value=\"Send Command\"></form></h2></li>"
    "</ul>"
    "<ul>"
      "<li>Date: "+ oldYear +"-"+ oldMonth + "-" + oldDay + " Time: " + oldHour + ":" + oldMinute+ "</li>"
      "<li>Date: "+ rtcYear +"-"+ rtcMonth + "-" + rtcDay + " Time: " + rtcHour + ":" + rtcMinute+ "</li>"
    "</ul>"
    "<ul>"
      "<li>Program will repeat every : "+ count+" Hours</li>"
      "<li>Next Watering Time is: "+Hour+ ":"+Minute+"</li>"
    "</ul>"
    "<ul>"
      "<li>Delayv is: "+Delayv+"</li>"
      "<li>Delayh is: "+Delayh+"</li>"
    "</ul>"
  "</nav>"
  "<article>"
    "<a>&#169 Sam Jabbar</a>"
  "</article>"
"</section>"
"<footer>"
"<style>"
"table {"
  "border-collapse: collapse;"
  "width: 100%;"
"}"
 "td {"
  "text-align: left;"
  "padding: 12px;"
"}"
"</style>"

"<div style=\"overflow-x:auto;\">"
"<table id=\"myTable\">"
  "<tr>"
    "<td>"+History[1]+"</td>"
    "<td>"+History[11]+"</td>"
    "<td>"+History[21]+"</td>"
    "<td>"+History[31]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[2]+"</td>"
    "<td>"+History[12]+"</td>"
    "<td>"+History[22]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[3]+"</td>"
    "<td>"+History[13]+"</td>"
    "<td>"+History[23]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[4]+"</td>"
    "<td>"+History[14]+"</td>"
    "<td>"+History[24]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[5]+"</td>"
    "<td>"+History[15]+"</td>"
    "<td>"+History[25]+"</td>"
  "</tr>"
    "<tr>"
    "<td>"+History[6]+"</td>"
    "<td>"+History[16]+"</td>"
    "<td>"+History[26]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[7]+"</td>"
    "<td>"+History[17]+"</td>"
    "<td>"+History[27]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[8]+"</td>"
    "<td>"+History[18]+"</td>"
    "<td>"+History[28]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[9]+"</td>"
    "<td>"+History[19]+"</td>"
    "<td>"+History[29]+"</td>"
  "</tr>"
   "<tr>"
    "<td>"+History[10]+"</td>"
    "<td>"+History[20]+"</td>"
    "<td>"+History[30]+"</td>"
  "</tr>"
"</table>"
"</div>"
"</footer>"
"</body>"
"</html>"
  "<title>Watering System</title>"
  );
}
void WateringOn(String Delay)
{
  digitalWrite(IN1, HIGH);
  digitalWrite(ENA, HIGH);
  delay(Delay.toInt());
  digitalWrite(IN1, LOW);
  digitalWrite(ENA, LOW);
  //delay(3000);
}
void MovingOn(int angle)
{
  servo.attach(2); 
  servo.write(servo.read()+angle);                 //command to rotate the servo to the specified angle
  delay(100);
  servo.detach();
}
void NextTime()
{
  int tot = count + oldHour.toInt();
  if(tot <= 24)
  {
    newHour = (String)tot;
    if(newHour == "24"){ newHour = "00";}
    if(newHour != "24" && newHour.length() == 1){newHour = ('0'+newHour);}
    Hour = newHour;
    Minute = newMinute;
  }
  else
  {
   newHour = (String)(tot - 24);
   if(newHour != "24" && newHour.length() == 1){newHour = ('0'+newHour);}
   Hour = newHour;
   Minute = newMinute;
   
  }
}

void ShowHistory()
{
  if(oldDay.toInt() == 1)
  {
    for(int i = 2; i < 32; i++)
    {
      History[i] = " ";
    }
  }
  History[oldDay.toInt()] = History[oldDay.toInt()]+"\n"+oldYear+"/"+oldMonth+"/"+oldDay+"-"+oldHour +":"+ newMinute + "\n";
  handleserver();
}
