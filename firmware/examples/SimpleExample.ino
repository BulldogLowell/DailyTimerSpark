/*
DailyTimer.h Library
BulldogLowell@gmail.com
April, 2016
** Library provides tools to set daily timers for control of devices such as lamps, appliances, etc. Developed primarilary for houshold presence simulation.
** Allows for setting ON and OFF times, days of the week (i.e. weekends, weekdays, Fridays) and the use of random on/off times using this Constructor:
   DailyTimer myTimer( START_HOUR, START_MINUTE, END_HOUR, END_MINUTE, DAYS_OF_WEEK, RANDOM or FIXED)

** Timers may bridge midnight, simply enter times accordingly:
   DailyTimer myTimer( 18, 30,  1, 45, WEEKENDS, FIXED);  // starts at 6:30pm Saturdays and Sundays and ends at 1:45am the next day.

** Automatically sets correct timer state on powerup, using isActive() in loop(), see example below.
** Random start and/or end times using this member function:
   myTimer.setRandomOffset(30, RANDOM_START);  //  Can be RANDOM_START, RANDOM_END, or both (RANDOM)  default random offfset is 15min

** Random days of week using this member function:
   myTimer.setRandomDays(4); // will select four days of the week, randomly.  Useful to run this member function once a week, for example.

** Select custom days of the week using this member function:
   myTimer.setDaysActive(0b10101010);  // e.g.Sunday, Tuesday, Thursday and Saturday Note: Days in this order:  0bSMTWTFS0 <- LSB is zero

** Set a timed event with just the start time as a trigger:
   myTimer.startTrigger();  // will return true when Start Time is passed if today is an active day. use it in loop()

** you can return the active days using this getter:
   byte myByte = myTimer.getDays();  // returns a byte... Sunday is the MSB, Saturday is the LSB << 1

** Dynamically set your start or end time (i.e. using some type of celestial function or web call to determine Sunrise or Sunset times)
   myTimer.setStartTime(byte hour, byte minute);
   myTimer.setEndTime(byte hour, byte minute);

*/
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//Program Start

#include "DailyTimerSpark.h"


DailyTimer timer1(18, 30,  1, 30, EVERY_DAY, RANDOM_END, customSeedGenerator);      // optional callback function for random number generation, see below example
DailyTimer timer2(12,  0, 13,  0, SATURDAY, FIXED);                                 // default is FIXED, this will randomize the start time only
DailyTimer timer3( 8, 30, 23, 30, WEEKENDS);                                        // creates with a default fixed start time and end time
bool timer1_LastState = false;

uint32_t lastUpdateTime = 0;

void setup()
{
  timer3.begin(); // use this for syncing the state change tools startTrigger() and endTrigger() on startup
  timer2.begin();
  timer1.begin();
  Serial.begin(9600);
  pinMode(D7, OUTPUT);
  timer1.setRandomOffset(15, RANDOM_END);
  //timer1.setDaysActive(WEEKDAYS);             // Set the timer to be active on weekdays
  //timer1.setDaysActive(B10101010);            // or define a custom day mask... SMTWTFS0  example: Sunday, Tuesday, Yhursday, Saturday
  //timer1.setRandomDays(4);                    // or four random days per week
  //timer3.setRandomOffset(5);                  // Change random start time, +/- 5 minutes... max 59 mins, default is 15mins
  //Serial.println(timer1.getDays(), BIN);      // getDays() returns active days as a byte in the format above
}

void loop()
{
  bool timerState = timer1.isActive();  //State Change method this block
  if(timerState != timer1_LastState)
  {
    if(timerState)
    {
      digitalWrite(D7, HIGH);
      Serial.println("LED is ON");
      Particle.publish("DailyTimer", "LED is ON", 60, PRIVATE);
    }
    else
    {
      digitalWrite(D7, LOW);
      Serial.println("LED is OFF");
      Particle.publish("DailyTimer", "LED is OFF", 60, PRIVATE);
    }
    timer1_LastState = timerState;
  }

  if(timer2.startTrigger())  // Boundary method following blocks
  {
    Serial.println("Timer 2 FIRED!");
    Particle.publish("DailyTimer", "Timer2 Fired", 60, PRIVATE);
  }
  if(timer3.startTrigger())
  {
    Serial.println("Timer 3 FIRED!");
    Particle.publish("DailyTimer", "Timer3 Fired", 60, PRIVATE);
  }
  if(timer3.endTrigger())
  {
    Serial.println("Timer 3 Expired!");
    Particle.publish("DailyTimer", "Timer3 is Inactive", 60, PRIVATE);
  }
  if(millis() - lastUpdateTime > 1000UL)
  {
    char timeBuffer[32] = "";
    Serial.println(Time.timeStr());
    lastUpdateTime += 1000UL;
  }
}

uint32_t customSeedGenerator()
{
  Serial.println("New Seed Created");
  return  random(0xFFFFFFFF);
}
