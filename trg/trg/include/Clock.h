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
    TRGClock(double offset,
	     double frequency,
	     const std::string & name = "a clock");

    /// Destructor
    virtual ~TRGClock();

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
    TRGTime minTRGTime(bool edge) const;

    /// returns max. GDLtime with clock.
    TRGTime maxTRGTime(bool edge) const;

  private:
    const double _offset;
    const double _frequency;
    const double _cycle;
    int _min;
    int _max;
    const std::string _name;
};

//-----------------------------------------------------------------------------

#ifdef TRG_NO_INLINE
#define inline
#else
#undef inline
#define TRGClock_INLINE_DEFINE_HERE
#endif

#ifdef TRGClock_INLINE_DEFINE_HERE

inline
double
TRGClock::offset(void) const {
    return _offset;
}

inline
const std::string &
TRGClock::name(void) const {
    return _name;
}

inline
double
TRGClock::minTiming(void) const {
    return _min * _cycle + _offset;
}

inline
double
TRGClock::maxTiming(void) const {
    return _max * _cycle + _offset;
}

inline
int
TRGClock::unit(double a) const {
    return int(a / _cycle) + 1;
}

#endif

#undef inline

} // namespace Belle2

#endif /* TRGClock_FLAG_ */
