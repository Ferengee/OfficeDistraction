#ifndef RFMessageControl_h
#define RFMessageControl_h
#include "Arduino.h"

#include "../VirtualWire/VirtualWire.h"
#define MAXMESSAGECOUNT 16
#define MAXRETRIES 3

#define MESSAGE_BUFFER_SIZE VW_MAX_PAYLOAD-4

#define MESSAGE 0
#define ACKNOWLEDGE 1
#define CONFIRM 2

typedef void (* MessageReceivedEventHandler) (uint8_t channel, uint8_t * message, uint8_t messageLength);

class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength);
  uint8_t * getMessage();
  uint8_t getChannel();
  uint8_t getRetriesLeft();
  void decrementRetriesLeft();
  
private:
  
  uint8_t m_messageType;
  uint8_t m_channel;
  uint8_t m_messageId;
  
  uint8_t m_retriesLeft;

  uint8_t m_messageBuffer[MESSAGE_BUFFER_SIZE]; // about 30 - (3 + 4) = 23 bytes
};

class RFMessageControl
{
public:
  RFMessageControl();
  /* 
   * Copies the contents of the message
   * writes max sizeof m_messageBuffer bytes
   * returns false if the queue is full 
   */
  bool sendMessage(uint8_t channel, uint8_t * message, uint8_t messageLength);
  /* m_retriesLeft is set to 0 */
  void acknowledge(uint8_t messageId);
  /* 
   * iterate queue, 
   * send all messages with  m_retriesLeft > 0
   */
  void sendRemainingMessages();
  void setMessageReceivedEventHandler(MessageReceivedEventHandler eh);

private:
  MessageQueueItem m_queue[MAXMESSAGECOUNT];
  /*
   * Encode the MessageQueueItem over the wire
   */
  void send(MessageQueueItem * item);
  /*
   * find the next item with m_retriesLeft == 0
   * init to MAXRETRIES
   * to be used in sendMessage
   */
  MessageQueueItem * getUnusedMessage();
  uint8_t m_lastMessageId;
  MessageReceivedEventHandler callback;
};



#endif