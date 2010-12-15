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

#include <iostream>
#include <vector>
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "trg/trg/Signal.h"
#include "trg/cdc/Layer.h"

#ifdef TRGCDC_SHORT_NAMES
#define CTWire TRGCDCWire
#endif

namespace Belle2 {

typedef HepGeom::Point3D<double>  Point3D;
typedef HepGeom::Vector3D<double>  Vector3D;
#define WireInnerLeft 0
#define WireInnerRight 1
#define WireLeft 2
#define WireRight 3
#define WireOuterLeft 4
#define WireOuterRight 5
#define MaxNeighbors 6

class TRGCDCWireHit;
class TRGCDCWireHitMC;

/// A class to represent a wire in CDC.
class TRGCDCWire {

  public:
    /// Constructor.
    TRGCDCWire(unsigned id,
		   unsigned localId,
		   TRGCDCLayer *,
		   const HepGeom::Point3D<double> & forwardPosition,
		   const HepGeom::Point3D<double> & backwardPosition);

    /// Destructor
    virtual ~TRGCDCWire();

  public:// Selectors
    /// returns id.
    unsigned id(void) const;

    /// returns local id in a wire layer.
    unsigned localId(void) const;

    /// returns layer id.
    unsigned layerId(void) const;

    /// returns local layer id in a super layer.
    unsigned localLayerId(void) const;

    /// returns super layer id.
    unsigned superLayerId(void) const;

    /// returns id of axial or stereo layer id.
    unsigned axialStereoLayerId(void) const;

    /// returns id of axial or stereo super layer id.
    unsigned axialStereoSuperLayerId(void) const;

    /// returns a pointer to a layer.
    const TRGCDCLayer & layer(void) const;

    /// returns a pointer to a TRGCDCWireHit.
    const TRGCDCWireHit * hit(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns true if this wire is in an axial layer.
    bool axial(void) const;

    /// returns true if this wire is in a stereo layer.
    bool stereo(void) const;

    /// returns true if this wire is in the inner part.
    bool innerPart(void) const;

    /// returns true if this wire is in the main part.
    bool mainPart(void) const;

    /// returns a pointer to a neighbor wire. This function is expensive.
    const TRGCDCWire * const neighbor(unsigned) const;

    /// returns true if a given wire is consective in a layer.
    bool consective(const TRGCDCWire &) const;

    /// returns true if a given wire is adjacent.
    bool adjacent(const TRGCDCWire &) const;

//     /// returns localId but if maxLocalId, return -1.
//     int localIdForPlus(void) const;

//     /// returns localId but if 0, return maxLocalId + 1.
//     int localIdForMinus(void) const;

    /// returns position in forward endplate.
    const HepGeom::Point3D<double>  & forwardPosition(void) const;

    /// returns position in backward endplate.
    const HepGeom::Point3D<double>  & backwardPosition(void) const;
    double * backwardPosition(double p[3]) const;

    /// returns middle position of a wire. z componet is 0.
    const HepGeom::Point3D<double>  & xyPosition(void) const;
    double * xyPosition(double p[3]) const;

    /// returns direction vector of the wire.
    const Vector3D & direction(void) const;

    /// calculates position and direction vector with sag correction.
//     void wirePosition(float zPosition,
// 		      HepGeom::Point3D<double>  & xyPosition,
// 		      HepGeom::Point3D<double>  & backwardPosition,
// 		      Vector3D & direction) const;

    /// returns cell size in phi.
    float cellSize(void) const;

    /// returns local id difference.
    int localIdDifference(const TRGCDCWire &) const;

    /// returns name.
    std::string name(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

  public:// TRG

    /// returns trigger output. Null will returned if no signal.
//  virtual const TRGSignal * triggerOutput(void) const;
    virtual const TRGSignal & triggerOutput(void) const;

  public:// Modifiers
    /// sets a pointer to TRGCDCWireHit.
    const TRGCDCWireHit * hit(const TRGCDCWireHit * const);

    /// appends a pointer to TRGCDCWireHitMC.
    const TRGCDCWireHitMC * hit(TRGCDCWireHitMC * const);

    /// sets state.
    unsigned state(unsigned newState);

    /// clears information.
    void clear(void);

  private:
    unsigned _state;
    const TRGCDCWireHit * _hit;
    std::vector<TRGCDCWireHitMC *> _mcHits;

    unsigned _id;
    unsigned _localId;
    const TRGCDCLayer * _layer;
    HepGeom::Point3D<double>  _xyPosition;
    HepGeom::Point3D<double>  _forwardPosition;
    HepGeom::Point3D<double>  _backwardPosition;
    Vector3D _direction;

    mutable TRGSignal _triggerOutput;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDCWire_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCWire_INLINE_DEFINE_HERE
#endif

#ifdef TRGCDCWire_INLINE_DEFINE_HERE

inline
unsigned
TRGCDCWire::id(void) const {
    return _id;
}

inline
unsigned
TRGCDCWire::localId(void) const {
    return _localId;
}

inline
unsigned
TRGCDCWire::layerId(void) const {
    return _layer->id();
}

inline
unsigned
TRGCDCWire::superLayerId(void) const {
    return _layer->superLayerId();
}

inline
unsigned
TRGCDCWire::localLayerId(void) const {
    return _layer->localLayerId();
}

inline
const TRGCDCLayer &
TRGCDCWire::layer(void) const {
    return * _layer;
}

inline
unsigned
TRGCDCWire::state(void) const {
    return _state;
}

inline
unsigned
TRGCDCWire::state(unsigned a) {
    return _state = a;
}

inline
const TRGCDCWireHit *
TRGCDCWire::hit(const TRGCDCWireHit * const h) {
    return _hit = h;
}

inline
const TRGCDCWireHit *
TRGCDCWire::hit(void) const {
    return _hit;
}

inline
const TRGCDCWireHitMC *
TRGCDCWire::hit(TRGCDCWireHitMC * const a) {
    _mcHits.push_back(a);
    return a;
}

inline
const HepGeom::Point3D<double>  &
TRGCDCWire::forwardPosition(void) const {
    return _forwardPosition;
}

inline
const HepGeom::Point3D<double>  &
TRGCDCWire::backwardPosition(void) const {
    return _backwardPosition;
}

inline
double *
TRGCDCWire::backwardPosition(double p[3]) const {
    p[0] = _backwardPosition.x();
    p[1] = _backwardPosition.y();
    p[2] = _backwardPosition.z();
    return p;
}

inline
const HepGeom::Point3D<double>  &
TRGCDCWire::xyPosition(void) const {
    return _xyPosition;
}

inline
double *
TRGCDCWire::xyPosition(double a[3]) const {
    a[0] = _xyPosition.x();
    a[1] = _xyPosition.y();
    a[2] = 0.;
    return a;
}

inline
const Vector3D &
TRGCDCWire::direction(void) const {
    return _direction;
}

inline
bool
TRGCDCWire::axial(void) const {
    return _layer->axial();
}

inline
bool
TRGCDCWire::stereo(void) const {
    return _layer->stereo();
}

inline
unsigned
TRGCDCWire::axialStereoLayerId(void) const {
    return _layer->axialStereoLayerId();
}

inline
unsigned
TRGCDCWire::axialStereoSuperLayerId(void) const {
    return _layer->axialStereoSuperLayerId();
}

inline
bool
TRGCDCWire::innerPart(void) const {
    std::cout << "TRGCDCWire::innerPart ... position not defined" << std::endl;
    if (layerId() < 14) return true;
    return false;
}

inline
bool
TRGCDCWire::mainPart(void) const {
    std::cout << "TRGCDCWire::mainPart ... position not defined" << std::endl;
    if (layerId() > 13) return true;
    return false;
}

inline
float
TRGCDCWire::cellSize(void) const {
    if (_layer) {
	return _layer->cellSize();
    }
    else {
#ifdef TRASAN_DEBUG
	std::cout << "TRGCDCWire::cellSize !!! can't return cell size";
	std::cout << " because no pointer to a layer" << std::endl;
#endif
	return 0.;
    }
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

#endif

#undef inline

} // namespace Belle2

#endif /* TRGCDCWire_FLAG_ */

