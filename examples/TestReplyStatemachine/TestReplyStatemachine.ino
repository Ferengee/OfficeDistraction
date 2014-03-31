#include "Schedulers.h"
#include <SimpleStateMachine.h>
#include <EventChannel.h>
#include <VirtualWire.h>
#include <RFSenderReceiver.h>

#define LED_PIN 13
#define SILENCE_COUNT 4
#define RESEND_TIMEOUT 400
#define RESEND_BACKOFF 100
#define WINNER_SETTLED 3000

#define MY_ID 0

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


typedef struct {
  Scheduler settleTimer;
  Scheduler replyScheduler;
  EventChannel channel;
  Schedulers schedulers;
  RFSenderReceiver senderReceiver;
  Machine * winnerCycle;
  message_t * first;
  message_t * buffer;
  
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
  context.senderReceiver.init(11,12, 2000);

  context.schedulers.attach(context.replyScheduler);
  context.schedulers.attach(context.settleTimer);
  context.winnerCycle = &winnerCycle;
  context.first = message_buffer;
  context.buffer = message_buffer + 1;
  
  /* testing
  context.schedulers.attach(testScheduler);
  testScheduler.every(3000, testChannelSendMessage, &context);
   --- */
  setup_machines(); 
}
void loop(){
  context.schedulers.trigger();
  if (context.senderReceiver.have_message()){
    uint8_t len = sizeof(message_t);
    context.senderReceiver.get_message((uint8_t *)context.buffer, &len);
    context.channel.send(MESSAGE, &context);
  }
  delay(5);
}

