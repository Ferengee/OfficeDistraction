#include "Schedulers.h"
#include <SimpleStateMachine.h>
#include <EventChannel.h>
#include "BaseSenderReceiver.h"

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

State starting, won, lost, done, waitForSilence, sendMessage, wait;

Vertex state_machine_links[10];

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
  BaseSenderReceiver senderReceiver;

} process_context_t;

process_context_t context;
void setup_machines();

void setup(){
  digitalWrite(POWER_PIN, HIGH);
  context.resendTimeout = RESEND_TIMEOUT;
  context.schedulers.attach(context.resendTimer);
  context.schedulers.attach(context.restartTimer);
  context.schedulers.attach(context.lifecycleTimer);
  context.schedulers.attach(context.silencePoll);

  setup_machines(); 
  
}
void loop(){
  context.schedulers.trigger();
}

