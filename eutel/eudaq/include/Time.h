#ifndef EUDAQ_INCLUDED_Time
#define EUDAQ_INCLUDED_Time

#include <eutel/eudaq/Platform.h>
#include <errno.h>
#include <ostream>
#include <iomanip>
#include <string>
#include <cstring>

#if EUDAQ_PLATFORM_IS(WIN32)
//# include <time.h>
//# include <afxwin.h>
# include <winsock2.h>
#else
# include <sys/time.h>
#endif

namespace eudaq {

  class Time {
  public:
    static const std::string DEFAULT_FORMAT;
    explicit Time(long sec, long usec = 0)
    {
      tv_usec = usec % 1000000;
      tv_sec = sec + usec / 1000000;
    }
    Time(int year, int month, int date, int hour = 0, int minute = 0, int sec = 0, int usec = 0);
    Time(timeval tv)
    {
      tv_usec = tv.tv_usec % 1000000;
      tv_sec = tv.tv_sec + tv.tv_usec / 1000000;
    }
    double Seconds() const { return tv_sec + tv_usec / 1e6; }
    Time& operator += (const timeval& other)
    {
      tv_usec += other.tv_usec;
      tv_sec += other.tv_sec + tv_usec / 1000000;
      tv_usec %= 1000000;
      return *this;
    }
    Time& operator -= (const timeval& other)
    {
      if (tv_usec < other.tv_usec) {
        tv_usec += 1000000 - other.tv_usec;
        tv_sec -= other.tv_sec + 1;
      } else {
        tv_usec -= other.tv_usec;
        tv_sec -= other.tv_sec;
      }
      return *this;
    }
    bool operator < (const timeval& other)
    {
      return tv_sec < other.tv_sec ||
             (tv_sec == other.tv_sec && tv_usec < other.tv_usec);
    }
    bool operator > (const timeval& other)
    {
      return tv_sec > other.tv_sec ||
             (tv_sec == other.tv_sec && tv_usec > other.tv_usec);
    }
    operator timeval() const
    {
      timeval tv;
      tv.tv_sec = tv_sec;
      tv.tv_usec = tv_usec;
      return tv;
    }
    std::string Formatted(const std::string& format = DEFAULT_FORMAT) const;
    static Time Current();
  private:
    long tv_sec;
    long tv_usec;
  };

  inline Time operator + (const timeval& lhs, const timeval rhs)
  {
    Time t(lhs);
    t += rhs;
    return t;
  }

  inline Time operator - (const timeval& lhs, const timeval rhs)
  {
    Time t(lhs);
    t -= rhs;
    return t;
  }

  inline bool operator == (const timeval& lhs, const timeval rhs)
  {
    return (lhs.tv_sec == rhs.tv_sec) && (lhs.tv_usec == rhs.tv_usec);
  }

  inline std::ostream& operator << (std::ostream& os, const timeval& tv)
  {
    if (tv.tv_sec < 0) {
      return os << '-' << Time(-tv.tv_sec - 1, 1000000 - tv.tv_usec);
    }
    return os << tv.tv_sec << '.' << std::setw(6) << std::setfill('0') << tv.tv_usec << std::setfill(' ');
  }

}

#endif // EUDAQ_INCLUDED_Time
