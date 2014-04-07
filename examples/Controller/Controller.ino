#include "Schedulers.h"
#include <SimpleStateMachine.h>
#include <EventChannel.h>
#include <VirtualWire.h>
#include <RFSenderReceiver.h>
#include <Relay.h>

#define LED_PIN 13
#define SILENCE_COUNT 4
#define RESEND_TIMEOUT 400
#define RESEND_BACKOFF 100
#define WINNER_SETTLED 3000
#define LIFECYCLE_TIMEOUT 10000

#define MY_ID 0

#define ALARM_PIN 4
#define CONTEXT ((process_context_t *)data)


enum MessageTypes {ANSWER, CURRENTLEAD, WINNER};

typedef struct {
  uint8_t senderId;
  uint8_t messageType;
  unsigned long uptime;

} message_t;

enum TransitionEvents {MESSAGE, NEXTQUESTION, TIMEOUT};

State newQuestion, oneAnswer, moreAnswers, winnerUnknown, winnerKnown;

Vertex state_machine_links[7];

Machine winnerCycle = Machine(winnerUnknown);
Machine questionCycle = Machine(newQuestion);

/* TODO:
 * Dont use a message to keep track of who won
 * but use a struct {id, started_at} which reads better 
 * (uptime is not the correct term for how it is used in the comparison)
 * 
 * use a formal method to comunicate the state over serial.
 * - who won
 * - when starting a new questionCycle
 * 
*/
typedef struct {
  Scheduler settleTimer;
  Scheduler replyScheduler;
  Scheduler lifecycleTimer;

  EventChannel channel;
  Schedulers schedulers;
  RFSenderReceiver senderReceiver;
  Machine * winnerCycle;
  message_t * first;
  message_t * buffer;
  message_t outputbuffer;
  
  Relay alarm;
  
} process_context_t;

process_context_t context;

  message_t message_buffer[2];
  
void setup_machines();

/* testing 
Scheduler testScheduler;
void testChannelSendMessage(void * data){
  Serial.println(" --- test channel Send Message --- ");
   CONTEXT->channel.send(MESSAGE, data); 
}
 --- */
void setup(){
  Serial.begin(9600);
  
  context.senderReceiver.init(12,11, 2000);

  context.schedulers.attach(context.replyScheduler);
  context.schedulers.attach(context.settleTimer);
  context.schedulers.attach(context.lifecycleTimer);

  context.winnerCycle = &winnerCycle;
  context.first = message_buffer;
  context.buffer = message_buffer + 1;
  context.alarm.init(ALARM_PIN);
  /* testing
  context.schedulers.attach(testScheduler);
  testScheduler.every(3000, testChannelSendMessage, &context);
   --- */
  setup_machines(); 
  Serial.println("Ready");
}
void loop(){
  context.schedulers.trigger();
  if (context.senderReceiver.have_message()){
    uint8_t len = sizeof(message_t);
    context.buffer->senderId = 0;
    context.buffer->messageType = 0;
    context.buffer->uptime = 0;

    if(context.senderReceiver.get_message((uint8_t *)context.buffer, &len)){
      if(len == sizeof(message_t)){
        context.channel.send(MESSAGE, &context);
      }
    }
  }
  delay(5);
}

