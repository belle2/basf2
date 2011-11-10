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

/// A class to represent a track segment hit in CDC.
class TRGCDCTrackSegmentHit : public TRGCDCWireHit {

  public:
    /// Constructor.
    TRGCDCTrackSegmentHit(const TRGCDCWire &);

    /// Constructor.
    TRGCDCTrackSegmentHit(const TRGCDCWire &,
			  float driftLeft,
			  float driftLeftError,
			  float driftRight,
			  float driftRightError,
			  float fudgeFacgtor = 1);

    /// Destructor
    virtual ~TRGCDCTrackSegmentHit();

  public:// Selectors

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
		      const std::string & prefix = std::string("")) const;

  public:// Modifiers

  public:// Static utility functions
    static int sortByWireId(const TRGCDCTrackSegmentHit ** a,
			    const TRGCDCTrackSegmentHit ** b);

  private:
    mutable unsigned _state;
};

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCTrackSegmentHit_FLAG_ */
