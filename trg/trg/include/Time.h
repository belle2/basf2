/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTime_FLAG_
#define TRGTime_FLAG_

#include <string>

namespace Belle2 {

  class TRGSignal;
  class TRGClock;

/// A class to represent a signal timing in the trigger system. Unit
/// is nano second.
  class TRGTime {

  public:

    /// Copy constructor.
    TRGTime(const TRGTime&);

    /// Constructor with clock. For rising edge, edge must be
    /// true. Given timing is syncronized to clock.
    TRGTime(double timing,
            bool edge,
            const TRGClock& clock,
            const std::string& name = "signal");

    /// Constructor with clock. For rising edge, edge must be
    /// true. Given timing is syncronized to clock.
    TRGTime(int clockPosition,
            bool edge,
            const TRGClock& clock,
            const std::string& name = "signal");

    /// Destructor
    virtual ~TRGTime();

  public:// Selectors.

    /// returns name.
    const std::string& name(void) const;

    /// sets and returns name.
    const std::string& name(const std::string& newName);

    /// returns edge information.
    bool edge(void) const;

    /// returns clock.
    const TRGClock& clock(void) const;

    /// returns timing in clock position.
    int time(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  public:// Operators

    /// changes clock.
    const TRGClock& clock(const TRGClock&);

    /// adding two TRGTime. A result is TRGSignal.
    TRGSignal operator&(const TRGTime&) const;

    /// adding two TRGTime. A result is TRGSignal.
    TRGSignal operator&(const TRGSignal&) const;

    /// oring two TRGTime. A result is TRGSignal.
    TRGSignal operator|(const TRGTime&) const;

    /// oring two TRGTime. A result is TRGSignal.
    TRGSignal operator|(const TRGSignal&) const;

    /// returns true if two are the same.
    bool operator==(const TRGTime&) const;

    /// returns false if two are the same.
    bool operator!=(const TRGTime&) const;

    /// sets and returns timing in clock position.
    int time(int newTime);

    /// reverse edge.
    TRGTime& reverse(void);

    /// delays by clock unit.
    TRGTime& shift(int unit);

  public://

    /// returns true if a is older than b.
    static bool sortByTime(const TRGTime& a, const TRGTime& b);

  private:

    /// Time in clock unit.
    int _time;

    /// Edge type. true : rising, false : falling.
    bool _edge;

    /// Clock.
    const TRGClock* _clock;

    /// Name.
    std::string _name;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGTime::name(void) const
  {
    return _name;
  }

  inline
  const std::string&
  TRGTime::name(const std::string& a)
  {
    return _name = a;
  }

  inline
  TRGTime&
  TRGTime::reverse(void)
  {
    _edge = ! _edge;
    return * this;
  }

  inline
  const TRGClock&
  TRGTime::clock(void) const
  {
    return * _clock;
  }

  inline
  bool
  TRGTime::edge(void) const
  {
    return _edge;
  }

  inline
  TRGTime&
  TRGTime::shift(int u)
  {
    _time += u;
    return * this;
  }

  inline
  int
  TRGTime::time(void) const
  {
    return _time;
  }

  inline
  int
  TRGTime::time(int a)
  {
    return _time = a;
  }

  inline
  bool
  TRGTime::operator==(const TRGTime& a) const
  {
    if (_time != a._time) return false;
    if (_edge != a._edge) return false;
    if (_clock != a._clock) return false;
    // no name check
    return true;
  }

  inline
  bool
  TRGTime::operator!=(const TRGTime& a) const
  {
    return (! operator==(a));
  }

} // namespace Belle2

#endif /* TRGTime_FLAG_ */
