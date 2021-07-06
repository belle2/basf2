/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef Time_hh
#define Time_hh

#include "daq/slc/base/Serializable.h"

#include <string>

namespace Belle2 {

  class Time : public Serializable {

    // constructors & destructors
  public:
    explicit Time();
    Time(const double);
    Time(const long, const long);
    ~Time();

    // member methods
  private:
    void adjust();

  public:
    void clear();
    void set();
    void set(const double);
    void set(const long, const long);
    double get() const;
    long getSecond() const;
    long getMicroSecond() const;
    std::string toString() const;

    // for serialization
  public:
    virtual void readObject(Reader&);
    virtual void writeObject(Writer&) const;

    // operators
  public:
    Time& operator = (const Time&);
    bool operator == (const Time&) const;
    Time operator + (const Time&) const;
    Time operator - (const Time&) const;

    // member data
  private:
    long m_s;
    long m_us;

  };

}

#endif
