#include <StreamParser.h>
#include <StreamWrapper.h>
#include <BufferManager.h>
#include <StateMachine.h>
#include <MiniRPC.h>

#include <Arduino.h>
#include <BigDomeButton.h>

int pwmLedPin = 11;
int buttonPin = A0;

BigDomeButton button = BigDomeButton(pwmLedPin, buttonPin);
void  buttonEvent (bool pressed){
  if (pressed)
    Serial.println("{\"class\": \"ButtonDownEvent\"}");
   else
    Serial.println("{\"class\": \"ButtonUpEvent\"}");   
}

class RemoteControlledLed : public MiniRPCMethod
{
public:
  RemoteControlledLed(){
    setName("led");
  }
  
  virtual void prepare(){
    get(intensity);
  }
 
  virtual void execute(){
    button->setLedBrightness(intensity);
  }

  virtual void error(){
    Serial.println("{\"class\": \"ArgumentException\", \"message\":\"Expected: led(<pin>, <intensity>)\"}");
  }
    
  BigDomeButton * button;
private:
  int intensity;
};


StreamWrapper in = StreamWrapper();
MiniRPCDispatcher d1 = MiniRPCDispatcher(&in);


RemoteControlledLed tm;

void setup(){
  Serial.begin(9600);
  pinMode(pwmLedPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  tm.button = &button;
  button.setButtonEventHandler(buttonEvent);
  in.setStream(&Serial);
  d1.registerMethod(&tm);

}


void loop(){
  button.sample();
  d1.update();
  delay(1);
}