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
#include "RTClib.h"

RTC_DS3231 rtc;
// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

String cDay = ""; 
String oldYear = "";
String oldMonth = "";
String oldDay = "";
String oldHour = "";
String oldMinute = "";
String Hours = "06";
String Minutes = "30";
String Direction = "vi";
String Delayv = "0";
String Delayh = "0";
String newHour = "06";
String newMinute = "30";

String History[32];

static String rtcYear;
static String rtcMonth;
static String rtcDay;
static String rtcHour;
static String rtcMinute;
static String rtcSecund;

const char* ssid = "ComHemF17542";
const char* password = "ytkwsqqu";

Servo servo;
ESP8266WebServer server(80); // Create a webserver object listens HTTP request on port 80

int count = 6; // repeat every 6 hours
int ENA = 10;
int IN1 = 0;
int timeflag = 0;


void setup() {
 // Servorun(true);
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
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
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
  
    server.begin(); // start the server
    
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
"<title>Cool-Plants automated system</title>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style> *{'box-sizing: border-box;'}"
/* Responsive layout - makes the two columns/boxes stack on top of each other instead of next to each other, on small screens */
"* {"
  "margin: 0;"
  "padding: 0;"
  "box-sizing: border-box;"
"}"

".page {"
    "display: flex;"
    "flex-wrap: wrap;"
    "}"
    
    ".section {"
    "width: 100%;"
    "height: 100px;"
    "display: flex;"
    "justify-content: center;"
    "align-items: center;"
    "}"
    
   ".menu {"
    "background-color: #C8C6FA;"
    "height: 100px;"
    "}"
    
    ".header {"
    //"background-color: #B2D6FF;"
    "justify-content: center;"
    "height: 450px;"
    "width: 100%;"
    "position:relative;"
    "top: 50px;"
    "}"
    
    ".feature-2 {"
    "justify-content: right;"
    "background-color: #C8C6FA;"
    "height: 40px;"
    "position:relative;"
    "top: 200px;"
    "}"
/* Mobile Styles */
"@media only screen and (max-width: 400px) {"
  "body {"
    "background-color: #F09A9D;" /* Red */
  "}"
"}"

/* Tablet Styles */
"@media only screen and (min-width: 401px) and (max-width: 960px) {"
  "body {"
    "background-color: #F5CF8E;" /* Yellow */
    "}"
"}"

/* Desktop Styles */
"@media only screen and (min-width: 961px) {"
  "body {"
    "background-color: #B2D6FF;" /* Blue */
  "}"
"}"
"body {"
  "margin: auto;"
"}"
".sea {"
  "background-color: #0077cc;"
  "position: fixed;"
  "top: 100%;"
  "width: 100%;"
  "height: 10%;"
  /*animation: sea 10s ease-in-out;
  animation-direction: alternate;
  animation-iteration-count: infinite*/
"}"

"svg {"
  "position: absolute;"
  "width: 100%;"
  "height:41%;"
  "bottom:99%;"
  "left: 0%;"
"}"

".water-fill {"
  "-webkit-animation: wave 0.9s infinite linear, fill-up 10s infinite ease-in-out alternate;"
   "animation: wave 0.9s infinite linear, fill-up 10s infinite ease-in-out alternate;"
"}"

"@keyframes wave {"
  "0% {"
    "x: -750px;"
  "}"
  "100% {"
    "x: 0;"
  "}"
"}"
"@-webkit-keyframes wave {"
  "0% {"
    "x: -750px;"
  "}"
  "100% {"
    "x: 0;"
  "}"
"}"
"@keyframes fill-up {"
  "0% {"
    "height: 0;"
    "y: 100%;"
  "}"
  "100% {"
    "height: 100%;"
    "y: 0%;"
  "}"
"}"
"@-webkit-keyframes fill-up {"
  "0% {"
    "height: 0;"
    "y: 100%;"
  "}"
  "100% {"
    "height: 100%;"
    "y: 0%;"
  "}"
"}"
"</style>"
"<link rel='stylesheet' href='styles.css'/>"
"</head>"
"<body>"
"<div class='page'>"
  "<div class='section menu'><h1>Welcome to Cool-Plants</h1></div>"
  "<div class='section header'>"
  "<head>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<link rel='stylesheet' href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">"
"<style>"
"body {"
  "font-family: Arial, Helvetica, sans-serif;"
"}"

"* {"
  "box-sizing: border-box;"
"}"

/* style the container */
".container {"
  "position: relative;"
  "border-radius: 5px;"
  "background-color: #f2f2f2;"
  "padding: 20px 0 30px 0;"
"}" 

/* style inputs and link buttons */
"input,"
".btn {"
  "width: 100%;"
  "padding: 12px;"
  "border: none;"
  "border-radius: 4px;"
  "margin: 5px 0;"
  "opacity: 0.85;"
  "display: inline-block;"
  "font-size: 17px;"
  "line-height: 20px;"
  "text-decoration: none;" /* remove underline from anchors */
"}"

"input:hover,"
".btn:hover {"
  "opacity: 1;"
"}"

/* style the submit button */
"input[type=submit] {"
  "background-color: #4CAF50;"
  "color: white;"
  "cursor: pointer;"
"}"

"input[type=submit]:hover {"
  "background-color: #45a049;"
"}"

/* Two-column layout */
".col {"
  "float: left;"
  "width: 50%;"
  "margin: auto;"
  "padding: 0 50px;"
  "margin-top: 6px;"
"}"

/* Clear floats after the columns */
".row:after {"
  "content: "";"
  "display: table;"
  "clear: both;"
"}"

/* vertical line */
".vl {"
  "position: absolute;"
  "left: 50%;"
  "transform: translate(-50%);"
  "border: 2px solid #ddd;"
  "height: 175px;"
"}"

/* text inside the vertical line */
".vl-innertext {"
  "position: absolute;"
  "top: 50%;"
  "transform: translate(-50%, -50%);"
  "background-color: #f1f1f1;"
  "border: 1px solid #ccc;"
  "border-radius: 50%;"
  "padding: 8px 10px;"
"}"

/* hide some text on medium and large screens */
".hide-md-lg {"
  "display: none;"
"}"

/* bottom container */
".bottom-container {"
  "text-align: center;"
  "background-color: #666;"
  "border-radius: 0px 0px 4px 4px;"
"}"

/* Responsive layout - when the screen is less than 650px wide, make the two columns stack on top of each other instead of next to each other */
"@media screen and (max-width: 650px) {"
  ".col {"
    "width: 100%;"
    "margin-top: 0;"
  "}"
  /* hide the vertical line */
  ".vl {"
    "display: none;"
  "}"
  /* show the hidden text on small screens */
  ".hide-md-lg {"
    "display: block;"
    "text-align: center;"
  "}"
"}"
 "margin: 0;"
"}"
".sea {"
  "background-color: #0077cc;"
  "position: fixed;"
  "top: 100%;"
  "width: 100%;"
  "height: 10%;"
  /*animation: sea 10s ease-in-out;
  animation-direction: alternate;
  animation-iteration-count: infinite*/
"}"

"svg {"
  "position: absolute;"
  "width: 100%;"
  "height:41%;"
  "bottom:99%;" 
  "left: 0%;"
"}"

".water-fill {"
  "-webkit-animation: wave 0.9s infinite linear, fill-up 10s infinite ease-in-out alternate;"
   "animation: wave 0.9s infinite linear, fill-up 10s infinite ease-in-out alternate;"
"}"
/* body {
  background: grey;
} */
".div {"
  "width: 200px;"
  "height: 150px;"
  "justify-content: center;"
  "margin: auto;"
  "background: linear-gradient(90deg, #3d3d3d 0%, #e0e0e0 50%, #3d3d3d 100%) center no-repeat;"
  "position: relative;"
"}"
" .scale {"
  "width: 33%;"
  "height: 85%;"
  "justify-content: center;"
  "border-top: 10px solid #818181;"
  "border-left: 10px solid #818181;"
  "border-bottom: 10px solid #ffffff;"
  "border-right: 10px solid #ffffff;"
  "position: absolute;"
  "bottom: 3%;"
  "left: 10%;"
  "background-color: RGB(209, 214, 217);"
"}"
".level {"
  "width: 100%;"
  "height: 10%;"
  "background: #3E70B2;"
  "position: absolute;"
  "bottom: 0;"
"}"
"</style>"
"</head>"
"<body>"
"<div class='container'>"
    "<div class='row'>"
     
      "<div class='col'>"
         "<nav>"
    "<ul>"
          "<form action=\"/Action\" method=\"POST\">" 
            "<input type=\"text\" name=\"action\" placeholder=\"Action\">"
            "<input type=\"submit\" value=\"Send Command\">"
          "</form>" 
    "</ul>"
    
     "<ul>"
          "<form action=\"/Move\" method=\"POST\">" 
            "<ul style='text-align:left'><input type=\"radio\" name=\"servo\" value=\"1\">Clockwise</ul>"
            "<ul><input type=\"radio\" name=\"servo\" value=\"0\">Unclockwise </ul>"
            "<input type=\"submit\" value=\"Move Servo\">"
          "</form>"       
    "</ul>"
    "<ul>"
      "<form action=\"/Counter\" method=\"POST\"><input type=\"text\" name=\"counter\" placeholder=\"Start every N hours \">"
        "<input type=\"text\" name=\"Delayv\" placeholder=\"VDir_Defualt = 1500 ms \"><input type=\"text\" name=\"Delayh\" placeholder=\"HDir_Defualt = 1500 ms \">"
        "<input type=\"submit\" value=\"Send Command\">"
      "</form>"
   
    "</ul>"
  "</nav>"
      "</div>"
      "</div>"
    "<div>"
     "<h2>"
      "Date: "+ oldYear +"-"+ oldMonth + "-" + oldDay + " Time: " + oldHour + ":" + oldMinute+ " <br>"
      "Date: "+ rtcYear +"-"+ rtcMonth + "-" + rtcDay + " Time: " + rtcHour + ":" + rtcMinute+ "<br>"
    "</h2>"
    "<br>"
    "<h2>"
      "Program will repeat every : "+ count+" Hours <br>"
      "Next Watering Time is: "+Hours+ ":"+Minutes+" <br>"
    "</h2>"
    "<br>"
    "<h2>"  
      "Delayv is: "+Delayv+"<br>"
      "Delayh is: "+Delayh+"<br>"
    "</h2>"
    "<br>"
       "<div class='col'>"
       "<nav>"
       "<h2 style='text-align:center'>Water Level </h2> <br>"
         "<div class= 'div' style='text-align:center'>"
         "<div class='scale'>"
         "<div class='level'>"  
         "</div>"
         "</div>"
         "</div>"
         "<nav>"
         "</div>"
    "</div>"
"</div>"
"</body>"
    "</div>"
  "</div>"

  "<div class='section feature-2'>"
  "<a>&#169 Sam Jabbar</a>"
  "</div>"
   "<div id='sea' class='sea'>"
  "<svg version='1.1' xmlns=\"http://www.w3.org/2000/svg\">"
    "<defs>"
      "<pattern id='water' width='.25' height='2' patternContentUnits='objectBoundingBox'>"
        "<path fill='#20293F' d=\"M0.25,1H0c0,0,0-0.659,0-0.916c0.083-0.303,0.158,0.334,0.25,0C0.25,0.327,0.25,1,0.25,0.5z\"/>"
      "</pattern>"
    "</defs>"  
    "<rect id='waves' class='water-fill' fill='url(#water)' width='3000'/> "
  "</svg>"
"</div>"
"</div>"
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
  if (server.hasArg("servo") && server.arg("servo") == "1") 
  {
     MovingOn(45);
  }
  else if (server.hasArg("servo") && server.arg("servo") == "0") 
  {
     MovingOn(-50);
  }
  
  if(server.arg("action")== "x" || server.arg("action")== "X" )
    {
      WateringOn(Delayv);
    }
   if(!server.hasArg("AutomaticWatering")&& server.arg("hour") != "" && server.arg("minute")!= "")
    {
      Hours = server.arg("hour");
      Minutes = server.arg("minute");
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
"<section>"
  "<nav>"
    "<ul>"
      "<li>"
          "<form action=\"/Action\" method=\"POST\">" 
            "<input type=\"text\" name=\"action\" placeholder=\"Action\">"
            "<input type=\"submit\" value=\"Send Command\">"
          "</form>" 
       "</li>"
    "</ul>"
    
     "<ul>"
      "<li>"
          "<form action=\"/Move\" method=\"POST\">" 
            "<input type=\"radio\" name=\"servo\" value=\"1\">Clockwise<BR>"
            "<input type=\"radio\" name=\"servo\" value=\"0\">Unclockwise<BR>"
            "<input type=\"submit\" value=\"Move Servo\">"
          "</form>" 
       "</li>"
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
      "<li>Next Watering Time is: "+Hours+ ":"+Minutes+"</li>"
    "</ul>"
    "<ul>"  
      "<li>Delayv is: "+Delayv+"</li>"
      "<li>Delayh is: "+Delayh+"</li>"
    "</ul>"
  "</nav>"
  "</section>"
  "<article>"
    "<a>&#169 Sam Jabbar</a>"
  "</article>"
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
  delay(500);
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
    Hours = newHour;
    Minutes = newMinute;
  }
  else
  {
   newHour = (String)(tot - 24);
   if(newHour != "24" && newHour.length() == 1){newHour = ('0'+newHour);}
   Hours = newHour;
   Minutes = newMinute;
   
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
