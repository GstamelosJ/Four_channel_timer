/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID "TMPLrQV8F4y3"
#define BLYNK_TEMPLATE_NAME "Irrigation 4channel"
#define BLYNK_FIRMWARE_VERSION        "0.1.1"
//#define BLYNK_AUTH_TOKEN "7a6YGSS22F0E4-gKascF1aSGVCq1SrDk"
#define USE_NODE_MCU_BOARD


#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
//#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI
#include "BlynkEdgent.h"

//#define APP_DEBUG
//#define BLYNK_NEW_LIBRARY
//#include <ESP8266WiFi.h>
//#include <BlynkSimpleEsp8266.h>
//#include <SimpleTimer.h>
#include <TimeLib.h> 
//#include <WidgetRTC.h>
#include <Dusk2Dawn.h>


//*********************************

SimpleTimer timer;

//WidgetRTC rtc;
WidgetTerminal terminal(V8);


#define server "blynk.cloud"   // or "blynk.cloud-com" for Blynk's cloud server
#define ch1 5                // on board GPIO 5 assignment
#define ch2 4                // on board GPIO 4 assignment
#define ch3 14                // on board GPIO 5 assignment
#define ch4 12                // on board GPIO 5 assignment

char Date[16];
char Time[16];
char auth[] = "7a6YGSS22F0E4-gKascF1aSGVCq1SrDk";
//char auth[] = "lF1hRVUlyyfNyYD7VDALVbmrAeVY0i3V";
char ssid[] = "COSMOTE-189DDC";
char pass[] = "UXYebdfUddddKqAq";
long startsecondswd;            // weekday start time in seconds
long stopsecondswd;             // weekday stop  time in seconds
long nowseconds;                // time now in seconds
bool isFirstConnect = true;
bool CH1, CH2, CH3, CH4;
long tim=0;
bool blynk_run=false;

String displaycurrenttimepluswifi;
int wifisignal;
int manual=0;
//int oldstatus;

//*******************connect check
typedef enum {
  CONNECT_TO_WIFI,
  AWAIT_WIFI_CONNECTION,
  CONNECT_TO_BLYNK,
  AWAIT_BLYNK_CONNECTION,
  MAINTAIN_CONNECTIONS,
  AWAIT_DISCONNECT
} CONNECTION_STATE;

CONNECTION_STATE connectionState;
uint8_t connectionCounter;
void clockvalue();
void activetoday();
void sendWifi();

void ConnectionHandler(void) {
  switch (connectionState) {
  case CONNECT_TO_WIFI:
    BLYNK_LOG("Connecting to %s.", ssid);
    WiFi.begin(ssid, pass);
    connectionState = AWAIT_WIFI_CONNECTION;
    connectionCounter = 0;
    break;

  case AWAIT_WIFI_CONNECTION:
    if (WiFi.status() == WL_CONNECTED) {
      BLYNK_LOG("Connected to %s", ssid);
      connectionState = CONNECT_TO_BLYNK;
    }
    else if (++connectionCounter == 50) {
      BLYNK_LOG("Unable to connect to %s. Retry connection.", ssid);
      WiFi.disconnect();
      connectionState = AWAIT_DISCONNECT;
      connectionCounter = 0;
    }
    break;

  case CONNECT_TO_BLYNK:
    BLYNK_LOG("Attempt to connect to Blynk server.");
    //Blynk.config(auth, IPAddress(139, 59, 206, 133), 80);
    Blynk.config(auth, server, 80);    
    Blynk.connect();
    connectionState = AWAIT_BLYNK_CONNECTION;
    connectionCounter = 0;
    break;

  case AWAIT_BLYNK_CONNECTION:
    if (Blynk.connected()) {
      BLYNK_LOG("Connected to Blynk server.");
      connectionState = MAINTAIN_CONNECTIONS;
    }
    else if (++connectionCounter == 50) {
      BLYNK_LOG("Unable to connect to Blynk server. Retry connection.");
      Blynk.disconnect();
      WiFi.disconnect();
      connectionState = AWAIT_DISCONNECT;
      connectionCounter = 0;
    }
    break;

  case MAINTAIN_CONNECTIONS:
    if (WiFi.status() != WL_CONNECTED) {
      BLYNK_LOG("Wifi connection lost. Reconnect.");
      Blynk.disconnect();
      WiFi.disconnect();
      connectionState = AWAIT_DISCONNECT;
      connectionCounter = 0;
    }
    else  if (!Blynk.connected()) {
      BLYNK_LOG("Blynk server connection lost. Reconnect.");
      Blynk.disconnect();
      connectionState = CONNECT_TO_BLYNK;
      blynk_run=false;
    }
    else {
      blynk_run=true;
      //Blynk.run();
    }
    break;

  case AWAIT_DISCONNECT:
    if (++connectionCounter == 10) {
      connectionState = CONNECT_TO_WIFI;
    }
    break;
  }
}

//*****************************************


void setup()
{
  Serial.begin(115200);
  delay(100);
  pinMode(ch1, OUTPUT);
  digitalWrite(ch1, LOW); // set ch1 OFF
   pinMode(ch2, OUTPUT);
  digitalWrite(ch2, LOW); // set ch2 OFF
  pinMode(ch3, OUTPUT);
  digitalWrite(ch3, LOW); // set ch3 OFF
   pinMode(ch4, OUTPUT);
  digitalWrite(ch4, LOW); // set ch4 OFF
  Serial.begin(115200);
  delay(250);
  Serial.println(String("Starting..."));
  blynk_run=false;
  /*Blynk.begin(auth, ssid, pass, server);
  int mytimeout = millis() / 1000;
  while (Blynk.connect() == false) { // try to connect to server for 10 seconds
    if((millis() / 1000) > mytimeout + 8){ // try local server if not connected within 9 seconds
       break;
    }


  }*/
  connectionState = CONNECT_TO_WIFI;
   //rtc.begin();
   timer.setInterval(2010L, activetoday);  // check every 2 SECONDS if schedule should run today 
   timer.setInterval(2000L, ConnectionHandler);  // check every 2s if still connected to server (previously reconnectBlynk) 
   timer.setInterval(5000L, clockvalue);  // check value for time
   timer.setInterval(5000L, sendWifi);    // Wi-Fi singal
  BlynkEdgent.begin();
}

/**************************************************************************/

BLYNK_CONNECTED() {
if (isFirstConnect) {
  Blynk.syncAll();
  //Blynk.notify("TIMER STARTING!!!!");
isFirstConnect = false;
}
Blynk.sendInternal("rtc", "sync"); //request current local time for device
}


void sendWifi() {
  wifisignal = map(WiFi.RSSI(), -105, -40, 0, 100);
}

void clockvalue() // Digital clock display of the time
{
  
Blynk.sendInternal("rtc", "sync"); //request current local time for device
 int gmthour = hour(tim);
  if (gmthour == 24){
     gmthour = 0;
  }
  String displayhour =   String(gmthour, DEC);
  int hourdigits = displayhour.length();
  if(hourdigits == 1){
    displayhour = "0" + displayhour;
  }
  String displayminute = String(minute(tim), DEC);
  int minutedigits = displayminute.length();  
  if(minutedigits == 1){
    displayminute = "0" + displayminute;
  }  

  displaycurrenttimepluswifi = "Clock:  " + displayhour + ":" + displayminute + "Signal:  " + wifisignal +" %";
  //Blynk.setProperty(V4, "label", displaycurrenttimepluswifi);
  terminal.println(displaycurrenttimepluswifi);
  terminal.flush();
  
}



void activetoday(){        // check if schedule should run today
  Serial.println("Entered activeToday");
  if(year(tim) != 1970){
    Serial.println("It's not 1970");

  /* if (mondayfriday==1) {  
    Blynk.syncVirtual(V4); // sync timeinput widget  
   }
   if (saturdaysunday==1) { 
    Blynk.syncVirtual(V6); // sync timeinput widget  
   }
   if (alldays==1) { 
    Blynk.syncVirtual(V8); // sync timeinput widget  
   }
   if (uptoyou==1) { 
    Blynk.syncVirtual(V10); // sync timeinput widget  
   }*/
   Blynk.syncVirtual(V4,V5,V6,V7); // sync timeinput widget 
  }
}

/*void checklastbuttonpressed (){
    if((mondayfriday==1)&&(saturdaysunday==0)){ oldstatus=1; }
    if((mondayfriday==0)&&(saturdaysunday==1)){ oldstatus=2; }
    if((mondayfriday==1)&&(saturdaysunday==1)){ oldstatus=3; }
    if(alldays==1){ oldstatus=4; }
    if(uptoyou==1){ oldstatus=5; }
    if((mondayfriday==0)&&(saturdaysunday==0)&&(alldays==0)&&(uptoyou==0)){ oldstatus=6; }  
}

*/
BLYNK_WRITE(InternalPinRTC) {   //check the value of InternalPinRTC  
  tim = param.asLong();      //store time in t variable
  Serial.print("Unix time: ");  
  Serial.print(hour(tim));              //prints time in UNIX format to Serial Monitor
  Serial.print(" : ");
  Serial.print(minute(tim));
  Serial.println();
}

BLYNK_WRITE(V0)  // Manual selection
{
  if (param.asInt()==1) {
	digitalWrite(ch1,1);
 CH2=true;
  

    
  } 
  else {
   digitalWrite(ch1,0);
   CH2=false;
}
}

BLYNK_WRITE(V1)  // Manual selection
{
  if (param.asInt()==1) {
  digitalWrite(ch2,1);
  CH1=true;

    
  } 
  else {
   digitalWrite(ch2,0);
   CH1=false;
}
}

BLYNK_WRITE(V2)  // Manual selection
{
  if (param.asInt()==1) {
  digitalWrite(ch3,1);
  CH1=true;

    
  } 
  else {
   digitalWrite(ch3,0);
   CH1=false;
}
}

BLYNK_WRITE(V3)  // Manual selection
{
  if (param.asInt()==1) {
  digitalWrite(ch4,1);
  CH1=true;

    
  } 
  else {
   digitalWrite(ch4,0);
   CH1=false;
}
}



BLYNK_WRITE(V4)// ch1 sceduler  
{     
    sprintf(Date, "%02d/%02d/%04d",  day(tim), month(tim), year(tim));
    sprintf(Time, "%02d:%02d:%02d", hour(tim), minute(tim), second(tim));
  
    TimeInputParam t(param);
  
    terminal.print("Checked schedule at: ");
    terminal.println(Time);
    terminal.flush();
    int dayadjustment = -1;  
    if(weekday(tim) == 1){
      dayadjustment =  6; // needed for Sunday, Time library is day 1 and Blynk is day 7
    }
    if(t.isWeekdaySelected(weekday(tim) + dayadjustment)){ //Time library starts week on Sunday, Blynk on Monday
    terminal.println("Monday-Friday ACTIVE today");
    terminal.flush();
    Dusk2Dawn greece(38.0529, 23.6943, (t.getTZ_Offset()/3600));
    int Sunrise  = 60*(greece.sunrise(year(tim), month(tim), day(tim), false));
    int Sunset  = 60*((greece.sunset(year(tim), month(tim), day(tim), false))+25);
    
    
    if (t.hasStartTime()) // Process start time
    {
        terminal.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute());
        terminal.flush();
      }
      
    else if(t.isStartSunset())
      {
        terminal.println(String("Start: ") + (int)((Sunset/3600)) + ":" + String(((Sunset%3600)/60),DEC));
        terminal.flush();
        Serial.println(String("Sunset : ") + Sunset);
        Serial.println(String("Time zone offset: ") + String(t.getTZ_Offset()/3600));
        
      }
      
    if (t.hasStopTime()) // Process stop time
      {
        terminal.println(String("Stop : ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.flush();
      }
      
    else if(t.isStopSunrise())
      {
        terminal.println(String("Stop: ") + String((Sunrise/3600)) + ":" + String(((Sunrise%3600)/60),DEC));
        terminal.flush();
        Serial.println(String("Sunrise : ") + Sunrise);
      }
      
      
    
    // Display timezone details, for information purposes only 
    terminal.println(String("Time zone: ") + t.getTZ()); // Timezone is already added to start/stop time 
  //  terminal.println(String("Time zone offset: ") + t.getTZ_Offset()); // Get timezone offset (in seconds)
    terminal.flush();
  
     for (int i = 1; i <= 7; i++) {  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)
        if (t.isWeekdaySelected(i)) {
        terminal.println(String("Day ") + i + " is selected");
        terminal.flush();
        }
      } 
    nowseconds = ((hour(tim) * 3600) + (minute(tim) * 60) + second(tim));
    if(t.isStartSunset())
      startsecondswd = Sunset;
    else
      startsecondswd = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
    //Serial.println(startsecondswd);  // used for debugging
    if(nowseconds >= startsecondswd){
        
      terminal.print("CH1 STARTED at");
      terminal.println(String(" ") + String(startsecondswd/3600) + ":" + String((startsecondswd%3600)/60));
      terminal.flush();
      if(nowseconds <= startsecondswd + 30){    // 90s on 60s timer ensures 1 trigger command is sent
        if(!CH1)  
        {//Blynk.notify("CH1 is turned ON");
          CH1=true;
         }  
        digitalWrite(ch1, HIGH); // set LED ON
        Blynk.virtualWrite(V0, 1);
       
        // code here to switch the relay ON
      }      
    }
    else{
      terminal.println("ch1 NOT STARTED today");
      terminal.flush();
   
    }
    if(t.isStopSunrise())
      stopsecondswd = Sunrise;
    else
      stopsecondswd = (t.getStopHour() * 3600) + (t.getStopMinute() * 60);
    //Serial.println(stopsecondswd);  // used for debugging
    if((nowseconds >= stopsecondswd)&& CH1){
      if(nowseconds <= stopsecondswd + 62){   // 90s on 60s timer ensures 1 trigger command is sent
        if (CH1) {
        //Blynk.notify("CH1 is turned OFF");
        CH1=false;
      }
        Blynk.virtualWrite(V0, 0);
      terminal.print("ch1 STOPPED at");
      terminal.println(String(" ") + String(stopsecondswd/3600) + ":" + String((stopsecondswd%3600)/60));
      terminal.flush();
        digitalWrite(ch1, LOW); // set LED OFF
        //Blynk.virtualWrite(V0, 0);
        
        // code here to switch the relay OFF
      }              
    }
   /* else{
      if(nowseconds >= startsecondswd){  
        digitalWrite(CH1, HIGH); // set LED ON    test
        Blynk.virtualWrite(V2, 1);
        terminal.println("Monday-Friday is ON");
        terminal.flush();
      
      }          
    }*/
  }
  else{
    terminal.println("CH1 INACTIVE today");
    terminal.flush();
    // nothing to do today, check again in 30 SECONDS time    
  }
  terminal.println();
}


BLYNK_WRITE(V5)  // ch2 scheduler  
{     
    sprintf(Date, "%02d/%02d/%04d",  day(tim), month(tim), year(tim));
    sprintf(Time, "%02d:%02d:%02d", hour(tim), minute(tim), second(tim));
  
    TimeInputParam t(param);
  
    terminal.print("ch2 Checked schedule at: ");
    terminal.println(Time);
    terminal.flush();
    int dayadjustment = -1;  
    if(weekday(tim) == 1){
      dayadjustment =  6; // needed for Sunday, Time library is day 1 and Blynk is day 7
    }
    if(t.isWeekdaySelected(weekday(tim) + dayadjustment)){ //Time library starts week on Sunday, Blynk on Monday
    terminal.println("ch2 ACTIVE today");
    terminal.flush();
    Dusk2Dawn greece(38.0529, 23.6943, (t.getTZ_Offset()/3600));
    int Sunrise  = 60*(greece.sunrise(year(tim), month(tim), day(tim), false));
    int Sunset  = 60*(greece.sunset(year(tim), month(tim), day(tim), false)+25);
    if (t.hasStartTime()) // Process start time
      {
        terminal.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute());
        terminal.flush();
      }
    
      
      else if(t.isStartSunset())
      {
        terminal.println(String("Start: ") + String((Sunset/3600),DEC) + ":" + String(((Sunset%3600)/60),DEC));
        terminal.flush();
      }
      
    if (t.hasStopTime()) // Process stop time
    {
        terminal.println(String("Stop : ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.flush();
      }
    
      
      else if(t.isStopSunrise())
      {
        terminal.println(String("Start: ") + String((Sunrise/3600),DEC) + ":" + String(((Sunrise%3600)/60),DEC));
        terminal.flush();
      }
      
    // Display timezone details, for information purposes only 
    terminal.println(String("Time zone: ") + t.getTZ()); // Timezone is already added to start/stop time 
  //  terminal.println(String("Time zone offset: ") + t.getTZ_Offset()); // Get timezone offset (in seconds)
    terminal.flush();
  
     for (int i = 1; i <= 7; i++) {  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)
        if (t.isWeekdaySelected(i)) {
        terminal.println(String("Day ") + i + " is selected");
        terminal.flush();
        }
      } 
    nowseconds = ((hour(tim) * 3600) + (minute(tim) * 60) + second(tim));
    if(t.isStartSunset())
      startsecondswd = Sunset;
    else
      startsecondswd = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
    Serial.println(startsecondswd);  // used for debugging
    if(nowseconds >= startsecondswd){    
      terminal.print("CH2 STARTED at");
      //terminal.println(String(" ") + t.getStartHour() + ":" + t.getStartMinute());
      terminal.println(String(" ") + String(startsecondswd/3600) + ":" + String((startsecondswd%3600)/60));
      terminal.flush();
      if(nowseconds <= startsecondswd + 30){    // 90s on 60s timer ensures 1 trigger command is sent
             if (!CH2) 
             {//Blynk.notify("Watering is turned ON");
              CH2 = true;
             }
        digitalWrite(ch2, HIGH); // set LED ON
        Blynk.virtualWrite(V1, 1);
        
        // code here to switch the relay ON
      }      
    }
    else{
      terminal.println("CH2 Device NOT STARTED today");
      terminal.flush();
   
    }
    if(t.isStopSunrise())
      stopsecondswd = Sunrise;
    else
      stopsecondswd = (t.getStopHour() * 3600) + (t.getStopMinute() * 60);
    //Serial.println(stopsecondswd);  // used for debugging
    if((nowseconds >= stopsecondswd)&& CH2){  
      
      if(nowseconds <= stopsecondswd + 60){   // 90s on 60s timer ensures 1 trigger command is sent
        if(CH2) 
        {//Blynk.notify("CH2 is turned OFF");
          CH2 = false;
        }
        digitalWrite(ch2, LOW); // set LED OFF
        Blynk.virtualWrite(V1, 0);
        terminal.print("ch2 STOPPED at");
        //terminal.println(String(" ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.println(String(" ") + String(stopsecondswd/3600) + ":" + String((stopsecondswd%3600)/60));
        terminal.flush();
        // code here to switch the relay OFF
      }              
    }
    /*else{
      if((nowseconds >= startsecondswd)&& !CH2){  
        digitalWrite(ch2, HIGH); // set LED ON    test
        Blynk.virtualWrite(V5, 1);
        terminal.println("CH2 is ON");
        terminal.flush();
      
      }          
    }*/
  }
  else{
    terminal.println("CH2 INACTIVE today");
    terminal.flush();
    // nothing to do today, check again in 30 SECONDS time    
  }
  terminal.println();
}


BLYNK_WRITE(V6) // ch3 scheduler  
{     
    sprintf(Date, "%02d/%02d/%04d",  day(tim), month(tim), year(tim));
    sprintf(Time, "%02d:%02d:%02d", hour(tim), minute(tim), second(tim));
  
    TimeInputParam t(param);
  
    terminal.print("ch3 Checked schedule at: ");
    terminal.println(Time);
    terminal.flush();
    int dayadjustment = -1;  
    if(weekday(tim) == 1){
      dayadjustment =  6; // needed for Sunday, Time library is day 1 and Blynk is day 7
    }
    if(t.isWeekdaySelected(weekday(tim) + dayadjustment)){ //Time library starts week on Sunday, Blynk on Monday
    terminal.println("ch3 ACTIVE today");
    terminal.flush();
    Dusk2Dawn greece(38.0529, 23.6943, (t.getTZ_Offset()/3600));
    int Sunrise  = 60*(greece.sunrise(year(tim), month(tim), day(tim), false));
    int Sunset  = 60*(greece.sunset(year(tim), month(tim), day(tim), false)+25);
    if (t.hasStartTime()) // Process start time
      {
        terminal.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute());
        terminal.flush();
      }
    
      
      else if(t.isStartSunset())
      {
        terminal.println(String("Start: ") + String((Sunset/3600),DEC) + ":" + String(((Sunset%3600)/60),DEC));
        terminal.flush();
      }
      
    if (t.hasStopTime()) // Process stop time
    {
        terminal.println(String("Stop : ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.flush();
      }
    
      
      else if(t.isStopSunrise())
      {
        terminal.println(String("Start: ") + String((Sunrise/3600),DEC) + ":" + String(((Sunrise%3600)/60),DEC));
        terminal.flush();
      }
      
    // Display timezone details, for information purposes only 
    terminal.println(String("Time zone: ") + t.getTZ()); // Timezone is already added to start/stop time 
  //  terminal.println(String("Time zone offset: ") + t.getTZ_Offset()); // Get timezone offset (in seconds)
    terminal.flush();
  
     for (int i = 1; i <= 7; i++) {  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)
        if (t.isWeekdaySelected(i)) {
        terminal.println(String("Day ") + i + " is selected");
        terminal.flush();
        }
      } 
    nowseconds = ((hour(tim) * 3600) + (minute(tim) * 60) + second(tim));
    if(t.isStartSunset())
      startsecondswd = Sunset;
    else
      startsecondswd = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
    Serial.println(startsecondswd);  // used for debugging
    if(nowseconds >= startsecondswd){    
      terminal.print("CH3 STARTED at");
      //terminal.println(String(" ") + t.getStartHour() + ":" + t.getStartMinute());
      terminal.println(String(" ") + String(startsecondswd/3600) + ":" + String((startsecondswd%3600)/60));
      terminal.flush();
      if(nowseconds <= startsecondswd + 30){    // 90s on 60s timer ensures 1 trigger command is sent
             if (!CH3) 
             {//Blynk.notify("Watering is turned ON");
              CH3 = true;
             }
        digitalWrite(ch3, HIGH); // set LED ON
        Blynk.virtualWrite(V2, 1);
        
        // code here to switch the relay ON
      }      
    }
    else{
      terminal.println("CH3 Device NOT STARTED today");
      terminal.flush();
   
    }
    if(t.isStopSunrise())
      stopsecondswd = Sunrise;
    else
      stopsecondswd = (t.getStopHour() * 3600) + (t.getStopMinute() * 60);
    //Serial.println(stopsecondswd);  // used for debugging
    if((nowseconds >= stopsecondswd)&& CH3){  
      
      if(nowseconds <= stopsecondswd + 60){   // 90s on 60s timer ensures 1 trigger command is sent
        if(CH3) 
        {//Blynk.notify("CH3 is turned OFF");
          CH3 = false;
        }
        digitalWrite(ch3, LOW); // set LED OFF
        Blynk.virtualWrite(V2, 0);
        terminal.print("ch3 STOPPED at");
        //terminal.println(String(" ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.println(String(" ") + String(stopsecondswd/3600) + ":" + String((stopsecondswd%3600)/60));
        terminal.flush();
        // code here to switch the relay OFF
      }              
    }
    /*else{
      if((nowseconds >= startsecondswd)&& !CH3){  
        digitalWrite(ch3, HIGH); // set LED ON    test
        Blynk.virtualWrite(V6, 1);
        terminal.println("CH3 is ON");
        terminal.flush();
        Blynk.notify("Ch3 is turned ON");
      
      }          
    }*/
  }
  else{
    terminal.println("CH3 INACTIVE today");
    terminal.flush();
    // nothing to do today, check again in 30 SECONDS time    
  }
  terminal.println();
}


BLYNK_WRITE(V7) // ch4 scheduler  
{     
    sprintf(Date, "%02d/%02d/%04d",  day(tim), month(tim), year(tim));
    sprintf(Time, "%02d:%02d:%02d", hour(tim), minute(tim), second(tim));
  
    TimeInputParam t(param);
  
    terminal.print("ch4 Checked schedule at: ");
    terminal.println(Time);
    terminal.flush();
    int dayadjustment = -1;  
    if(weekday(tim) == 1){
      dayadjustment =  6; // needed for Sunday, Time library is day 1 and Blynk is day 7
    }
    if(t.isWeekdaySelected(weekday(tim) + dayadjustment)){ //Time library starts week on Sunday, Blynk on Monday
    terminal.println("ch4 ACTIVE today");
    terminal.flush();
    Dusk2Dawn greece(38.0529, 23.6943, (t.getTZ_Offset()/3600));
    int Sunrise  = 60*(greece.sunrise(year(tim), month(tim), day(tim), false));
    int Sunset  = 60*(greece.sunset(year(tim), month(tim), day(tim), false)+25);
    if (t.hasStartTime()) // Process start time
      {
        terminal.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute());
        terminal.flush();
      }
    
      
      else if(t.isStartSunset())
      {
        terminal.println(String("Start: ") + String((Sunset/3600),DEC) + ":" + String(((Sunset%3600)/60),DEC));
        terminal.flush();
      }
      
    if (t.hasStopTime()) // Process stop time
    {
        terminal.println(String("Stop : ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.flush();
      }
    
      
      else if(t.isStopSunrise())
      {
        terminal.println(String("Start: ") + String((Sunrise/3600),DEC) + ":" + String(((Sunrise%3600)/60),DEC));
        terminal.flush();
      }
      
    // Display timezone details, for information purposes only 
    terminal.println(String("Time zone: ") + t.getTZ()); // Timezone is already added to start/stop time 
  //  terminal.println(String("Time zone offset: ") + t.getTZ_Offset()); // Get timezone offset (in seconds)
    terminal.flush();
  
     for (int i = 1; i <= 7; i++) {  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)
        if (t.isWeekdaySelected(i)) {
        terminal.println(String("Day ") + i + " is selected");
        terminal.flush();
        }
      } 
    nowseconds = ((hour(tim) * 3600) + (minute(tim) * 60) + second(tim));
    if(t.isStartSunset())
      startsecondswd = Sunset;
    else
      startsecondswd = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
    Serial.println(startsecondswd);  // used for debugging
    if(nowseconds >= startsecondswd){    
      terminal.print("CH4 STARTED at");
      //terminal.println(String(" ") + t.getStartHour() + ":" + t.getStartMinute());
      terminal.println(String(" ") + String(startsecondswd/3600) + ":" + String((startsecondswd%3600)/60));
      terminal.flush();
      if(nowseconds <= startsecondswd + 30){    // 90s on 60s timer ensures 1 trigger command is sent
             if (!CH4) 
             {
              CH4 = true;
             }
        digitalWrite(ch4, HIGH); // set LED ON
        Blynk.virtualWrite(V3, 1);
        
        // code here to switch the relay ON
      }      
    }
    else{
      terminal.println("CH4 Device NOT STARTED today");
      terminal.flush();
   
    }
    if(t.isStopSunrise())
      stopsecondswd = Sunrise;
    else
      stopsecondswd = (t.getStopHour() * 3600) + (t.getStopMinute() * 60);
    //Serial.println(stopsecondswd);  // used for debugging
    if((nowseconds >= stopsecondswd)&& CH4){  
      
      if(nowseconds <= stopsecondswd + 60){   // 90s on 60s timer ensures 1 trigger command is sent
        if(CH4) 
        {
          CH4 = false;
        }
        digitalWrite(ch4, LOW); // set LED OFF
        Blynk.virtualWrite(V3, 0);
        terminal.print("ch4 STOPPED at");
        //terminal.println(String(" ") + t.getStopHour() + ":" + t.getStopMinute());
        terminal.println(String(" ") + String(stopsecondswd/3600) + ":" + String((stopsecondswd%3600)/60));
        terminal.flush();
        // code here to switch the relay OFF
      }              
    }
    /*else{
      if((nowseconds >= startsecondswd)&& !CH4){  
        digitalWrite(ch4, HIGH); // set LED ON    test
        Blynk.virtualWrite(V7, 1);
        terminal.println("CH4 is ON");
        terminal.flush();
      
      }          
    }*/
  }
  else{
    terminal.println("CH4 INACTIVE today");
    terminal.flush();
    // nothing to do today, check again in 30 SECONDS time    
  }
  terminal.println();
}

/*********************************************************************/



void loop() {

  BlynkEdgent.run();
  timer.run();
  if(blynk_run)
    Blynk.run();
}

