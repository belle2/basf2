#ifndef Time_hh
#define Time_hh

#include "daq/slc/base/Serializable.h"

#include <string>

namespace Belle2 {

  class Time : public Serializable {

    // constructors & destructors
  public:
    explicit Time() throw();
    Time(const double) throw();
    Time(const long, const long) throw();
    ~Time() throw();

    // member methods
  private:
    void adjust() throw();

  public:
    void clear() throw();
    void set() throw();
    void set(const double) throw();
    void set(const long, const long) throw();
    double get() const throw();
    long getSecond() const throw();
    long getMicroSecond() const throw();
    std::string toString() const throw();

    // for serialization
  public:
    virtual void readObject(Reader&) throw(IOException);
    virtual void writeObject(Writer&) const throw(IOException);

    // operators
  public:
    Time& operator = (const Time&) throw();
    bool operator == (const Time&) const throw();
    Time operator + (const Time&) const throw();
    Time operator - (const Time&) const throw();

    // member data
  private:
    long m_s;
    long m_us;

  };

}

#endif
