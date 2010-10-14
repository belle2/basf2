//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerTrackSegment.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef CDCTriggerTrackSegment_FLAG_
#define CDCTriggerTrackSegment_FLAG_

#include <iostream>
#include <vector>
#include "trigger/cdc/CDCTriggerWire.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#define CTTSegment CDCTriggerTrackSegment
#endif

namespace Belle2 {

class CDCTriggerLayer;

/// A class to represent a wire in CDC.
class CDCTriggerTrackSegment : public CDCTriggerWire {

  public:
    /// Constructor.
    CDCTriggerTrackSegment(const CDCTriggerWire & w,
			   unsigned n,
			   const int * shape,
			   const CDCTriggerLayer * layer);

    /// Destructor
    virtual ~CDCTriggerTrackSegment();

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
    const CDCTriggerLayer * const layer(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns name.
    std::string name(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

  public:// Modifiers
    /// sets state.
    unsigned state(unsigned newState);

    /// clears information.
    void clear(void);

  private:
    unsigned _state;
    unsigned _id;
    unsigned _localId;
    const CDCTriggerLayer * _layer;
};

//-----------------------------------------------------------------------------

#ifdef CDCTriggerTrackSegment_NO_INLINE
#define inline
#else
#undef inline
#define CDCTriggerTrackSegment_INLINE_DEFINE_HERE
#endif

#ifdef CDCTriggerTrackSegment_INLINE_DEFINE_HERE

inline
unsigned
CDCTriggerTrackSegment::id(void) const {
    return _id;
}

inline
unsigned
CDCTriggerTrackSegment::localId(void) const {
    return _localId;
}

inline
unsigned
CDCTriggerTrackSegment::layerId(void) const {
    return _layer->id();
}

inline
unsigned
CDCTriggerTrackSegment::superLayerId(void) const {
    return _layer->superLayerId();
}

inline
const CDCTriggerLayer * const
CDCTriggerTrackSegment::layer(void) const {
    return _layer;
}

inline
unsigned
CDCTriggerTrackSegment::state(void) const {
    return _state;
}

inline
unsigned
CDCTriggerTrackSegment::state(unsigned a) {
    return _state = a;
}

#endif

#undef inline

} // namespace Belle2

#endif /* CDCTriggerTrackSegment_FLAG_ */

