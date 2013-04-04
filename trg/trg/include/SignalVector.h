//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGSignalVector.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bundle of digitized signals.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGSignalVector_FLAG_
#define TRGSignalVector_FLAG_

#include <vector>

#ifdef TRG_SHORT_NAMES
#define TRGSVector TRGSignalVector
#endif

namespace Belle2 {

class TRGSignal;

/// A class to represent a bundle of digitized signals. Given
/// TRGSignal should exist while this object alive.
class TRGSignalVector : public std::vector<const TRGSignal *> {

  public:

    /// Default constructor.
    TRGSignalVector();

    /// Constructor with name.
    TRGSignalVector(const std::string & name);

    /// Copy constructor.
    TRGSignalVector(const TRGSignalVector &);

    /// Constructor.
    TRGSignalVector(const TRGSignal &);

    /// Destructor
    virtual ~TRGSignalVector();

  public:// Selectors

    /// returns name.
    const std::string & name(void) const;

    /// sets and returns name.
    const std::string & name(const std::string & newName);

    /// returns true if there is a signal.
    bool active(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;

  public:// Operators

    /// returns new TRGSignalVector.
//    TRGSignalVector operator+(const TRGSignalVector &) const;

    /// appends TRGSignal.
    TRGSignalVector & operator+=(const TRGSignal &);

  private:

    /// Name.
    std::string _name;

    /// TRGSignals.
    std::vector<const TRGSignal *> _signals;
};

//-----------------------------------------------------------------------------

inline
const std::string &
TRGSignalVector::name(void) const {
    return _name;
}

inline
const std::string &
TRGSignalVector::name(const std::string & newName) {
    return _name = newName;
}

} // namespace Belle2

#endif /* TRGSignalVector_FLAG_ */
