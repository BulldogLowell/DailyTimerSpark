#include "DailyTimerSpark.h"
#include "application.h"


DailyTimer::DailyTimer(byte StartHour, byte StartMinute, byte endHour, byte endMinute, EventDays DayOfWeek)
{
  _startTime.hour = constrain(StartHour, 0, 23);
  _startTime.minute = constrain(StartMinute, 0, 59);
  _endTime.hour = constrain(endHour, 0, 23);
  _endTime.minute = constrain(endMinute, 0, 59);
  this->setDaysActive(dayTemplate[DayOfWeek]);
  _randomType = FIXED;
  _offset = 15;
  _randomCallback = NULL;
}

DailyTimer::DailyTimer(byte StartHour, byte StartMinute, byte endHour, byte endMinute, EventDays DayOfWeek, RandomTime randomTime)
{
  _startTime.hour = constrain(StartHour, 0, 23);
  _startTime.minute = constrain(StartMinute, 0, 59);
  _endTime.hour = constrain(endHour, 0, 23);
  _endTime.minute = constrain(endMinute, 0, 59);
  this->setDaysActive(dayTemplate[DayOfWeek]);
  _randomType = randomTime;
  _offset = 15;
  _randomCallback = NULL;
}

DailyTimer::DailyTimer(byte StartHour, byte StartMinute, byte endHour, byte endMinute, EventDays DayOfWeek, RandomTime randomTime, uint32_t (*randomCallback)())
{
  _startTime.hour = constrain(StartHour, 0, 23);
  _startTime.minute = constrain(StartMinute, 0, 59);
  _endTime.hour = constrain(endHour, 0, 23);
  _endTime.minute = constrain(endMinute, 0, 59);
  this->setDaysActive(dayTemplate[DayOfWeek]);
  _randomType = randomTime;
  _offset = 15;
  _randomCallback = randomCallback;
}

bool DailyTimer::begin()
{
  _state = this->isActive();
}

void DailyTimer::setDaysActive(EventDays days)
{
  _onMask = dayTemplate[days];
  _offMask = _onMask >> 1;
}
void DailyTimer::setDaysActive(byte activeDays)
{
  _onMask = activeDays;
  _offMask = _onMask >> 1;
}

bool DailyTimer::startTrigger()
{
  bool currentState = this->isActive();
  if (currentState != _state && currentState == true)
  {
    _state = currentState;
    return true;
  }
  return false;
}

bool DailyTimer::endTrigger()
{
  bool currentState = this->isActive();
  if (currentState != _state && currentState == false)
  {
    _state = currentState;
    return true;
  }
  return false;
}

void DailyTimer::setRandomOffset(int random_minutes, RandomTime randomSetting)
{
  _offset = constrain(random_minutes, 0, 59);
  if (_offset == 0)
  {
    _randomType = FIXED;
  }
  else
  {
    _randomType = randomSetting;
  }
}

void DailyTimer::setStartTime(byte hour, byte minute)
{
  _startTime.hour = constrain(hour, 0, 23);
  _startTime.minute = constrain(minute, 0, 59);
}

void DailyTimer::setEndTime(byte hour, byte minute)
{
  _endTime.hour = constrain(hour, 0, 23);
  _endTime.minute = constrain(minute, 0, 59);
}

byte DailyTimer::setRandomDays(byte number_Days)
{
  if (_randomCallback)
  {
    randomSeed(_randomCallback());
  }
  else
  {
    randomSeed(analogRead(A0));
  }
  byte mask = 0;
  byte array[8] = {0};
  for (int i = 0; i < number_Days; i++)
  {
    array[i] = 1;
  }
  for(int i = 0; i < 7; i++)
  {
    byte index = random(i, 7);
    byte temp = array[i];
    array[i] = array[index];
    array[index] = temp;
  }
  for (int i = 0; i < 7; i++)
  {
    mask |= (array[i] << i);
  }
  _onMask = mask << 1;
  return _onMask;
}

byte DailyTimer::getDays()
{
  return _onMask;
}

bool DailyTimer::isActive()
{
  if (_currentDay != Time.weekday() && _randomType) // once a day, generate new random offsets
  {
    if (_randomCallback)
    {
      randomSeed(_randomCallback());
    }
    else
    {
      randomSeed(analogRead(A0));
    }
    if (_randomType == RANDOM  || _randomType == RANDOM_START)
    {
      int hrs = _startTime.hour * 60 + _startTime.minute;
      hrs += constrain(random(-1 * _offset, _offset), 1, (24 * 60) - 1);
      _randomStartTime.minute = hrs % 60;
      _randomStartTime.hour = hrs / 60;
    }
    if (_randomType == RANDOM || _randomType == RANDOM_END)
    {
      int hrs = _endTime.hour * 60 + _endTime.minute;
      hrs += constrain(random(-1 * _offset, _offset), 1, (24 * 60) - 1);
      _randomEndTime.minute = hrs % 60;
      _randomEndTime.hour = hrs / 60;
    }
    _currentDay = Time.weekday();
  }
  time_t on_time = tmConvert_t(Time.year(), Time.month(), Time.day(), (_randomType == RANDOM || _randomType == RANDOM_START) ? _randomStartTime.hour : _startTime.hour, (_randomType == RANDOM || _randomType == RANDOM_START) ? _randomStartTime.minute : _startTime.minute, 0);
  time_t off_time = tmConvert_t(Time.year(), Time.month(), Time.day(), (_randomType == RANDOM || _randomType == RANDOM_END) ? _randomEndTime.hour : _endTime.hour, (_randomType == RANDOM || _randomType == RANDOM_END) ? _randomEndTime.minute : _endTime.minute, 0);
  time_t now_time = Time.local();
  byte weekDay = Time.weekday();
  byte today = 0b00000001 << (8 - weekDay);
  if (today & dayTemplate[SUNDAY])
  {
    today |= 0b00000001;
  }
  if ((today & _onMask) && (today & _offMask))  // if it is supposed to turn both on and off today
  {
    if (on_time < off_time)
    {
      return (now_time > on_time && now_time < off_time);
    }
    else if (off_time < on_time)
    {

      return (now_time > on_time || now_time < off_time);
    }
    else // if both on and off are set to the same time, I'm confused... so let's do nothing
    {
      return false;
    }
  }
  else if (today & _onMask)
  {
    if (on_time < off_time)
    {
      return (now_time > on_time && now_time < off_time);
    }
    else
    {
      return (now_time > on_time);
    }
  }
  else if (today & _offMask)
  {
    return now_time < off_time;
  }
  else
  {
    return false;
  }
}

time_t DailyTimer::tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  struct tm t;
  t.tm_year = YYYY-1900;
  t.tm_mon = MM - 1;
  t.tm_mday = DD;
  t.tm_hour = hh;
  t.tm_min = mm;
  t.tm_sec = ss;
  t.tm_isdst = 0;
  time_t t_of_day = mktime(&t);
  return t_of_day;
}
