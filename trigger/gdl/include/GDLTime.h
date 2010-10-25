//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : GDLTime.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a signal timing in the trigger system.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef GDLTime_FLAG_
#define GDLTime_FLAG_

#include <float.h>
#include <string>

namespace Belle2 {

class GDLClock;
class GDLSignal;

/// A class to represent a signal timing in the trigger system. Unit is nano second.
class GDLTime {

  public:
    /// Copy constructor.
    GDLTime(const GDLTime &);

    /// Constructor wich clock. For rising edge, edge must be true. Given timing is syncronized to clock.
    GDLTime(double timing,
	    bool edge,
	    const GDLClock & clock,
	    const std::string & name = "signal");

    /// Destructor
    virtual ~GDLTime();

  public:// Selectors.

    /// returns name.
    const std::string & name(void) const;

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// returns edge information.
    bool edge(void) const;

    /// returns clock.
    const GDLClock & clock(void) const;

    /// returns timing in clock position.
    int time(void) const;

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  public:// Operators

    /// adding two GDLTime. A result is GDLSignal.
    GDLSignal operator&(const GDLTime &) const;
    GDLSignal operator&(const GDLSignal &) const;

    /// oring two GDLTime. A result is GDLSignal.
    GDLSignal operator|(const GDLTime &) const;
    GDLSignal operator|(const GDLSignal &) const;

    /// sets and returns timing in clock position.
    int time(int newTime);

    /// reverse edge.
    GDLTime & reverse(void);

    /// delays by clock unit.
    GDLTime & shift(int unit);

  public://

    /// returns true if a is older than b.
    static bool sortByTime(const GDLTime & a, const GDLTime & b);

  private:
    int _time;
    bool _edge;                              // true : rising, false : falling
    const GDLClock * _clock;
    std::string _name;
};

//-----------------------------------------------------------------------------

#ifdef GDLTime_NO_INLINE
#define inline
#else
#undef inline
#define GDLTime_INLINE_DEFINE_HERE
#endif

#ifdef GDLTime_INLINE_DEFINE_HERE

inline
const std::string &
GDLTime::name(void) const {
    return _name;
}

inline
const std::string &
GDLTime::name(const std::string & a) {
    return _name = a;
}

inline
GDLTime &
GDLTime::reverse(void) {
    _edge = ! _edge;
    return * this;
}

inline
const GDLClock &
GDLTime::clock(void) const {
    return * _clock;
}

inline
bool
GDLTime::edge(void) const {
    return _edge;
}

inline
GDLTime &
GDLTime::shift(int u) {
    _time += u;
    return * this;
}

inline
int
GDLTime::time(void) const {
    return _time;
}

inline
int
GDLTime::time(int a) {
    return _time = a;
}

#endif

#undef inline

} // namespace Belle2

#endif /* GDLTime_FLAG_ */
