//-----------------------------------------------------------fmt version 0.00--
// $Id: Range.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Header file for Multi-TDC data
//-----------------------------------------------------------------------------
// Filename : MultiTDC.h
// Section  : CDC TSF
// Owner    : Yoshihito Iwasaki
// Email    : yiwaskai@kekvax.kek.jp
//-----------------------------------------------------------------------------
// Description :
//-----------------------------------------------------------------------------
// Version |    Date     | Explanation of changes
//   00.00 | 03-Aug-1996 | Creation
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.3  1998/08/31 07:31:55  yiwasaki
// minor bug fixes of Range.h and Pulse.cc
//
// Revision 1.2  1998/07/26 23:52:53  katayama
// endif coment fixed
//
// Revision 1.1  1998/05/22 18:04:53  yiwasaki
// Range, Pulse added
//
// Revision 1.3  1997/10/30 17:34:07  yiwasaki
// Range::within2 added
//
// Revision 1.2  1997/10/15 08:31:25  yiwasaki
// Slightly modified
//
// Revision 1.1  1997/06/10 08:10:18  katayama
// The first version from Y. Iwasaki
//
//-----------------------------------------------------------------------------

#ifndef Range_FLAG_
#define Range_FLAG_


namespace Belle {

//
//...To define band width...
//
/// to specify 1-dim region or range by two floats
  class Range {

  public:
    /// Constructor
    Range();

    /// Copy constructor
    Range(const Range&);

    /// Constructor
    Range(float low, float high);

    /// Destructor
    virtual ~Range();

  public:// Selectors
    /// returns lower limit.
    virtual float low(void) const;

    /// returns higher limit.
    virtual float high(void) const;

    /// returns center.
    virtual float center(void) const;

    /// returns width.
    virtual float width(void) const;

  public:// Modifiers
    /// sets lower limit.
    virtual float low(float lowIn);

    /// sets higher limit.
    virtual float high(float highIn);

    /// sets range.
    virtual Range& set(float low, float high);

    /// sets range by center and width.
    virtual Range& setByCenter(float center, float width);

  public:// Operators
    /// Copy operator
    Range& operator = (const Range&);

    /// returns true if range is the same.
    bool operator == (const Range&) const;

    /// returns true if range is different.
    bool operator != (const Range&) const;

    /// returns true if two are overlaped each other.
    bool operator & (const Range&) const;

    /// returns true if given value is within a range.
    bool within(const float value) const;

    /// returns true if given value is within a range.
    bool within2(const float value) const;

    /// returns true if given Range is within(included in) a range.
    bool within(const Range&) const;

  public:// Common interfaces
    /// displays debug information.
    virtual int dump(void) const;

  private:
    /// Lower limit
    float _low;
    /// Higher limit
    float _high;
  };

//-----------------------------------------------------------------------------

#ifdef TANA_NO_INLINE
#define inline
#else
#undef inline
#define Range_INLINE_DEFINE_HERE
#endif

#ifdef Range_INLINE_DEFINE_HERE

  inline
  float
  Range::low(void) const
  {
    return _low;
  }

  inline
  float
  Range::low(float i)
  {
    if (i > _high) i = _high;
    return _low = i;
  }

  inline
  float
  Range::high(void) const
  {
    return _high;
  }

  inline
  float
  Range::high(float i)
  {
    if (i < _low) i = _low;
    return _high = i;
  }

  inline
  float
  Range::center(void) const
  {
    return (_low + _high) / 2.;
  }

  inline
  float
  Range::width(void) const
  {
    return (_high - _low);
  }

  inline
  Range&
  Range::set(float iLow, float iHigh)
  {
    if (iHigh > iLow) {
      _low = iLow;
      _high = iHigh;
    } else {
      _low = iHigh;
      _high = iLow;
    }
    return * this;
  }

  inline
  Range&
  Range::setByCenter(float center, float width)
  {
    _low = center - width;
    _high = center + width;
    return * this;
  }

  inline
  Range&
  Range::operator = (const Range& ib)
  {
    _low = ib.low();
    _high = ib.high();
    return * this;
  }

  inline
  bool
  Range::within(const float f) const
  {
    if (_low == -999. && _high == -999.) {
      return false;
    }
    if (_low == -999.) {
      if (f <= _high) return true;
      return false;
    }
    if (_high == -999.) {
      if (f >= _low) return true;
    }
    if (f >= _low && f <= _high) return true;
    return false;
  }

  inline
  bool
  Range::within2(const float f) const
  {
    if (_low == -999. && _high == -999.) {
      return true;
    }
    if (_low == -999.) {
      if (f <= _high) return true;
      return false;
    }
    if (_high == -999.) {
      if (f >= _low) return true;
    }
    if (f >= _low && f <= _high) return true;
    return false;
  }

  inline
  bool
  Range::operator != (const Range& a) const
  {
    if ((* this) == a) return false;
    return true;
  }

  inline
  bool
  Range::operator & (const Range& a) const
  {
    if (within(a.low())) return true;
    if (within(a.high())) return true;
    if (a.within(low())) return true;
    if (a.within(high())) return true;
    return false;
  }

  inline
  bool
  Range::within(const Range& a) const
  {
    if (within(a.low()) && within(a.high())) return true;
    return false;
  }

#endif

#undef inline

} // namespace Belle

#endif /* Range_FLAG_ */

