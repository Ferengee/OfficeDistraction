#include "RFMessageControl.h"
#include "Arduino.h"

MessageQueueItem::MessageQueueItem()
{
  m_messageType = ACKNOWLEDGE;
  m_channel = 0;
  m_messageId = 0;
  m_retriesLeft = 0;
}

void MessageQueueItem::init(uint8_t channel, uint8_t messageId, uint8_t * message, uint8_t messageLength)
{
  m_channel = channel;
  m_messageId = messageId;
  m_retriesLeft = MAXRETRIES;
  
  int i;
  for (i=0; i < MESSAGE_BUFFER_SIZE; i++)
  {
     m_messageBuffer[i] = 0;
  }
  
  memcpy(m_messageBuffer, message, min(MESSAGE_BUFFER_SIZE, messageLength)); 
}