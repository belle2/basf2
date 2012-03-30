//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegment.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackSegment_FLAG_
#define TRGCDCTrackSegment_FLAG_

#include <vector>
#include "trg/cdc/Cell.h"
#include "trg/cdc/TrackSegmentHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTSegment TRGCDCTrackSegment
#endif

namespace Belle2 {

class TRGSignal;
class TRGCDCWire;
class TRGCDCLayer;
class TRGCDCLUT;

/// A class to represent a wire in CDC.
class TRGCDCTrackSegment : public TRGCDCCell {

  public:
    /// Constructor.
    TRGCDCTrackSegment(unsigned id,
                       const TRGCDCLayer & layer,
                       const TRGCDCWire & w,
		       const TRGCDCLUT * lut,
                       const std::vector<const TRGCDCWire *> & wires);

    /// Destructor
    virtual ~TRGCDCTrackSegment();

  public:// Selectors

    /// returns a vector containing pointers to a wire.
    const std::vector<const TRGCDCWire *> & wires(void) const;

    /// returns name.
    std::string name(void) const;

    /// returns a wire.
    const TRGCDCWire * operator[](unsigned id) const;

    /// returns trigger output. Null will returned if no signal.
    const TRGSignal & triggerOutput(void) const;

    /// returns a pointer to a TRGCDCTrackSegmentHit.
    const TRGCDCTrackSegmentHit * hit(void) const;

    /// returns hit pattern.
    unsigned hitPattern(void) const;

    /// returns LUT.
    const TRGCDCLUT * LUT(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
              const std::string & prefix = std::string("")) const;

  public:// Utility functions

    /// returns axial segments.
/*     static const std::vector<const TRGCDCTrackSegment *> */
/* 	axial(const std::vector<const TRGCDCTrackSegment *> & list); */

    /// returns \# of stereo segments.
    static unsigned
	nStereo(const std::vector<const TRGCDCTrackSegment *> & list);

  public:// Modifiers

    /// clears information.
    void clear(void);

    /// simulates TF hit using wire information.
    void simulate(void);

    /// sets a pointer to a TRGCDCTrackSegmentHit.
    const TRGCDCTrackSegmentHit * hit(const TRGCDCTrackSegmentHit * const);

  private:

    /// LookUp Table.
    const TRGCDCLUT * const _lut;

    /// Wires.
    std::vector<const TRGCDCWire *> _wires;

    /// Wire hits.
    std::vector<const TRGCDCWireHit *> _hits;

    /// Trigger signal.
    TRGSignal _signal;

    /// Track segment hit.
    const TRGCDCTrackSegmentHit * _hit;

  // Friends
    friend class TRGCDC;
};

//-----------------------------------------------------------------------------

inline
const std::vector<const TRGCDCWire *> &
TRGCDCTrackSegment::wires(void) const {
    return _wires;
}

inline
const TRGCDCWire *
TRGCDCTrackSegment::operator[](unsigned id) const {
    if (id < _wires.size())
	return _wires[id];
    return 0;
}

inline
const TRGSignal &
TRGCDCTrackSegment::triggerOutput(void) const {
    return _signal;
}

inline
const TRGCDCTrackSegmentHit *
TRGCDCTrackSegment::hit(const TRGCDCTrackSegmentHit * const h) {
    return _hit = h;
}

inline
const TRGCDCTrackSegmentHit *
TRGCDCTrackSegment::hit(void) const {
    return _hit;
}

inline
const TRGCDCLUT *
TRGCDCTrackSegment::LUT(void) const {
    return _lut;
}

} // namespace Belle2

#endif /* TRGCDCTrackSegment_FLAG_ */
