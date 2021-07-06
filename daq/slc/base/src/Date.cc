/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/base/Date.h"

#include <cstring>

using namespace Belle2;

Date::Date(time_t time)
{
  set(time);
}

Date::Date(const Date& date)
{
  set(date.m_time);
}

Date::Date()
{
  set();
}

Date::~Date() {}

void Date::set()
{
  m_time = time(NULL);
  m_tm = localtime(&m_time);
}

void Date::set(time_t time)
{
  m_time = time;
  m_tm = localtime(&m_time);
}

int Date::getSecond() const
{
  return m_tm->tm_sec;
}

int Date::getMinitue() const
{
  return m_tm->tm_min;
}

int Date::getHour() const
{
  return m_tm->tm_hour;
}

int Date::getDay() const
{
  return m_tm->tm_mday;
}

int Date::getMonth() const
{
  return m_tm->tm_mon + 1;
}

int Date::getYear() const
{
  return m_tm->tm_year + 1900;
}

const char* Date::toString(const char* format) const
{
  memset(m_str, 0, sizeof(m_str));
  if (format == NULL) {
    strftime(m_str, 31, "%Y-%m-%d %H:%M:%S", m_tm);
  } else {
    strftime(m_str, 31, format, m_tm);
  }
  return m_str;
}
