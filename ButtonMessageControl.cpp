#include "ButtonMessageControl.h"
#include "Arduino.h"

void ButtonMessageControl::handleIncommingReply(MessageQueueItem * item){
  handleIncommingMessage(item);
}
void ButtonMessageControl::handleIncommingMessage(MessageQueueItem * item){
  /*
   * get data from message
   * decode commands
   * execute commands
   * 
   */
  CommandContext context(this);
  context.setMessageQueueItem(item);
  od_command_header_t * cmd = (od_command_header_t *)item->getData();
  switch (cmd->label){
    case BUTTON:
      if(handleButtonCommand != NULL){
        handleButtonCommand(&context, cmd->digital);
      }
      break;
    case LED:
      if(handleLedCommand != NULL){
        handleLedCommand(&context, cmd->pwm);
      }
      break;
    case VIBRATE:
      if(handleVibrateCommand != NULL){
        handleVibrateCommand(&context, cmd->pwm);
      }
      break;
  }
  context.finalizeMessage();
}

bool ButtonMessageControl::sendButtonEvent(bool pressed){
  od_command_header_t cmd;
  cmd.label = BUTTON;
  cmd.digital.value = (uint8_t)pressed;
  return sendMessage((uint8_t)MASTER, (uint8_t *)&cmd, (uint8_t)(sizeof(od_command_header_t)));
}

bool ButtonMessageControl::sendLedCommand(uint8_t toChannelID, pwm_command_t pwm){
 od_command_header_t cmd;
  cmd.label = LED;
  cmd.pwm = pwm;
 
  return sendMessage(toChannelID, (uint8_t *)&cmd, (uint8_t)(sizeof(od_command_header_t)));
}
bool ButtonMessageControl::sendLedCommand(uint8_t toChannelID, uint8_t shape,uint8_t offset,uint8_t duration,uint8_t amplitude,uint8_t period){
  pwm_command_t cmd;
  cmd.shape = shape;
  cmd.offset = offset; 
  cmd.duration = duration;
  cmd.amplitude = amplitude, 
  cmd.period = period;
  return sendLedCommand(toChannelID, cmd);
}

bool ButtonMessageControl::sendVibrateCommand(uint8_t toChannelID, pwm_command_t pwm){
  od_command_header_t cmd;
  cmd.label = VIBRATE;
  cmd.pwm = pwm;
 
  return sendMessage(toChannelID, (uint8_t *)&cmd, (uint8_t)(sizeof(od_command_header_t)));
}
bool ButtonMessageControl::sendVibrateCommand(uint8_t toChannelID, uint8_t shape,uint8_t offset,uint8_t duration,uint8_t amplitude,uint8_t period){
  pwm_command_t cmd;
  cmd.shape = shape;
  cmd.offset = offset; 
  cmd.duration = duration;
  cmd.amplitude = amplitude, 
  cmd.period = period;
  return sendVibrateCommand(toChannelID, cmd);
}

void CommandContext::finalizeMessage()
{
  if(!_replied){
    od_command_header_t cmd;
    cmd.label = ACKNOWLEDGE;
    reply(&cmd);
  }
}
void CommandContext::reply(od_command_header_t* command)
{
  _replied = true;
  _item->setData((uint8_t *)command, sizeof(od_command_header_t));
}
AbstractRFMessageControl* CommandContext::control()
{
  return _control;
}
