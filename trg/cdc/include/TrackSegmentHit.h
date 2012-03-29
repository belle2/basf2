//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegmentHit.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track segment hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackSegmentHit_FLAG_
#define TRGCDCTrackSegmentHit_FLAG_

#include "trg/cdc/WireHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTSHit TRGCDCTrackSegmentHit
#endif

namespace Belle2 {

class TRGCDCTrackSegment;

/// A class to represent a track segment hit in CDC.
class TRGCDCTrackSegmentHit : public TRGCDCWireHit {

  public:

    /// Constructor.
//  TRGCDCTrackSegmentHit(const TRGCDCWire &);
    TRGCDCTrackSegmentHit(const TRGCDCTrackSegment &);

    /// Constructor.
//  TRGCDCTrackSegmentHit(const TRGCDCWire &,
// 			  const TRGCDCWireHit &);
    TRGCDCTrackSegmentHit(const TRGCDCTrackSegment &,
			  const TRGCDCWireHit &);
// 			  float driftLeft,
// 			  float driftLeftError,
// 			  float driftRight,
// 			  float driftRightError,
// 			  float fudgeFacgtor = 1);

    /// Destructor
    virtual ~TRGCDCTrackSegmentHit();

  public:// Selectors

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
		      const std::string & prefix = std::string("")) const;

    /// returns a pointer to a track segment.
    const TRGCDCTrackSegment & trackSegment(void) const;

  public:// Modifiers

  public:// Static utility functions

    /// Sorting funnction.
    static int sortByWireId(const TRGCDCTrackSegmentHit ** a,
			    const TRGCDCTrackSegmentHit ** b);

  private:

    /// Track segment
    TRGCDCTrackSegment * _segment;

    /// State.
    mutable unsigned _state;
};

//-----------------------------------------------------------------------------

inline
const TRGCDCTrackSegment &
TRGCDCWireHit::trackSegment(void) const {
    return * _segment;
}

} // namespace Belle2

#endif /* TRGCDCTrackSegmentHit_FLAG_ */
