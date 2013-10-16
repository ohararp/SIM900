
/*
 *  This Module is setup to allow for basica communication to the SIM900 module from the Atmega32U4 (Arduino Lilypad USB).
 *  Description: This example shows how to send a SMS to a desired phone.
 *  This example only shows the AT commands (and the answers of the module) used
 *  to send the SMS. For more information about the AT commands, refer to the AT 
 *  command manual.
 *  The "Serial" port is related to the pc serial port
 *  The "Serial1" port is related to the uC UART attached to the SIM900 module
 *  This example is based on example code from - http://www.cooking-hacks.com/documentation/tutorials/arduino-gprs-gsm-quadband-sim900
*/

int LipoPwrPin = 5;  // Physical load switch that switches power on and off to the SIM900 from the battery
int PowerKeyPin = 6; // A Pin connectoed to the SIM900 through a transistor that commands the Sim900 to switch on and off.

int8_t answer;
int onModulePin= 2;
char aux_string[30];
char phone_number[]="9374308516";

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
  
  //init power pins
  pinMode(LipoPwrPin, OUTPUT);
  pinMode(PowerKeyPin, OUTPUT);
  
  //Run Power On Routine
  power_on();
  
    Serial.println("Starting...");
    power_on();
    
    delay(3000);
    
    // sets the PIN code
    sendATcommand("AT+CPIN=1234", "OK", 2000); // Pin Code set at Sim Card Purchase
    
    delay(3000);
    
    Serial.println("Connecting to the network...");

    while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 
            sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );

    Serial.print("Setting SMS mode...");
    sendATcommand("AT+CMGF=1", "OK", 1000);    // sets the SMS mode to text
    Serial.println("Sending SMS");
    
    sprintf(aux_string,"AT+CMGS=\"%s\"", phone_number);
    answer = sendATcommand(aux_string, ">", 2000);    // send the SMS number
    if (answer == 1)
    {
        Serial.println("Test-Arduino-Hello World");
        
        Serial1.println("Test-Arduino-Hello World");
        Serial1.write(0x1A);

        answer = sendATcommand("", "OK", 20000);
        if (answer == 1)
        {
            Serial.print("Sent ");    
        }
        else
        {
            Serial.print("Error Code =  ");
        }
    }
    else
    {
        Serial.print("error ");
        Serial.println(answer, DEC);
    }

}

void loop() {

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
