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
class TRGCDCLink;

/// A class to find tracks using Hough algorithm
class TRGCDCHoughFinder {

  public:

    /// Contructor.
    TRGCDCHoughFinder(const std::string & name,
                      const TRGCDC &,
                      unsigned nX,
                      unsigned nY,
		      unsigned peakMin);

    /// Destructor
    virtual ~TRGCDCHoughFinder();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// do track finding.
    int doit(std::vector<TRGCDCTrack *> & trackList);

  public:

    /// sets and returns switch to do perfect finding.
    bool perfect(bool);

  private:

    /// selects the best(fastest) hits in each super layer.
    std::vector<TRGCDCLink *> selectBestHits(
	const std::vector<TRGCDCLink *> & links) const;

  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC & _cdc;

    /// Hough planes, for + and - charges.
    TRGCDCHoughPlaneMulti2 * _plane[2];

    /// Circle Hough transformtion.
    TRGCDCHoughTransformationCircle _circleH;

    /// Peak finder.
    TRGCDCPeakFinder _peakFinder;

    /// Min. peak height for the peak finder.
    const unsigned _peakMin;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCHoughFinder::name(void) const {
    return _name;
}

} // namespace Belle2

#endif
