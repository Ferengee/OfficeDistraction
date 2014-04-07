#include "Schedulers.h"
#include <SimpleStateMachine.h>
#include <EventChannel.h>
#include <VirtualWire.h>
#include <RFSenderReceiver.h>
#include <PWMControl.h>

/* config sender id per button */
#define SENDER_ID 4
#define POWER_PIN 4
#define LED_PIN 6
#define SILENCE_COUNT 4
#define RESEND_TIMEOUT 600
#define RESEND_BACKOFF 60
#define RESTART_TIMEOUT 3000
#define LOSE_TIMEOUT 10000
#define SHUTDOWN_TIMEOUT 10000
#define SILENCE_POLL 10
/*
 * Read the Statemachines tab to see the state machine configuration
 * 
 */

/*
 * TODO: use pwm control to make the light glow
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
  PWMControl led;

} process_context_t;

process_context_t context;



void setup_machines();

void setup(){
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, LOW);

  context.senderReceiver.init(12,11, 2000);
  message.senderId = SENDER_ID;
  message.messageType = ANSWER;
  
  context.resendTimeout = RESEND_TIMEOUT;
  context.schedulers.attach(context.resendTimer);
  context.schedulers.attach(context.restartTimer);
  context.schedulers.attach(context.lifecycleTimer);
  context.schedulers.attach(context.silencePoll);
  context.messageCycle = &messageCycle;
  
  context.led.init(LED_PIN);
  context.led.sine(0, 120, 255, 64);
  setup_machines(); 
  
}
void loop(){
  context.schedulers.trigger();
  context.led.update();
   if (context.senderReceiver.have_message()){
    uint8_t len = sizeof(message_t);
    if(context.senderReceiver.get_message((uint8_t *)&reply, &len)){
      if(len != sizeof(message_t) || reply.senderId == 0){
        //pass
      } else if(reply.senderId != message.senderId){
        /* not us */
        if(reply.uptime > millis() || reply.messageType == WINNER){
          context.channel.send(LOSE, &context);
        } else {
          context.channel.send(ACKNOWLEDGE, &context);
        }
        
      }else{
        /* us */
        if(reply.messageType == WINNER){
          context.channel.send(WIN, &context);
        } else {
          context.channel.send(ACKNOWLEDGE, &context);
        }
      }
    }
  }
  delay(5);
}

