#ifndef RFMessageControl_h
#define RFMessageControl_h
#include "Arduino.h"
#include "OfficeDistractionConfig.h"
#include "MessageQueueItem.h"
#include "MessageQueueSorter.h"

#include "BaseSenderReceiver.h"
#define MIN_SEND_TIMEOUT 250

typedef void (* MessageReceivedEventHandler) (MessageQueueItem item);

typedef bool (* MessageParameterEquals) (MessageQueueItem * item, uint8_t value);

typedef void (* NotifyDiscartedItem)(MessageQueueItem* item);



/*
 * send messages via sendMessage()
 * receive messages via the MessageReceivedEventHandler callback
 * trigger processing of messages in the sending queue via update()
 * which has to be called or periodically, with out it no messages get sent, sendMessages only
 * puts messages in the queue
 */


class AbstractRFMessageControl
{
public:
  AbstractRFMessageControl(BaseSenderReceiver * transceiver);

  /* 
   * Copies the contents of the message
   * writes max sizeof m_messageBuffer bytes
   * returns false if the queue is full 
   * 
   */
  bool sendMessage(uint8_t toChannelID, uint8_t * message, uint8_t messageLength);
  void acknowledge(MessageQueueItem * acknowledgement);
  void sendAcknowledge(MessageQueueItem * existing, uint8_t messageType);

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
  
  uint8_t getChannelID();
  /*
   * channel = (sender|receiver) for MESSAGE and CONFIRM types of messages
   * channel = (receiver|sender) for ACKNOWLEDGE types of messages
   */
  uint8_t getChannel(uint8_t toChannelID);
  void setChannelID(uint8_t channelID);

  NotifyDiscartedItem notifyDiscartedItem;
protected:
  virtual void handleIncommingMessage(MessageQueueItem* existing);

private:
  MessageQueueItem m_sending[MAXMESSAGECOUNT];
  MessageQueueItem m_received[MAXMESSAGECOUNT];
 
  MessageQueueSorter m_sendingSorter;
  MessageQueueSorter m_receivedSorter;


  /*
   * Encode the MessageQueueItem over the wire
   */
  void send(MessageQueueItem * item);
  /*
   * find the next item with m_retriesLeft == 0
   * init to MAXRETRIES
   * to be used in sendMessage
   */
  bool getUnusedMessage(MessageQueueItem ** item, MessageQueueSorter * sorter);
  /* 
   * find the message which corresponds with the given parameters
   * If more than one message matches the parameters, the first one is returned
   * if a parameter is set to -1, this parameter is ignored
   */
  bool findMessage(int channel, int messageId, int retriesLeft, MessageQueueItem ** item, MessageQueueItem * queue);
  /* returns true for a MessageQueueItem item.getChannel() where item type is MESSAGE or CONFIRM
   * which is send to us
   * 
   * for the acknowledge type of MessageQueueItems the channel is the same from the message they acknowledge
   * so from their perspective sender and receiver are in reverse order. 
   */
  bool toUs(uint8_t channel);
  /*
   * returns true if we send the original message
   * so it also returns true on a received acknowledgement which was send in reply to one of our messages.
   */
  bool fromUs(uint8_t channel);

  uint8_t m_lastMessageId;
  unsigned long m_lastDecrementRun;
  
  uint8_t m_ourChannelID;
  BaseSenderReceiver * m_transceiver;
  long unsigned int m_lastSendAt;
};

class RFMessageControl: public AbstractRFMessageControl
{
public:
  RFMessageControl(BaseSenderReceiver * transceiver) : AbstractRFMessageControl(transceiver){}
  virtual void handleIncommingMessage(MessageQueueItem * item);
  void setMessageReceivedEventHandler(MessageReceivedEventHandler eh);

private:
    MessageReceivedEventHandler callback;

};

#endif