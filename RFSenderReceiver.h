#ifndef RFSenderReceiver_h
#define RFSenderReceiver_h
#include "Arduino.h"
#include <VirtualWire.h>
#include "BaseSenderReceiver.h"

class RFSenderReceiver : public BaseSenderReceiver
{
public:
  RFSenderReceiver();
  virtual bool send(uint8_t * buf, uint8_t len);
  virtual bool have_message();
  virtual bool get_message(uint8_t * buf, uint8_t * len);     
  virtual void init(int txPin, int rxPin, int speed);
  virtual bool is_receiving();
  virtual bool is_transmitting();
};

#endif