#ifndef Vibrate_h
#define Vibrate_h

class VibrateMotor
{
public:
  VibrateMotor(int pwmPin);
  void setIntensity(int intensity);
private:

  int m_pwmPin;

};

#endif