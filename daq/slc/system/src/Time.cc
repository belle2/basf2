/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/Time.h"

#include "daq/slc/base/Reader.h"
#include "daq/slc/base/Writer.h"

#include <sys/time.h>

#include <cmath>
#include <cstdlib>
#include <sstream>

using namespace Belle2;

#define MEGA 1000000

Time::Time()
{
  timeval tv;
  gettimeofday(&tv, 0);
  m_s = tv.tv_sec;
  m_us = tv.tv_usec;
}

Time::Time(const double t)
{
  set(t);
}

Time::Time(const long s, const long us)
  : m_s(s), m_us(us)
{
  adjust();
}

Time::~Time()
{
}

void Time::adjust()
{
  if (MEGA <= labs(m_us)) {
    m_s += m_us / MEGA;
    m_us = m_us - (m_us / MEGA) * MEGA;
  }
  if (0 < m_s && m_us < 0) {
    m_s--;
    m_us += MEGA;
  }
  if (m_s < 0 && 0 < m_us) {
    m_s++;
    m_us -= MEGA;
  }
}

void Time::clear()
{
  m_s = m_us = 0;
}

void Time::set()
{
  timeval tv;
  gettimeofday(&tv, 0);
  m_s = tv.tv_sec;
  m_us = tv.tv_usec;
}

void Time::set(const double t)
{
  double s, us;
  us = modf(t, &s);
  m_s = (long)s;
  m_us = (long)(us * 1000000);
  adjust();
}

void Time::set(const long s, const long us)
{
  m_s = s;
  m_us = us;
  adjust();
}

double Time::get() const
{
  return (double)m_s + ((double)m_us) / 1000000.;
}

long Time::getSecond() const
{
  return m_s;
}

long Time::getMicroSecond() const
{
  return m_us;
}

std::string Time::toString() const
{
  std::stringstream ss;
  ss << m_s << "." << m_us;
  return ss.str();
}

void Time::readObject(Reader& r)
{
  m_s = r.readLong();
  m_us = r.readLong();
}

void Time::writeObject(Writer& w) const
{
  w.writeLong(m_s);
  w.writeLong(m_us);
}

Time& Time::operator = (const Time& t)
{
  m_s =  t.m_s;
  m_us =  t.m_us;
  return *this;
}

bool Time::operator == (const Time& t) const
{
  return (m_s == t.m_s) && (m_us == t.m_us);
}

Time Time::operator + (const Time& t) const
{
  Time tt = *this;
  tt.m_s += t.m_s;
  tt.m_us += t.m_us;
  return tt;
}

Time Time::operator - (const Time& t) const
{
  Time tt = *this;
  tt.m_s -= t.m_s;
  tt.m_us -= t.m_us;
  return tt;
}

