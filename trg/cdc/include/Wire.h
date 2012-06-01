//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Wire.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCWire_FLAG_
#define TRGCDCWire_FLAG_

#include <vector>
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "trg/trg/Signal.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Layer.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCWire TRGCDCWire
#endif

namespace Belle2 {

typedef HepGeom::Point3D<double> Point3D;
typedef HepGeom::Vector3D<double> Vector3D;
#define WireInnerLeft 0
#define WireInnerRight 1
#define WireLeft 2
#define WireRight 3
#define WireOuterLeft 4
#define WireOuterRight 5
#define MaxNeighbors 6

class TRGCDC;
class TRGCDCWireHit;
class TRGCDCWireHitMC;
class TRGCDCTrackSegment;

/// A class to represent a wire in CDC.
class TRGCDCWire : public TRGCDCCell {

  public:
    /// Constructor.
    TRGCDCWire(unsigned id,
	       unsigned localId,
	       const TRGCDCLayer &,
	       const HepGeom::Point3D<double> & forwardPosition,
	       const HepGeom::Point3D<double> & backwardPosition);

//     /// Constructor with a track segment.
//     TRGCDCWire(const TRGCDCTrackSegment * segment,
// 	       const TRGCDCWire * wire);

    /// Destructor
    virtual ~TRGCDCWire();

  public:// Selectors

    /// returns a pointer to a TRGCDCWireHit.
    const TRGCDCWireHit * hit(void) const;

//     /// returns a pointer to a TRGCDCTrackSegment.
//     const TRGCDCTrackSegment * const segment(void) const;

    /// returns a pointer to a neighbor wire. This function is expensive.
    const TRGCDCWire * const neighbor(unsigned) const;

    /// returns true if a given wire is consective in a layer.
    bool consective(const TRGCDCWire &) const;

    /// returns true if a given wire is adjacent.
    bool adjacent(const TRGCDCWire &) const;

    /// returns name.
    std::string name(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
              const std::string & prefix = std::string("")) const;

  public:// Modifiers

    /// clears information.
    void clear(void);

    /// returns a pointer to a TRGCDCWireHit.
    const TRGCDCWireHit * hit(const TRGCDCWireHit *);

    /// appends a pointer to TRGCDCWireHitMC.
    const TRGCDCWireHitMC * hit(TRGCDCWireHitMC *);

  public:// TRG

    /// returns an input to the trigger. This is sync'ed to 1GHz clock.
    const TRGSignal & timing(void) const;

  private:

    /// MC wire hit.
//  std::vector<const TRGCDCWireHitMC * const> _mcHits;
    std::vector<const TRGCDCWireHitMC *> _mcHits;

    /// Trigger output.
    mutable TRGSignal _timing;

    /// Complete access from TRGCDC.
    friend class TRGCDC;
};

//-----------------------------------------------------------------------------

inline
const TRGCDCWireHit *
TRGCDCWire::hit(const TRGCDCWireHit * h) {
    return (const TRGCDCWireHit *) TRGCDCCell::hit((const TRGCDCCellHit *) h);
}

inline
const TRGCDCWireHit *
TRGCDCWire::hit(void) const {
    return (const TRGCDCWireHit *) TRGCDCCell::hit();
}

inline
const TRGCDCWireHitMC *
TRGCDCWire::hit(TRGCDCWireHitMC * const a) {
    _mcHits.push_back(a);
    return a;
}

inline
bool
TRGCDCWire::consective(const TRGCDCWire & w) const {
    if (neighbor(2) == & w) return true;
    else if (neighbor(3) == & w) return true;
    return false;
}

inline
bool
TRGCDCWire::adjacent(const TRGCDCWire & w) const {
    for (unsigned i = 0; i < 7; i++)
        if (neighbor(i) == & w)
            return true;
    return false;
}

inline
const TRGSignal &
TRGCDCWire::timing(void) const {
    return _timing;
}

} // namespace Belle2

#endif /* TRGCDCWire_FLAG_ */

