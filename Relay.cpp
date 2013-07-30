#include "Relay.h"

Relay::Relay(int commPin)
{
  pin = commPin; 
  pinMode(pin, OUTPUT);
  state = LOW;
  
}

void Relay::on()
{
  state = HIGH;
  update();
}

void Relay::off()
{
  state = LOW;
  update();
}

void Relay::toggle()
{
  if(state == HIGH)
    state = LOW;
  else
    state = HIGH;
  
  update();
}

void Relay::update()
{
  digitalWrite(pin, state);
}
