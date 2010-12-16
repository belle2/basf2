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

    /// returns state.
    unsigned state(void) const;

    /// returns name.
    std::string name(void) const;

    /// returns a wire.
    const TRGCDCWire * operator[](unsigned id) const;

    /// returns trigger output. Null will returned if no signal.
    const TRGSignal & triggerOutput(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

  public:// Modifiers

    /// sets state.
    unsigned state(unsigned newState);

    /// clears information.
    void clear(void);

    /// simulates TF hit using wire information.
    void simulate(void);

  private:
    unsigned _state;
    unsigned _id;
    unsigned _localId;
    const TRGCDCLayer * _layer;
    std::vector<const TRGCDCWire *> _wires;
    TRGSignal _signal;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCTrackSegment_INLINE_DEFINE_HERE
#endif

#ifdef TRGCDCTrackSegment_INLINE_DEFINE_HERE

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
    return _wires[id];
}

inline
const TRGSignal &
TRGCDCTrackSegment::triggerOutput(void) const {
    return _signal;
}

#endif

#undef inline

} // namespace Belle2

#endif /* TRGCDCTrackSegment_FLAG_ */
