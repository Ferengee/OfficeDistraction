#include "Vibrate.h"
#include "Arduino.h"

VibrateMotor::VibrateMotor(int pwmPin)
{
    m_pwmPin = m_pwmPin;
}

void VibrateMotor::setIntensity(int intensity)
{
  analogWrite(m_pwmPin, intensity);
}