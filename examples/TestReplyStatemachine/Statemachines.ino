#define CONTEXT ((process_context_t *)data)


void sendReply(void * data){
  Serial.println("Sending Reply");
  if(CONTEXT->winnerCycle->getCurrentState() == &winnerKnown){
    CONTEXT->first->messageType = WINNER;
  } else {
    CONTEXT->first->messageType = CURRENTLEAD;
  }
  CONTEXT->senderReceiver.send((uint8_t *)CONTEXT->first, sizeof(message_t));

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
  Serial.println("emitting timeout");
  CONTEXT->channel.send(TIMEOUT, data);
}

void acceptAnswer(int token, void * data){
  Serial.println("Alarm");
  swapMessageBuffer(CONTEXT);
  CONTEXT->settleTimer.once(WINNER_SETTLED, emitTimeout, data);
  CONTEXT->replyScheduler.once(RESEND_TIMEOUT, sendReply, data);
}

/*  updateAnswer
  - reset reply scheduler
  - update message in context (if needed) */
void updateAnswer(int token, void * data){
  Serial.println("Update Answer");
  if(CONTEXT->buffer->uptime < CONTEXT->first->uptime){
    swapMessageBuffer(CONTEXT);
  }
  CONTEXT->replyScheduler.once(RESEND_TIMEOUT, sendReply, data);
}

/*  initQuestionContext
  - wipe answer from context */
void initQuestionContext(int token, void * data){
  Serial.println("Setup context");
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
