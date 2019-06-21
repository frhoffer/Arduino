# Arduino
Arduino and Nodemcu Projects

Digital Scoreboard (Arduino):
  This sketch is the DIY Volleyball LED Scoreboard using an Arduino Uno. This uses a customized ShiftSTR Library.
  
Oil Tank (NODEMCU):
  This sketch mesures the amount of fuel left in my oil tank using NODEMCU and JSN-SR04T. This uses The NewPing Library (v1.91) slightly modified to work with nodemcu. https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
  I made changes to NewPing.h ONE_PIN_ENABLED false. Changed NewPing.cpp changed delayMicroseconds(10); to delayMicroseconds(20);
  
Stranger Things (NODEMCU):
  This sketch is using the FastLED libraries and 
