#include "MessageQueueItem.h"

MessageQueueItem::MessageQueueItem()
{
 
}


void MessageQueueItem::setChannel(uint8_t channel){
   m_messageData.head.channel = channel;
}
uint8_t MessageQueueItem::getChannel(){
  return m_messageData.head.channel;
}
void MessageQueueItem::setMessageId(uint8_t messageId){
  m_messageData.head.id = messageId;
}
uint8_t MessageQueueItem::getMessageId(){
  return  m_messageData.head.id;
}

void MessageQueueItem::setRetriesLeft(uint8_t retries){
   m_messageData.head.retries = retries;
}
uint8_t MessageQueueItem::getRetriesLeft(){
  return m_messageData.head.retries;
}

uint8_t * MessageQueueItem::getBuffer(){
  return (uint8_t *)&m_messageData;
}

uint8_t * MessageQueueItem::getData(){
  return (uint8_t *)&(m_messageData.data);
}

void MessageQueueItem::getMessage(uint8_t * message, uint8_t * length){
  uint8_t toCopy = min(m_messageData.head.length, *length);

  memcpy(message, m_messageData.data, toCopy);
  *length = toCopy;
  
}


uint8_t MessageQueueItem::decrementRetriesLeft()
{
   uint8_t retriesLeft = m_messageData.head.retries;
   retriesLeft--;
   /* don't overflow */
   if (m_messageData.head.retries > retriesLeft){
     m_messageData.head.retries = retriesLeft; 
   }
   return m_messageData.head.retries;
}
void MessageQueueItem::init(uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength)
{
  memset(&m_messageData,0,sizeof(m_messageData));
  setChannel(channel);
  setMessageId(messageId);
  setRetriesLeft(MAXRETRIES);
  setData(message,  messageLength);
}

void MessageQueueItem::setData( uint8_t * message, uint8_t messageLength)
{
  m_messageData.head.length =  min(sizeof(m_messageData.data), messageLength);
  memcpy(m_messageData.data, message, m_messageData.head.length);  
}

void MessageQueueItem::init(uint8_t * messageData)
{
  memset(&m_messageData,0,sizeof(message_data_t));
  memcpy(&m_messageData , messageData, sizeof(message_data_t)); 
}

uint8_t MessageQueueItem::getLength()
{
  return m_messageData.head.length;
}


void MessageQueueItem::destroy()
{
  setRetriesLeft(0);
}

bool MessageQueueItem::isDestroyed()
{
  return getRetriesLeft() < 1;
}

