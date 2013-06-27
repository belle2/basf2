//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGState.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a state of multi bits
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGState_FLAG_
#define TRGState_FLAG_

#include <vector>
#include "trg/trg/Time.h"

namespace Belle2 {

class TRGTime;

/// A class to represent a state of multi bits
class TRGState {

  public:

    /// Default constructor.
    TRGState(unsigned bitSize);

    /// Constructor.
    TRGState(std::vector<bool> states);

    // /// Copy constructor.
    // TRGState(const TRGState &);

    /// Destructor
    virtual ~TRGState();

  public:// Selectors

    /// returns bit size.
    unsigned size(void) const;

    /// returns true if there are active bits.
    bool active(void) const;

    /// returns true if there are active bits.
    bool active(unsigned bitPosition) const;

    /// returns pattern in unsigned.
    unsigned long long pattern(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

  public:// Modifiers

    /// clears state.
    void clear(void);

  public:// Operators

    // /// returns AND result.
    // TRGState operator&(const TRGState &) const;

    // /// returns AND result.
    // TRGState operator&(const TRGTime &) const;

    // /// returns AND result.
    // TRGState & operator&=(const TRGState &);

    // /// returns AND result.
    // TRGState & operator&=(const TRGTime &);

    // /// returns OR result.
    // TRGState operator|(const TRGState &) const;

    // /// returns OR result.
    // TRGState operator|(const TRGTime &) const;

    // /// returns OR result.
    // TRGState & operator|=(const TRGState &);

    // /// returns OR result.
    // TRGState & operator|=(const TRGTime &);

  private:

    /// bit size;
    unsigned _size;

    /// bit state.
    unsigned * _state;
};

//-----------------------------------------------------------------------------

inline
unsigned
TRGState::size(void) const {
    return _size;
}

inline
bool
TRGState::active(unsigned a) const {
    const unsigned wp = a / sizeof(unsigned);
    const unsigned bp = a % sizeof(unsigned);
    if (_state[wp] & (1 << bp))
	return true;
    else
	return false;
}

inline
unsigned long long
TRGState::pattern(void) const {
    unsigned n = _size / sizeof(unsigned);
    if (_size % sizeof(unsigned)) ++n;
    unsigned long long a = 0;
    const unsigned s = sizeof(unsigned);
    for (unsigned i = 0; i < n; i++) {
	const unsigned long long b = _state[i];
	a += b << (i * s);
    }
    return a;
}

} // namespace Belle2

//-----------------------------------------------------------------------------

namespace std {

    ostream & operator<<(ostream &, const Belle2::TRGState &);

}

#endif /* TRGState_FLAG_ */
