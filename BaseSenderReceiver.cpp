#include "BaseSenderReceiver.h"

BaseSenderReceiver::BaseSenderReceiver(){
  
}


TestSenderReceiver::TestSenderReceiver()
{
  m_start = 0;
  m_end = 0;
  m_sendcount = 0;
  m_other = NULL;
}

bool TestSenderReceiver::send(uint8_t* buf, uint8_t len)
{
  if(m_other != NULL)
    return m_other->receive(buf, len);
  return false;
  
}


bool TestSenderReceiver::receive(uint8_t * buf, uint8_t len)
{
  m_sendcount++;
  //Serial.println("TestSenderReceiver::send");
  uint8_t buffer[40];
  uint8_t buflen = 40;
  MessageQueueItem incommingItem;
  
  int end = (m_end + 1) % MAXMESSAGECOUNT;
  bool succes = end != m_start;
  if (succes){
    memcpy(incommingItem.getBuffer(), buf, min(MESSAGE_BUFFER_SIZE, len)); 
    
    incommingItem.getMessage(buffer, &buflen);
    
    Serial.print("  send message: ");
    if(incommingItem.getMessageType() % 2 == 0)
      Serial.print("-> ");
    else
      Serial.print("<- ");
    Serial.print(incommingItem.getMessageId());
    Serial.print(":");
    Serial.print(incommingItem.getMessageType());
    Serial.print(":");
    Serial.print(m_sendcount);
    Serial.print(":");
    Serial.println((char *)buffer);

    
    if (m_sendcount > 10 && ((m_sendcount % 5) == 0 ||(m_sendcount % 6) == 2)){
      Serial.print("  - dropped message message: ");
      Serial.print(incommingItem.getMessageId());
      Serial.print(":");
      Serial.print(incommingItem.getMessageType());
      Serial.print(":");
      Serial.println(m_sendcount);
    } else {
      
      MessageQueueItem * item = &m_buffer[m_end];
      memcpy(item->getBuffer(), buf, min(MESSAGE_BUFFER_SIZE, len)); 

      m_end = end;
     
    }
  }
  return succes;
}

bool TestSenderReceiver::have_message()
{
  return m_start != m_end;
}
bool TestSenderReceiver::get_message(uint8_t * buf, uint8_t * len)
{
  bool succes = have_message();
  if (succes){
    Serial.print("get message: ");
    Serial.println(m_start);
    MessageQueueItem * item = &m_buffer[m_start];
    memcpy(buf, item->getBuffer(), min(MESSAGE_BUFFER_SIZE, *len)); 
    memset(item->getBuffer(),0,MESSAGE_BUFFER_SIZE);
    m_start = (m_start + 1) % MAXMESSAGECOUNT;
  }
  return succes;
}
