#include "PWMControl.h"

PWMControl::PWMControl(int pwmPin)
{
  _pin = pwmPin;
}
PWMControl::PWMControl()
{
}

void PWMControl::update()
{
  switch(_shape){
    case SINE:
      writeSine();
      break;
    case SQUARE:
      writeSquare();
      break;
    case OFF:
      analogWrite(_pin, 0);
      break;
    case ON:
      analogWrite(_pin, _amplitude);
      break;  
  }
}

void PWMControl::set(uint8_t shape, uint8_t offset, uint8_t duration, uint8_t amplitude, uint8_t period)
{
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
   * 2pi = 5.12 seconds if _period = 256
   * 10 * pi / _period  * millis
   */
  analogWrite(_pin, 0.5 * (renderSine() + 1) * _amplitude );
}

double PWMControl::renderSine(){
  unsigned long x = (millis() - _start) * (10 * PI / _period);
   return sin(x);
}

void PWMControl::writeSquare()
{
  int amplitude = _amplitude;
  if ((renderSine() + 1) * 128 < _shift)
    amplitude = 0;
  analogWrite(_pin, amplitude);
}
