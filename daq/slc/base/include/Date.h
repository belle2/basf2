/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Date_hh
#define _Belle2_Date_hh

#include <time.h>

namespace Belle2 {

  class Date {

  public:
    Date(time_t time);
    Date(const Date& date);
    Date();
    ~Date();

  public:
    void set();
    void set(time_t time);
    time_t get() const { return m_time; }
    int getSecond() const;
    int getMinitue() const;
    int getHour() const;
    int getDay() const;
    int getMonth() const;
    int getYear() const;
    const char* toString(const char* format = NULL) const;

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
