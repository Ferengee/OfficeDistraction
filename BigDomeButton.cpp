#include "BigDomeButton.h"

BigDomeButton::BigDomeButton(int pwmLedPin, int buttonPin)
{
  m_pwmLedPin = pwmLedPin;
  m_buttonPin = buttonPin;
}

void dumpSampleBuffer(int *buf, int max)
{
  int i;
  Serial.print("m_sampleBuffer => {");
  for(i = 1; i < max ; i++)
  {
    
    Serial.print(buf[i]);
    Serial.print(",");
  }
  Serial.println("}");
}
void BigDomeButton::sample()
{
  int i;
  
  int currentValue = digitalRead(m_buttonPin);
  for(i = 1; i < BDB_BUFFERLENGTH; i++){
    m_sampleBuffer[i-1] = m_sampleBuffer[i];
  }
  m_sampleBuffer[BDB_BUFFERLENGTH - 1] = currentValue;
  //dumpSampleBuffer(m_sampleBuffer, BDB_BUFFERLENGTH);
  bool complete = true;
  for(i = 0; i < BDB_BUFFERLENGTH; i++){
    if (m_sampleBuffer[i] != currentValue){
      
      complete = false;
      break;
    }
  }
  
  if (complete && m_lastState != currentValue){
    m_lastState = currentValue;
    callback(currentValue);
  }
}

void BigDomeButton::setButtonEventHandler(BigDomeEventHandler eh)
{
  callback = eh;
}

void BigDomeButton::setLedBrightness(int brightness)
{
  analogWrite(m_pwmLedPin, brightness);
}
