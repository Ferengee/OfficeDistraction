#ifndef PWMCONTROL_H
#define PWMCONTROL_H
#include <Arduino.h>

enum {OFF, ON, SINE, SQUARE};

class PWMControl
{
public:
  PWMControl(int pwmPin);
  void sine(uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period){
    set(SINE, offset, duration, amplitude, period);
  }
  void square(uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period){ 
    set(SQUARE, offset, duration, amplitude, period);
  }
  void off(){ set(OFF, 0,0,0,0); }
  void on(uint8_t amplitude){ set(ON, 0, 0, amplitude, 0); }
  void set(uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period);
  void update();

private:
  int pin;
  uint8_t _shape; // sine | square | on |off
  uint8_t _offset;  // 0..255 * (period / 256)
  uint8_t _duration; // 0..256 * 0.1sec
  uint8_t _amplitude; // 0..256 
  uint8_t _period; // 0..256 * 0.02 sec (max 5.12 sec)
  uint8_t _shift; // (0..256 - 128 ) / 256 
  unsigned long _start;
  int _pin;
  void writeSquare();
  void writeSine();
  double renderSine();

};
#endif
