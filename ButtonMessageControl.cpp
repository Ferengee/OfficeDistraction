#include "ButtonMessageControl.h"
#include "Arduino.h"


void ButtonMessageControl::handleIncommingMessage(MessageQueueItem * item){
  /*
   * get data from message
   * decode commands
   * execute commands
   * 
   */
  
  uint8_t * data = item->getData();
  uint8_t target = data[0];
  digital_command_t * digital_cmd = NULL;
  pwm_command_t * pwm_cmd = NULL;
  switch (target){
    case BUTTON:
      if(handleButtonCommand != NULL){
        digital_cmd = (digital_command_t *)(data +1);
        handleButtonCommand(digital_cmd->value);
      }
      break;
    case LED:
      if(handleLedCommand != NULL){
        pwm_cmd = (pwm_command_t * )(data + 1);
        handleLedCommand(pwm_cmd->shape, pwm_cmd->offset, pwm_cmd->duration, pwm_cmd->amplitude, pwm_cmd->period);
      }
      break;
    case VIBRATE:
      if(handleVibrateCommand != NULL){
        pwm_cmd = (pwm_command_t * )(data + 1);
        handleVibrateCommand(pwm_cmd->shape, pwm_cmd->offset, pwm_cmd->duration, pwm_cmd->amplitude, pwm_cmd->period);
      }
      break;
  }
}

bool ButtonMessageControl::sendButtonEvent(bool pressed){
  uint8_t event[sizeof(digital_command_t) + 1];
  event[0] = BUTTON;
  digital_command_t * cmd = (digital_command_t *)(event + 1);
  cmd->value = (uint8_t)pressed;
  return sendMessage((uint8_t)MASTER, event, (uint8_t)(sizeof(digital_command_t) + 1));
}

bool ButtonMessageControl::sendLedCommand(uint8_t toChannelID, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period){
  sendCommand(toChannelID, LED, shape, offset, duration, amplitude, period);
}

bool ButtonMessageControl::sendVibrateCommand(uint8_t toChannelID, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period){
  sendCommand(toChannelID, VIBRATE, shape, offset, duration, amplitude, period);
}

bool ButtonMessageControl::sendCommand(uint8_t toChannelID, uint8_t target, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period)
{
  uint8_t event[sizeof(digital_command_t) + 1];
  event[0] = target;
  pwm_command_t * pwm_cmd = (pwm_command_t *)(event + 1);
  pwm_cmd->shape = shape;
  pwm_cmd->offset = offset;
  pwm_cmd->duration = duration;
  pwm_cmd->amplitude = amplitude;
  pwm_cmd->period = period;
  
  return sendMessage(toChannelID, event, (uint8_t)(sizeof(digital_command_t) + 1));

}
