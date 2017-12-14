#ifndef _Belle2_Date_hh
#define _Belle2_Date_hh

#include <time.h>
#include <string>

namespace Belle2 {

  class Date {

  public:
    Date(time_t time);
    Date(const Date& date);
    Date();
    ~Date() throw();

  public:
    void set() throw();
    void set(time_t time) throw();
    time_t get() const throw() { return m_time; }
    int getSecond() const throw();
    int getMinitue() const throw();
    int getHour() const throw();
    int getDay() const throw();
    int getMonth() const throw();
    int getYear() const throw();
    const char* toString(const char* format = NULL) const throw();

  public:
    const Date& operator=(const Date& date)
    {
      set(date.m_time);
      return *this;
    }

  private:
    time_t m_time;
    struct tm* m_tm;
    mutable char m_str[31];

  };

};

#endif
