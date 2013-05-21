#ifndef RFMessageControl_h
#define RFMessageControl_h
#include "Arduino.h"

#include "../VirtualWire/VirtualWire.h"
#define MAXMESSAGECOUNT 16
#define MAXRETRIES 3

#define MESSAGE 0
#define ACKNOWLEDGE 1
#define CONFIRM 2

typedef void (* MessageReceivedEventHandler) (byte channel, uint8_t * message);

class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (byte channel, uint8_t * message, byte messageId);
  uint8_t * getMessage();
  byte getChannel();
  byte getRetriesLeft();
  void decrementRetriesLeft();
  
private:
  
  byte m_messageType;
  byte m_channel;
  byte m_messageId;
  
  byte m_retriesLeft;

  uint8_t m_messageBuffer[VW_MAX_PAYLOAD - 4*sizeof(byte)]; // about 30 - (3 + 4) = 23 bytes
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
  bool sendMessage(byte channel, uint8_t * message, byte messageLength);
  /* m_retriesLeft is set to 0 */
  void acknowledge(byte messageId);
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
  byte m_lastMessageId;
  MessageReceivedEventHandler callback;
};



#endif