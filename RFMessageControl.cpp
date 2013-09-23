#include "RFMessageControl.h"
#include "Arduino.h"

/*
 * TODO: 
 * - remove message type
 * - rewrite message/acknowledge to request/reply
 * - delegate responsibility of reply to handleIncommingMessage
 * - make it obvious that requests should be replied
 * 
 */

AbstractRFMessageControl::AbstractRFMessageControl(BaseSenderReceiver * transceiver)
{
  m_transceiver = transceiver;
  m_lastDecrementRun = 0;
  m_lastMessageId = 0;
  m_sendingSorter.init(m_sending);
  m_receivedSorter.init(m_received);
  notifyDiscartedItem = NULL;
}

bool AbstractRFMessageControl::sendMessage(uint8_t toChannelID, uint8_t * message, uint8_t messageLength){
  MessageQueueItem * item = NULL;
  uint8_t channel = getChannel(toChannelID);
  bool succes = m_sendingSorter.getUnusedItem(&item);
  if (succes)
  {
    item->init(channel, m_lastMessageId++, message, messageLength);    
  }
  return succes;
}

void AbstractRFMessageControl::acknowledge(MessageQueueItem * acknowledgement){
  MessageQueueItem * item = NULL;
  if(findMessage(acknowledgement->getChannel(), acknowledgement->getMessageId(), -1, &item, m_sending)){
    item->destroy();
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

bool AbstractRFMessageControl::findMessage(int channel, int messageId, int retriesLeft, MessageQueueItem ** item, MessageQueueItem * queue)
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
void AbstractRFMessageControl::sendRemainingMessages(){
  unsigned long int now = millis();
  unsigned long int delta = now - m_lastSendAt;
  if(delta < MIN_SEND_TIMEOUT)
    return;
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
	m_lastSendAt = now;
	break;
      }
    }
  }
  m_sendingSorter.reorder();
}

/* sent full buffer for now
 * optimize to send only relevant data later
 */
bool AbstractRFMessageControl::send(MessageQueueItem * item){
  uint8_t length = sizeof(message_data_t);
  return m_transceiver->send(item->getBuffer(),length);
}

void AbstractRFMessageControl::sendAcknowledge(MessageQueueItem * existing){
  //existing->transition(messageType);
  send(existing);
}

/*
 *  channel = sender|receiver
 *     8Bit =   4Bit|4Bit
 */
bool AbstractRFMessageControl::isRequest(uint8_t channel)
{
  uint8_t receiver =  channel & 0xF;
  return receiver == m_ourChannelID;
}

bool AbstractRFMessageControl::isReply(uint8_t channel)
{
  uint8_t sender = (channel >> 4) & 0xF;
  return sender == m_ourChannelID;

}

void AbstractRFMessageControl::handleIncommingMessages(){

  uint8_t buffer[sizeof(message_data_t)] = {0};

  MessageQueueItem received = MessageQueueItem();
  MessageQueueItem * existing = NULL;
  uint8_t length = sizeof(message_data_t);
  while (m_transceiver->get_message(buffer, &length)){
    received.init(buffer, length);
    uint8_t messageId = received.getMessageId();
    uint8_t channel = received.getChannel();
    
    if(isReply(channel)){
      acknowledge(&received);
      // received can contain a command that needs to be handled
      handleIncommingReply(&received);
      
    } else if(isRequest(channel)){
      
      /* lookup if we already have that message */
      bool found = findMessage(channel, messageId, -1, &existing, m_received);

      if(!found){
        found = m_receivedSorter.getUnusedItem(&existing);
        if(found){
          existing->init(buffer, length);
          existing->setRetriesLeft(MAXRETRIES + 1);
          handleIncommingMessage(existing);
        }
      }
      if(found){
        sendAcknowledge(existing);
      }
    
    }
  }
  m_receivedSorter.reorder();
}

void AbstractRFMessageControl::handleIncommingMessage(MessageQueueItem* received){}
void AbstractRFMessageControl::handleIncommingReply(MessageQueueItem* received){}

void AbstractRFMessageControl::decrementReceivedMessagesRetriesLeft()
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
       // not relevant anymore
       // all items are discarted
       // what we want to know is how may resends we did
       if(retriesLeft == 0 && notifyDiscartedItem != NULL)
         notifyDiscartedItem(item);
     }
  }
}

void AbstractRFMessageControl::update(){
  unsigned long now = millis();
  if (now - m_lastDecrementRun > MIN_SEND_TIMEOUT){
    decrementReceivedMessagesRetriesLeft();
    m_lastDecrementRun = now;
  }
  sendRemainingMessages();
  handleIncommingMessages();
}

uint8_t AbstractRFMessageControl::getChannelID(){
  return m_ourChannelID;
}

/*
 *  channel = sender|receiver
 *     8Bit =   4Bit|4Bit
 */
uint8_t AbstractRFMessageControl::getChannel(uint8_t toChannelID){
  return ((m_ourChannelID & 0xF) << 4) | (toChannelID & 0xF);
  
}

void AbstractRFMessageControl::setChannelID(uint8_t channelId){
  m_ourChannelID = channelId;
}

void RFMessageControl::setMessageReceivedEventHandler(MessageReceivedEventHandler eh){
  callback = eh; 
}

void RFMessageControl::handleIncommingMessage(MessageQueueItem * item){
  if(callback != NULL && !item->isDestroyed()){
    callback(*item);
  }
}

