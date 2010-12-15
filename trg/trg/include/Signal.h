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

#ifdef TRIGGER_SHORT_NAMES
#define TRGSignal TRGSignal
#endif

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

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// returns \# of edges.
    unsigned nEdges(void) const;

    /// returns true if there is a signal.
    bool active(void) const;

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  public:// Modifiers

    /// clears contents.
    void clear(void);

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

  private:
    static std::vector<TRGTime> andOperation(const std::vector<TRGTime> &);
    static std::vector<TRGTime> orOperation(const std::vector<TRGTime> &);

  private:
    std::vector<TRGTime> _history;
    std::string _name;
};

//-----------------------------------------------------------------------------

#ifdef TRGSignal_NO_INLINE
#define inline
#else
#undef inline
#define TRGSignal_INLINE_DEFINE_HERE
#endif

#ifdef TRGSignal_INLINE_DEFINE_HERE

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

#endif

#undef inline

} // namespace Belle2

#endif /* TRGSignal_FLAG_ */
