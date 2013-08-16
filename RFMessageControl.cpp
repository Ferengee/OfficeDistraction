#include "RFMessageControl.h"
#include "Arduino.h"


RFMessageControl::RFMessageControl(BaseSenderReceiver * transceiver)
{
  m_transceiver = transceiver;
  m_lastDecrementRun = 0;
  m_lastMessageId = 0;
  m_sendingSorter.init(m_sending);
  m_receivedSorter.init(m_received);
  notifyDiscartedItem = NULL;
}

bool RFMessageControl::sendMessage(uint8_t toChannelID, uint8_t * message, uint8_t messageLength){
  MessageQueueItem * item = NULL;
  uint8_t channel = getChannel(toChannelID);
  bool succes = m_sendingSorter.getUnusedItem(&item);
  if (succes)
  {
    item->init(channel, m_lastMessageId++, message, messageLength);    
  }
  return succes;
}

bool RFMessageControl::getUnusedMessage(MessageQueueItem ** item, MessageQueueSorter * sorter){
  // should be sorter->getUnusedMessage()
  return sorter->getUnusedItem(item);
  //return findMessage(-1, -1, 0, item, queue);
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

/*
 * create a map of booleans and flag if a channel has been seen
 * send only one message per channel in each run
 */
void RFMessageControl::sendRemainingMessages(){
  MessageQueueItem * item;
  MessageQueueItem ** sortedQueue = m_sendingSorter.reorder();
  bool channelSeen[MAXMESSAGECOUNT] = {false};
  int i;
  int j;
  for (i=0; i < MAXMESSAGECOUNT; i++)
  {
    item = sortedQueue[i];
    if(!channelSeen[i]){
      /* update the rest of the channelSeen map */
      for(j=i; j < MAXMESSAGECOUNT; j++)
      {
        if(sortedQueue[j]->getChannel() == item->getChannel()){
          channelSeen[j] = true; // dont acidently set it to false if it was true already
        }
      }
      if (item->getRetriesLeft() > 0){
        item->decrementRetriesLeft();
        send(item);
      }
    }
  }
  m_sendingSorter.reorder();
}

/* sent full buffer for now
 * optimize to send only relevant data later
 */
void RFMessageControl::send(MessageQueueItem * item){
  uint8_t length = MESSAGE_BUFFER_SIZE;
  m_transceiver->send(item->getBuffer(),length);
}

void RFMessageControl::sendAcknowledge(MessageQueueItem * existing, uint8_t messageType){
  existing->transition(messageType);
  send(existing);
}

/*
 *  channel = sender|receiver
 *     8Bit =   4Bit|4Bit
 */
bool RFMessageControl::toUs(uint8_t channel)
{
  uint8_t receiver =  channel & 0xF;
  return receiver == m_ourChannelID;
}

bool RFMessageControl::fromUs(uint8_t channel)
{
  uint8_t sender = (channel >> 4) & 0xF;
  return sender == m_ourChannelID;

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
  MessageQueueItem received;
  MessageQueueItem * existing;
  uint8_t length = MESSAGE_BUFFER_SIZE;
  while (m_transceiver->get_message(buffer, &length)){
    received.init(buffer);
    uint8_t messageType = received.getMessageType(); 
    uint8_t messageId = received.getMessageId();
    uint8_t channel = received.getChannel();
    
    if(messageType == ACKNOWLEDGE || messageType == ACKNOWLEDGE_CONFIRM){
      acknowledge(&received);
    } else if(toUs(channel)){
      
      /* lookup if we already have that message */
      bool found = findMessage(channel, messageId, -1, &existing, m_received);

      if(messageType == MESSAGE){

        if(!found){
          found = m_receivedSorter.getUnusedItem(&existing);
          if(found){
            existing->init(buffer);
          }
        }
        if(found){
          sendAcknowledge(existing, ACKNOWLEDGE);
        }
      } else if (messageType == CONFIRM){
        if(found){
          if(callback != NULL && !existing->isDestroyed()){
            callback(*existing);
          }
          sendAcknowledge(existing, ACKNOWLEDGE_CONFIRM);
          existing->destroy();
        }
      }
    }
  }
  m_receivedSorter.reorder();
}


void RFMessageControl::decrementReceivedMessagesRetriesLeft()
{
  MessageQueueItem * item;
  int i;
  int retriesLeft;
  for (i=0; i < MAXMESSAGECOUNT; i++)
  {
     item = &m_received[i];
     if (item->getRetriesLeft() > 0){
       item->decrementRetriesLeft();
       retriesLeft = item->getRetriesLeft();
       if(retriesLeft == 0 && notifyDiscartedItem != NULL)
         notifyDiscartedItem(item);
     }
  }
}

void RFMessageControl::update(){
  unsigned long now = millis();
  int deltaT = now - m_lastDecrementRun;
  if (deltaT > 1000){
    decrementReceivedMessagesRetriesLeft();
    m_lastDecrementRun = now;
  }
  sendRemainingMessages();
  handleIncommingMessages();
}

uint8_t RFMessageControl::getChannelID(){
  return m_ourChannelID;
}

/*
 *  channel = sender|receiver
 *     8Bit =   4Bit|4Bit
 */
uint8_t RFMessageControl::getChannel(uint8_t toChannelID){
  return ((m_ourChannelID & 0xF) << 4) | (toChannelID & 0xF);
  
}

void RFMessageControl::setChannelID(uint8_t channelId){
  m_ourChannelID = channelId;
}

void RFMessageControl::setMessageReceivedEventHandler(MessageReceivedEventHandler eh){
  callback = eh; 
}
