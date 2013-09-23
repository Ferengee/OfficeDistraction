#include <VirtualWire.h>
#include <SampledButton.h>

#include <ButtonMessageControl.h>
#include <RFSenderReceiver.h>
#include <PWMControl.h>




RFSenderReceiver tr = RFSenderReceiver();




ButtonMessageControl bmc = ButtonMessageControl(&tr);

void handleButtonCommand(CommandContext * context, digital_command_t cmd)
{
  Serial.print("handle button: ");
  Serial.println(cmd.value);
  od_command_header_t response; 
  response.label= LED;
  response.pwm.shape = SINE;
  response.pwm.offset = 125; 
  response.pwm.duration = 10;
  response.pwm.amplitude = 255, 
  response.pwm.period = 30;
  context->reply(&response);
}

void setup(){
 
  bmc.setChannelID(MASTER);

  bmc.handleButtonCommand = handleButtonCommand;



}

void loop(){

  bmc.update();

//
}
