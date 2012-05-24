//-----------------------------------------------------------------------------
// $Id: TUtilities.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TUtilities.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Definitions of utility functions
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/11/24 01:33:16  yiwasaki
// addition of debug utilitiese
//
//-----------------------------------------------------------------------------

#ifndef TUtilities_FLAG_
#define TUtilities_FLAG_


namespace Belle {

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

  class TPoint2D;

  extern const float PI2;
  extern const TPoint2D Origin;

  float
  PositiveRadian(float);

  bool
  InRangeRadian(float phi0, float phi1, float phi);

  float
  DistanceRadian(float a, float b);

} // namespace Belle

#endif
