/*
 *  This Module is setup to allow for basic communication to the SIM900 module from the Atmega328 (Arduino Lilypad 3.3V 8MHz).
 *  Description: This example shows how to send a SMS to a desired phone.
 *  This example only shows the AT commands (and the answers of the module) used
 *  to send the SMS. For more information about the AT commands, refer to the AT 
 *  command manual.
 *  The "Serial" port is related to the pc serial port
 *  The "Serial1" port is related to the uC UART attached to the SIM900 module
 *  This example is based on example code from - http://www.cooking-hacks.com/documentation/tutorials/arduino-gprs-gsm-quadband-sim900
*/

#include "LowPower.h"
#include <SoftwareSerial.h>
SoftwareSerial SIM900(8, 9); // configure software serial port

int LedPin = 13; // Blinky Led
int LipoPwrPin = 7;  // Physical load switch that switches power on and off to the SIM900 from the battery
int PowerKeyPin = 6; // A Pin connected to the SIM900 through a transistor that commands the Sim900 to switch on and off.
int TempPin = A0; // Temperature Pin Tied to TC1046

int8_t answer;
int onModulePin= 2;
char aux_string[30];
char sms_string[140];
char phone_number[]="9374308516"; // Ryan
//char phone_number[]="5308480141"; //Tate

float TempF;
byte offMins = 15; // Set time in mins for reporting interval
word offInts = offMins * 60 / 8; // 60 seconds per min/8 second intervals

//******************************************************************************  
// Setup Routine
//******************************************************************************
void setup() {
  // initialize both serial ports:
  SIM900.begin(9600);
  Serial.begin(9600);

  //init power pins
  pinMode(LedPin, OUTPUT);
  pinMode(LipoPwrPin, OUTPUT);
  pinMode(PowerKeyPin, OUTPUT);
  pinMode(TempPin, INPUT);
  
  // Turn on Led
  digitalWrite(LedPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  power_on();
  delay(10000);

  // Send Out Variable Confirmation
  Serial.println("Starting...");
  Serial.print("offmins - " );
  Serial.print(offMins);
  Serial.print("offmins - " );
  Serial.print(offInts);
  
  // Test Network Connection
  Serial.println("Connecting to the network...");
  while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 
          sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0);

  Serial.println("CREG PASSED...");
  // Get Temperature and Put in Message
  // Putting to strings together is tricky - http://forum.arduino.cc/index.php/topic,182271.0.html 
  TempF=GetTempF();  
  sprintf(sms_string,"Initial Reading - %d.%02d", (int)TempF, (int)(TempF * 1000.0) % 1000);
  
  // Send the Temperature
  Serial.println("Sending Temp...");
  sendSMS(sms_string);
}
//******************************************************************************  
// Main Loop
//******************************************************************************
void loop() {
  
  // Enter power down state for 8 s with ADC and BOD module disabled
  digitalWrite(LedPin, LOW);  // led off
  power_off();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    
  for (word i=0; i <= offInts-1; i++){
    
    // Blink Led
    digitalWrite(LedPin, HIGH);
    delay(100);
    digitalWrite(LedPin, LOW);  

    // Enter power down state for 8 s with ADC and BOD module disabled
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }

  // Send the Temperature Info
  wakeupSendTemp();
}
// ******************************************************************************
void wakeupSendTemp(){
  digitalWrite(LedPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("Starting...");
  
  // Turn On Sim900
  power_on();
  delay(10000);
  
  // Test Network Connection
  Serial.println("Connecting to the network...");
  while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 
          sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );

  Serial.println("CREG PASSED...");
  // Get Temperature and Put in Message
  // Putting to strings together is tricky - http://forum.arduino.cc/index.php/topic,182271.0.html 
  TempF=GetTempF();  
  sprintf(sms_string,"Temp - %d.%02d", (int)TempF, (int)(TempF * 1000.0) % 1000);
  
  // Send the Temperature
  Serial.println("Sending Temp...");
  sendSMS(sms_string);
  
  
  // turn the LED off
  digitalWrite(LedPin, LOW); 
  
  // Power Off the SIM900
  power_off();
  
}  
//******************************************************************************
void power_on(){
    uint8_t answer=0;
    
    // checks if the module is started
    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {
        // turn on power from the battery
        digitalWrite(LipoPwrPin,HIGH);
        delay(3000);      
        // power on pulse
        digitalWrite(PowerKeyPin,HIGH);
        delay(3000);
        digitalWrite(PowerKeyPin,LOW);
    
        // waits for an answer from the module
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            Serial.println("Connecting to Sim900...");
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }
    
}
//******************************************************************************
void power_off(){
  
  // turn on power from the Sim900
        digitalWrite(LipoPwrPin,HIGH);
        delay(3000);      
        // power on pulse
        digitalWrite(PowerKeyPin,HIGH);
        delay(3000);
        digitalWrite(PowerKeyPin,LOW);
        
  // turn off power from the battery
        digitalWrite(LipoPwrPin,LOW);
}
//******************************************************************************
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;
    memset(response, '\0', 100);    // Initialice the string
    delay(100);
    while( SIM900.available() > 0) SIM900.read();    // Clean the input buffer
    SIM900.println(ATcommand);    // Send the AT command 

    Serial.print("Out - ");
    Serial.println(ATcommand);
    
    x = 0;
    previous = millis();
    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(SIM900.available() != 0){    
            response[x] = SIM900.read();
             
            x++;
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer) != NULL)    
            {
              answer = 1;
            }
        }
    // Waits for the asnwer with time out
    }while((answer == 0) && ((millis() - previous) < timeout));    

    return answer;
    Serial.println("In - ");
    Serial.println(response);
}
//******************************************************************************
int8_t sendSMS(char* SMSmessage){

Serial.print("Setting SMS mode...");
    sendATcommand("AT+CMGF=1", "OK", 1000);    // sets the SMS mode to text
    Serial.println("Sending SMS");
    
    sprintf(aux_string,"AT+CMGS=\"%s\"", phone_number);
    answer = sendATcommand(aux_string, ">", 2000);    // send the SMS number
    if (answer == 1)
    {
        Serial.println(SMSmessage);
        SIM900.println(SMSmessage);
        SIM900.write(0x1A);
 
        answer = sendATcommand("", "OK", 20000);
        if (answer == 1)
        {
            Serial.println("Sent ");    
        }
        else
        {
            Serial.println("TimeOut");
        }
    }
    else
    {
        Serial.print("error ");
        Serial.println(answer, DEC);
    }
     return answer;
}
//******************************************************************************
float GetTempF() {
  // Get Temperature of TC1046
  float temp;  
  temp = analogRead(TempPin) / 1024.0 * 3.3;
  temp = (temp - 0.424) / 0.00625;
  temp = (temp * 1.8) + 32.0;
  Serial.print("Temperature F =  ");
  Serial.println(temp,2);
  return temp;
}
