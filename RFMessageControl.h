#ifndef RFMessageControl_h
#define RFMessageControl_h
#include "Arduino.h"
#include "OfficeDistractionConfig.h"
#include "MessageQueueItem.h"
#include "BaseSenderReceiver.h"

typedef void (* MessageReceivedEventHandler) (uint8_t channel, uint8_t * message, uint8_t messageLength);

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
  RFMessageControl(BaseSenderReceiver * transceiver);

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
  void decrementReceivedMessagesRetriesLeft();
  
  void setMessageReceivedEventHandler(MessageReceivedEventHandler eh);
  uint8_t getChannel();
  void setChannel(uint8_t channel);
  

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
  bool getUnusedMessage(MessageQueueItem ** item,  MessageQueueItem * queue);
  /* 
   * find the message which corresponds with the given parameters
   * If more than one message matches the parameters, the first one is returned
   * if a parameter is set to -1, this parameter is ignored
   */
  bool findMessage(int channel, int messageId, int retriesLeft, MessageQueueItem ** item, MessageQueueItem * queue);
  uint8_t m_lastMessageId;
  unsigned long m_lastDecrementRun;
  MessageReceivedEventHandler callback;
  
  uint8_t m_ourChannel;
  BaseSenderReceiver * m_transceiver;
};

#endif