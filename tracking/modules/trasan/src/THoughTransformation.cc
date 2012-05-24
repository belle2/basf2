//-----------------------------------------------------------------------------
// $Id: THoughTransformation.cc 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : THoughTransformation.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : An abstract class to represent a Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/03/11 03:57:51  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/02/18 04:07:26  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------


#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/THoughTransformation.h"

namespace Belle {

  THoughTransformation::THoughTransformation(const std::string& name)
    : _name(name)
  {
  }

  THoughTransformation::~THoughTransformation()
  {
  }

  TPoint2D
  THoughTransformation::convert(const TPoint2D& p) const
  {
    return p;
  }

} // namespace Belle

