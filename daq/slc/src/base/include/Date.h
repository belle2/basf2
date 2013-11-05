#ifndef _Belle2_Date_hh
#define _Belle2_Date_hh

#include <time.h>
#include <string>

namespace Belle2 {

  class Date {

  private:
    time_t _time;
    struct tm* _tm;

  public:
    Date(time_t time);
    Date();
    ~Date() throw();

  public:
    void set() throw();
    void set(time_t time) throw();
    time_t get() const throw() { return _time; }
    int getSecond() const throw();
    int getMinitue() const throw();
    int getHour() const throw();
    int getDay() const throw();
    int getMonth() const throw();
    int getYear() const throw();
    const char* toString(const char* format = NULL) const throw();
    //std::string toString(const char* format = NULL) const throw();

  };

};

#endif
