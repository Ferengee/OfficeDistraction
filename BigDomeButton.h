#ifndef BigDomeButton_h
#define BigDomeButton_h
#include <WProgram.h>

/*
 *  callback function
 *  is called if the button changes state
 * 
 */
typedef void (* BigDomeEventHandler) (bool pressed);

class BigDomeButton
{
public:
  BigDomeButton(int, pwmLedPin, int buttonPin);
  void sample();
  void setButtonEventHandler(BigDomeEventHandler eh);
  void setLedBrightness(int brightness); //0-255
  
private:
  int pwmLedPin;
  int buttonPin;
  int sampleBuffer[3];
};
#endif