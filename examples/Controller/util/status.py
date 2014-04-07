#!/usr/bin/python

import serial
from serial import SerialException
from pygame import mixer

mixer.init()
mixer.music.load('/usr/share/evolution/3.6/sounds/default_alarm.wav')   
#mixer.music.load('/home/ferengee/Music/bunny.mp3')

portnumber = -1

def connect(portnumber):
  while portnumber < 4: 
    portnumber += 1
    try:
      connection = serial.Serial('/dev/ttyACM%d' % portnumber)
      return connection
    except SerialException as error:
      print("Failed to connect to port: %d" % portnumber)
      print(error)
      

def handle(cmd, value):
    
  if cmd == 'Alarm':
    if value == 'on':
      mixer.music.rewind()
      mixer.music.play()
    else:
      mixer.music.fadeout(200000)  
  if cmd == 'Winner':
    print("Button with ID: %s is the winner!" % value)
      
connection = connect(portnumber)      
running = True
while running:
  try:  
    cmd, value = connection.readline().strip().split(":")
    handle(cmd, value)
    
  except SerialException as error:
    print(error)
    connection = connect(portnumber)
  except KeyboardInterrupt:
    running = False
  except:
    pass
    
      
    
  