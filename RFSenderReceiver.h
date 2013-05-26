#ifndef RFSenderReceiver_h
#define RFSenderReceiver_h
#include "Arduino.h"
#include "../VirtualWire/VirtualWire.h"
#include "BaseSenderReceiver.h"

class RFSenderReceiver
{
public:
  RFSenderReceiver(int txPin, int rxPin, int speed);
  virtual bool send(uint8_t * buf, uint8_t len);
  virtual bool have_message();
  virtual bool get_message(uint8_t * buf, uint8_t * len);     
  
};

#endif