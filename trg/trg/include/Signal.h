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
#include "trg/trg/Time.h"

namespace Belle2 {

class TRGTime;

/// A class to represent a digitized signal. Unit is nano second.
class TRGSignal {

  public:

    /// Default constructor.
    TRGSignal();

    /// Constructor with name.
    TRGSignal(const std::string & name);

    /// Copy constructor.
    TRGSignal(const TRGSignal &);

    /// Constructor.
    TRGSignal(const TRGTime &);

    /// Destructor
    virtual ~TRGSignal();

  public:// Selectors

    /// returns name.
    const std::string & name(void) const;

    /// returns clock.
    const TRGClock & clock(void) const;

    /// returns \# of edges.
    unsigned nEdges(void) const;

    /// returns true if there is a signal.
    bool active(void) const;

    /// returns true if signal is active in given clock position.
    bool active(int clockPosition) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

  public:// Modifiers

    /// clears contents.
    void clear(void);

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// changes clock.
    const TRGClock & clock(const TRGClock &);

  public:// Operators

    /// returns AND result.
    TRGSignal operator&(const TRGSignal &) const;

    /// returns AND result.
    TRGSignal operator&(const TRGTime &) const;

    /// returns AND result.
    TRGSignal & operator&=(const TRGSignal &);

    /// returns AND result.
    TRGSignal & operator&=(const TRGTime &);

    /// returns OR result.
    TRGSignal operator|(const TRGSignal &) const;

    /// returns OR result.
    TRGSignal operator|(const TRGTime &) const;

    /// returns OR result.
    TRGSignal & operator|=(const TRGSignal &);

    /// returns OR result.
    TRGSignal & operator|=(const TRGTime &);

    /// returns widen signals. Signals wider than "width" will be untouched.
    TRGSignal & widen(unsigned width);

    /// returns timing of i'th edge.
    const TRGTime * operator[](unsigned i) const;

  private:

    /// And operation.
    static std::vector<TRGTime> andOperation(const std::vector<TRGTime> &);

    /// Or operation
    static std::vector<TRGTime> orOperation(const std::vector<TRGTime> &);

  private:

    /// Timing history.
    std::vector<TRGTime> _history;

    /// Name.
    std::string _name;
};

//-----------------------------------------------------------------------------

inline
const std::string &
TRGSignal::name(void) const {
    return _name;
}

inline
const std::string &
TRGSignal::name(const std::string & newName) {
    return _name = newName;
}

inline
TRGSignal
TRGSignal::operator&(const TRGTime & l) const {
    TRGSignal t(* this);
    TRGSignal left(l);
    return t & left;
}

inline
TRGSignal &
TRGSignal::operator&=(const TRGTime & l) {
    TRGSignal left(l);
    return (* this) &= left;
}

inline
TRGSignal
TRGSignal::operator|(const TRGTime & l) const {
    TRGSignal t(* this);
    TRGSignal left(l);
    return t | left;
}

inline
TRGSignal &
TRGSignal::operator|=(const TRGTime & l) {
    TRGSignal left(l);
    return (* this) |= left;
}

inline
unsigned
TRGSignal::nEdges(void) const {
    return _history.size();
}

inline
void
TRGSignal::clear(void) {
    _history.clear();
}

inline
bool
TRGSignal::active(void) const {
    if (_history.size())
        return true;
    return false;
}

inline
bool
TRGSignal::active(int a) const {
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
const TRGTime *
TRGSignal::operator[](unsigned i) const {
    return & _history[i];
}

inline
const TRGClock &
TRGSignal::clock(void) const {

    //...Assuming clock is same for all TRGTime.
    return _history[0].clock();
}

inline
const TRGClock &
TRGSignal::clock(const TRGClock & c) {

    for (unsigned i = 0; i < _history.size(); i++)
	_history[i].clock(c);
	
    return _history[0].clock();
}

} // namespace Belle2

#endif /* TRGSignal_FLAG_ */
