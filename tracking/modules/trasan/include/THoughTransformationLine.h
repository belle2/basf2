//-----------------------------------------------------------------------------
// $Id: THoughTransformationLine.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : THoughTransformationLine.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent line Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2004/04/23 09:51:45  yiwasaki
// New files for Hough finder
//
//-----------------------------------------------------------------------------

#ifndef THoughTransformationLine_FLAG_
#define THoughTransformationLine_FLAG_


#include <string>
#include "tracking/modules/trasan/THoughTransformation.h"

namespace Belle {

  class TPoint2D;

/// A class to represent line Hough transformation.
  class THoughTransformationLine : public THoughTransformation {

  public:
    /// Contructor.
    THoughTransformationLine(const std::string& name);

    /// Destructor
    virtual ~THoughTransformationLine();

  public:// Selectors
    /// returns Y coordinate in a Hough parameter plane.
    virtual float y(float xReal, float yReal, float x) const;

    /// returns true if Y diverges in given region.
    virtual bool diverge(float xReal, float yReal, float x0, float x1) const;

    /// returns true if Y diverges in given region.
    virtual bool positiveDiverge(float xReal, float yReal, float x0, float x1)
    const;

    /// returns true if Y diverges in given region.
    virtual bool negativeDiverge(float xReal, float yReal, float x0, float x1)
    const;
  };

} // namespace Belle

#endif
