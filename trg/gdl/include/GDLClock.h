//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : GDLClock.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a clock.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef GDLClock_FLAG_
#define GDLClock_FLAG_

#include <string>
#include "trigger/gdl/GDLTime.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#endif

namespace Belle2 {
    class GDLClock;
}

namespace Belle2_GDL {

    //...This should be moved to GDL module...
    extern const Belle2::GDLClock GDLSystemClock;
}

namespace Belle2 {

/// A class to represent a digitized signal. Unit is nano second.
class GDLClock {

  public:
    /// Constructor. "offset" is in unit of ns. "frequency" is in unit of MHz.
    GDLClock(double offset,
	     double frequency,
	     const std::string & name = "a clock");

    /// Destructor
    virtual ~GDLClock();

  public:// Selectors

    /// returns clock point.
    int time(double timing) const;

    /// returns min \# of clocks to cover given time period.
    int unit(double period) const;

    /// returns offset.
    double offset(void) const;

    /// returns name.
    const std::string & name(void) const;

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  public://

    /// returns min. clock point.
    int min(void) const;

    /// returns max. clock point.
    int max(void) const;

    /// returns min. timing.
    double minTiming(void) const;

    /// returns max. timing.
    double maxTiming(void) const;

    /// returns min. GDLtime with clock.
    GDLTime minGDLTime(bool edge) const;

    /// returns max. GDLtime with clock.
    GDLTime maxGDLTime(bool edge) const;

  private:
    const double _offset;
    const double _frequency;
    const double _cycle;
    int _min;
    int _max;
    const std::string _name;
};

//-----------------------------------------------------------------------------

#ifdef GDLClock_NO_INLINE
#define inline
#else
#undef inline
#define GDLClock_INLINE_DEFINE_HERE
#endif

#ifdef GDLClock_INLINE_DEFINE_HERE

inline
double
GDLClock::offset(void) const {
    return _offset;
}

inline
const std::string &
GDLClock::name(void) const {
    return _name;
}

inline
double
GDLClock::minTiming(void) const {
    return _min * _cycle + _offset;
}

inline
double
GDLClock::maxTiming(void) const {
    return _max * _cycle + _offset;
}

inline
int
GDLClock::unit(double a) const {
    return int(a / _cycle) + 1;
}

#endif

#undef inline

} // namespace Belle2

#endif /* GDLClock_FLAG_ */
