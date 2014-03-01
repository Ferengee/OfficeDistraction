#include <SimpleStateMachine.h>
#include <Schedulers.h>
#include <EventChannel.h>
#include <VirtualWire.h>

#define MY_ID 1

typedef struct {
  uint8_t ids;
  uint8_t request_count;
  unsigned long uptime;
} message_data_t;

bool succes = false;

/*
 * listen: try to receive a message, decode it and send an event on the event channel
 */
class RFTransceiver
{
private:
  EventChannel * eventChannel;
  int id;
public:
  void setId(int id){ this.id = id;}
  void sendMessage(uint8_t request_count){
    message_data_t msg;
    msg.uptime = millis();
    msg.request_count = request_count;
    msg.ids = this->id;
    
    digitalWrite(13, true); // Flash a light to show transmitting
    vw_send((uint8_t *)&msg, sizeof(message_data_t));
    // I do want to wait until the message is gone, because i cant do anything else 
    // which would be usefull
    // button presses are ignored, incomming messages cant be decoded while transmitting anyway
    vw_wait_tx(); // Wait until the whole message is gone
    
    digitalWrite(13, false);
  
  }

  
  bool tryReceiveMessage(bool *succes){
    //TODO: move wait to trigger delta, dont want to actualy wait
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


}

Machine failure;
Machine flow;

State lost;
State startState;
State waitForSilence;
State sendMessage;
State acknowledged;
State won;


Vertex links[7];


void setup()
{
  Vertex * l = links;
  waitForSilence.on(l++, SILENCE)->to(sendMessage);
  //TODO: retry loop
  sendMessage.on(l++, ACK_MSG)->to(acknowledged);
  acknowledged.on(l++, ACK_WIN)->to(won);

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

