#include "system/Time.h"

#include "base/Reader.h"
#include "base/Writer.h"

#include <sys/time.h>

#include <cmath>
#include <cstdlib>
#include <sstream>

using namespace Belle2;

#define MEGA 1000000

Time::Time() throw()
{
  timeval tv;
  gettimeofday(&tv, 0);
  _s = tv.tv_sec;
  _us = tv.tv_usec;
}

Time::Time(const double t) throw()
{
  set(t);
}

Time::Time(const long s, const long us) throw() : _s(s), _us(us)
{
  adjust();
}

Time::~Time() throw()
{
}

void Time::adjust() throw()
{
  if (MEGA <= labs(_us)) {
    _s += _us / MEGA;
    _us = _us - (_us / MEGA) * MEGA;
  }
  if (0 < _s && _us < 0) {
    _s--;
    _us += MEGA;
  }
  if (_s < 0 && 0 < _us) {
    _s++;
    _us -= MEGA;
  }
}

void Time::clear() throw()
{
  _s = _us = 0;
}

void Time::set() throw()
{
  timeval tv;
  gettimeofday(&tv, 0);
  _s = tv.tv_sec;
  _us = tv.tv_usec;
}

void Time::set(const double t) throw()
{
  double s, us;
  us = modf(t, &s);
  _s = (long)s;
  _us = (long)(us * 1000000);
  adjust();
}

void Time::set(const long s, const long us) throw()
{
  _s = s;
  _us = us;
  adjust();
}

double Time::get() const throw()
{
  return (double)_s + ((double)_us) / 1000000.;
}

long Time::getSecond() const throw()
{
  return _s;
}

long Time::getMicroSecond() const throw()
{
  return _us;
}

std::string Time::toString() const throw()
{
  std::stringstream ss;
  ss << _s << "." << _us;
  return ss.str();
}

void Time::readObject(Reader& r) throw(IOException)
{
  _s = r.readLong();
  _us = r.readLong();
}

void Time::writeObject(Writer& w) const throw(IOException)
{
  w.writeLong(_s);
  w.writeLong(_us);
}

Time& Time::operator = (const Time& t) throw()
{
  _s =  t._s;
  _us =  t._us;
  return *this;
}

bool Time::operator == (const Time& t) const throw()
{
  return (_s == t._s) && (_us == t._us);
}

Time Time::operator + (const Time& t) const throw()
{
  Time tt = *this;
  tt._s += t._s;
  tt._us += t._us;
  return tt;
}

Time Time::operator - (const Time& t) const throw()
{
  Time tt = *this;
  tt._s -= t._s;
  tt._us -= t._us;
  return tt;
}

