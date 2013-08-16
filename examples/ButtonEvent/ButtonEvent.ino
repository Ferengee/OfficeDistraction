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
  /*
   * constructor, prepare and execute methods
   * must be public for the dispatcher to be called
   */
public:
  // constructor
  RemoteControlledLed(){
    //this rpc method will be called with: led(...)
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

/*
 * create a stream wrapper and
 * create a dispatcher with it
 * The StreamWrapper can be subclassed to wrap types of streams
 * that don't extend Stream like for example the String class
 */
StreamWrapper in = StreamWrapper();
MiniRPCDispatcher d1 = MiniRPCDispatcher(&in);

/*
 * create an instance of our RPCMethod class 
 */
RemoteControlledLed tm;

void setup(){
  Serial.begin(9600);
  pinMode(pwmLedPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  tm.button = &button;
  button.setButtonEventHandler(buttonEvent);
   in.setStream(&Serial);
  /*
   * Don't forget to register your RPCMethod instance
   * The maximum of RPCMethods that a dispatcher can register
   * is set with MAX_METHOD_COUNT in MiniRPC.h
   */
  d1.registerMethod(&tm);

}


void loop(){
  button.sample();
  d1.update();
  delay(1);
}