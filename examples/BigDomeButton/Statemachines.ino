

#define CONTEXT ((process_context_t *)data)

void emitLifecycleTimeOut(void * data){
  CONTEXT->channel.send(LIFECYCLETIMEOUT, data);  
}
void initLifeCycle(int token, void * data){
  CONTEXT->lifecycleTimer.once(LOSE_TIMEOUT, emitLifecycleTimeOut, data);  
}

void initShutdownTimer(int token, void * data){
  CONTEXT->messageCycle->stop();
  if(token == WIN){
    CONTEXT->led.on(255);
  } else {
    CONTEXT->led.square(0, 120, 200, 20);
  }
  CONTEXT->lifecycleTimer.once(SHUTDOWN_TIMEOUT, emitLifecycleTimeOut, data);
}
void powerOff(int token, void * data){
  /* drain the voltage which keeps the primary mosfet open
    using a secondary mosfet
  */
  digitalWrite(POWER_PIN, HIGH);
  exit(0);
}

void emitSilence(void * data){
  CONTEXT->channel.send(SILENCE, data);  
}
void pollSilence(void * data){
  if(CONTEXT->senderReceiver.is_receiving()){
    CONTEXT->silenceCounter.reset();
  }else{
    CONTEXT->silenceCounter.trigger();
  }
}
void initSilenceCounter(int token, void * data){
  //TODO configure a reset for the silence test
  //CONTEXT->senderReciever
  CONTEXT->silencePoll.every(SILENCE_POLL, pollSilence, data);
  CONTEXT->silenceCounter.start(SILENCE_COUNT, emitSilence, data);
}

void emitRetry(void * data){
  CONTEXT->channel.send(RETRY, data);  
}

void initResendTimer(int token, void * data){
  message.uptime = millis();

  CONTEXT->senderReceiver.send((uint8_t *)&message, sizeof(message_t));
  CONTEXT->resendTimeout = CONTEXT->resendTimeout + (RESEND_BACKOFF * random(0,4)) ;
  CONTEXT->resendTimer.once(CONTEXT->resendTimeout, emitRetry, data);

}
void initRestartTimer(int token, void * data){
  CONTEXT->resendTimeout = RESEND_TIMEOUT;
  CONTEXT->restartTimer.once(RESTART_TIMEOUT, emitRetry, data);
}



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
  sendMessage.on(l++, RETRY)->to(waitForSilence);
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
  context.channel.addListener(lifeCycle);
  context.channel.addListener(messageCycle);

  lifeCycle.start(&context);
  messageCycle.start(&context);
}

