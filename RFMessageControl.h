#ifndef RFMessageControl_h
#define RFMessageControl_h
#include "Arduino.h"

#include "../VirtualWire/VirtualWire.h"
#define MAXMESSAGECOUNT 16
#define MAXRETRIES 3

typedef void (* MessageReceivedEventHandler) (int channel, uint8_t * message);

class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (int channel, uint8_t * message, int messageId);
  uint8_t * getMessage();
  int getChannel();
  int getRetriesLeft();
  void decrementRetriesLeft();
  
private:
  int m_retriesLeft;
  int m_channel;
  uint8_t m_messageBuffer[VW_MAX_PAYLOAD - 3*sizeof(int)]; // about 21 bytes
  int m_messageId;
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
  bool sendMessage(int channel, uint8_t * message, int messageLength);
  /* m_retriesLeft is set to 0 */
  void acknowledge(int messageId);
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
  int m_lastMessageId;
  MessageReceivedEventHandler callback;
};



#endif