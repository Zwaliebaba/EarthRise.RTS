#include "Time.h"

DefineFunction(Time_Current)
{
  Time self;
  time_t time = std::time(nullptr);
  std::tm localTime;
  localtime_s(&localTime, &time);

  self.second = localTime.tm_sec;
  self.minute = localTime.tm_min;
  self.hour = localTime.tm_hour;
  self.day = localTime.tm_mday;
  self.month = localTime.tm_mon + 1;
  self.year = 1900 + localTime.tm_year;
  return self;
}
