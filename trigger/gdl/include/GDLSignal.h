//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : GDLSignal.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a digitized signal.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef GDLSignal_FLAG_
#define GDLSignal_FLAG_

#include <vector>
#include "trigger/gdl/GDLTime.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#endif

namespace Belle2 {

class GDLTime;

/// A class to represent a digitized signal. Unit is nano second.
class GDLSignal {

  public:
    /// Default constructor.
    GDLSignal();

    /// Copy constructor.
    GDLSignal(const GDLSignal &);

    /// Constructor.
    GDLSignal(const GDLTime &);

    /// Destructor
    virtual ~GDLSignal();

  public:// Selectors

    /// returns name.
    const std::string & name(void) const;

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// returns \# of edges.
    unsigned nEdges(void) const;

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  public:// Operators

    /// returns AND result.
    GDLSignal operator&(const GDLSignal &) const;

    /// returns AND result.
    GDLSignal operator&(const GDLTime &) const;

    /// returns AND result.
    GDLSignal & operator&=(const GDLSignal &);

    /// returns AND result.
    GDLSignal & operator&=(const GDLTime &);

    /// returns OR result.
    GDLSignal operator|(const GDLSignal &) const;

    /// returns OR result.
    GDLSignal operator|(const GDLTime &) const;

    /// returns OR result.
    GDLSignal & operator|=(const GDLSignal &);

    /// returns OR result.
    GDLSignal & operator|=(const GDLTime &);

    /// returns widen signals. Signals wider than "width" will be untouched.
    GDLSignal & widen(unsigned width);

  private:
    static std::vector<GDLTime> andOperation(const std::vector<GDLTime> &);
    static std::vector<GDLTime> orOperation(const std::vector<GDLTime> &);

  private:
    std::vector<GDLTime> _history;
    std::string _name;
};

//-----------------------------------------------------------------------------

#ifdef GDLSignal_NO_INLINE
#define inline
#else
#undef inline
#define GDLSignal_INLINE_DEFINE_HERE
#endif

#ifdef GDLSignal_INLINE_DEFINE_HERE

inline
const std::string &
GDLSignal::name(void) const {
    return _name;
}

inline
const std::string &
GDLSignal::name(const std::string & newName) {
    return _name = newName;
}

inline
GDLSignal
GDLSignal::operator&(const GDLTime & l) const {
    GDLSignal t(* this);
    GDLSignal left(l);
    return t & left;
}

inline
GDLSignal &
GDLSignal::operator&=(const GDLTime & l) {
    GDLSignal left(l);
    return (* this) &= left;
}

inline
GDLSignal
GDLSignal::operator|(const GDLTime & l) const {
    GDLSignal t(* this);
    GDLSignal left(l);
    return t | left;
}

inline
GDLSignal &
GDLSignal::operator|=(const GDLTime & l) {
    GDLSignal left(l);
    return (* this) |= left;
}

inline
unsigned
GDLSignal::nEdges(void) const {
    return _history.size();
}

#endif

#undef inline

} // namespace Belle2

#endif /* GDLSignal_FLAG_ */
