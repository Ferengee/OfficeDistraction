#include <SimpleStateMachine.h>
#include <Schedulers.h>
#include <EventChannel.h>
#include <SampledButton.h>
#include <VirtualWire.h>

#define MY_ID 0


int led_pin = A0;
int button_pin = 2;
int rg1 = A5;
int rg2 = A4;
bool state = false;


typedef struct {
  uint8_t ids;
  uint8_t request_count;//message type, 0 master ack, 1 master win, 2..10 retry 
  unsigned long uptime;
} message_data_t;


SampledButton button = SampledButton(button_pin);


void toggleRG(){
  state = digitalRead(rg1);
  digitalWrite(rg1, !state);
  digitalWrite(rg2, state);
}

void clicked(bool up, int deltaT){
    if(!up){
      toggleRG();
      Serial.println("click"); 
    }
}

void sendReply(uint8_t id){
  message_data_t msg;
  msg.uptime = millis();
  msg.request_count = 0;
  msg.ids = id; 
  vw_send((uint8_t *)&msg, sizeof(message_data_t));
  vw_wait_tx(); // Wait until the whole message is gone
}

void testReceive(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  /*
  if(vw_have_message()){
    Serial.println("message Available"); 
  }
  */
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    uint8_t id = 2;
    
    digitalWrite(13, true); // Flash a light to show received good message
    // Message with a good checksum received, dump it.
    message_data_t * msg = (message_data_t *)buf; 
    
    
    Serial.print("ID: ");
    Serial.print(msg->ids);
    Serial.print(", request_count: ");
    Serial.print(msg->request_count);
    Serial.print(", uptime: ");
    Serial.print(msg->uptime);
    Serial.println("");
    digitalWrite(13, false);
    
    if(state){
      id = msg->ids;
      Serial.println("- ack");  
    }else{
      Serial.println("-   nack");  
    }
    delay(200);
    sendReply(id);
  } 
  //unsigned long start = millis();
 /* vw_wait_rx();
  Serial.print("delta: ");
  Serial.println(millis() - start);
  Serial.print("bad message count: ");
  Serial.println(vw_get_rx_bad());

  */
}

void setup(){
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);
  pinMode(13, OUTPUT);
  vw_rx_start(); // Start the receiver PLL running
  // Initialise the IO and ISR
//  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000); // Bits per sec
  
  pinMode(button_pin, INPUT);
  digitalWrite(button_pin, HIGH);
  pinMode(rg1, OUTPUT);
  pinMode(rg2, OUTPUT); 
  toggleRG();
  button.setClickEventHandler(clicked);
  Serial.println("setup finished...");

}

void loop(){
  button.sample();
  digitalWrite(led_pin, vw_rx_active());
  delay(20);
  testReceive();
  
}