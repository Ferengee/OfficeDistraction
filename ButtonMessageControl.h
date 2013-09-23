#ifndef ButtonMessageControl_h
#define ButtonMessageControl_h
#include "RFMessageControl.h"
#define MASTER 0
/*
 * enum of command types
 * array of callback functions?
 * 
 */
enum { ACKNOWLEDGE, BUTTON, LED, VIBRATE };

typedef struct pwm_command_s {
  uint8_t shape;
  uint8_t offset;
  uint8_t duration; 
  uint8_t amplitude; 
  uint8_t period;
} pwm_command_t;

typedef struct {
  uint8_t value;
} digital_command_t;

typedef struct {
  uint8_t label;
  union {
    pwm_command_t pwm;
    digital_command_t digital;
  };
} od_command_header_t;

/*
 * keep reference and metadata for command handlers
 * allows for the handler to reply on the message
 * but will send and ACKNOWLEDGE command if the handler does not wish to reply
 * 
 */
class CommandContext
{
public:
  CommandContext(AbstractRFMessageControl * control) { _replied = false; _control = control;}
  /*
   * Allow for the handler to reply immediately
   */
  void reply(od_command_header_t * command);
  /*
   * to be called from within the control to set the correct MessageQueueItem
   */
  void setMessageQueueItem(MessageQueueItem * item){_item = item;}
  /*
   * control finalizes the MessageQueueItem, updates the command if needed to ACKNOWLEDGE
   */
  void finalizeMessage();
  AbstractRFMessageControl * control();

private:
  bool _replied;
  AbstractRFMessageControl * _control;
  MessageQueueItem * _item;
};

typedef void (* DigitalCommand) (CommandContext * context, digital_command_t cmd);
typedef void (* PWMCommand) (CommandContext * context, pwm_command_t cmd);

class ButtonMessageControl: public AbstractRFMessageControl
{
public:
  ButtonMessageControl(BaseSenderReceiver * transceiver) : AbstractRFMessageControl(transceiver){
    handleButtonCommand = NULL;
    handleVibrateCommand = NULL;
    handleLedCommand = NULL;
  }
  virtual void handleIncommingMessage(MessageQueueItem * item);
  virtual void handleIncommingReply(MessageQueueItem * item);

  bool sendButtonEvent(bool pressed);
  bool sendLedCommand(uint8_t toChannelID, pwm_command_t pwm);
  bool sendLedCommand(uint8_t toChannelID, uint8_t shape,uint8_t offset,uint8_t duration,uint8_t amplitude,uint8_t period);
  bool sendVibrateCommand(uint8_t toChannelID, pwm_command_t pwm);
  bool sendVibrateCommand(uint8_t toChannelID, uint8_t shape,uint8_t offset,uint8_t duration,uint8_t amplitude,uint8_t period);
  
  PWMCommand handleLedCommand;
  PWMCommand handleVibrateCommand;
  DigitalCommand handleButtonCommand;
private:

};


#endif