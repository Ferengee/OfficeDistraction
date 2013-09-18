#include <VirtualWire.h>
#include <SampledButton.h>

#include <ButtonMessageControl.h>
#include <RFSenderReceiver.h>
#include <PWMControl.h>




RFSenderReceiver tr = RFSenderReceiver();




ButtonMessageControl bmc = ButtonMessageControl(&tr);

void handleButtonCommand(bool value)
{
  Serial.print("handle button: ");
  Serial.println(value);
  bmc.sendLedCommand(1, SINE, 125, 10, 255, 30);
}

void setup(){
 
  bmc.setChannelID(MASTER);

  bmc.handleButtonCommand = handleButtonCommand;



}

void loop(){

  bmc.update();

//
}
