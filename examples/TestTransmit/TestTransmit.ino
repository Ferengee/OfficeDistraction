#include <VirtualWire.h>

#define MY_ID 1

typedef struct {
  uint8_t ids;
  uint8_t request_count;
  unsigned long uptime;
} message_data_t;

int led_pin = A0;
bool succes = false;

void sendMessage(uint8_t request_count){
  message_data_t msg;
  msg.uptime = millis();
  msg.request_count = request_count;
  msg.ids = MY_ID;
  
  digitalWrite(13, true); // Flash a light to show transmitting
  vw_send((uint8_t *)&msg, sizeof(message_data_t));
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(13, false);
 
}


bool tryReceiveMessage(bool *succes){
  *succes = vw_wait_rx_max(200);
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if(vw_get_message(buf, &buflen)){
    message_data_t * msg = (message_data_t *)buf; 
    if(msg->request_count == 0){
      *succes = (msg->ids == MY_ID);
      return true;
    }else{
      return false; 
    }
  }
  return false;
}

void setup()
{
  pinMode(led_pin, OUTPUT);
  pinMode(13, OUTPUT);

  vw_rx_start(); // Start the receiver PLL running
  // Initialise the IO and ISR
//  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000); // Bits per sec
  
  
  //
  uint8_t request_count = 1;
  // listen for silence
  while(millis() > 20 || vw_rx_active()){
    delay(1);
  };
  
  while(request_count < 7){
    sendMessage(request_count);
    if(tryReceiveMessage(&succes)){
        break;
    }else{
      request_count++;
      delay(15 * request_count);
    }
  }
}
void loop()
{
  //digitalWrite(led_pin, vw_rx_active());
  digitalWrite(13, HIGH);
  delay(100);
  if(!succes)
    digitalWrite(13,LOW);
  delay(100);
}

