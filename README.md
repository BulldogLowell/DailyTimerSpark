# DailyTimerSpark


DailyTimer.h Library
BulldogLowell@gmail.com
April, 2016


Library provides tools to set daily timers for control of devices such as lamps, appliances, etc. Developed primarilary for houshold presence simulation.
Allows for setting ON and OFF times, days of the week (i.e. weekends, weekdays, Fridays) and the use of random on/off times using this Constructor:
   DailyTimer myTimer( START_HOUR, START_MINUTE, END_HOUR, END_MINUTE, DAYS_OF_WEEK, RANDOM or FIXED)

Timers may bridge midnight, simply enter times accordingly:
   DailyTimer myTimer( 18, 30,  1, 45, WEEKENDS, FIXED);  // starts at 6:30pm Saturdays and Sundays and ends at 1:45am the next day.

Automatically sets correct timer state on powerup, using isActive() in loop(), see example below.
Random start and/or end times using this member function:
   myTimer.setRandomOffset(30, RANDOM_START);  //  Can be RANDOM_START, RANDOM_END, or both (RANDOM)  default random offfset is 15min

Random days of week using this member function:
   myTimer.setRandomDays(4); // will select four days of the week, randomly.  Useful to run this member function once a week, for example.

Select custom days of the week using this member function:
   myTimer.setDaysActive(0b10101010);  // e.g.Sunday, Tuesday, Thursday and Saturday Note: Days in this order:  0bSMTWTFS0 <- LSB is zero

Set a timed event with just the start time as a trigger:
   myTimer.startTrigger();  // will return true when Start Time is passed if today is an active day. use it in loop()

You can return the active days using this getter:
   byte myByte = myTimer.getDays();  // returns a byte... Sunday is the MSB, Saturday is the LSB << 1

Dynamically set your start or end time (i.e. using some type of celestial function or web call to determine Sunrise or Sunset times)
   myTimer.setStartTime(byte hour, byte minute);
   myTimer.setEndTime(byte hour, byte minute);


Use this webhook to automatially get sunrise/sunset times plus your DST offset from weatherunderground:

sun_time.json:
```
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
```

