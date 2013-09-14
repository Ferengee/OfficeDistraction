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
      digital_cmd = (digital_command_t *)(data +1);
      _handleButtonCommand(digital_cmd->value);
      break;
    case LED:
      pwm_cmd = (pwm_command_t * )(data + 1);
      _handleLedCommmand(pwm_cmd->shape, pwm_cmd->offset, pwm_cmd->duration, pwm_cmd->amplitude, pwm_cmd->period);
      break;
    case VIBRATE:
      pwm_cmd = (pwm_command_t * )(data + 1);
      _handleVibrateCommmand(pwm_cmd->shape, pwm_cmd->offset, pwm_cmd->duration, pwm_cmd->amplitude, pwm_cmd->period);

      break;
  }
}

void ButtonMessageControl::sendButtonEvent(bool pressed){
  uint8_t event[sizeof(digital_command_t) + 1];
  event[0] = BUTTON;
  digital_command_t * cmd = (digital_command_t *)(event + 1);
  cmd->value = (uint8_t)pressed;
  sendMessage((uint8_t)MASTER, event, (uint8_t)(sizeof(digital_command_t) + 1));
}

void ButtonMessageControl::sendLedCommand(uint8_t toChannelID, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period){
  
}

void ButtonMessageControl::sendVibrateCommand(uint8_t toChannelID, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period){
  
}