#ifndef RFMessageControl_h
#define RFMessageControl_h

#define MAXMESSAGECOUNT 16
#define MAXRETRIES 3

typedef void (* MessageReceivedEventHandler) (int channel, char * message);


class MessageQueueItem
{
public:
  MessageQueueItem();
  void init (int channel, char * message, int messageId);
  char * getMessage();
  int getChannel();
  int getRetriesLeft();
  void decrementRetriesLeft();
  
private:
  int m_retriesLeft;
  int m_channel;
  char * m_message;MessageReceivedHandler
  int m_messageId;
};

class RFMessageControl;
{
public:
  RFMessageControl(RFReceiver receiver, RFTransmitter transmitter);
  /* returns false if the queue is full */
  bool sendMessage(int channel, char * message);
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
  void send(MessageQueueItem item);
  /*
   * find the next item with m_retriesLeft == 0
   * init to MAXRETRIES
   */
  bool addMessage(int channel, char * message);
  int m_lastMessageId;
  MessageReceivedEventHandler callback;
};



#endif