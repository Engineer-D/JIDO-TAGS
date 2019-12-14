//include GPS library                          
#include <TinyGPS++.h>

//initialize gps
TinyGPSPlus gps;

//variable to hold longitude and latitude
double latitude, longitude;

//include software serial
#include <SoftwareSerial.h>

//define pins for software serial
SoftwareSerial SIM800L(10,11);
//push button
#define button 12
boolean lastButton = LOW; //Variable containing the previous
boolean currentButton = LOW; //Variable containing the current

//other variables
String response;
int lastStringLength = response.length();

String link;

void setup() {
  //Begin Serial communications
  Serial.begin(9600);
  Serial.println("GPS Starting");

  //begin software serial for GSM Module
  SIM800L.begin(9600);  
  
  SIM800L.println("AT+CMGF=1"); //Selects SMS message format as text. Default format is Protocol Data Unit (PDU)
  Serial.println("SIM800L started at 9600");
  delay(1000);
  Serial.println("Setup Complete! SIM800L is Ready!");
  SIM800L.println("AT+CNMI=2,2,0,0,0"); //Decides how newly arrived SMS messages should be handled
}

void loop() 
{
  GPS();
  if(digitalRead(button)==HIGH)
  {
    sendSMS1()
     Serial.println("SMS Sent!!");
  }
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
