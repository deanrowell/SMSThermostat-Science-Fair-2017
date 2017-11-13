/* 
 *GPRS library written by Lawliet Zou for Seeed Technology Inc. and is licensed under the MIT public domain license  -  https://github.com/Seeed-Studio
 *Program written by Dean Rowell for FCBOE Science Fair at Starr's Mill High School -  https://github.com/deanrowell
*/

#include <GPRS_Shield_Arduino.h>  //inclusion of necessary libraries
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    7        // declaration of SoftwareSerial variables for GSM module
#define PIN_RX    8
#define BAUDRATE  9600

#define MESSAGE_LENGTH 160      // creation of array for storing received message
char message[MESSAGE_LENGTH];
int messageIndex = 0;         // variable for storing number of unread messages

char phone[16];         // variable for storing phone number of sender  
char datetime[24];      // variable for storing date and time of message receipt
int messageInt = 0;     // variable for storing integer form of message, if applicable
int diff = 0;           // variable for storing calculated difference between previous temp and new temp
int currentTemp = 70;   // variable for storing current temp
int foo = 0;            // simply a variable for storing how many times the applyTemp() loop has run
const int upButton = 11;      // alias for upButton pin
const int downButton = 10;    // alias for downButton pin
bool firstPress = true;   // variable for storing the true/false status of whether or not this time through the applyTemp() loop is the first one or not
const int heatSwitch = 3;   //aliases for heat/cool switches
const int heatSwitch2 = 2;
const int heatSwitch3 = 5;
String messageText;       // variable for storing the received message for comparsion with a few command phrases
char sendStr[128];        // variable for storing the message that will be sent back to the sender confirming receipt of the message

GPRS GPRS(PIN_TX,PIN_RX,BAUDRATE);//RX,TX,PWR,BaudRate  //initialization of the GSM module

void setup() {
  pinMode(upButton, INPUT);         //setting pinModes of the pins connected to the thermostat
  pinMode(downButton, INPUT);
  pinMode(heatSwitch, OUTPUT);
  pinMode(heatSwitch2, OUTPUT);
 pinMode(heatSwitch3, OUTPUT);
 digitalWrite(upButton ,LOW);         //making sure the thermostat up/down buttons are LOW for proper operation
 digitalWrite(downButton, LOW);
 heatOnCmd(0);                    //function call to set thermostat to cool by default
GPRS.init();              //initialize GSM module
  delay(10000);         //wait 10sec for GSM module to connect to the network
}

void loop() {
 
  foo = 0;            //set flags back to their default values
  firstPress = true;
  
  messageIndex = GPRS.isSMSunread();      //set messageIndex to number of unread messages

  if (messageIndex > 0) {                //if there is atleast one unread message,  
      
      GPRS.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);       //read the message and store its info to its respective variables
      GPRS.deleteSMS(messageIndex);         //delete message to avoid filling up system memory

      messageText = String(message);        //set messageText equal to the string variant of the message char array

      if (atoi(message) == 0) {       //if the received message is not an integer, check for command phrases:
        
         if (messageText.equals("current temp")) {        //if the message says "current temp",
            snprintf(sendStr, sizeof(sendStr), "current temp is %d", currentTemp );       //create char array consitsing of message to sender and current temp
            GPRS.sendSMS(phone,sendStr);                 //send this message to sender
         }

         if (messageText.equals("heat")) {           //if the message says "heat",
            heatOnCmd(1);         // set thermostat to heat mode
            GPRS.sendSMS(phone,"set to heat");        //confirm to sender
         }

         if (messageText.equals("cool")) {       //if message says "cool",
            heatOnCmd(0);         //set thermostat to cool mode
            GPRS.sendSMS(phone,"set to cool");    //confirm to sender
         }
          
         if (messageText.equals("info")) {     //if my parents forget how to use the thermostat, they can send 'info' for a list of commands
            GPRS.sendSMS(phone,"send integer to change temp. send 'heat' for heat or 'cool' for cool. send 'current temp' for current temp.");    //send a list of commands
         }
          
         if (messageText.equals("who is the best chemistry teacher")) {
            GPRS.sendSMS(phone,"Answer: Gant Man"); 
         }
      }
        
      
      else{    // otherwise...
          if (atoi(message) != 0) {   //if the message is an integer
              messageInt = atoi(message);   //set the variable messageInt to the converted message integer
      
              diff = messageInt - currentTemp;    //set the diff variable to the difference between the incoming temp and the current temp

              applyTemp(diff);      // apply the difference in temperatures to the thermostat

              snprintf(sendStr, sizeof(sendStr), "temp set to %d", currentTemp );       // make a char array appending the new temp to a message
              GPRS.sendSMS(phone,sendStr);      //send this message
          }
      }
  }
}

void applyTemp(int diff) {        //function to apply the diff to the thermostat
    int  absDiff = abs(diff);    //set the absDiff variable to the absolute value of the diff variable, for setting the amount of times the program will run through the applyTemp() loop

    if (diff >0) {              // if the difference is positive...
        while(foo < absDiff  ) {    //while the flag 'foo' is less than the difference...   (this causes the loop to be ran equal to the diff)
            if (firstPress == true) {     //if this is the first press...
                 buttonPress(upButton, 1100);     //actuate the button for one full second to tell the thermostat the temperature is being set
                 firstPress = false;          //this is no longer the first press because the first press just happened
                 delay (1000);              // wait for the thermostat to get ready
            }
        
            else if (firstPress == false) {       //if this is not the first press...
                 buttonPress(upButton, 150);      //actuate the button for 150ms
                 delay(750);                      //wait 750ms between presses
                 foo++;                     //increment the flag
            }
        }     
     } 

    if (diff <0) {                          //if the diff is negative...
        while(foo < absDiff  ) {                    //while the flag 'foo' is less than the difference...   (this causes the loop to be ran equal to the diff)
            if (firstPress == true) {             //if this is the first press...
                buttonPress(downButton, 1100);    //actuate the button for one full second to tell the thermostat the temperature is being set
                firstPress = false;         //this is no longer the first press because the first press just happened
                delay (1000);            // wait for the thermostat to get ready
            }
            else if (firstPress == false) {          //if this is not the first press...
                buttonPress(downButton, 150);         //actuate the button for 150ms
                delay(750);                       //wait 750ms between presses
                foo++;                            //increment the flag
            }
        }    
    }
    
    currentTemp = currentTemp + diff;  // set the new current temp based on the observed difference
}



void buttonPress(int p, int d) {        //function to simulate actuation of the button on the thermostat
  pinMode(p, OUTPUT);                     //set the pin input to the function as output, simulating actuation
  
  digitalWrite(p ,LOW);            //make sure pin is still set low
  
  delay(d);                               //keep pin actuated for desired time
  
  pinMode(p, INPUT);                //set pin back to input, simulating deactivation of button
  
  digitalWrite(p ,LOW);           //make sure pin is still set low
}

void heatOnCmd(int x) {                   //function to turn on/off all relays controlling heat/cool switches
  digitalWrite(heatSwitch, x);            //set all three pins either on or off
  digitalWrite(heatSwitch2, x);
  digitalWrite(heatSwitch3, x);
}

