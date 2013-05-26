#include "MessageQueueItem.h"

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
uint8_t MessageQueueItem::getChannel(){
  return m_messageBuffer[1];
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
uint8_t MessageQueueItem::getRetriesLeft(){
  return m_messageBuffer[3];
}

uint8_t * MessageQueueItem::getBuffer(){
  return m_messageBuffer;
}

void MessageQueueItem::getMessage(uint8_t * message, uint8_t * length){
  uint8_t toCopy = min(MESSAGE_SIZE, *length);

  memcpy(message, m_messageBuffer + MESSAGE_HEADER_LENGTH, toCopy);
  *length = toCopy;
  
}


void MessageQueueItem::decrementRetriesLeft()
{
   uint8_t retriesLeft = m_messageBuffer[3];
   retriesLeft--;
   /* don't overflow */
   if (m_messageBuffer[3] > retriesLeft){
     m_messageBuffer[3] = retriesLeft; 
   }
}
void MessageQueueItem::init(uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength)
{
  memset(m_messageBuffer,0,sizeof(m_messageBuffer));
  setMessageType(MESSAGE);
  setChannel(channel);
  setMessageId(messageId);
  setRetriesLeft(MAXRETRIES);
  
  memcpy(m_messageBuffer + MESSAGE_HEADER_LENGTH, message, min(MESSAGE_SIZE, messageLength)); 
}

void MessageQueueItem::init(uint8_t * messageBuffer)
{
  memcpy(m_messageBuffer , messageBuffer, MESSAGE_BUFFER_SIZE); 
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

void MessageQueueItem::transition(uint8_t acknowledgementType){
  setMessageType(acknowledgementType);
  setRetriesLeft(1);
}

