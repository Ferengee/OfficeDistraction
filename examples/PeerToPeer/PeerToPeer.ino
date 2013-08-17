#include <BigDomeButton.h>
#include <RFSenderReceiver.h>
#include <MessageQueueSorter.h>
#include <BaseSenderReceiver.h>
#include <RFMessageControl.h>
#include <PWMControl.h>
#include <Relay.h>
#include <MessageQueueItem.h>
#include <Vibrate.h>
#include <OfficeDistractionConfig.h>
#include <VirtualWire.h>
#include <MiniRPC.h>

RFSenderReceiver tr = RFSenderReceiver();
RFMessageControl control = RFMessageControl(&tr);

void receiveMessageItem(MessageQueueItem item){
  Serial.print("Message received:") ;
  uint8_t message[MESSAGE_SIZE];
  uint8_t length = MESSAGE_SIZE;
  item.getMessage(message, &length);
  Serial.print(":");
  Serial.print(item.getMessageType());
  Serial.print(":");
  Serial.println((char *)message);
}

void notifyDiscartedItem(MessageQueueItem* item)
{
  Serial.print("Discarted item: ");
  Serial.print(item->getMessageId());
  Serial.print(":");
  Serial.println(item->getMessageType());
}

class Send : public MiniRPCMethod
{
public:

  Send(){
    setName("send");
    otherId = 0;
  }
  
  virtual void prepare(){
    get(message, length);
  }
 
  virtual void execute(){
    control->sendMessage(otherId, (uint8_t *)message, 6);
  }

  virtual void error(){
    Serial.println("{\"class\": \"ArgumentException\", \"message\":\"Expected: send(char * message)\"}");
  }

  RFMessageControl * control;
  int otherId;
private:
  char * message;
  int length;
};

class SetChannel : public MiniRPCMethod
{
public:
  SetChannel(){
    setName("setChannel");
  }
  
  virtual void prepare(){
    get(ourId);
    get(otherId);
  }
 
  virtual void execute(){
    control->setChannelID((uint8_t)ourId);
    sendMethod->otherId = otherId;
  }

  virtual void error(){
    Serial.println("{\"class\": \"ArgumentException\", \"message\":\"Expected: setChannel(int <ourId>, int <otherId>)\"}");
  }

  RFMessageControl * control;
  Send * sendMethod;
private:
  int ourId;
  int otherId;
};

StreamWrapper in = StreamWrapper();
MiniRPCDispatcher dispatcher = MiniRPCDispatcher(&in);

Send sendMethod;
SetChannel setChannel;

void setup(){
  Serial.begin(9600);
  control.setChannelID(0);
  control.setMessageReceivedEventHandler(receiveMessageItem);
  control.notifyDiscartedItem = notifyDiscartedItem;
  tr.init(11,10,2000);
  in.setStream(&Serial);
  
  sendMethod.control = &control;
  setChannel.control = &control;
  setChannel.sendMethod = &sendMethod;
  
  dispatcher.registerMethod(&sendMethod);
  dispatcher.registerMethod(&setChannel);

  Serial.println("setup finished...");
}

void loop(){
  control.update();
  dispatcher.update();
  delay(1);
}
