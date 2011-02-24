//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : PeakFinder.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find peaks in Hough Plane
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCPeakFinder_FLAG_
#define TRGCDCPeakFinder_FLAG_

#include <string>
#include <vector>

#include "trg/cdc/HoughPlane.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCPFinder TRGCDCPeakFinder
#endif

namespace Belle2 {

/// A class to find peaks in Hough Plane
class TRGCDCPeakFinder {

  public:

    /// Contructor.
    TRGCDCPeakFinder(const std::string & name);

    /// Destructor
    virtual ~TRGCDCPeakFinder();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// do peak finding.
    std::vector<TRGPoint2D *> doit(TRGCDCHoughPlane & hp,
				   const unsigned threshold,
				   const bool centerIsPeak) const;

  private:

    /// do peak finding. This is a copy from "trasan".
    std::vector<TRGPoint2D *> peaks5(TRGCDCHoughPlane & hp,
				     const unsigned threshold,
				     const bool centerIsPeak) const;

  private:

    /// Name
    const std::string _name;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCPeakFinder::name(void) const {
    return _name;
}

} // namespace Belle2

#endif
