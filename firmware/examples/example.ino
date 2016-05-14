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
/*
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Use this webhook to automatially get sunrise/sunset times plus your DST offset from weatherunderground:

sun_time.json:

{
	"event": "sun_time",
	"url": "http://api.wunderground.com/api/getYourOwnApiKey/astronomy/q/{{my-state}}/{{my-city}}.json",
	"requestType": "POST",
	"headers": null,
	"query": null,
	"responseTemplate": "{{#sun_phase}}{{sunrise.hour}}~{{sunrise.minute}}~{{sunset.hour}}~{{sunset.minute}}~{{#moon_phase}}{{current_time.hour}}~{{current_time.minute}}{{/moon_phase}}~{{/sun_phase}}",
	"responseTopic": "{{SPARK_CORE_ID}}_sun_time",
	"json": null,
	"auth": null,
	"coreid": null,
	"deviceid": null,
	"mydevices": true
}

*/
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//Program Start

#include "DailyTimerSpark.h"
#define SUN_UPDATE_INTERVAL 30*60*1000UL

const char* cityLocation = "Princeton";  //City for my Photon
const char* stateLocation = "NJ";     // State for my Photon

char publishString[125] = "";
char timeVars[125] = "";

struct deviceTime{
  int Hour;
  int Minute;
};

deviceTime sunset  = {18,30};
deviceTime sunrise = { 5,30};

DailyTimer timer1(sunset.Hour, sunset.Minute, 1, 30, EVERY_DAY, RANDOM_END, customSeedGenerator);      // optional callback function for random number generation, see below example
DailyTimer timer2(12, 00, 13,  0, SATURDAY, FIXED);                                                    // default is FIXED, this will randomize the start time only
DailyTimer timer3( 8, 30, 23, 30, WEEKENDS);                                                           // creates with a default fixed start time and end time
bool timer1_LastState = false;
bool timerState;

uint32_t lastUpdateTime = 0;
uint32_t lastSunTime = 0;

void setup()
{
  timer3.begin(); // use this for syncing the state change tools startTrigger() and endTrigger() on startup
  timer2.begin();
  timer1.begin();
  Serial.begin(9600);
  pinMode(D7, OUTPUT);
  strcpy(publishString, "{\"my-city\": \"");
  strcat(publishString, cityLocation);
  strcat(publishString, "\", \"my-state\": \"");
  strcat(publishString, stateLocation);
  strcat(publishString, "\" }");
  Particle.function("ToggleLED", toggleLED);
  Particle.subscribe(System.deviceID(), webhookHandler, MY_DEVICES);
  Particle.variable("State", timerState);
  Particle.variable("SunTime", timeVars, STRING);
  timer1.setRandomOffset(45, RANDOM_END);
  //timer1.setDaysActive(WEEKDAYS);             // Set the timer to be active on weekdays
  //timer1.setDaysActive(B10101010);            // or define a custom day mask... SMTWTFS0  example: Sunday, Tuesday, Yhursday, Saturday
  //timer1.setRandomDays(4);                    // or four random days per week
  //timer3.setRandomOffset(5);                  // Change random start time, +/- 5 minutes... max 59 mins, default is 15mins
  //Serial.println(timer1.getDays(), BIN);      // getDays() returns active days as a byte in the format above
  Particle.publish("pushover", "Timer Restart", 60, PRIVATE);
  Particle.publish("sun_time", publishString, 60, PRIVATE);
}

void loop()
{
  timerState = timer1.isActive();
  if(timerState != timer1_LastState)
  {
    if(timerState)
    {
      digitalWrite(D7, HIGH);
      Serial.println("LED is ON");
      Particle.publish("pushover", "LED is ON", 60, PRIVATE);
    }
    else
    {
      digitalWrite(D7, LOW);
      Serial.println("LED is OFF");
      Particle.publish("pushover", "LED is OFF", 60, PRIVATE);
    }
    timer1_LastState = timerState;
  }
  if(timer2.startTrigger())
  {
    Serial.println("Timer 2 FIRED!");
    Particle.publish("pushover", "Timer2 Fired", 60, PRIVATE);
  }
  if(timer3.startTrigger())
  {
    Serial.println("Timer 3 FIRED!");
    Particle.publish("pushover", "Timer3 Fired", 60, PRIVATE);
  }
  if(timer3.endTrigger())
  {
    Serial.println("Timer 3 Expired!");
    Particle.publish("pushover", "Timer3 is Inactive", 60, PRIVATE);
  }
  if(millis() - lastUpdateTime > 1000UL)
  {
    char timeBuffer[32] = "";
    Serial.println(Time.timeStr());
    lastUpdateTime += 1000UL;
  }
  if(millis() - lastSunTime > SUN_UPDATE_INTERVAL)
  {
    Particle.publish("sun_time", publishString, 60, PRIVATE);
    lastSunTime += SUN_UPDATE_INTERVAL;
  }
}

uint32_t customSeedGenerator()
{
  Serial.println("New Seed Created");
  return  random(0xFFFFFFFF);
}

void webhookHandler(const char *event, const char *data)
{
  if (strstr(event, "sun_time"))
  {
    gotSunTime(event, data);
  }
}

void gotSunTime(const char * event, const char * data)
{
  deviceTime currentTime;
  char sunriseBuffer[25] = "";
  strcpy(sunriseBuffer, data);
  sunrise.Hour = atoi(strtok(sunriseBuffer, "\"~"));
  sunrise.Minute = atoi(strtok(NULL, "~"));
  sunset.Hour = atoi(strtok(NULL, "~"));
  sunset.Minute = atoi(strtok(NULL, "~"));
  currentTime.Hour = atoi(strtok(NULL, "~"));
  currentTime.Minute = atoi(strtok(NULL, "~"));
  Time.zone(0);
  Time.zone(utcOffset(Time.hour(), currentTime.Hour));
  timer1.setStartTime(sunset.Hour, sunset.Minute);
  //timer1.setEndTime(sunrise.Hour, sunrise.Minute);
  char buffer[125] = "";
  sprintf(buffer, "Sunrise=%02d:%02d, Sunset=%02d:%02d", sunrise.Hour, sunrise.Minute, sunset.Hour, sunset.Minute);
  strcpy(timeVars, buffer);
}

int utcOffset(int utcHour, int localHour)  // sorry Baker Island, this won't work for you (UTC-12)
{
  if (utcHour == localHour)
  {
    return 0;
  }
  else if (utcHour > localHour)
  {
    if (utcHour - localHour >= 12)
    {
      return 24 - utcHour + localHour;
    }
    else
    {
      return localHour - utcHour;
    }
  }
  else
  {
    if (localHour - utcHour > 12)
    {
      return  localHour - 24 - utcHour;
    }
    else
    {
      return localHour - utcHour;
    }
  }
}

int toggleLED(String command)
{
  if (command == "1")
  {
    digitalWrite(D7, HIGH);
  }
  else if (command == "0")
  {
    digitalWrite(D7, LOW);
  }
  else if(command == "-1")
  {
    digitalWrite(D7, !digitalRead(D7));
  }
  char message[125] = "";
  sprintf(message, "LED is %s", digitalRead(D7)? "ON" : "OFF");
  Particle.publish("pushover", message, 60, PRIVATE);
  return digitalRead(D7);
}
