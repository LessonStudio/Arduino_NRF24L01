#include <SPI.h>
#include "RF24.h"
RF24 radio(9, 10); // Establish a new RF24 object
const uint64_t send_pipe=0xB01DFACECEL;//This will be this device
const uint64_t recv_pipe=0xDEADBEEFF1L;//This will be the other device

#define WHITE_PIN 2
#define RED_PIN 3

void setup()
{
  Serial.begin(9600); // Set up communcations with the serial monitor in the arduino IDE
  
  pinMode(WHITE_PIN, INPUT); // Prepare the pins for input
  pinMode(RED_PIN, INPUT);

  Serial.println("Start");
  radio.begin();// Basically turn on communications with the device
  radio.setPALevel(RF24_PA_LOW);//RF24_PA_MAX is max power
  radio.setRetries(15,15);//This will improve reliability
  radio.openWritingPipe(recv_pipe);//Set up the two way communications with the named device
  radio.openReadingPipe(1,send_pipe);
  
  radio.startListening();// Start listening for data which gives the device a kick
}
//These are the four codes that we will work with, the final three being those that are transmitted
#define CODE_NONE 0
#define CODE_MOTOR_START 1
#define CODE_MOTOR_STOP 2
#define CODE_MOTOR_EXPLODE 3
unsigned long message_code=CODE_NONE;//This is where the code to be sent will reside
void loop()
{
  //Check the status of the two buttons
  int white_pin_status=digitalRead(WHITE_PIN);
  int red_pin_status=digitalRead(RED_PIN);
  
  if(white_pin_status==1 && red_pin_status==1)//If both are pushed then send explode code
  {
    message_code=CODE_MOTOR_EXPLODE;
  }
  else if(white_pin_status==1)//Send start code if white is pushed
  {
    message_code=CODE_MOTOR_START;
  }
  else if(red_pin_status==1)//Send stop if red is pushed
  {
    message_code=CODE_MOTOR_STOP;
  }
  else// If nothing is pushed then, well, do nothing.
  {
    message_code=CODE_NONE;
  }
  if(message_code!=CODE_NONE)//If something is to be done then send the code
  {
    radio.stopListening();//We are sending not listening for now
    if(!radio.write(&message_code, sizeof(unsigned long)))// Send the message_code and check to see if it comes back false
    {
      Serial.println("Failed");
    }

    radio.startListening();//Go back to listening and wait for the ack signal.

    unsigned long started_waiting_at=micros();//This notes the time 
    boolean timeout=false;//Assume for the moment that there is no timeout

    while(!radio.available())//Keep looping while no ack has come in
    {
      if(micros()-started_waiting_at>200000)//If the loop has been going for more than 1/5th of a second then give up
      {
        timeout=true; //Note that it has failed
        break;
      }      
    }
    if(timeout==true)//If the previous looped marked as failure then 
    {
      Serial.println("Timeout");
    }
    else// If it didn't fail to ack then read in and printout the results.
    {
      unsigned long in_data;
      radio.read(&in_data, sizeof(unsigned long));
      Serial.println("In Data: "+String(in_data));
    }
    delay(1000);
  }
 
}

