#ifndef MessageQueueItem_h
#define MessageQueueItem_h
#include "Arduino.h"
#include "OfficeDistractionConfig.h"


typedef struct message_header_s {
  message_header_s() : type(0),id(0), channel(0), retries(0), length(0){}
  uint8_t type;
  uint8_t id;
  uint8_t channel;
  uint8_t retries;
  uint8_t length;
} message_header_t;
  
typedef struct {
  message_header_t head;
  uint8_t data[MESSAGE_SIZE];
} message_data_t;

class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength);
  void init(uint8_t* messageBuffer, uint8_t length);

  void destroy();
  bool isDestroyed();
  
  void getMessage(uint8_t * message, uint8_t * length);
  
  uint8_t * getBuffer();
  uint8_t getMessageId();

  uint8_t getChannel();
  uint8_t getRetriesLeft();
  void setRetriesLeft(uint8_t retries);

  uint8_t getMessageType();
  
  void decrementRetriesLeft();
  void transition(uint8_t acknowledgementType);
  uint8_t getLength();
  message_data_t m_messageData;
private:  
  void setMessageType(uint8_t messageType);
  

  void setChannel(uint8_t channel);
  void setMessageId(uint8_t messageId);
  
  
};


#endif