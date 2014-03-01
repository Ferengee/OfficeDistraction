#include <VirtualWire.h>
#include <SampledButton.h>

#include <ButtonMessageControl.h>
#include <RFSenderReceiver.h>
#include <PWMControl.h>

#include <MiniRPC.h>

int button_pin = 2;
int led_pin = 5;
int vibrate_pin = 3;


RFSenderReceiver tr = RFSenderReceiver();

// BaseSenderReceiver tr = RFSenderReceiver();
PWMControl led = PWMControl(5);
PWMControl vibrate = PWMControl(3);

SampledButton button = SampledButton(button_pin);

ButtonMessageControl bmc = ButtonMessageControl(&tr);

StreamWrapper in = StreamWrapper();
MiniRPCDispatcher dispatcher = MiniRPCDispatcher(&in);

void clicked(bool up, int deltaT){
    bmc.sendButtonEvent(up);
}

void handleVibrateCommmand(CommandContext * context, pwm_command_t cmd)
{
  vibrate.set(cmd.shape, cmd.offset, cmd.duration, cmd.amplitude, cmd.period);
}
void handleLedCommand(CommandContext * context, pwm_command_t cmd)
{
  led.set(cmd.shape, cmd.offset, cmd.duration, cmd.amplitude, cmd.period);
}

class Stats : public MiniRPCMethod
{
public:

  Stats(){
    setName("stats");
  }
  
  virtual void execute(){
    int min, max, failures, succeses;
    bmc->reportSendTime(min, max);
    Serial.print("min send time: ");
    Serial.println(min);
    
    Serial.print("max send time: ");
    Serial.println(max);
    
    bmc->reportMessageCount(failures, succeses);
    Serial.print("failure count: ");
    Serial.println(failures);
    
    Serial.print("succes count: ");
    Serial.println(succeses);
  }

  ButtonMessageControl * bmc;
};



Stats stats;

void setup(){
  Serial.begin(9600);

  tr.init(12,11,2000);

  in.setStream(&Serial);
  
  stats.bmc = &bmc;
  
  dispatcher.registerMethod(&stats);

  pinMode(button_pin, INPUT);  
  pinMode(led_pin, OUTPUT);  
  pinMode(vibrate_pin, OUTPUT);  
  digitalWrite(button_pin, HIGH);  //enable pullup resistor
  bmc.setChannelID(1);
  bmc.handleVibrateCommand = handleVibrateCommmand;
  bmc.handleLedCommand = handleLedCommand;
  button.setClickEventHandler(clicked);
  Serial.println("setup finished...");

}

void loop(){
  button.sample();
  bmc.update();
  led.update();
  vibrate.update();
  dispatcher.update();
//
}
