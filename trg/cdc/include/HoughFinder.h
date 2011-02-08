//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughFinder.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughFinder_FLAG_
#define TRGCDCHoughFinder_FLAG_

#include <string>

#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHFinder TRGCDCHoughFinder
#endif

namespace Belle2 {

class TRGCDC;

/// A class to find tracks using Hough algorithm
class TRGCDCHoughFinder {

  public:
    /// Contructor.
    TRGCDCHoughFinder(const std::string & name,
                      const TRGCDC &,
                      unsigned nX,
                      unsigned nY);

    /// Destructor
    virtual ~TRGCDCHoughFinder();

  public:
    /// returns name.
    std::string name(void) const;

    /// do track finding.
    int doit(void);

  private:
    /// Name
    const std::string _name;
    /// CDCTRG
    const TRGCDC & _cdc;
    /// Hough planes, for + and - charges
    TRGCDCHoughPlaneMulti2 * _plane[2];
    /// Circle Hough transformtion
    TRGCDCHoughTransformationCircle _circleH;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCHoughFinder::name(void) const {
    return _name;
}

} // namespace Belle2

#endif
