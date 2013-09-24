
#include <VirtualWire.h>
#include <SampledButton.h>

#include <ButtonMessageControl.h>
#include <RFSenderReceiver.h>
#include <PWMControl.h>

#include <MiniRPC.h>
#include <StreamWrapper.h>
#include <StateMachine.h>
#include <BufferManager.h>
#include <Arduino.h>

#define RELAIS_PIN 4

RFSenderReceiver tr = RFSenderReceiver();
ButtonMessageControl bmc = ButtonMessageControl(&tr);

class RemoteControlledPWM : public MiniRPCMethod
{
public:
  RemoteControlledPWM(){
    master = NULL;
  }
  virtual void prepare(){
    get(toChannelID);
    get(shape);
    get(offset);
    get(duration);
    get(amplitude);
    get(period);
  }
  virtual void execute(){
    if(master != NULL)
      master->sendLedCommand(toChannelID, shape, offset, duration, amplitude, period);
  }

protected:
  ButtonMessageControl * master;
private:
  int toChannelID, shape, offset, duration, amplitude, period;
};

class RemoteControlledLed : public RemoteControlledPWM
{
public:
  RemoteControlledLed(){
    setName("led"); 
  }
};

class RemoteControlledVibrate : public RemoteControlledPWM
{
public:
  RemoteControlledVibrate(){
    setName("vibrate");
    master = NULL;
  }
};


class RemoteControlledRelay : public MiniRPCMethod
{
public:
  RemoteControlledRelay(){
    setName("relais");
    pinMode(RELAIS_PIN, OUTPUT);
  }
  virtual void prepare(){
    get(state);
  }
  virtual void execute(){
    digitalWrite(RELAIS_PIN, (state > 0));
  }

private:
  int state;

};


StreamWrapper in = StreamWrapper();
MiniRPCDispatcher d1 = MiniRPCDispatcher(&in);

RemoteControlledLed rc_led;
RemoteControlledVibrate rc_vibrate;
RemoteControlledRelay relais;

void setup()
{
  tr.init(12,11,2000);

  Serial.begin(9600);

  in.setStream(&Serial);
 
  d1.registerMethod(&rc_led);
  d1.registerMethod(&rc_vibrate);

}

void loop(){
    d1.update();
}
