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
// Filename : HoughTransformation.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : An abstract class to represent a Hough transformation.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughTransformation_FLAG_
#define TRGCDCHoughTransformation_FLAG_

#include <string>

#ifdef TRGCDC_SHORT_NAMES
#define TCHTransformation TRGCDCHoughTransformation
#endif

namespace Belle2 {

  class TRGPoint2D;

/// An abstract class to represent a Hough transformation.
  class TRGCDCHoughTransformation {

  public:
    /// Contructor.
    TRGCDCHoughTransformation(const std::string& name);

    /// Destructor
    virtual ~TRGCDCHoughTransformation();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns Y coordinate in a Hough parameter plane.
    virtual float y(float xReal, float yReal, float x) const = 0;

    /// returns true if Y diverges in given region.
    virtual bool diverge(float xReal, float yReal, float x0, float x1)
    const = 0;

    /// returns true if Y diverges in given region.
    virtual bool positiveDiverge(float xReal, float yReal, float x0, float x1)
    const = 0;

    /// returns true if Y diverges in given region.
    virtual bool negativeDiverge(float xReal, float yReal, float x0, float x1)
    const = 0;

//     /// returns a gradient at given regsion.
//     virtual float gradient(float xReal, float yReal, float x) const = 0;

    /// converts Point2D(r, phi) in real plane into Point2D(r, phi) in Hough plane.
    virtual TRGPoint2D convert(const TRGPoint2D&) const;

  public:// Modifiers

  private:
    /// name
    const std::string _name;
  };

  inline
  std::string
  TRGCDCHoughTransformation::name(void) const
  {
    return _name;
  }

} // namespace Belle

#endif
