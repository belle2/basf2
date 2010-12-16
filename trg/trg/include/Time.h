//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGTime.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a signal timing in the trigger system.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGTime_FLAG_
#define TRGTime_FLAG_

#include <float.h>
#include <string>

#ifdef TRGCDC_SHORT_NAMES
#define TRGTime TRGTime
#endif

namespace Belle2 {

class TRGSignal;
class TRGClock;

/// A class to represent a signal timing in the trigger system. Unit is nano second.
class TRGTime {

  public:
    /// Copy constructor.
    TRGTime(const TRGTime &);

    /// Constructor wich clock. For rising edge, edge must be true. Given timing is syncronized to clock.
    TRGTime(double timing,
	    bool edge,
	    const TRGClock & clock,
	    const std::string & name = "signal");

    /// Destructor
    virtual ~TRGTime();

  public:// Selectors.

    /// returns name.
    const std::string & name(void) const;

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// returns edge information.
    bool edge(void) const;

    /// returns clock.
    const TRGClock & clock(void) const;

    /// returns timing in clock position.
    int time(void) const;

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  public:// Operators

    /// adding two TRGTime. A result is TRGSignal.
    TRGSignal operator&(const TRGTime &) const;
    TRGSignal operator&(const TRGSignal &) const;

    /// oring two TRGTime. A result is TRGSignal.
    TRGSignal operator|(const TRGTime &) const;
    TRGSignal operator|(const TRGSignal &) const;

    /// sets and returns timing in clock position.
    int time(int newTime);

    /// reverse edge.
    TRGTime & reverse(void);

    /// delays by clock unit.
    TRGTime & shift(int unit);

  public://

    /// returns true if a is older than b.
    static bool sortByTime(const TRGTime & a, const TRGTime & b);

  private:
    int _time;
    bool _edge;                              // true : rising, false : falling
    const TRGClock * _clock;
    std::string _name;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGTime_INLINE_DEFINE_HERE
#endif

#ifdef TRGTime_INLINE_DEFINE_HERE

inline
const std::string &
TRGTime::name(void) const {
    return _name;
}

inline
const std::string &
TRGTime::name(const std::string & a) {
    return _name = a;
}

inline
TRGTime &
TRGTime::reverse(void) {
    _edge = ! _edge;
    return * this;
}

inline
const TRGClock &
TRGTime::clock(void) const {
    return * _clock;
}

inline
bool
TRGTime::edge(void) const {
    return _edge;
}

inline
TRGTime &
TRGTime::shift(int u) {
    _time += u;
    return * this;
}

inline
int
TRGTime::time(void) const {
    return _time;
}

inline
int
TRGTime::time(int a) {
    return _time = a;
}

#endif

#undef inline

} // namespace Belle2

#endif /* TRGTime_FLAG_ */
