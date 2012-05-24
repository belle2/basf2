//-----------------------------------------------------------------------------
// $Id: TPeakFinder.h 10528 2008-06-20 00:08:20Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TPeakFinder.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find peaks in THoughPlan.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2005/11/03 23:20:36  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.4  2005/04/18 23:42:05  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.3  2005/03/11 03:58:35  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.2  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.1  2004/02/18 04:07:47  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#ifndef TPeakFinder_FLAG_
#define TPeakFinder_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>
#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"

namespace Belle {

  class TPoint2D;
  class THoughPlane;

/// A class to find peaks in THoughPlan
  class TPeakFinder {

  public:
    /// Constructor.
    TPeakFinder(void);

    /// Destructor
    virtual ~TPeakFinder();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// finds peaks. TPoint2D objects in AList should be deleted by user.
    AList<TPoint2D> peaks(const THoughPlane& plane,
                          const unsigned threshold,
                          const unsigned nTargetPeaks,
                          const unsigned peakSeparation) const;
    AList<TPoint2D> peaks5(THoughPlane& plane,
                           const unsigned threshold,
                           bool centerIsPeak = false) const;
    AList<TPoint2D> peaks6(THoughPlane& plane,
                           const unsigned threshold,
                           bool centerIsPeak = false) const;

  private:
  };

  inline
  std::string
  TPeakFinder::name(void) const
  {
    return "TPeakFinder";
  }

} // namespace Belle

#endif
