#ifndef BigDomeButton_h
#define BigDomeButton_h
#include "Arduino.h"

#define BDB_BUFFERLENGTH 3
/*
 *  callback function
 *  is called if the button changes state
 * 
 */
typedef void (* BigDomeEventHandler) (bool pressed);

class BigDomeButton
{
public:
  BigDomeButton(int pwmLedPin, int buttonPin);
  void sample();
  void setButtonEventHandler(BigDomeEventHandler eh);
  void setLedBrightness(int brightness); //0-255
  
private:
  int m_pwmLedPin;
  int m_buttonPin;
  BigDomeEventHandler callback;
  int m_sampleBuffer[BDB_BUFFERLENGTH];
  int m_lastState;
};
#endif