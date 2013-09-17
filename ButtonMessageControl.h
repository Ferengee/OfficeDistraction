#ifndef ButtonMessageControl_h
#define ButtonMessageControl_h
#include "RFMessageControl.h"
#define MASTER 0
/*
 * enum of command types
 * array of callback functions?
 * 
 */

typedef void (* DigitalCommand) (bool value);
typedef void (* PWMCommand) (uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period);

enum { BUTTON, LED, VIBRATE };

typedef struct {
  uint8_t shape;
  uint8_t offset;
  uint8_t duration; 
  uint8_t amplitude; 
  uint8_t period;
} pwm_command_t;

typedef struct {
  uint8_t value;
} digital_command_t;

class ButtonMessageControl: public AbstractRFMessageControl
{
public:
  ButtonMessageControl(BaseSenderReceiver * transceiver) : AbstractRFMessageControl(transceiver){
    handleButtonCommand = NULL;
    handleVibrateCommand = NULL;
    handleLedCommand = NULL;
  }
  virtual void handleIncommingMessage(MessageQueueItem * item);
  bool sendButtonEvent(bool pressed);
  bool sendLedCommand(uint8_t toChannelID, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period);
  bool sendVibrateCommand(uint8_t toChannelID, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period);
  
  PWMCommand handleLedCommand;
  PWMCommand handleVibrateCommand;
  DigitalCommand handleButtonCommand;
private:
  bool sendCommand(uint8_t toChannelID, uint8_t target, uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period);

};

#endif