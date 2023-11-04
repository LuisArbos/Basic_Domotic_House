/*
Write a code to control an inteligent/domotic house system. 
The house will have a central solar collector system and some different house zones, each one with his own time on and time off. 
With these times the heating of each zone will be activated and deactivated. There are also some extra options, as a manually on/off button for all the house, a reset in case we want everything to come back to factory options,
and also a button travel to keep the house hot while we're not at home.
*/

#include <TimerOne.h>

//FACTORY OPTIONS - All this information will be stored inside the EEPROM
const int hour1 = 20, hour2 = 23, min1 = 30, sec = 0;
const float gain = 1.5, offset = 0.5, hysteresis = 1.5, targetTemperature = 25.0, accumulatorTemperature = 40.0, colectorTemperature = 60.0, activeTimeMin = 20; 
const int buttonOnOff = 12, buttonReset = 11, buttonTravel = 10; //Outputs for the different buttons we need.

//Structs for every part of the project
typedef struct t_time { //Clock struct
  int secondsCount, minCount, hourCount;
};

typedef struct t_solar_collector{ //Solar Collector struct
  float temperature;
  short int pump, valve;
};

typedef struct t_heating_zone{ //Heating zone struct
  float hysteresis, gain, targetTemperature;
  short int valve;
};

typedef struct machine { //Central struct of the system, will include the clock, the solar collector and the heating zones. (We'll have 3 heating zones and 2 solar collectors)
  t_time newClock, timeON[4], timeOFF[4], activeTime; //We set these variable to 4, instead of 3, because we'll have a problem if we set it to 3.
  t_solar_collector solarCollector[2];
  t_heating_zone heatingZone[3];
  short int mainPump, mainValve, boilerOnOff;
  float accumulatorTemperature;
};
machine myMachine;

//FUNCTIONS
void SetTime(t_time *x) { //Function to set/modify the time
  char Time[9] = __TIME__;
  x->secondsCount = Time[7] - 48 + (Time[6] - 48) * 10;
  x->minCount = Time[4] - 48 + (Time[3] - 48) * 10;
  x->hourCount = Time[1] - 48 + (Time[0] - 48) * 10;
}
void Shell(t_time *x, int h, int m, int s) {
  x->secondsCount = s;
  x->minCount = m;
  x->hourCount = h;
}
int auxiliar1, auxiliar2;

void CompTime(t_time *x, t_time *y, int *z) { //Function to compare the time, so we know when to activate and deactivate each zone, z is the output, 0 unactive, 1 activate, 2 deactivate
  if ((x->secondsCount + x->minCount * 60 + x->hourCount * 3600) > (y->secondsCount + y->minCount * 60 + y->hourCount * 3600)) {
    *z = 0;
  }
  else if ((x->secondsCount + x->minCount * 60 + x->hourCount * 3600) == (y->secondsCount + y->minCount * 60 + y->hourCount * 3600)) {
    *z = 1;
  }
  else if ((x->secondsCount + x->minCount * 60 + x->hourCount * 3600) < (y->secondsCount + y->minCount * 60 + y->hourCount * 3600)) {
    *z = 2;
  }
}


void setup() {
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(myClock);
  Serial.begin(9600);
  //SET THE PINS
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,INPUT_PULLUP);
  pinMode(7,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(buttonOnOff,INPUT_PULLUP);
  pinMode(buttonReset,INPUT_PULLUP);
  pinMode(buttonTravel,INPUT_PULLUP);
  //INTIAL SET VALUES
  SetTime(&myMachine.newClock);
  Shell(&myMachine.timeON[1], hour1, min1, sec);
  Shell(&myMachine.timeON[2], hour1, min1, sec);
  Shell(&myMachine.timeON[3], hour1, min1, sec);
  Shell(&myMachine.timeOFF[1], hour2, min1, sec);
  Shell(&myMachine.timeOFF[2], hour2, min1, sec);
  Shell(&myMachine.timeOFF[3], hour2, min1, sec);
  Serial.println("Console v1.0 ON");
  Serial.println("Write help to see all available commands");
}

//Clock function
void myClock() {
  if (myMachine.newClock.secondsCount < 60) {
    myMachine.newClock.secondsCount = myMachine.newClock.secondsCount + 1;
  }
  if (myMachine.newClock.secondsCount == 60 && myMachine.newClock.minCount < 59) {
    myMachine.newClock.secondsCount = 0;
    myMachine.newClock.minCount = myMachine.newClock.minCount + 1;
  }
  else if (myMachine.newClock.secondsCount == 60 && myMachine.newClock.minCount == 59 && myMachine.newClock.hourCount < 23) {
    myMachine.newClock.secondsCount = 0;
    myMachine.newClock.minCount = 0;
    myMachine.newClock.hourCount = myMachine.newClock.hourCount + 1;
  }
  else if (myMachine.newClock.secondsCount == 60 && myMachine.newClock.minCount == 59 && myMachine.newClock.hourCount == 23) {
    myMachine.newClock.secondsCount = 0;
    myMachine.newClock.minCount = 0;
    myMachine.newClock.hourCount = 0;
  }
}

void loop() {
  //Analogic inputs to check all zones temperature
  float tempZone1 = (analogRead(A0)*(5.0/1023.0))*14.0-10.0; //With these values we'll be good between -10 ºC and 60 ºC and a voltage between 0-5V.
  float tempZone2 = (analogRead(A1)*(5.0/1023.0))*14.0-10.0; //With these values we'll be good between -10 ºC and 60 ºC and a voltage between 0-5V.
  float tempZone3 = (analogRead(A2)*(5.0/1023.0))*14.0-10.0; //With these values we'll be good between -10 ºC and 60 ºC and a voltage between 0-5V.
  float tempCol1 = (analogRead(A3)*(5.0/1023.0))*25.0-25.0; //With these values we'll be good between -25 ºC and 100 ºC and a voltage between 0-5V.
  float tempCol2 = (analogRead(A4)*(5.0/1023.0))*25.0-25.0; //With these values we'll be good between -25 ºC and 100 ºC and a voltage between 0-5V.
  float tempAcum = (analogRead(A5)*(5.0/1023.0))*12.0-10.0; //With these values we'll be good between -10 ºC and 50 ºC and a voltage between 0-5V.
  //Serial.print(tempZone1);Serial.print(' ');Serial.print(tempZone2);Serial.print(' ');Serial.print(tempZone3);Serial.print(' ');Serial.print(tempCol1);Serial.print(' ');Serial.print(tempCol2);Serial.print(' ');Serial.print(tempAcum);Serial.println(' ');
  //Serial.print(digitalRead(buttonOnOff));Serial.print(' ');Serial.print(digitalRead(buttonReset));Serial.print(' ');Serial.println(digitalRead(buttonTravel));
  
  int secondsCopy, minCopy, hourCopy;
  char myTime[9], ONTimez1[9], OFFTimez1[9], ONTimez2[9], OFFTimez2[9], ONTimez3[9], OFFTimez3[9];
  String cmd, Zone, On, Off, th, tm, ts, th2, tm2, ts2;
  int zone, timeh, timem, times, timeh2, timem2, times2;
  noInterrupts();
  secondsCopy = myMachine.newClock.secondsCount;
  minCopy = myMachine.newClock.minCount;
  hourCopy = myMachine.newClock.hourCount;
  interrupts();
  String Seconds(secondsCopy);
  String Mins(minCopy);
  String Hours(hourCopy);
  CompTime(&myMachine.newClock, &myMachine.timeON[1], auxiliar1); //Comparing the time to know if we need to activate the heating system
  CompTime(&myMachine.newClock, &myMachine.timeOFF[1], auxiliar2); //Comparing the time to know if we need to deactivate the heating system
  sprintf(myTime, "%02d:%02d:%02d", hourCopy, minCopy, secondsCopy);

  if (Serial.available()) {
    cmd = Serial.readStringUntil(' ');
    if (cmd == "SetZone") { //Set the zone and all the info, zone, on (hour, min and second), off (hour, min and second)
      Zone = Serial.readStringUntil(' ');
      On = Serial.readStringUntil(' ');
      th = Serial.readStringUntil(':');
      tm = Serial.readStringUntil(':');
      ts = Serial.readStringUntil(' ');
      Off = Serial.readStringUntil(' ');
      th2 = Serial.readStringUntil(':');
      tm2 = Serial.readStringUntil(':');
      ts2 = Serial.readString();
      zone = Zone.toInt();
      timeh = th.toInt();
      timeh2 = th2.toInt();
      timem = tm.toInt();
      timem2 = tm2.toInt();
      times = ts.toInt();
      times2 = ts2.toInt();
      if (zone == 1) {
        Serial.println("Command accepted " + cmd + " zone " + zone);
        Shell(&myMachine.timeON[1], timeh, timem, times);
        Shell(&myMachine.timeOFF[1], timeh2, timem2, times2);
        Serial.println("ON Time has been updated for zone 1, new ON time: " + th + ":" + tm + ":" + ts);
        Serial.println("OFF Time has been updated for zone 1, new OFF time: " + th2 + ":" + tm2 + ":" + ts2);
        Serial.println(" ");
      }
      else if (zone == 2) {
        Serial.println("Command accepted " + cmd + " zone " + zone);
        Shell(&myMachine.timeON[2], timeh, timem, times);
        Shell(&myMachine.timeOFF[2], timeh2, timem2, times2);
        Serial.println("ON Time has been updated for zone 2, new ON time: " + th + ":" + tm + ":" + ts);
        Serial.println("OFF Time has been updated for zone 2, new OFF time: " + th2 + ":" + tm2 + ":" + ts2);
        Serial.println(" ");
      }
      else if (zone == 3) {
        Serial.println("Command accepted " + cmd + " zone " + zone);
        Shell(&myMachine.timeON[3], timeh, timem, times);
        Shell(&myMachine.timeOFF[3], timeh2, timem2, times2);
        Serial.println("ON Time has been updated for zone 3, new ON time: " + th + ":" + tm + ":" + ts);
        Serial.println("OFF Time has been updated for zone 3, new OFF time:  " + th2 + ":" + tm2 + ":" + ts2);
        Serial.println(" ");
      }
      else {
        Serial.print("Incorrect zone, please enter one of the following valid zone values:");
        Serial.println(" 1 2 3");
      }
    }
    //help to see all available commands
    else if (cmd == "help" ) {
      Serial.println("Available commands:");
      Serial.println("SetTime hh:mm:ss to set the clock");
      Serial.println("SetZone zone ON hh:mm:ss OFF hh:mm:ss to configure on and off time for a zone");
      Serial.println("Config to print the current configuration");
      Serial.println("help to see all available commands");
      Serial.println(" ");
    }
    //Set Time to change the clock time
    else if (cmd == "SetTime") {
      th = Serial.readStringUntil(':');
      tm = Serial.readStringUntil(':');
      ts = Serial.readString();
      timeh = th.toInt();
      timem = tm.toInt();
      times = ts.toInt();
      Serial.println("Comannd accepted " + cmd);
      Shell(&myMachine.newClock, timeh, timem, times);
      Serial.println("Time updated, new time: " + th + ":" + tm + ":" + ts);
      Serial.println(" ");
    }
    //Config to print all the current config on screen
    else if (cmd == "Config") {
      sprintf(ONTimez1, "%02d:%02d:%02d", myMachine.timeON[1].hourCount, myMachine.timeON[1].minCount, myMachine.timeON[1].secondsCount);
      sprintf(OFFTimez1, "%02d:%02d:%02d", myMachine.timeOFF[1].hourCount, myMachine.timeOFF[1].minCount, myMachine.timeOFF[1].secondsCount);
      sprintf(ONTimez2, "%02d:%02d:%02d", myMachine.timeON[2].hourCount, myMachine.timeON[2].minCount, myMachine.timeON[2].secondsCount);
      sprintf(OFFTimez2, "%02d:%02d:%02d", myMachine.timeOFF[2].hourCount, myMachine.timeOFF[2].minCount, myMachine.timeOFF[2].secondsCount);
      sprintf(ONTimez3, "%02d:%02d:%02d", myMachine.timeON[3].hourCount, myMachine.timeON[3].minCount, myMachine.timeON[3].secondsCount);
      sprintf(OFFTimez3, "%02d:%02d:%02d", myMachine.timeOFF[3].hourCount, myMachine.timeOFF[3].minCount, myMachine.timeOFF[3].secondsCount);
      Serial.print("Current time "); Serial.println(myTime);
      String line1 = "             ON time                   OFF time";
      String line2 = "Zone 1      " + String(ONTimez1) + "                 " + String(OFFTimez1);
      String line3 = "Zone 2      " + String(ONTimez2) + "                 " + String(OFFTimez2);
      String line4 = "Zone 3      " + String(ONTimez3) + "                 "+ String(OFFTimez3);
      String Configuration[4][1] = {
        {line1},
        {line2},
        {line3},
        {line4}
      };

      Serial.println(line1); 
      Serial.println(line2);
      Serial.println(line3); 
      Serial.println(line4);
      
  }
    // Not valid command
    else {
      Serial.println("Command not valid " + cmd);
      Serial.println("Please write help to see all available commands ");
    }
  }
}
