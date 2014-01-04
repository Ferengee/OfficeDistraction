#include <SampledButton.h>

int button_pin = 2;
int rg1 = A5;
int rg2 = A4;
bool state = false;

SampledButton button = SampledButton(button_pin);


void toggleRG(){
  state = digitalRead(rg1);
  digitalWrite(rg1, !state);
  digitalWrite(rg2, state);
}

void clicked(bool up, int deltaT){
    if(!up){
      toggleRG();
      Serial.println("click"); 
    }
}

void setup(){
  Serial.begin(9600);
  Serial.println("setup finished...");
  pinMode(button_pin, INPUT);
  digitalWrite(button_pin, HIGH);
  pinMode(rg1, OUTPUT);
  pinMode(rg2, OUTPUT); 
  toggleRG();
  button.setClickEventHandler(clicked);

}

void loop(){
  button.sample();
  delay(20);
}