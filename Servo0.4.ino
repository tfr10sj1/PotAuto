#include <Servo.h>
#include <ESP8266WiFi.h>;
#include <ESP8266HTTPClient.h>;
#include <ESP8266WebServer.h>
String cDay = ""; 
String oldYear = "";
String oldMonth = "";
String oldDay = "";
String oldHour = "";
String oldMinute = "";

String Hour = "06";
String Minute = "30";

String Direction = "vi";
String Delayv = "1500";
String Delayh = "1500";
String newHour = "06";
String newMinute = "30";
int angle = 0;
int MAX_angle = 105;
int MIN_angle = 9;
int MID_angle = 32;
int a = 0;
int b = 0;
int limit = 0;
int timeflag = 0;
static String History[32];
ESP8266WebServer server(80); // Create a webserver object listens HTTP request on port 80

const char* ssid = "ComHemF17542";
const char* password = "ytkwsqqu";

Servo servo;
int count = 6; // repeat every 6 hours
//int tag = 0;
int ENA = 4;
int IN1 = 0;
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  // set all the motor control pins to outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
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
    //Serial.println("HTTP server started");
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
  server.handleClient();                     // Listen for HTTP requests
  String currentYear = "";
  String currentMonth = "";
  String currentDay = "";
  String currentHour = "";
  String currentMinute = "";
  if (WiFi.status() == WL_CONNECTED) 
  { //Check WiFi connection status
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
       if(currentYear != oldYear){
          oldYear = currentYear;
       }
       if(currentMonth != oldMonth){
          oldMonth = currentMonth;
       }
        if(currentDay != oldDay){
          oldDay = currentDay;
          cDay = currentDay;
        }
        if(currentHour != oldHour){
          oldHour = currentHour;
        }
        if(currentMinute != oldMinute){
          oldMinute = currentMinute;
        }
    }
    http.end();   //Close connection
  }
  if(newHour == currentHour && newMinute == currentMinute && timeflag == 0)
  {
    //MovingOn(0);
    //WateringOn(Delayh);
    //MovingOn(180);
    WateringOn(Delayv);
    NextTime();
    timeflag = 1;
    Serial.println("History: ");
    ShowHistory();
    delay(600);    //Send a request every 30 seconds
  } 
  else if(newHour != currentHour || newMinute != currentMinute && timeflag == 1)
  {
    timeflag = 0;
  }
}
void handleRoot() 
{// When URI / is requested, make login Webpage
  server.send(200, "text/html", "<form action=\"/login\" method=\"POST\"><input type=\"text\" name=\"uname\" placeholder=\"Username\"></br><input type=\"password\" name=\"pass\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Login\"></form>");
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
  if(server.arg("action") == "Bl" || server.arg("action") == "bl" )
    {
      dirandtime(80);
    }
  if(server.arg("action") == "VI" || server.arg("action") == "vi" )
    {
      dirandtime(-180);
    }
  if(server.arg("action") == "RO" || server.arg("action") == "ro" )
    {
      dirandtime(60);
    }
  if (server.arg("action")== "tt")
  {  
    servo.attach(15); 
          // attach the signal pin of servo to pin9 of arduino
    for(angle = 13; angle < MAX_angle; angle += 1)    // command to move from 0 degrees to 180 degrees 
    {                                 
      servo.write(angle);                 //command to rotate the servo to the specified angle
      delay(100);                
    } 
    servo.detach();
    delay(1500);
    //Serial.println(angle);
  }
  if(server.arg("action")== "-tt"){
    servo.attach(15); 
    for(angle ; angle >= MIN_angle; angle -= 1)     // command to move from 180 degrees to 0 degrees 
    {                                
      servo.write(angle);              //command to rotate the servo to the specified angle
      delay(100);                        
    } 
      delay(1500);
      servo.detach();
      //Serial.println(angle);
    } 
  if(server.arg("action")== "t")
    {
      servo.attach(15); 
      if(angle > MID_angle)
      {
        for(angle ; angle >= MID_angle; angle -= 1)     // command to move from 180 degrees to 0 degrees 
        {                                
          servo.write(angle);              //command to rotate the servo to the specified angle
          delay(100);                        
        } 
      }
      else
       {
          for(angle ; angle <= MID_angle; angle += 1)     // command to move from 180 degrees to 0 degrees 
        {                                
          servo.write(angle);              //command to rotate the servo to the specified angle
          delay(100);                        
        }  
       }
        delay(1500);
        servo.detach();
      //  Serial.println(angle);
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
  server.send(200, "text/html", "<h1>Action = " +server.arg("action")+"</h1>");
}
void handleserver()
{
  HTTPClient http;  //Declare an object of class HTTPClient
  //http.begin("http://188.150.76.88:2222/jsfs.html");  //Specify request destination
  int httpCode = http.GET();  //Send the request
  server.send(200, "text/html", "<h1><form action=\"/Action\" method=\"POST\"><input type=\"text\" name=\"action\" placeholder=\"Action\"><br><input type=\"submit\" value=\"Send Command\"></form></h1><p>Date: "+ oldYear +"-"+ oldMonth + "-" + oldDay + " Time: " + oldHour + ":" + oldMinute+ "<h1><form action=\"/Counter\" method=\"POST\"><input type=\"text\" name=\"counter\" placeholder=\"Start every N hours \"><input type=\"text\" name=\"Delayv\" placeholder=\"VDir_Defualt = 1500 ms \"><input type=\"text\" name=\"Delayh\" placeholder=\"HDir_Defualt = 1500 ms \"><br><input type=\"submit\" value=\"Send Command\"></form></h1><p> Program will repeat every : "+ count+" Hours </p><p>Next Watering Time is: "+Hour+ ":"+Minute+"</p><p>Delayv is: "+Delayv+"</p><p>Delayh is: "+Delayh+"</p><p>History: "+History[0]+"</p><p>"+History[1]+"</p><p>"+History[2]+"</p><p>"+History[3]+"</p><p>"+History[4]+"</p><p>"+History[5]+"</p><p>"+History[6]+"</p><p>"+History[7]+"</p><p>"+History[8]+"</p><p>"+History[9]+"</p><p>"+History[10]+"</p><p>"+History[11]+"</p><p>"+History[12]+"</p><p>"+History[13]+"</p><p>"+History[14]+"</p><p>"+History[15]+"</p><p>"+History[16]+"</p><p>"+History[17]+"</p><p>"+History[18]+"</p><p>"+History[19]+"</p><p>"+History[20]+"</p><p>"+History[21]+"</p><p>"+History[22]+"</p><p>"+History[23]+"</p><p>"+History[24]+"</p><p>"+History[25]+"</p><p>"+History[26]+"</p><p>"+History[27]+"</p><p>"+History[28]+"</p><p>"+History[29]+"</p><p>"+History[30]+"</p><p>"+History[31]+"</p>");
}
void WateringOn(String Delay)
{
  digitalWrite(IN1, HIGH);
  digitalWrite(ENA, HIGH);
  delay(Delay.toInt());
  analogWrite(ENA, 0);
  delay(3000);
}
void MovingOn(int angle)
{
  servo.attach(15);
  servo.write(angle);
  delay(1500);
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
  if(oldDay.toInt() == cDay.toInt())
  {
     History[oldDay.toInt()] = History[oldDay.toInt()]+"\n"+oldMonth+"-"+oldDay+"-"+oldHour +":"+ newMinute + "\n";
  } 
  else
  {
    History[oldDay.toInt()] = History[oldDay.toInt()]+"\n"+oldMonth+"-"+oldDay+"-"+oldHour +":"+ newMinute + "\n";
  }
  handleserver();
}
void dirandtime(int vinkel)
{
  servo.attach(15);
  Serial.println("Direction: ");
  Serial.print(Direction);  
 // delay(5000);
  servo.write(0);
 // delay(9000);
  servo.write(vinkel);
  Serial.println("vinkel: ");
  Serial.print(vinkel);  
  switch (vinkel) 
      {
        case -180:     
//          delay(6000);
//          Direction = "vi";
//          Serial.println("Direction: ");
//          Serial.print(Direction);

          servo.detach();
          break;
        case 80:
          delay(5000);  
          Direction = "bl";
          Serial.println("Direction: ");
          Serial.print(Direction); 
          servo.detach();
          break;
        case 60:
          delay(3000);
          Direction = "ro";
          Serial.println("Direction: ");
          Serial.print(Direction);   
          servo.detach();
          break;
      }
      servo.detach();
}

