#include "daq/slc/base/Date.h"

#include <cstring>
#include <sstream>
#include <iostream>

using namespace Belle2;

Date::Date(time_t time)
{
  set(time);
}

Date::Date()
{
  set();
}

Date::~Date() throw() {}

void Date::set() throw()
{
  m_time = time(NULL);
  m_tm = localtime(&m_time);
}

void Date::set(time_t time) throw()
{
  m_time = time;
  m_tm = localtime(&m_time);
}

int Date::getSecond() const throw()
{
  return m_tm->tm_sec;
}

int Date::getMinitue() const throw()
{
  return m_tm->tm_min;
}

int Date::getHour() const throw()
{
  return m_tm->tm_hour;
}

int Date::getDay() const throw()
{
  return m_tm->tm_mday;
}

int Date::getMonth() const throw()
{
  return m_tm->tm_mon + 1;
}

int Date::getYear() const throw()
{
  return m_tm->tm_year + 1900;
}

const char* Date::toString(const char* format) const throw()
{
  static char ss[256];
  if (format == NULL) {
    strftime(ss, 255, "%Y-%m-%d %H:%M:%S", m_tm);
  } else {
    strftime(ss, 255, format, m_tm);
  }
  return ss;
}
