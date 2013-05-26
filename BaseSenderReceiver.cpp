#include "BaseSenderReceiver.h"

BaseSenderReceiver::BaseSenderReceiver(){
  
}


TestSenderReceiver::TestSenderReceiver()
{
  m_start = 0;
  m_end = 0;
}

bool TestSenderReceiver::send(uint8_t * buf, uint8_t len)
{
  Serial.println("TestSenderReceiver::send");
  int end = (m_end + 1) % MAXMESSAGECOUNT;
  bool succes = end != m_start;
  if (succes){
    
    MessageQueueItem item = m_buffer[m_end];
    memcpy(item.getBuffer(), buf, min(MESSAGE_BUFFER_SIZE, len)); 
    m_end = end;
    
    Serial.print(item.getMessageType());
    Serial.print(item.getMessageId());
    Serial.println(item.getChannel());
    
    uint8_t m1[30];
    uint8_t l1 = 30;
    item.getMessage(m1,&l1);
    Serial.println((char *)m1);

  }
  return succes;
}

bool TestSenderReceiver::have_message()
{
  //debug
  Serial.print("TestSenderReceiver::have_message: ");
  if (m_start != m_end){
    Serial.println("true");
  }
  //
  return m_start != m_end;
}
bool TestSenderReceiver::get_message(uint8_t * buf, uint8_t * len)
{
  
  Serial.println("TestSenderReceiver::get_message");
  bool succes = have_message();
  if (succes){
    Serial.println("found");
    MessageQueueItem item = m_buffer[m_start];
    memcpy(buf, item.getBuffer(), min(MESSAGE_BUFFER_SIZE, *len)); 
    m_start = (m_start + 1) % MAXMESSAGECOUNT;
  }
  return succes;
}
