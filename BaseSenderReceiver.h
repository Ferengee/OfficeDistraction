#ifndef BaseSenderReceiver_h
#define BaseSenderReceiver_h

#include "Arduino.h"
#include "MessageQueueItem.h"

class BaseSenderReceiver
{
public:
  BaseSenderReceiver(){};
  virtual bool send(uint8_t * buf, uint8_t len){ return false;}
  virtual bool have_message(){ return false;}
  virtual bool get_message(uint8_t * buf, uint8_t * len){ return false;} 
};

#endif