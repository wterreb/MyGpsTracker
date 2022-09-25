#include "SIM900.h"
#include <SoftwareSerial.h>
#include "SMS.h"
#include "GPS.h"

SMSGSM sms;
GPS gps;
int SecondsCntr = 0;
byte TenSecondsCntr = 0;
int MinutesCntr = 5;
const char *MY_PHONE_NUMBER = "02040782039";


//To change pins for Software Serial, use the two lines in GSM.cpp.

//GSM Shield for Arduino
//www.open-electronics.org
//this code is based on the example of Arduino Labs.

//Simple sketch to send and receive SMS.

int numdata;
boolean started=false;
boolean GpsStarted=false;
char smsbuffer[160];
char good_location[30];
char n[20];
int timer1_counter;
boolean every_ten_seconds = false;
#define ledPin 13


void setup()
{
     //Serial connection.
     Serial.begin(115200);
     Serial.println("GSM Shield testing.");
     //Start configuration of shield with baudrate.
     if (gsm.begin(115200)) {
          Serial.println("\nstatus=READY");
          started=true;
     } else Serial.println("\nstatus=IDLE");
     if ( gps.Enable() ) {
         GpsStarted=true;
     }

     pinMode(ledPin, OUTPUT);

// initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  timer1_counter = 3036;   // preload timer 65536-16MHz/256/1Hz
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts

};

//Timer interrupt each second
ISR(TIMER1_OVF_vect)        // interrupt service routine
{                      
   TCNT1 = timer1_counter;   // preload timer
   digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
   SecondsCntr++; 
   TenSecondsCntr++;
   if (TenSecondsCntr == 10) {
      TenSecondsCntr = 0;
      every_ten_seconds = true;
   }
   else {
      every_ten_seconds = false;
   }

}


bool SendLocationSMS(char* location, char* phoneNumber){
      bool retval = false;
      char SmsContents[160];
 
      sprintf(SmsContents,"https://www.google.com/maps/place/%s\n%d:%d",location,MinutesCntr,SecondsCntr);
      Serial.print (MinutesCntr);Serial.print (":");Serial.print (SecondsCntr); Serial.print ("  ");
      Serial.print("SMS to ");Serial.print(phoneNumber); Serial.print(" : "); Serial.println(SmsContents);

      char result = sms.SendSMS(phoneNumber, SmsContents);
      if (result == 1) {
        Serial.println("SMS sent OK");
        retval = true;
      }
      else {
         Serial.print("SMS result : ");Serial.println(result);
      }
      return retval;
}


void loop()
{
     Serial.println (""); Serial.print (MinutesCntr);Serial.print (":");Serial.print (SecondsCntr); Serial.print ("  ");
     if (every_ten_seconds) { 
         if (GpsStarted) {
            int gpsStatus = gps.IsFixed();     
            if (gpsStatus == 0) {
                Serial.println ("WAITING FOR GPS FIX");
            }
            if (gpsStatus == 1) {
                char location[30];
                if (gps.GetLocation(location))
                {
                    strcpy(good_location, location);
                    Serial.print (good_location); 
                }      
            }
         }
    
         if ((MinutesCntr >= 5) && strlen(good_location) > 10)
         {  
             Serial.print("MinutesCntr = ");Serial.println(MinutesCntr);     
             if (SendLocationSMS(good_location, MY_PHONE_NUMBER)) {
                ResetTimers();
             }
             else {
                Serial.println("SMS failed.");
             }
         }
  
    
         if(started) {
              //Read if there are messages on SIM card and print them.
              char position = sms.IsSMSPresent(SMS_UNREAD);
              char phone_number[20];
              if (position) {
                // read new SMS
                sms.GetSMS(position, phone_number, smsbuffer, 160);
                Serial.println((int)position);
                Serial.println(phone_number);
                Serial.println(smsbuffer);
                for (int x=0; x<strlen(smsbuffer); x++) {
                  smsbuffer[x] = tolower(smsbuffer[x]);
                }
    
                if (strstr((char *)smsbuffer, "gps") != NULL);
                {
                    SendLocationSMS(good_location, phone_number);
                }
                Serial.println("Deleting");
                sms.DeleteSMS(position);
              }  
         }
     }
     UpdateCounters();
     delay(1000);  
}


void ResetTimers() {
  Serial.println("Resetting Timers");
  SecondsCntr = 0;
  MinutesCntr = 0;
}

void UpdateCounters() {
    
    
    if (SecondsCntr >= 60) {
      MinutesCntr++;
      SecondsCntr = 0;
    }
}



  
/*
  boolean notConnected = true;
  while (notConnected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  */



/*
void Passthrough()
{
  if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    Serial.write(Serial1.read());   // read it and send it out Serial (USB)
  }
}


void loop() {
  if (PassThroughMode)
  {
      Passthrough();
  }
  else {
      ProcessUserCommands();
      ProcessModemResponses();
  }
}

void ProcessModemResponses() {
    if (Serial1.available()) {    
      String reponse = Serial1.readString();
      Serial.write(Serial1.read());  // Allways show A9G responses
    }
}

int GetStatus(String response)
{
  retval = STATUS_UNKNOWN;

  if(response.indexOf("OK") > 0) {
    return STATUS_OK;
  }
  if(response.indexOf("+LOCATION: GPS NOT FIX NOW") > 0) {
    return STATUS_NO_FIX;
  }
  
https://www.google.com/maps/place/-36.693218,174.738165
}


void SendCommand(String cmd) {
  if (debug_mode) {
     Serial.println(cmd);
  }
  else {
    Serial1.println(cmd);
  }
  
}


bool SendSMS (String phoneNumber,String text):

  sms.beginSMS(phoneNumber);
  sms.print(txtMsg);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n");


    SendCommand("AT+CMGF=1");
    SendCommand("AT+CMGS=\" + format(phoneNumber));
    delay(500);
    SendCommand('{}' + format(text));
    delay(500);
    self.__sendCommandEspecial()



String bool SendGpsLocation()
{
    bool retval = false;
    String coordinates = "";
    Serial1.println("AT+LOCATION=2");
    if (Serial.available()) {
        String coordinates = Serial.readString(); 
    }
    if(coordinatesponse.indexOf("+LOCATION: GPS NOT FIX NOW") > 0) {
        retval = false
    }
    else {
      if (coordinates.indexOf(",") > 0)) {
          String msg = "https://www.google.com/maps/place/" +  coordinates;
          retval = SendSMS(msg);
      }
    }
    return retval;
 
  } 
    
)
  

void ProcessUserCommands() {
  if (Serial.available()) {
      String cmd = Serial.readString(); 
      cmd.trim();
      Serial.println("");
      Serial.print("!");
      Serial.print(cmd);   //  !!!!!!!!!!!!!!!!!!!!!!!!
      Serial.print("#");
      if ( cmd == "start")
      {
        PassThroughMode = true;
        Serial.println("Passthrough Mode Started");
      }
      else {
         Serial1.println(cmd);
      }
  }
}
*/
