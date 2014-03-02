#include "Schedulers.h"
#include "EventChannel.h"

#define POWER_PIN 4
#define LED_PIN 13
#define SILENCE_COUNT 4
#define RESEND_TIMEOUT 400
#define RESTART_TIMEOUT 1000
#define LOSE_TIMEOUT 10000
#define SHUTDOWN_TIMEOUT 5000
/*
 * Read the Statemachines tab to see the state machine configuration
 * 
 */
enum TransitionEvents {WIN, LOSE, LIFECYCLETIMEOUT, SILENCE, RETRY, ACKNOWLEDGE};


typedef struct {
  Scheduler timer;
  EventChannel * channel;
} lifecycle_context_t;

typedef struct {
  CountdownTimer silenceCounter;
  Scheduler resendTimer;
  Scheduler restartTimer;
  EventChannel * channel;

} messagecycle_context_t;

Schedulers schedulers;
EventChannel eventChannel;

void lifecycleTimeOut(void * data){
  ((lifecycle_context_t *)data)->channel->send(LIFECYCLETIMEOUT, data);  
}
void initLifeCycle(int token, void * data){
  lifecycle_context_t * context = (lifecycle_context_t *)data;
  context->timer.once(LOSE_TIMEOUT, lifecycleTimeOut, data);
  
}


void initShutdownTimer(int token, void * data){
  if(token == WIN){
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  ((lifecycle_context_t *)data)->timer.once(SHUTDOWN_TIMEOUT, lifecycleTimeOut, data);
}
void powerOff(int token, void * data){
  digitalWrite(POWER_PIN, LOW);
}

void emitSilence(void * data){
  ((messagecycle_context_t *)data)->channel->send(SILENCE, data);  
}
void initSilenceCounter(int token, void * data){
  //TODO configure a reset for the silence test
  //((messagecycle_context_t *)data)->senderReciever
  ((messagecycle_context_t *)data)->silenceCounter.start(SILENCE_COUNT, emitSilence, data);
}

void emitRetry(void * data){
  ((messagecycle_context_t *)data)->channel->send(RETRY, data);  
}

void initResendTimer(int token, void * data){
  ((messagecycle_context_t *)data)->resendTimer.once(RESEND_TIMEOUT, emitRetry, data);

}
void initRestartTimer(int token, void * data){
  ((messagecycle_context_t *)data)->restartTimer.once(RESTART_TIMEOUT, emitRetry, data);
}

messagecycle_context_t messagecycleContext;
lifecycle_context_t lifecycleContext;

void setup(){
  //digitalWrite(POWER_PIN, HIGH);
  messagecycleContext.channel = &eventChannel;
  lifecycleContext.channel = &eventChannel;
  
  setup_machines(); 
  
}
void loop(){}

