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

namespace Belle2 {

  class TRGSignal;
  class TRGState;
  class TRGClock;

/// A class to represent a bundle of digitized signals. Given
/// TRGSignal should exist while this object alive.
  class TRGSignalVector : public std::vector<TRGSignal> {

  public:

    /// Default constructor.
//  TRGSignalVector(const TRGClock & = Belle2_GDL::GDLSystemClock);

    /// Constructor with name.
    TRGSignalVector(const std::string& name,
//        const TRGClock & = Belle2_GDL::GDLSystemClock,
                    const TRGClock&,
                    unsigned size = 0);

    /// Copy constructor.
    TRGSignalVector(const TRGSignalVector&);

    /// Constructor.
    TRGSignalVector(const TRGSignal&);

    /// Destructor
    virtual ~TRGSignalVector();

  public:// Selectors

    /// returns name.
    const std::string& name(void) const;

    /// sets and returns name.
    const std::string& name(const std::string& newName);

    /// returns clock.
    const TRGClock& clock(void) const;

    /// returns true if there is a signal.
    bool active(void) const;

    /// returns true if there is a signal at clock position.
    bool active(int clockPosition) const;

    /// returns a list of clock position of state change.
    std::vector<int> stateChanges(void) const;

    /// returns state at given clock position.
    TRGState state(int clockPosition) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  public:// Modifiers

    /// sets state at given clock.
    const TRGSignalVector& set(const TRGState&, int clockPosition);

  public:// Operators

    /// changes clock.
    const TRGClock& clock(const TRGClock&);

    /// appends TRGSignal.
    TRGSignalVector& operator+=(const TRGSignal&);

    /// appends TRGSignalVector.
    TRGSignalVector& operator+=(const TRGSignalVector&);

    /// compare two TRGSignalVectors.
    bool operator==(const TRGSignalVector&) const;

    /// compare two TRGSignalVectors.
    bool operator!=(const TRGSignalVector&) const;

  private:

    /// Name
    std::string _name;

    /// Clock
    const TRGClock* _clock;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGSignalVector::name(void) const
  {
    return _name;
  }

  inline
  const std::string&
  TRGSignalVector::name(const std::string& newName)
  {
    return _name = newName;
  }

  inline
  const TRGClock&
  TRGSignalVector::clock(void) const
  {
    return * _clock;
  }

  inline
  bool
  TRGSignalVector::operator!=(const TRGSignalVector& a) const
  {
    return (! operator==(a));
  }

} // namespace Belle2

#endif /* TRGSignalVector_FLAG_ */
