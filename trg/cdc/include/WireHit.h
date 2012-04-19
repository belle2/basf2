//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : WireHit.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCWireHit_FLAG_
#define TRGCDCWireHit_FLAG_

#include "trg/cdc/CellHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCWHit TRGCDCWireHit
#endif

namespace Belle2 {

class TRGCDCWire;
class TRGCDCWireHitMC;

/// A class to represent a wire hit in CDC.
    class TRGCDCWireHit : public TRGCDCCellHit {

  public:
    /// Constructor.
    TRGCDCWireHit(const TRGCDCWire &,
		  unsigned indexCDCHit = 0,
		  unsigned indexCDCSimHit = 0,
		  float driftLeft = 0,
		  float driftLeftError = 0,
		  float driftRight = 0,
		  float driftRightError = 0,
		  float fudgeFacgtor = 1);

    /// Destructor
    virtual ~TRGCDCWireHit();

  public:// Selectors

    /// returns a pointer to a TRGCDCWire.
    const TRGCDCWire & wire(void) const;

    /// This will be removed.
    const TRGCDCWireHitMC * mc(void) const;

    /// This will be removed.
    const TRGCDCWireHitMC * mc(TRGCDCWireHitMC *);

  public:// Static utility functions

    /// Sort function. This will be removed.
    static int sortByWireId(const TRGCDCWireHit ** a,
			    const TRGCDCWireHit ** b);

  private:

    /// This will be removed.
    const TRGCDCWireHitMC * _mc;

};

//-----------------------------------------------------------------------------

inline
const TRGCDCWireHitMC *
TRGCDCWireHit::mc(void) const {
    return _mc;
}

inline
const TRGCDCWireHitMC *
TRGCDCWireHit::mc(TRGCDCWireHitMC * a) {
    return _mc = a;
}

} // namespace Belle2

#endif /* TRGCDCWireHit_FLAG_ */
