#ifndef DailyTimerSpark_h
#define DailyTimerSpark_h

#include "application.h"


enum EventDays{SUNDAY = 0, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, WEEKENDS, WEEKDAYS, EVERY_DAY};

enum RandomTime{FIXED, RANDOM, RANDOM_START, RANDOM_END};

struct TimerTime{ // bounded 00:00 to 23:59
  uint8_t hour;
  uint8_t minute;
};

const byte dayTemplate[10] = {
  /*SMTWTFSS*/                    // the mask is set with an extra bit for determining off times for days of the week where off time is earlier than on time (i.e. stradles Midnight)
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  0b00000010,
  0b10000010, // Weekends
  0b01111100, // Weekdays
  0b11111110  // Everyday
};

class DailyTimer{
  public:
    DailyTimer(byte StartHour, byte StartMinute, byte endHour, byte endMinute, EventDays DayOfWeek);
    DailyTimer(byte startHour, byte startMinute, byte endHour, byte endMinute, EventDays DayOfWeek, RandomTime randomTime);
    DailyTimer(byte startHour, byte startMinute, byte endHour, byte endMinute, EventDays DayOfWeek, RandomTime randomTime, uint32_t (*randomCallback)());
    void setRandomOffset(int random_minutes, RandomTime randomSetting = RANDOM);
    void setDaysActive(EventDays days);
    void setDaysActive(byte activeDays);
    byte setRandomDays(byte number_Days);
    bool isActive();
    bool startTrigger();
    bool endTrigger();
    bool begin();
    void setStartTime(byte hour, byte minute);
    void setEndTime(byte hour, byte minute);
    byte getDays();

  private:
    byte _onMask  = dayTemplate[EVERY_DAY];
    byte _offMask = dayTemplate[EVERY_DAY];
    bool _state;
    TimerTime _startTime;
    TimerTime _endTime;
    TimerTime _randomStartTime;
    TimerTime _randomEndTime;
    byte _randomType = FIXED;
    byte _currentDay = 0;
    uint8_t _offset = 15;
    time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);
    uint32_t (*_randomCallback)();
};

#endif
