#include "RFMessageControl.h"
#include "Arduino.h"

MessageQueueItem::MessageQueueItem()
{
}


void MessageQueueItem::setMessageType(uint8_t messageType){
  m_messageBuffer[0] = messageType;
}

uint8_t MessageQueueItem::getMessageType(){
  return m_messageBuffer[0];
}

void MessageQueueItem::setChannel(uint8_t channel){
  m_messageBuffer[1] = channel;
}
void MessageQueueItem::setMessageId(uint8_t messageId){
  m_messageBuffer[2] = messageId;
}
uint8_t MessageQueueItem::getMessageId(){
  return m_messageBuffer[2];
}

void MessageQueueItem::setRetriesLeft(uint8_t retries){
  m_messageBuffer[3] = retries;
}

uint8_t * MessageQueueItem::getBuffer(){
  return m_messageBuffer;
}

void MessageQueueItem::getMessage(uint8_t * message, uint8_t * length){
  memcpy(message, m_messageBuffer + MESSAGE_HEADER_LENGTH,min(MESSAGE_BUFFER_SIZE, *length));
}


void MessageQueueItem::decrementRetriesLeft()
{
   m_messageBuffer[3]--;
}
void MessageQueueItem::init(uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength)
{
  setMessageType(MESSAGE);
  setChannel(channel);
  setMessageId(messageId);
  setRetriesLeft(MAXRETRIES);
  
  int i;
  for (i=MESSAGE_HEADER_LENGTH; i < MESSAGE_BUFFER_SIZE + MESSAGE_HEADER_LENGTH; i++)
  {
     m_messageBuffer[i] = 0;
  }
  
  memcpy(m_messageBuffer + MESSAGE_HEADER_LENGTH, message, min(MESSAGE_BUFFER_SIZE, messageLength)); 
}

void MessageQueueItem::destroy()
{
  setRetriesLeft(0);
}

void MessageQueueItem::acknowledge(uint8_t acknowledgementType)
{
  if(getMessageType() == MESSAGE && acknowledgementType == ACKNOWLEDGE){
    setMessageType(CONFIRM);
    setRetriesLeft(MAXRETRIES);
  } else if(getMessageType() == CONFIRM && acknowledgementType == ACKNOWLEDGE_CONFIRM) {
    destroy();
  }
}

RFMessageControl::RFMessageControl(int txPin, int rxPin, int speed)
{
  vw_set_tx_pin ((uint8_t)txPin);
  vw_set_rx_pin ((uint8_t)rxPin);
  vw_set_ptt_inverted(true);
  vw_setup ((uint16_t)speed);
  vw_rx_start();   
}

bool RFMessageControl::sendMessage(uint8_t channel, uint8_t * message, uint8_t messageLength){
  MessageQueueItem * item;
  bool succes = getUnusedMessage(item, m_sending);
  if (succes)
  {
    item->init(channel, m_lastMessageId++, message, messageLength);
  }
  return succes;
}

bool RFMessageControl::getUnusedMessage(MessageQueueItem * item, MessageQueueItem * queue){
  findMessage(-1, -1, 0, item, queue);
}

void RFMessageControl::acknowledge(MessageQueueItem * acknowledgement){
  MessageQueueItem * item;
  if(findMessage(acknowledgement->getChannel(), acknowledgement->getMessageId(), -1, item, m_sending)){
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

bool RFMessageControl::findMessage(int channel, int messageId, int retriesLeft, MessageQueueItem * item, MessageQueueItem * queue)
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
      item = candidate;
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
void send(MessageQueueItem * item){
  uint8_t length = VW_MAX_PAYLOAD;
  vw_send(item->getBuffer(),length);
}
void RFMessageControl::handleIncommingMessages(){}
void RFMessageControl::update(){
  sendRemainingMessages();
  handleIncommingMessages();
}
