#include "PWMControl.h"

PWMControl::PWMControl(int pwmPin)
{
  _pin = pwmPin;
  _shape = OFF;
}
PWMControl::PWMControl()
{
}
PWMControl::PWMControl()
{
  _pin = -1;
}

void PWMControl::update()
{
  if(_pin == -1)
    return;
  
  switch(_shape){
    
    case SINE:
      writeSine();
      break;
    case SQUARE:
      writeSquare();
      break;
  }
  if(_shape > OFF && (millis() - _start) > (_duration * 100)) 
    set(OFF, 0,0,0,0);
}

void PWMControl::set(uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period)
{
  if(_pin == -1)
    return;
   
  switch(shape){
    case OFF:
      analogWrite(_pin, 0);
      break;
    case ON:
      analogWrite(_pin, _amplitude);
      break;    
  }
  _shape = shape;
  _offset = offset;
  _duration = duration;
  _amplitude = amplitude;
  _period = period;
  _start = millis();
}

void PWMControl::writeSine()
{
  
  /*
   * 2pi = 5.12 seconds if _period = 255
   * 10 * pi / _period  * millis
   */
  analogWrite(_pin, 0.5 * (renderSine() + 1) * _amplitude );
}

double PWMControl::renderSine(){
  double x = (millis() - _start) * ( PI /( ((int)_period + 1) * 10));
  return sin(x);
}

void PWMControl::writeSquare()
{
  int amplitude = _amplitude;
  if (((renderSine() + 1) * 128) < _shift)
    amplitude = 0;
  analogWrite(_pin, amplitude);
}
