#include "RFSenderReceiver.h"

RFSenderReceiver::RFSenderReceiver(int txPin, int rxPin, int speed)
{
  if (speed > 0){
    vw_set_tx_pin ((uint8_t)txPin);
    vw_set_rx_pin ((uint8_t)rxPin);
    vw_set_ptt_inverted(true);
    vw_setup ((uint16_t)speed);
    vw_rx_start(); 
  } 
}