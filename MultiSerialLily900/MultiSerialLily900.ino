
/*
This Module is setup to allow for basica communication to the SIM900 module from the Atmega32U4 (Arduino Lilypad USB).
The "Serial" port is related to the pc serial port
The "Serial1" port is related to the uC UART attached to the SIM900 module

This example is based on example code from - http://www.cooking-hacks.com/documentation/tutorials/arduino-gprs-gsm-quadband-sim900

Good Resources:
Common Commands - http://rwsdev.net/wp-content/uploads/2013/02/Sim900-rev01-Application-Note.pdf
AT Manual - http://www.cooking-hacks.com/skin/frontend/default/cooking/pdf/SIM900_AT_Command_Manual.pdf
Simcom Terminal Setup - http://www.seeedstudio.com/wiki/GPRS_Shield_V2.0#Fun_with_AT_Commands


*/

int LipoPwrPin = 5;  // Physical load switch that switches power on and off to the SIM900 from the battery
int PowerKeyPin = 6; // A Pin connectoed to the SIM900 through a transistor that commands the Sim900 to switch on and off.


void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
  
  //init power pins
  pinMode(LipoPwrPin, OUTPUT);
  pinMode(PowerKeyPin, OUTPUT);
  
  //Run Power On Routine
  power_on();
}

void loop() {
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte); 
  }
  
  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte); 
  }
}

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
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }
    
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialice the string
    
    delay(100);
    
    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer
    
    Serial1.println(ATcommand);    // Send the AT command 


    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial1.available() != 0){    
            response[x] = Serial1.read();
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
}
