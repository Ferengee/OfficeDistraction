#include "RFMessageControl.h"
#include <sys/types.h>
#include "Arduino.h"


RFMessageControl::RFMessageControl(BaseSenderReceiver * transceiver)
{
  m_transceiver = transceiver;
  m_lastDecrementRun = 0;
  m_lastMessageId = 0;
}

bool RFMessageControl::sendMessage(uint8_t channel, uint8_t * message, uint8_t messageLength){
  MessageQueueItem * item = NULL;
  Serial.println("RFMessageControl::sendMessage");
  bool succes = getUnusedMessage(&item, m_sending);
  if (succes)
  {
    item->init(channel, m_lastMessageId++, message, messageLength);    
  }
  return succes;
}

bool RFMessageControl::getUnusedMessage(MessageQueueItem ** item, MessageQueueItem * queue){
  return findMessage(-1, -1, 0, item, queue);
}

void RFMessageControl::acknowledge(MessageQueueItem * acknowledgement){
  MessageQueueItem * item = NULL;
  if(findMessage(acknowledgement->getChannel(), acknowledgement->getMessageId(), -1, &item, m_sending)){
    // only acknowledge if type == MESSAGE and other.type == ACKNOWLEDGE
    //                  or type == CONFIRM and other.type == ACKNOWLEDGE_CONFIRM
    item->acknowledge(acknowledgement->getMessageType());
  }
}

bool MessageChannelEquals(MessageQueueItem * item, uint8_t channel)
{
  return item->getChannel() == channel;
}

bool MessageMessageIdEquals(MessageQueueItem * item, uint8_t messageId)
{
  return item->getMessageId() == messageId;
}

bool MessageRetriesLeftEquals(MessageQueueItem * item, uint8_t retriesLeft)
{
  return item->getRetriesLeft() == retriesLeft;
}

bool RFMessageControl::findMessage(int channel, int messageId, int retriesLeft, MessageQueueItem ** item, MessageQueueItem * queue)
{
  MessageParameterEquals testFunctions[3];
  uint8_t values[3];
  int testCount = 0;
  
  if(channel != -1){
    values[testCount] = (uint8_t)channel;
    testFunctions[testCount] = MessageChannelEquals;
    testCount++;
  }
  if(messageId != -1){
    values[testCount] = (uint8_t)messageId;
    testFunctions[testCount] = MessageMessageIdEquals;
    testCount++;
  }
  if(retriesLeft != -1){
    values[testCount] = (uint8_t)retriesLeft;
    testFunctions[testCount] = MessageRetriesLeftEquals;
    testCount++;
  }
    
  MessageQueueItem * candidate;
  int i, f;
  bool found;
  
  for (i=0; i < MAXMESSAGECOUNT; i++)
  {
    printf("searching queue ");
    printf("index: %d ", i);
    candidate = &queue[i];
    found = true;
    for(f=0; f < testCount; f++){
      found = testFunctions[f](candidate, values[f]);
      if (!found){
        break;
      }
    }
    if(found){
      *item = candidate;
      break; 
    }
  }
  return found;
}

void RFMessageControl::sendRemainingMessages(){
  MessageQueueItem * item;
  int i;
  for (i=0; i < MAXMESSAGECOUNT; i++)
  {
     item = &m_sending[i];
     if (item->getRetriesLeft() > 0){
       item->decrementRetriesLeft();
       send(item);
     }
  }
}
/* sent full buffer for now
 * optimize to send only relevant data later
 */
void RFMessageControl::send(MessageQueueItem * item){
  uint8_t length = MESSAGE_BUFFER_SIZE;
  m_transceiver->send(item->getBuffer(),length);
}

void RFMessageControl::handleIncommingMessages(){
  /*
   * has_new_message = vw_get_message (buf, len)
   * convert buffer to MessageQueueItem
   * check message type
   * if type == ACKNOWLEDGE || type == ACKNOWLEDGE_CONFIRM => acknowledge(item)
   * if type == MESSAGE || type == CONFIRM =>
   *  check message channel == ours // still have to implement setting our channel
   *  if type == MESSAGE => remember message, send ACKNOWLEDGE
   *  if type == CONFIRM => forget message, send ACKNOWLEDGE_CONFIRM, call callback function            
   */
    uint8_t buffer[MESSAGE_BUFFER_SIZE];
    MessageQueueItem * item;
    uint8_t length = MESSAGE_BUFFER_SIZE;
    while (m_transceiver->get_message(buffer, &length)){
      
    }
    
  
}

void RFMessageControl::decrementReceivedMessagesRetriesLeft()
{
  MessageQueueItem * item;
  int i;
  for (i=0; i < MAXMESSAGECOUNT; i++)
  {
     item = &m_received[i];
     if (item->getRetriesLeft() > 0){
       item->decrementRetriesLeft();
     }
  }
}

void RFMessageControl::update(){
  unsigned long now = millis();
  int deltaT = now - m_lastDecrementRun;
  if (deltaT > 1000){
    decrementReceivedMessagesRetriesLeft();
  }
  sendRemainingMessages();
  handleIncommingMessages();
}

uint8_t RFMessageControl::getChannel(){
  return m_ourChannel;
}
void RFMessageControl::setChannel(uint8_t channel){
  m_ourChannel = channel;
}
  
