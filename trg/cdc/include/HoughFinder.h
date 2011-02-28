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
#include "trg/cdc/PeakFinder.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHFinder TRGCDCHoughFinder
#endif

namespace Belle2 {

class TRGCDC;
class TRGCDCPeakFinder;
class TRGCDCTrack;

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

    /// returns version.
    std::string version(void) const;

    /// do track finding.
    int doit(std::vector<TRGCDCTrack *> & trackList);

    /// returns switch to do perfect finding.
    bool perfect(void) const;

  public:

    /// sets and returns switch to do perfect finding.
    bool perfect(bool);

  private:

    /// do perfect finding.
    int doitPerfectly(std::vector<TRGCDCTrack *> & trackList);

  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC & _cdc;

    /// Switch to do perfect finding.
    bool _perfect;

    /// Hough planes, for + and - charges.
    TRGCDCHoughPlaneMulti2 * _plane[2];

    /// Circle Hough transformtion.
    TRGCDCHoughTransformationCircle _circleH;

    /// Peak finder.
    TRGCDCPeakFinder _peakFinder;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCHoughFinder::name(void) const {
    return _name;
}

inline
bool
TRGCDCHoughFinder::perfect(void) const {
    return _perfect;
}

inline
bool
TRGCDCHoughFinder::perfect(bool a) {
    return _perfect = a;
}

} // namespace Belle2

#endif
