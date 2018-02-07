#include <eutel/eudaq/Time.h>
#include <eutel/eudaq/Platform.h>
#include <eutel/eudaq/Exception.h>
#include <ctime>
#include <iostream>

#if EUDAQ_PLATFORM_IS(WIN32)
# define WIN32_LEAN_AND_MEAN
//# include <afxwin.h>
# include <Windows.h>
# include <time.h>
# define EPOCHFILETIME (116444736000000000LL)
#endif

namespace eudaq {

  const std::string Time::DEFAULT_FORMAT = "%Y-%m-%d %H:%M:%S.%3";

#if EUDAQ_PLATFORM_IS(WIN32)

  // Based on timeval.h by Wu Yongwei
  Time Time::Current()
  {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    LARGE_INTEGER li;
    li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    __int64 t = li.QuadPart;
    t -= EPOCHFILETIME;     /* Offset to the Epoch time */
    t /= 10;                /* In microseconds */

    timeval tv;
    tv.tv_sec  = (long)(t / 1000000);
    tv.tv_usec = (long)(t % 1000000);

    return tv;
  }

#else

  Time Time::Current()
  {
    timeval tv;
    if (gettimeofday(&tv, 0)) {
      EUDAQ_THROW(std::string("Error getting current time: ") + strerror(errno));
    }
    return tv;
  }

#endif

  Time::Time(int year, int month, int date, int hour, int minute, int sec, int usec)
  {
    struct tm time;
    //time.tm_gmtoff = 0;
    time.tm_isdst = -1;
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = date;
    time.tm_hour = hour;
    time.tm_min = minute;
    time.tm_sec = sec + usec / 1000000;
    tv_sec = static_cast<long>(mktime(&time));
    tv_usec = usec % 1000000;
  }

  std::string Time::Formatted(const std::string& format) const
  {
    char buf[256];
    std::string fmt = format;
    size_t i, pos = 0;
    while ((i = fmt.find('%', pos)) != std::string::npos) {
      //std::cout << "i=" << i << std::endl;
      if (i >= fmt.length() - 1) break;
      pos = i + 2;
      //std::cout << "pos=" << pos << std::endl;
      int c = fmt[i + 1] - '0';
      if (c >= 1 && c <= 6) {
        //std::cout << "c=" << c << std::endl;
        std::string usecs = to_string(tv_usec, 6);
        //std::cout << "fmt=" << fmt << std::endl;
        fmt = std::string(fmt, 0, i) + std::string(usecs, 0, c) + std::string(fmt, i + 2);
        //std::cout << "fmt=" << fmt << std::endl;
        pos += c - 2;
        //std::cout << "pos=" << pos << std::endl;
      }
    }
    time_t t = tv_sec;
    struct tm* tm = std::localtime(&t);
    std::strftime(buf, sizeof buf, fmt.c_str(), tm);
    return std::string(buf);
  }

}
