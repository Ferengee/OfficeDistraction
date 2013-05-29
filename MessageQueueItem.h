#ifndef MessageQueueItem_h
#define MessageQueueItem_h
#include "Arduino.h"
#include "OfficeDistractionConfig.h"

class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength);
  void init(uint8_t * messageBuffer);

  void destroy();
  bool isDestroyed();
  
  void getMessage(uint8_t * message, uint8_t * length);
  
  uint8_t * getBuffer();
  uint8_t getMessageId();

  uint8_t getChannel();
  uint8_t getRetriesLeft();
  
  uint8_t getMessageType();
  
  void decrementRetriesLeft();
  void acknowledge(uint8_t acknowledgementType);
  void transition(uint8_t acknowledgementType);

private:  
  void setMessageType(uint8_t messageType);
  

  void setChannel(uint8_t channel);
  void setMessageId(uint8_t messageId);
  
  void setRetriesLeft(uint8_t retries);

  uint8_t m_messageBuffer[MESSAGE_BUFFER_SIZE]; // about 30 - (3 ) = 27 bytes
};


#endif