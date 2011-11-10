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
#include "trg/cdc/Wire.h"
#include "trg/cdc/TrackSegmentHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTSegment TRGCDCTrackSegment
#endif

namespace Belle2 {

class TRGSignal;
class TRGCDCLayer;

/// A class to represent a wire in CDC.
class TRGCDCTrackSegment : public TRGCDCWire {

  public:
    /// Constructor.
    TRGCDCTrackSegment(unsigned id,
                       const TRGCDCWire & w,
                       const TRGCDCLayer * layer,
                       const std::vector<const TRGCDCWire *> & cells);

    /// Destructor
    virtual ~TRGCDCTrackSegment();

  public:// Selectors

    /// returns id.
    unsigned id(void) const;

    /// returns local id in a TS layer.
    unsigned localId(void) const;

    /// returns layer id.
    unsigned layerId(void) const;

    /// returns super layer id.
    unsigned superLayerId(void) const;

    /// returns a pointer to a TS layer.
    const TRGCDCLayer * layer(void) const;

    /// returns a vector containing pointers to a wire.
    const std::vector<const TRGCDCWire *> & wires(void) const;

    /// returns true if this is in axial layer.
    bool axial(void) const;

    /// returns true if this is in stereo layer.
    bool stereo(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns name.
    std::string name(void) const;

    /// returns a wire.
    const TRGCDCWire * operator[](unsigned id) const;

    /// returns trigger output. Null will returned if no signal.
    const TRGSignal & triggerOutput(void) const;

    /// returns a pointer to a TRGCDCTrackSegmentHit.
    const TRGCDCTrackSegmentHit * hit(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
              const std::string & prefix = std::string("")) const;

  public:// Utility functions

    /// returns axial segments.
    static const std::vector<const TRGCDCTrackSegment *>
	axial(const std::vector<const TRGCDCTrackSegment *> & list);

    /// returns \# of stereo segments.
    static unsigned
	nStereo(const std::vector<const TRGCDCTrackSegment *> & list);

  public:// Modifiers

    /// sets state.
    unsigned state(unsigned newState);

    /// clears information.
    void clear(void);

    /// simulates TF hit using wire information.
    void simulate(void);

    /// sets a pointer to a TRGCDCTrackSegmentHit.
    const TRGCDCTrackSegmentHit * hit(const TRGCDCTrackSegmentHit * const);

  private:

    /// State.
    unsigned _state;

    /// ID.
    unsigned _id;

    /// Local ID.
    unsigned _localId;

    /// Layer.
    const TRGCDCLayer * _layer;

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
unsigned
TRGCDCTrackSegment::id(void) const {
    return _id;
}

inline
unsigned
TRGCDCTrackSegment::localId(void) const {
    return _localId;
}

inline
unsigned
TRGCDCTrackSegment::layerId(void) const {
    return _layer->id();
}

inline
unsigned
TRGCDCTrackSegment::superLayerId(void) const {
    return _layer->superLayerId();
}

inline
const TRGCDCLayer *
TRGCDCTrackSegment::layer(void) const {
    return _layer;
}

inline
unsigned
TRGCDCTrackSegment::state(void) const {
    return _state;
}

inline
unsigned
TRGCDCTrackSegment::state(unsigned a) {
    return _state = a;
}

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
bool
TRGCDCTrackSegment::axial(void) const {
    return _wires[0]->axial();
}

inline
bool
TRGCDCTrackSegment::stereo(void) const {
    return _wires[0]->stereo();
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

} // namespace Belle2

#endif /* TRGCDCTrackSegment_FLAG_ */
