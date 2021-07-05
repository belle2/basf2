/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Belle2 {

/// A class to represent a state of multi bits
  class TRGState {

  public:

    /// Default constructor.
    TRGState(unsigned bitSize = 0);

    /// Constructor with unsigned.
    TRGState(unsigned bitSize, unsigned value);

    /// Constructor.
    // cppcheck-suppress noExplicitConstructor
    TRGState(std::vector<bool> states);

    /// Constructor.
    TRGState(unsigned bitSize, const bool* const states);

    /// Constructor. order: 0=>inVector[0] is lsb. 1=>inVector[0] is msb.
    TRGState(std::vector<unsigned>& states, unsigned order);

    /// Constructor. type: 0-> binary, 1->hex
    TRGState(const char*, unsigned type);

    /// Default copy constructor.
    TRGState(const TRGState&) = default;

    /// Destructor
    virtual ~TRGState();

  public:// Selectors

    /// returns bit size.
    unsigned size(void) const;

    /// returns true if there are active bits.
    bool active(void) const;

    /// returns true if there are active bits.
    bool active(unsigned bitPosition) const;

    /// returns a filled array.
    void copy2bool(bool* array) const;

    /// returns subset from i with n bits.
    TRGState subset(unsigned i, unsigned n) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  public:// Modifiers

    /// clears state.
    void clear(void);

    /// sets state at bit i.
    const TRGState& set(unsigned position, bool state = true);

    /// sets state.
    const TRGState& set(unsigned position,
                        unsigned size,
                        const bool* const array);

    /// sets state.
    const TRGState& set(unsigned position,
                        unsigned size,
                        unsigned value);

    /// sets state.
    const TRGState& set(unsigned position, const TRGState&);

  public:// Operators

    /// Conversion to unsigned.
    operator unsigned() const;

    /// Conversion to unsigned long long
    operator unsigned long long() const;

    /// Conversion to string
    operator std::string() const;

    /// appends TRGState (as MSB).
    TRGState& operator+=(const TRGState&);

    /// returns state of i'th bit.
    bool operator[](unsigned i) const;

    /// Copy operator.
    TRGState& operator=(const TRGState&);

    /// Copy operator.
    bool operator<(const TRGState&) const;

    /// shifts bits.
    TRGState& shift(unsigned i);

  public:// Utilities

    /// Coverts from bool array to unsigned.
    static unsigned toUnsigned(unsigned n, const bool* array);

  private:

    /// size of unsigned.
    static const unsigned _su;

    /// bit size of unsigned.
    static const unsigned _bsu;

    /// bit size.
    unsigned _size;

    /// bit storage size.
    unsigned _n;

    /// bit state.
    unsigned* _state;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TRGState::size(void) const
  {
    return _size;
  }

  inline
  bool
  TRGState::active(void) const
  {
    for (unsigned i = 0; i < _n; i++)
      if (_state[i])
        return true;
    return false;
  }

  inline
  bool
  TRGState::active(unsigned a) const
  {
    const unsigned wp = a / _bsu;
    const unsigned bp = a % _bsu;
    if (_state[wp] & (1 << bp))
      return true;
    else
      return false;
  }

  inline
  bool
  TRGState::operator[](unsigned a) const
  {
#ifdef TRG_DEBUG
    if (a >= _size)
      std::cout << "TRGState::operator[] !!! bit size overflow"
                << ":bit size=" << _size << ",specified position=" << a
                << std::endl;
#endif

    const unsigned wp = a / _bsu;
    const unsigned bp = a % _bsu;
    if (_state[wp] & (1 << bp))
      return true;
    else
      return false;
  }

  inline
  TRGState::operator unsigned() const
  {
#ifdef TRG_DEBUG
    unsigned n = _size / _bsu;
    if (_size % _bsu) ++n;
    if (n > 1)
      std::cout << "TRGState::operator unsigned() !!! bit size overflow"
                << ":bit size=" << _size << ",max bit size with unsigned="
                << _bsu << std::endl;
#endif
    return _state[0];

  }

  inline
  TRGState::operator unsigned long long() const
  {
    unsigned n = _size / _bsu;
    if (_size % _bsu) ++n;
    const unsigned c = sizeof(unsigned long long);
    if (n > c)
#ifdef TRG_DEBUG
      std::cout << "TRGState::operator unsigned long long() "
                << "!!! bit size overflow"
                << ":bit size=" << _size
                << ",max bit size with unsigned long long="
                << c << std::endl;
#endif
    n = c;
    unsigned long long a = 0;
    const unsigned s = _bsu;
    for (unsigned i = 0; i < n; i++) {
      const unsigned long long b = _state[i];
      a += b << (i * s);
    }
    return a;
  }

  inline
  TRGState::operator std::string() const
  {
    unsigned nWords = _size / _bsu;
    if (_size % _bsu) ++nWords;
    unsigned lastNHex = (_size % _bsu);
    if (lastNHex == 0) lastNHex = 8;
    else {
      lastNHex = lastNHex / 4;
      if ((_size % _bsu) % 4) ++lastNHex;
    }
    std::stringstream t_stringstream;
    t_stringstream << std::setw(lastNHex) << std::setfill('0') << std::hex << _state[nWords - 1];
    for (unsigned iWord = 1; iWord < nWords; iWord++) {
      t_stringstream << std::setw(8) << std::setfill('0') << std::hex << _state[nWords - 1 - iWord];
    }
    return t_stringstream.str();
  }

  inline
  void
  TRGState::copy2bool(bool* a) const
  {
    for (unsigned i = 0; i < _size; i++) {
      const unsigned wp = i / _bsu;
      const unsigned bp = i % _bsu;
      if (_state[wp] & (1 << bp))
        a[i] = true;
      else
        a[i] = false;
    }
  }

  inline
  const TRGState&
  TRGState::set(unsigned p, unsigned n, const bool* const a)
  {
    if (a) {
      for (unsigned i = 0; i < n; i++) {
        const unsigned wp = (p + i) / _bsu;
        const unsigned bp = (p + i) % _bsu;
        if (a[i])
          _state[wp] |= (1 << bp);
        else
          _state[wp] &= ~(1 << bp);
      }
    }
#ifdef TRG_DEBUG
    else std::cout << "NULL pointer found in TRGState::set"
                     << std::endl;
#endif
    return * this;
  }

  inline
  const TRGState&
  TRGState::set(unsigned p, unsigned n, unsigned a)
  {
    for (unsigned i = 0; i < n; i++) {
      const unsigned wp = (p + i) / _bsu;
      const unsigned bp = (p + i) % _bsu;
      if (a & (1 << i))
        _state[wp] |= (1 << bp);
      else
        _state[wp] &= ~(1 << bp);
    }
    return * this;
  }

  inline
  const TRGState&
  TRGState::set(unsigned n, bool a)
  {
    const unsigned wp = n / _bsu;
    const unsigned bp = n % _bsu;
    if (a)
      _state[wp] |= (1 << bp);
    else
      _state[wp] &= ~(1 << bp);
    return * this;
  }

  inline
  const TRGState&
  TRGState::set(unsigned p, const TRGState& s)
  {
    const unsigned n = s.size();
    for (unsigned i = 0; i < n; i++) {
      const unsigned wp = (p + i) / _bsu;
      const unsigned bp = (p + i) % _bsu;
      if (s[i])
        _state[wp] |= (1 << bp);
      else
        _state[wp] &= ~(1 << bp);
    }
    return * this;
  }

  inline
  unsigned
  TRGState::toUnsigned(unsigned n, const bool* a)
  {
#ifdef TRG_DEBUG
    if (n > 8) std::cout << "given array size(" << n << ") is too big"
                           << std::endl;
#endif

    unsigned b = 0;
    for (unsigned i = 0; i < n; i++) {
      if (a[i]) {
        const unsigned bp = i % _bsu;
        b |= (1 << bp);
      }
    }
    return b;
  }

  inline
  TRGState&
  TRGState::shift(unsigned a)
  {

    if (! active()) return * this;

    if (a == 0) return * this;

    for (int i = _size - 1; i >= 0; --i) {
      if (i < int(a))
        set(i, false);
      else
        set(i, active(i - a));
    }

    return * this;
  }

  inline
  void
  TRGState::clear(void)
  {
    for (unsigned i = 0; i < _n; i++)
      _state[i] = 0;
  }

} // namespace Belle2

//-----------------------------------------------------------------------------

namespace std {

  ostream& operator<<(ostream&, const Belle2::TRGState&);

}

#endif /* TRGState_FLAG_ */
