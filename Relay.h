#ifndef Relay_h
#define Relay_h
#include <Arduino.h>

class Relay
{
public:
  Relay(int commPin);
  Relay();

  void init(int commPin);
  void toggle();
  void on();
  void off();
  void update();
  
private:
    int pin;
    int state;
};
#endif
