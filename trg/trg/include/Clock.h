/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGClock.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a clock.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGClock_FLAG_
#define TRGClock_FLAG_

#include <string>
#include "trg/trg/SignalVector.h"

namespace Belle2 {
  class TRGClock;
  class TRGTime;
}

namespace Belle2_GDL {

  //...This should be moved to GDL module...
  extern const Belle2::TRGClock GDLSystemClock;
}

namespace Belle2 {

/// A class to represent a digitized signal. Unit is nano second.
  class TRGClock {

  public:

    /// Constructor. "offset" is in unit of ns. "frequency" is in unit of MHz.
    TRGClock(const std::string& name,
             double offset,
             double frequency);

    /// Constructor with clock source.
    TRGClock(const std::string& name,
             const TRGClock&,
             unsigned multiplicationFactor,
             unsigned divisionFactor = 1);

    /// Destructor
    virtual ~TRGClock();

  public:// Selectors

    /// returns name.
    const std::string& name(void) const;

    /// returns clock position.
    int position(double timing) const;

    /// returns clock position in the source clock.
    int positionInSourceClock(double timing) const;

    /// returns absolute time of clock position
    double absoluteTime(int clockPosition) const;

    /// returns over shoot.
    double overShoot(double timing) const;

    /// returns min \# of clocks to cover given time period.
    int unit(double period) const;

    /// returns offset of the clock zero position in nano second.
    double offset(void) const;

    /// returns frequency in MHz.
    double frequency(void) const;

    /// returns phase of given timing in degree (0 to 360).
    double phase(double timing) const;

    /// returns the clock counter.
    const TRGSignalVector& clockCounter(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  public://

    /// returns min. clock point.
    int min(void) const;

    /// returns max. clock point.
    int max(void) const;

    /// returns min. timing.
    double minTiming(void) const;

    /// returns max. timing.
    double maxTiming(void) const;

    /// returns min. TRGtime with clock.
    TRGTime minTRGTime(bool edge) const;

    /// returns max. TRGtime with clock.
    TRGTime maxTRGTime(bool edge) const;

  private:

    /// Name.
    const std::string _name;

    /// Clock source.
    const TRGClock* _source;

    /// Multiplication factor.
    const unsigned _multi;

    /// Division factor.
    const unsigned _div;

    /// Clock offset in nano second.
    const double _offset;

    /// Frequency in MHz
    const double _frequency;

    /// Clock cycle in nano second.
    const double _cycle;

    /// Clock min. count.
    int _min;

    /// Clock max. count.
    int _max;

    /// Clock counter
    mutable TRGSignalVector* _clockCounter;
  };

//-----------------------------------------------------------------------------

  inline
  double
  TRGClock::offset(void) const
  {
    return _offset;
  }

  inline
  const std::string&
  TRGClock::name(void) const
  {
    return _name;
  }

  inline
  double
  TRGClock::minTiming(void) const
  {
//    return _min * _cycle + _offset;
    return _min * _cycle;
  }

  inline
  double
  TRGClock::maxTiming(void) const
  {
//    return _max * _cycle + _offset;
    return _max * _cycle;
  }

  inline
  int
  TRGClock::unit(double a) const
  {
    return int(a / _cycle) + 1;
  }

  inline
  double
  TRGClock::frequency(void) const
  {
    return _frequency;
  }

  inline
  int
  TRGClock::min(void) const
  {
    return _min;
  }

  inline
  int
  TRGClock::max(void) const
  {
    return _max;
  }

  inline
  int
  TRGClock::positionInSourceClock(double a) const
  {
    return _source->position(a);
  }

} // namespace Belle2

#endif /* TRGClock_FLAG_ */
