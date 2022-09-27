#include "SIM900.h"
#include <SoftwareSerial.h>
#include "SMS.h"
#include "GPS.h"
#include "dtostrf.h"

SMSGSM sms;
GPS gps;
int SecondsCntr = 0;
byte TenSecondsCntr = 0;
int MinutesCntr = 0;
boolean every_ten_seconds = true;
//const char *MY_PHONE_NUMBER = "02040782039";
char my_phone_number[15];
double _10Bits = 1024.0;   // 2^10


int numdata;
boolean started=false;
boolean GpsStarted=false;
char smsbuffer[160];
char good_location[30];
char n[20];
int timer1_counter;
int RXLED = 17;
int BEEPER_ENA = 2;
int BEEPER_PWR = 3;
int BATT_VOLT = A6;
const double DRONE_FINDER_MIN_VOLTAGE = 3.5;
const double GPS_MIN_VOLTAGE = 3.3;
const double BATTERY_PRESENCE_MIN_VOLTAGE = 3.0;
boolean isBeeping = false;
int interval_minutes = 5;   //  Default is to send an SMS every 5 minutes
bool firstFix = false;


void setup()
{
     Serial.begin(115200);
     Serial.println("MyGpsTracker");
     
     pinMode(RXLED, OUTPUT);  // Set RX LED as an output
     digitalWrite(RXLED, HIGH);   // set the RX LED OFF
     pinMode(BEEPER_ENA, OUTPUT);
     pinMode(BEEPER_PWR, OUTPUT);
     digitalWrite(BEEPER_PWR, HIGH);
     digitalWrite(BEEPER_ENA, HIGH);
     strcpy (good_location, "");
     
     //Start configuration of shield with baudrate.
     if (gsm.begin(115200)) {
          Serial.println("\nstatus=READY");
          gsm.GetPhoneNumber(1,my_phone_number);
          Serial.print("Phone Number : "); Serial.println(my_phone_number);
          started=true;
     } else Serial.println("\nstatus=IDLE");
     if ( gps.Enable() ) {
         GpsStarted=true;
     }

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
   SecondsCntr++; 
   TenSecondsCntr++;
   if (TenSecondsCntr >= 10) {
      TenSecondsCntr = 0;
      every_ten_seconds = true;
   }
   else {
      every_ten_seconds = false;
   }

}


void SendInstructionSMS(char* phoneNumber)
{
    char SmsContents[160];
    strcat(SmsContents,"\nValid Commands:\n");
    strcat(SmsContents,"1. Info\n");
    strcat(SmsContents,"2. Beep on/off\n");  
    strcat(SmsContents,"3. Interval <interval>\n");
    strcat(SmsContents,"4. Save Phone Number\n");
    digitalWrite(BEEPER_ENA, LOW);
    char result = sms.SendSMS(phoneNumber, SmsContents);
    digitalWrite(BEEPER_ENA, HIGH);
}

bool SendLocationSMS(char* location, char* phoneNumber, double battVoltage){
      bool retval = false;
      char SmsContents[160];

      char voltageStr[20];
      dtostrf(battVoltage, 0, 2, voltageStr);
      sprintf(SmsContents,"https://www.google.com/maps/place/%s\n%d:%d\n%s\nbeep=%d\n%d minutes",location,MinutesCntr,SecondsCntr, voltageStr,isBeeping, interval_minutes);
      Serial.print (MinutesCntr);Serial.print (":");Serial.print (SecondsCntr); Serial.print ("  ");
      Serial.print("Sending SMS to ");Serial.print(phoneNumber); Serial.print(" : "); Serial.println(SmsContents);

      digitalWrite(BEEPER_ENA, LOW);
      char result = sms.SendSMS(phoneNumber, SmsContents);
      digitalWrite(BEEPER_ENA, HIGH);
      if (result == 1) {
        Serial.println("SMS sent OK");
        strcpy(good_location,"");
        retval = true;
      }
      else {
         Serial.print("SMS result : ");Serial.println(result);
      }
      return retval;
}

int ExtractMinutes(char *instr) {
  char *prefix = "interval ";
  char buf[20];
  memset(buf,'\0',sizeof(buf));
  strncpy(buf,instr+strlen(prefix),strlen(instr)-strlen(prefix));
  return atoi(buf);
}

void GpsObtainedFix() {
    for (int i=0; i<5 ;i++)
    {
        digitalWrite(BEEPER_ENA, LOW);
        delay(100);
        digitalWrite(BEEPER_ENA, HIGH);
        delay(100);
    }
}

void loop()
{
     double  Battvoltage = analogRead(BATT_VOLT) * (5.2 / _10Bits);
     Serial.println (""); Serial.print("Battvoltage = "); Serial.print(Battvoltage);  Serial.print ("  ");
     Serial.print (MinutesCntr); Serial.print (":"); Serial.print (SecondsCntr); Serial.print ("  ");

    if (Battvoltage >= BATTERY_PRESENCE_MIN_VOLTAGE)  // Battery presence detected
    {   
       if (Battvoltage <= DRONE_FINDER_MIN_VOLTAGE) {
          digitalWrite(BEEPER_PWR, LOW);
       }
       if (Battvoltage <= GPS_MIN_VOLTAGE){
          // Send one last SMS before switching the GPS off
          SendLocationSMS(good_location, my_phone_number, Battvoltage);  
          gps.Disable();
          GpsStarted = false;
          Serial.println ("*** REMOVED GPS POWER TO PROTECT BATTERY ***");      
       }
    }

    if (isBeeping) {
        digitalWrite(BEEPER_ENA, LOW);
        delay(200);
        digitalWrite(BEEPER_ENA, HIGH);
        delay(200);
    }
    
    if (every_ten_seconds) { 
         if (GpsStarted) {
            int gpsStatus = gps.IsFixed();     
            if (gpsStatus == 0) {
                Serial.print ("WAITING FOR GPS FIX");
                digitalWrite(RXLED, HIGH);   // set the RX LED OFF
                firstFix = false;
            }
            if (gpsStatus == 1) {
                digitalWrite(RXLED, LOW);   // set the RX LED ON
                if (!firstFix) {
                    firstFix = true;
                    GpsObtainedFix();
                }
                char location[30];
                if (gps.GetLocation(location))
                {
                    strcpy(good_location, location);
                    Serial.print (good_location); 
                }
            }
         }
    
         if ((MinutesCntr >= interval_minutes) && strlen(good_location) > 10)
         {     
             if (SendLocationSMS(good_location, my_phone_number, Battvoltage)) {
                ResetTimers();
             }
             else {
                Serial.println("SMS failed.");
             }
         }
         
  
         if(started) {
              //Read if there are messages on SIM card and print them.
              char smsIndex = sms.IsSMSPresent(SMS_UNREAD);
              char phone_number[20];
              if (smsIndex) {
                // read new SMS
                sms.GetSMS(smsIndex, phone_number, smsbuffer, 160);
                Serial.println("");
                Serial.println("****************");
                Serial.println("New SMS received");
                Serial.println((int)smsIndex);
                Serial.println(phone_number);
                Serial.println(smsbuffer);
                Serial.println("****************");
                for (int x=0; x<strlen(smsbuffer); x++) {
                  smsbuffer[x] = tolower(smsbuffer[x]);
                }
                
                bool valid_instruction = false;
;
                if (strstr(smsbuffer, "info") != NULL)
                {
                    SendLocationSMS(good_location, phone_number, Battvoltage);
                    valid_instruction = true;
                }
   
                if (strstr(smsbuffer, "beep on") != NULL)
                {
                    isBeeping = true;
                    valid_instruction = true;
                }

                if (strstr(smsbuffer, "beep off") != NULL)
                {
                    isBeeping = false;
                    valid_instruction = true;
                }

                if (strstr(smsbuffer, "interval ") != NULL)
                {
                    interval_minutes = ExtractMinutes(smsbuffer);
                    Serial.print("INTERVAL=");Serial.println(interval_minutes);
                    valid_instruction = true;
                }

                if (strstr(smsbuffer, "save phone number") != NULL)
                {
                    gsm.WritePhoneNumber(1, phone_number);
                    gsm.GetPhoneNumber(1, my_phone_number);
                    Serial.print("restored number = "); Serial.println(my_phone_number);
                    SendLocationSMS(good_location, my_phone_number, Battvoltage);
                    valid_instruction = true;       
                }

                if ( valid_instruction == false) {
                   SendInstructionSMS(phone_number);
                }
                else {
                   MinutesCntr = interval_minutes;
                }

                Serial.println("Deleting SMS");
                sms.DeleteSMS(smsIndex);
              }  
         }
     }
     UpdateCounters();
     delay(1000);  
}


void ResetTimers() {
  Serial.println("Resetting Timers");
  SecondsCntr = -10;  // Compensate for checking every 10 seconds only
  MinutesCntr = 0;
}

void UpdateCounters() {
    if (SecondsCntr >= 60) {
      MinutesCntr++;
      SecondsCntr = 0;
    }
}



  
