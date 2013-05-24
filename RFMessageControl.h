#ifndef RFMessageControl_h
#define RFMessageControl_h
#include "Arduino.h"

#include "../VirtualWire/VirtualWire.h"
#define MAXMESSAGECOUNT 16
#define MAXRETRIES 3
#define MESSAGE_HEADER_LENGTH 4
#define MESSAGE_BUFFER_SIZE VW_MAX_PAYLOAD-MESSAGE_HEADER_LENGTH

#define MESSAGE 0
#define ACKNOWLEDGE 1
#define CONFIRM 2
#define ACKNOWLEDGE_CONFIRM 3

typedef void (* MessageReceivedEventHandler) (uint8_t channel, uint8_t * message, uint8_t messageLength);


class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength);
  void destroy();
  
  void getMessage(uint8_t * message, uint8_t * length);
  
  uint8_t * getBuffer();
  uint8_t getMessageId();

  uint8_t getChannel();
  uint8_t getRetriesLeft();
  
  uint8_t getMessageType();
  
  void decrementRetriesLeft();
  void acknowledge(uint8_t acknowledgementType);
  
private:  
  void setMessageType(uint8_t messageType);
  

  void setChannel(uint8_t channel);
  void setMessageId(uint8_t messageId);
  
  void setRetriesLeft(uint8_t retries);

  uint8_t m_messageBuffer[VW_MAX_PAYLOAD]; // about 30 - (3 ) = 27 bytes
};

typedef bool (* MessageParameterEquals) (MessageQueueItem * item, uint8_t value);

/*
 * send messages via sendMessage()
 * receive messages via the MessageReceivedEventHandler callback
 * trigger processing of messages in the sending queue via update()
 * which has to be called or periodically, with out it no messages get sent, sendMessages only
 * puts messages in the queue
 */
class RFMessageControl
{
public:
  RFMessageControl(int txPin, int rxPin, int speed);
  /* 
   * Copies the contents of the message
   * writes max sizeof m_messageBuffer bytes
   * returns false if the queue is full 
   * 
   */
  bool sendMessage(uint8_t channel, uint8_t * message, uint8_t messageLength);
  void acknowledge(MessageQueueItem * acknowledgement);
  /* 
   * iterate queue, 
   * send all messages with  m_retriesLeft > 0
   */
  void sendRemainingMessages();
  void handleIncommingMessages();
  /*
   * decrementReceivedRetriesLeft if needed
   * sendRemainingMessages()
   * handleIncommingMessages()
   */
  void update();
  void setMessageReceivedEventHandler(MessageReceivedEventHandler eh);

private:
  MessageQueueItem m_sending[MAXMESSAGECOUNT];
  MessageQueueItem m_received[MAXMESSAGECOUNT];
  /*
   * Encode the MessageQueueItem over the wire
   */
  void send(MessageQueueItem * item);
  /*
   * find the next item with m_retriesLeft == 0
   * init to MAXRETRIES
   * to be used in sendMessage
   */
  bool getUnusedMessage(MessageQueueItem * item,  MessageQueueItem * queue);
  /* 
   * find the message which corresponds with the given parameters
   * If more than one message matches the parameters, the first one is returned
   * if a parameter is set to -1, this parameter is ignored
   */
  bool findMessage(int channel, int messageId, int retriesLeft, MessageQueueItem * item, MessageQueueItem * queue);
  uint8_t m_lastMessageId;
  MessageReceivedEventHandler callback;
};

class SendTester
{
public:
  SendTester();
  bool send(uint8_t * buf, uint8_t len);
  bool have_message();
  bool get_message(uint8_t * buf, uint8_t * len);     
private:
  MessageQueueItem m_buffer[MAXMESSAGECOUNT];
  int m_start;
  int m_end;
};

#endif