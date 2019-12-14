 /******************************************Include libraries***********************************************/
//include GPS library                          
#include <TinyGPS++.h>

//include adafruit fona library so to send code to thingspeak
#include <Adafruit_FONA.h>

//include software serial
#include <SoftwareSerial.h> //library included to ensure software serial communication
/*************************************Initialize library**************************************************/
//initialize gps
TinyGPSPlus gps; //initialize gps variable for the library tinygps

//define pins for software serial
SoftwareSerial SIM800L(10,11); //Rx TX for sim800l
Adafruit_FONA SIM800 = Adafruit_FONA(7);
/****************************************Define Variables*************************************************/
//variable to hold longitude and latitude
double latitude, longitude; //variable latitude for latitude and longitude for longitude
char latitude_string[20],longitude_string[20];

//push button
#define button 13 // variable for the push button on digitalpin 13

//other variables
String link; //this link is the one that contains the google map api call
char http_cmd[80]; // variable that holds the link to send to thingspeak server

//change api key
char url_string[] = "api.thingspeak.com/update?api_key=86HJRJJ8VRDLLJ6O&field1";

//connectivity variable
int net_status; //variable to check network status
uint16_t statuscode; 
int16_t length;
String response = ""; // variable to hold response 
char buffer[512];
boolean gprs_on = false; //Code to check if GPRS is on
boolean tcp_on = false; //Code to activate TCP

void setup() {
  //Begin Serial communications
  Serial.begin(9600);
  //begin software serial for GSM Module
  SIM800L.begin(9600);  
  //setup serially
  Serial.println(F("GPS Starting")); //print on serial monitor GPS Starting
  Serial.println(F("Initializing SIM800L....")); // print on serial monitor Initializing SIM800L....

  //check if sim800l is correctly 
  if (!SIM800.begin(SIM800L)) 
  {            
    Serial.println(F("Couldn't find SIM800L"));
    while (1);
  }
  Serial.println(F("SIM800L is OK"));
  SIM800L.println(F("AT+CMGF=1")); //Selects SMS message format as text. Default format is Protocol Data Unit (PDU)
  Serial.println("SIM800L started at 9600");
  delay(1000);
  Serial.println(F("Setup Complete! SIM800L is Ready!"));
  SIM800L.println(F("AT+CNMI=2,2,0,0,0")); //Decides how newly arrived SMS messages should be handled

  //register sim800l module to network
  Serial.println(F("Waiting to be registered to network..."));
  net_status = SIM800.getNetworkStatus(); //code to get network status
  //if it doesn't work try again
  while(net_status != 1)\
  {
     net_status = SIM800.getNetworkStatus();
     delay(2000);
  }
  Serial.println(F("Registered to home network!"));
  Serial.print(F("Turning on GPRS... "));
  delay(2000); 
  while(!gprs_on)
  {
    if (!SIM800.enableGPRS(true))
    {  
        Serial.println(F("Failed to turn on GPRS"));
        Serial.println(F("Trying again..."));
        delay(2000);
        gprs_on = false;
    }
    else
    {
        Serial.println("GPRS now turned on");
        delay(2000);
        gprs_on = true;   
    } 
  }
  
}

void loop() 
{
  GPS();
  //if distress button is pressed
  if(digitalRead(button)==HIGH)
  {
    sendSMS1();
     Serial.println("SMS Sent!!");
  }

  //converting longitude and latitude  to string so to  send online
  dtostrf(latitude, 5, 0, latitude_string);
  dtostrf(longitude, 5, 0, longitude_string);

  sprintf(http_cmd,"%s=%s&field2=%s",url_string,latitude_string,longitude_string);
  delay(500); //2000

  //sending data to thingspeak
  while(!tcp_on)
  {
      if (!SIM800.HTTP_GET_start(http_cmd, &statuscode, (uint16_t *)&length)) 
      {
           Serial.println("Failed!");
           Serial.println("Trying again...");
           tcp_on = false;
      }
      else
      {
        tcp_on = true;
        while (length > 0) 
        {
           while (SIM800.available()) 
           {
             char c = SIM800.read();
             response += c;
             length--;
           }
        }
        Serial.println(response);
        if(statuscode == 200)
        {
          Serial.println("Success!");
          tcp_on = false;
        }
      }
      delay(1000); //2000
  }
  delay(1000); //2000
}

void GPS(){
  if(Serial.available()) {
    gps.encode(Serial.read());
  }
  if(gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    link = "www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6) ;
    Serial.println(link);
  
  }
}

void sendSMS1()
{
  SIM800L.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(100);  // Delay of 1000 milli seconds or 1 second
  SIM800L.println("AT+CMGS=\"+2348166700905\"\r"); // Replace x with mobile number
  delay(100); //1000
  SIM800L.println(link);// The SMS text you want to send
  delay(100);
  SIM800L.println((char)26);// ASCII code of CTRL+Z
  delay(100); //1000
   Serial.println("Sending SMS to David!!");
}
