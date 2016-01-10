#include <SPI.h>
#include "RF24.h" // This is the RF24 library that may need to be installed through the Manage Libraries feature in the IDE.

#include <Servo.h>//Include and create a servo object for controlling the servo motor
Servo servo;

RF24 radio(9, 10);//Create a commuications object for talking to the NRF24L01
const uint64_t send_pipe=0xB01DFACECEL;//These are just arbitrary 64bit numbers to use as pipe identifiers
const uint64_t recv_pipe=0xDEADBEEFF1L;//They must be the same on both ends of the communciations

//As in the transmit code these are the possible motor codes.
#define CODE_NONE 0
#define CODE_MOTOR_START 1
#define CODE_MOTOR_STOP 2
#define CODE_MOTOR_EXPLODE 3

#define SERVO_PIN 6//This is obviously the pin which tells the servo what to do

//These three variables are for controlling the motor's sweeping back and fourth
bool motor_running=false;
float position=90;
int direction=1;

void setup()
{
  Serial.begin(9600);//Set up comm with the IDE serial monitor
  Serial.println("Ready for commands");
  radio.begin();//Start up the radio object
  radio.setRetries(15,15);//This will improve reliability of the module if it encounters interference
  radio.setPALevel(RF24_PA_LOW);//This sets the power low. This will reduce the range. RF24_PA_MAX would increase the range
  radio.openWritingPipe(send_pipe);//Thses are the reverse of the transmit code.
  radio.openReadingPipe(1,recv_pipe);
  radio.startListening();//Give the module a kick
  servo.attach(SERVO_PIN);//Spool up the servo
}

void loop()
{
  unsigned long motor_code=CODE_NONE;

  if( radio.available())//Keep checking on each loop to see if any data has come in
  {
    while(radio.available())//Loop while there is incoming data. The packets are one unsigned long in total so it shoudl only loop once
    {
      radio.read(&motor_code, sizeof(unsigned long));//Stuff the incoming packet into the motor_code variable
    }
    radio.stopListening();//We have heard so now we will send an ack
    radio.write(&motor_code, sizeof(unsigned long));//Turn the motor code around and send it back
    radio.startListening();//Go back to listening as this is what this mostly does

    //set the motor_running varible depending upon the code received
    if(motor_code==CODE_MOTOR_START)
    {
      Serial.println("THE MOTOR HAS STARTED");
      motor_running=true;
    }
    else if(motor_code==CODE_MOTOR_STOP)
    {
      Serial.println("THE MOTOR HAS STOPPED");
      motor_running=false;
    }
    else if(motor_code==CODE_MOTOR_EXPLODE)
    {
      Serial.println("*** BOOM !!! *** THE MOTOR HAS EXPLODED");
      motor_running=false;
    }
  }
  if(motor_running==true) //If any of the above code set the motor_running to true then, run the motor
  {
    //This just sweeps the motor back and fourth.
    position+=0.4f*(float)direction;
    if(position>170.0f || position<10.0f){direction*=-1;}
    servo.write(position);
    delay(10);
  }
}

