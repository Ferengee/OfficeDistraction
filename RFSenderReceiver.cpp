#include "RFSenderReceiver.h"

void RFSenderReceiver::init(int txPin, int rxPin, int speed)
{
  if (speed > 0){
    vw_set_tx_pin ((uint8_t)txPin);
    vw_set_rx_pin ((uint8_t)rxPin);
    vw_set_ptt_inverted(true);
    vw_setup ((uint16_t)speed);
    vw_rx_start(); 
  } 
}


bool RFSenderReceiver::get_message(uint8_t* buf, uint8_t* len)
{
  return vw_get_message(buf, len);
}

bool RFSenderReceiver::have_message()
{
  return vw_have_message();
  
}

bool RFSenderReceiver::send(uint8_t* buf, uint8_t len)
{
  bool success = vw_send(buf, len);
  vw_wait_tx(); // 
  return success;
}

RFSenderReceiver::RFSenderReceiver()
{

}
