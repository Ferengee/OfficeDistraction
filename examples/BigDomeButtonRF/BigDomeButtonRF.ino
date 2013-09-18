#include <VirtualWire.h>
#include <SampledButton.h>

#include <ButtonMessageControl.h>
#include <RFSenderReceiver.h>
#include <PWMControl.h>

int button_pin = 2;
int led_pin = 5;
int vibrate_pin = 3;


RFSenderReceiver tr = RFSenderReceiver();

// BaseSenderReceiver tr = RFSenderReceiver();
PWMControl led = PWMControl(5);
PWMControl vibrate = PWMControl(3);

SampledButton button = SampledButton(button_pin);

ButtonMessageControl bmc = ButtonMessageControl(&tr);

void clicked(bool up, int deltaT){
    bmc.sendButtonEvent(up);
}

void handleVibrateCommmand(uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period)
{
  vibrate.set(shape, offset, duration, amplitude, period);
}
void handleLedCommand(uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period)
{
  led.set(shape, offset, duration, amplitude, period);
}



void setup(){
  pinMode(button_pin, INPUT);  
  pinMode(led_pin, OUTPUT);  
  pinMode(vibrate_pin, OUTPUT);  
  digitalWrite(button_pin, HIGH);  //enable pullup resistor
  bmc.setChannelID(1);
  bmc.handleVibrateCommand = handleVibrateCommmand;
  bmc.handleLedCommand = handleLedCommand;
  button.setClickEventHandler(clicked);

}

void loop(){
  button.sample();
  bmc.update();
  led.update();
  vibrate.update();
//
}
