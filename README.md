# Basic Heating House System
This code was my first official full program coded in C++ implemented in an Arduino Uno Board. It was an exercise for a subject in colleagues, and it basically is a basic program to create and control a basic domotic house which includes only the heating system part.

The main idea is we a house that contains:
  - Two solar collector
  - Multiple independant heating zones (3 in this example, but could be more)
  - Each zone has its own temperature sensors and a timer to set on and off the heat.
  - A on/off button to turn on and off all the house heating system.
  - A travel button to let the system know we'll be out for a couple days.
  - A reset button to reverse all the manual changes and left everything back to factory settings.

Each zone will be independant to the others, in temperature and timer.
