#define CONTEXT ((process_context_t *)data)


void printMessage(message_t * m){
  Serial.print("id :");
  Serial.print(m->senderId);
  Serial.print(",uptime :");
  Serial.print((int)m->uptime);
  Serial.print(",type :");
  Serial.println((int)m->messageType);
 
}

void sendReply(void * data){
  log("Sending Reply");
 
  if(CONTEXT->winnerCycle->getCurrentState() == &winnerKnown){
    CONTEXT->first->messageType = WINNER;
  } else {
    CONTEXT->first->messageType = CURRENTLEAD;
  }
  printMessage(CONTEXT->first);
  message_t * output = &CONTEXT->outputbuffer;
  memcpy(output, CONTEXT->first, sizeof(message_t));
  output->uptime = millis() - output->uptime; 
  CONTEXT->senderReceiver.send((uint8_t *)output, sizeof(message_t));

  //CONTEXT->channel.send(LIFECYCLETIMEOUT, data);  
}


void swapMessageBuffer(process_context_t * context){
  message_t * tmp;
  tmp = context->first;
  context->first = context->buffer;
  context->buffer = tmp;
}

/*  acceptAnswer 
  - copy msg in context 
  - reset reply scheduler 
  - signal alarm */
void emitTimeout(void * data){
  log("emitting timeout");
  CONTEXT->channel.send(TIMEOUT, data);
}

void emitNextQuestion(void * data){
  log("emitting next question");
  Serial.println("Alarm:off");

  CONTEXT->channel.send(NEXTQUESTION, data);
}

void notifySerial(int token, void * data){
  Serial.print("Winner:");
  Serial.println(CONTEXT->first->senderId);
}

void acceptAnswer(int token, void * data){
  CONTEXT->buffer->uptime = millis() - CONTEXT->buffer->uptime;
  swapMessageBuffer(CONTEXT);

  log("Alarm");
  Serial.println("Alarm:on");
  CONTEXT->lifecycleTimer.once(LIFECYCLE_TIMEOUT, emitNextQuestion, data);
  CONTEXT->settleTimer.once(WINNER_SETTLED, emitTimeout, data);
  CONTEXT->replyScheduler.once(RESEND_TIMEOUT, sendReply, data);
}

/*  updateAnswer
  - reset reply scheduler
  - update message in context (if needed) */
void updateAnswer(int token, void * data){
  /* convert to relative uptime (relative to ours) */
  CONTEXT->buffer->uptime = millis() - CONTEXT->buffer->uptime;
  log("--- Update Answer");

  /* compare the time the peers started */
  if(CONTEXT->first->uptime >= CONTEXT->buffer->uptime){
    swapMessageBuffer(CONTEXT);
  }
  CONTEXT->replyScheduler.once(RESEND_TIMEOUT, sendReply, data);
}

/*  initQuestionContext
  - wipe answer from context */
void initQuestionContext(int token, void * data){
  log("Setup context");
  CONTEXT->buffer->senderId = 0;
  CONTEXT->first->senderId = 0;
}



void setup_machines(){
  Vertex * l = state_machine_links;

/*  Handle messages */
  newQuestion.on(l++, MESSAGE)->to(oneAnswer);
  oneAnswer.on(l++, NEXTQUESTION)->to(newQuestion);
  oneAnswer.on(l++, MESSAGE)->to(moreAnswers);
  moreAnswers.on(l++, MESSAGE)->to(moreAnswers);
  moreAnswers.on(l++, NEXTQUESTION)->to(newQuestion);

/*  do we have a winner */
  winnerUnknown.on(l++, TIMEOUT)->to(winnerKnown);
  winnerKnown.on(l++, NEXTQUESTION)->to(winnerUnknown);

/* winner */
  winnerKnown.enterfunc = notifySerial;

/*  message cycle functions */
  oneAnswer.enterfunc = acceptAnswer;
  moreAnswers.enterfunc = updateAnswer;
  newQuestion.enterfunc = initQuestionContext;

/*  machines */
  context.channel.addListener(winnerCycle);
  context.channel.addListener(questionCycle);

  winnerCycle.start(&context);
  questionCycle.start(&context);

/*  reply scheduler */

/*  reset: replyTimer->once(REPLYTIMEOUT, sendReply, questionContext) */
}
