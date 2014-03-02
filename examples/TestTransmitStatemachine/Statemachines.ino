#include <SimpleStateMachine.h>
#include <EventChannel.h>



State starting, won, lost, done, waitForSilence, sendMessage, wait;

Vertex state_machine_links[9];

Machine lifeCycle = Machine(starting);
Machine messageCycle = Machine(waitForSilence);


void setup_machines(){
  Vertex * l = state_machine_links;

/*  life cycle */
  starting.on(l++, WIN)->to(won);
  starting.on(l++, LIFECYCLETIMEOUT)->to(lost);
  starting.on(l++, LOSE)->to(lost);
  won.on(l++, LIFECYCLETIMEOUT)->to(done);
  lost.on(l++, LIFECYCLETIMEOUT)->to(done);

/*  message cycle */
  waitForSilence.on(l++, SILENCE)->to(sendMessage);
  sendMessage.on(l++, RETRY)->to(sendMessage);
  sendMessage.on(l++, ACKNOWLEDGE)->to(wait);
  wait.on(l++, RETRY)->to(waitForSilence);

/*  life cycle functions */
  starting.enterfunc = initLifeCycle;
  lost.enterfunc = initShutdownTimer;
  won.enterfunc = initShutdownTimer;
  done.enterfunc = powerOff;

/*  message cycle functions */
  waitForSilence.enterfunc = initSilenceCounter;
  sendMessage.enterfunc = initResendTimer;
  wait.enterfunc = initRestartTimer;

/*  machines */
  eventChannel.addListener(lifeCycle);
  eventChannel.addListener(messageCycle);

  lifeCycle.start(&messagecycleContext);
  messageCycle.start(&lifecycleContext);
}

