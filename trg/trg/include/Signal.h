//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGSignal.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a digitized signal.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGSignal_FLAG_
#define TRGSignal_FLAG_

#include <vector>
#include "trg/trg/Clock.h"
#include "trg/trg/Time.h"

namespace Belle2 {

/// A class to represent a digitized signal. Unit is nano second.
  class TRGSignal {

  public:

    /// Constructor.
    TRGSignal(const TRGClock& = Belle2_GDL::GDLSystemClock);

    /// Constructor with clock and timing(t0 leading, t1 trailing).
    TRGSignal(const TRGTime& t0, const TRGTime& t1);

    /// Constructor with clock and timing(t0 leading, t1 trailing).
    TRGSignal(const TRGClock& c, int t0, int t1);

    /// Constructor with clock and timing(t0 leading, t1 trailing). t0
    /// and t1 are in absolute time.
    TRGSignal(const TRGClock& c, double t0, double t1);

    /// Constructor with name.
    explicit TRGSignal(const std::string& name,
                       const TRGClock& = Belle2_GDL::GDLSystemClock);

    /// Copy constructor.
    TRGSignal(const TRGSignal&);

    /// Constructor.
    explicit TRGSignal(const TRGTime&);

    /// Default assignment operator
    TRGSignal& operator=(const TRGSignal&) = default;

    /// Destructor
    virtual ~TRGSignal();

  public:// Selectors

    /// returns name.
    const std::string& name(void) const;

    /// returns clock.
    const TRGClock& clock(void) const;

    /// returns \# of signals.
    unsigned nSignals(void) const;

    /// returns \# of edges.
    unsigned nEdges(void) const;

    /// returns width of i'th signal (i=0,1,2,...).
    unsigned width(unsigned i = 0) const;

    /// returns true if there is a signal.
    bool active(void) const;

    /// returns true if there is a signal.
    bool active(int clk0, int clk1) const;

    /// returns true if signal is active in given clock position.
    bool state(int clockPosition) const;

    /// returns true if signal is active and rising edge in give clock
    /// position.
    bool riseEdge(int clockPosition) const;

    /// returns a list of clock position of state change.
    std::vector<int> stateChanges(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  public:// Modifiers

    /// sets and returns name.
    const std::string& name(const std::string& newName);

    /// clears contents.
    void clear(void);

    /// changes clock.
    const TRGClock& clock(const TRGClock&);

    /// makes a pulse with leading edge at t0 and with trailing edge
    /// at t1. t0 and t1 in absolute time.
    const TRGSignal& set(double t0, double t1);

    /// makes a pulse with leading edge at clock t0 and with trailing
    /// edge at clock t1.
    const TRGSignal& set(int t0, int t1, bool state = true);

    /// clear(or unset) with leading edge at clock t0 and with trailing
    /// edge at clock t1.
    const TRGSignal& unset(int t0, int t1);

    /// makes signal inverted.
    const TRGSignal& invert(void);

  public:// Operators

    /// returns AND result.
    TRGSignal operator&(const TRGSignal&) const;

    /// returns AND result.
    TRGSignal operator&(const TRGTime&) const;

    /// returns AND result.
    TRGSignal& operator&=(const TRGSignal&);

    /// returns AND result.
    TRGSignal& operator&=(const TRGTime&);

    /// returns OR result.
    TRGSignal operator|(const TRGSignal&) const;

    /// returns OR result.
    TRGSignal operator|(const TRGTime&) const;

    /// returns OR result.
    TRGSignal& operator|=(const TRGSignal&);

    /// returns OR result.
    TRGSignal& operator|=(const TRGTime&);

    /// returns widen signals. Signals wider than "width" will be untouched.
    TRGSignal& widen(unsigned width);

    /// returns timing of i'th edge.
    const TRGTime* operator[](unsigned i) const;

    /// returns true if two are the same.
    bool operator==(const TRGSignal&) const;

    /// returns true if two are the same.
    bool operator!=(const TRGSignal&) const;

  private:

    // /// And operation.
    // static std::vector<TRGTime> andOperation(const std::vector<TRGTime>&);

    /// Or operation
    static std::vector<TRGTime> orOperation(const std::vector<TRGTime>&);

    /// Sort operation.
    void sort(void);

    /// Self-consitency check. True is return if something wrong.
    bool consistencyCheck(void) const;

  private:

    /// Name.
    std::string _name;

    /// Clock.
    const TRGClock* _clock;

    /// Timing history.
    std::vector<TRGTime> _history;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGSignal::name(void) const
  {
    return _name;
  }

  inline
  const std::string&
  TRGSignal::name(const std::string& newName)
  {
    return _name = newName;
  }

  inline
  TRGSignal
  TRGSignal::operator&(const TRGTime& l) const
  {
    TRGSignal t(* this);
    TRGSignal left(l);
    return t & left;
  }

  inline
  TRGSignal&
  TRGSignal::operator&=(const TRGTime& l)
  {
    TRGSignal left(l);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return (* this) &= left;
  }

  inline
  TRGSignal
  TRGSignal::operator|(const TRGTime& l) const
  {
    TRGSignal t(* this);
    TRGSignal left(l);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return t | left;
  }

  inline
  TRGSignal&
  TRGSignal::operator|=(const TRGTime& l)
  {
    TRGSignal left(l);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return (* this) |= left;
  }

  inline
  unsigned
  TRGSignal::nSignals(void) const
  {
    return _history.size() / 2;
  }

  inline
  unsigned
  TRGSignal::nEdges(void) const
  {
    return _history.size();
  }

  inline
  void
  TRGSignal::clear(void)
  {

#if TRG_DEBUG
    consistencyCheck();
#endif

    _history.clear();
  }

  inline
  bool
  TRGSignal::active(void) const
  {
    if (_history.size())
      return true;
    return false;
  }

  inline
  bool
  TRGSignal::state(int a) const
  {
    if (_history.size()) {
      bool last = false;
      for (unsigned i = 0; i < _history.size(); i++) {
        if (_history[i].time() <= a)
          last = _history[i].edge();
        else if (_history[i].time() > a)
          break;
      }
      return last;
    }
    return false;

  }

  inline
  bool
  TRGSignal::riseEdge(int a) const
  {
    if (_history.size()) {
      bool last = false;
      for (unsigned i = 0; i < _history.size(); i++) {
        if (_history[i].time() < a) {
          continue;
        } else if (_history[i].time() == a) {
          last = _history[i].edge();
        } else if (_history[i].time() > a) {
          break;
        }
      }
      return last;
    } else
      return false;
  }

  inline
  const TRGTime*
  TRGSignal::operator[](unsigned i) const
  {
    return & _history[i];
  }

  inline
  const TRGClock&
  TRGSignal::clock(void) const
  {
    return * _clock;
  }

  inline
  bool
  TRGSignal::operator!=(const TRGSignal& a) const
  {
    return (! operator==(a));
  }

  inline
  bool
  TRGSignal::active(int c0, int c1) const
  {
    for (unsigned i = 0; i < _history.size(); i++) {
      if (! _history[i].edge()) {
        const int t0 = _history[i - 1].time();
        const int t1 = _history[i].time();

        if ((c1 > t0) && (c0 < t1))
          return true;
        if ((c1 < t0) && (c1 < t1))
          return false;
      }
    }
    return false;
  }

} // namespace Belle2

#endif /* TRGSignal_FLAG_ */
