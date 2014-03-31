#include "Schedulers.h"
#include <SimpleStateMachine.h>
#include <EventChannel.h>
#include <VirtualWire.h>
#include <RFSenderReceiver.h>

/* config sender id per button */
#define SENDER_ID 1
#define POWER_PIN 4
#define LED_PIN 13
#define SILENCE_COUNT 4
#define RESEND_TIMEOUT 400
#define RESEND_BACKOFF 100
#define RESTART_TIMEOUT 3000
#define LOSE_TIMEOUT 10000
#define SHUTDOWN_TIMEOUT 5000
#define SILENCE_POLL 10
/*
 * Read the Statemachines tab to see the state machine configuration
 * 
 */
enum TransitionEvents {WIN, LOSE, LIFECYCLETIMEOUT, SILENCE, RETRY, ACKNOWLEDGE};

enum MessageTypes {ANSWER, CURRENTLEAD, WINNER};

typedef struct {
  uint8_t senderId;
  uint8_t messageType;
  unsigned long uptime;

} message_t;

message_t message;
message_t reply;

State starting, won, lost, done, waitForSilence, sendMessage, wait;

Vertex state_machine_links[9];

Machine lifeCycle = Machine(starting);
Machine messageCycle = Machine(waitForSilence);

typedef struct {
  Scheduler lifecycleTimer;
  CountdownTimer silenceCounter;
  Scheduler resendTimer;
  Scheduler restartTimer;
  EventChannel channel;
  Schedulers schedulers;
  int resendTimeout;
  Scheduler silencePoll;
  RFSenderReceiver senderReceiver;
  Machine * messageCycle;

} process_context_t;

process_context_t context;
void setup_machines();

void setup(){
  digitalWrite(POWER_PIN, HIGH);
  context.senderReceiver.init(11,12, 2000);
  message.senderId = SENDER_ID;
  message.messageType = ANSWER;
  
  context.resendTimeout = RESEND_TIMEOUT;
  context.schedulers.attach(context.resendTimer);
  context.schedulers.attach(context.restartTimer);
  context.schedulers.attach(context.lifecycleTimer);
  context.schedulers.attach(context.silencePoll);
  context.messageCycle = &messageCycle;
  setup_machines(); 
  
}
void loop(){
  context.schedulers.trigger();
   if (context.senderReceiver.have_message()){
    uint8_t len = sizeof(message_t);
    if(context.senderReceiver.get_message((uint8_t *)&reply, &len)){
      if(reply.senderId != message.senderId){
        Serial.println("not us");
        /* not us */
        if(reply.uptime > millis() || reply.messageType == WINNER){
          context.channel.send(LOSE, &context);
        } else {
          context.channel.send(ACKNOWLEDGE, &context);
        }
        
      }else{

        /* us */
        if(reply.messageType == WINNER){
          Serial.println("us:WINNER");

          context.channel.send(WIN, &context);
        } else {
          Serial.println("us:ACKNOWLEDGE");

          context.channel.send(ACKNOWLEDGE, &context);
        }
      }
    }
  }
  delay(5);
}

