//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerWire.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef CDCTriggerWire_FLAG_
#define CDCTriggerWire_FLAG_

#include <iostream>
#include <vector>
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "trigger/cdc/CDCTriggerLayer.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#define CTWire CDCTriggerWire
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

class CDCTriggerWireHit;
class CDCTriggerWireHitMC;

/// A class to represent a wire in CDC.
class CDCTriggerWire {

  public:
    /// Constructor.
    CDCTriggerWire(unsigned id,
		   unsigned localId,
		   CDCTriggerLayer *,
		   const HepGeom::Point3D<double> & forwardPosition,
		   const HepGeom::Point3D<double> & backwardPosition);

    /// Destructor
    virtual ~CDCTriggerWire();

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
    const CDCTriggerLayer & layer(void) const;

    /// returns a pointer to a CDCTriggerWireHit.
    const CDCTriggerWireHit * const hit(void) const;

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
    const CDCTriggerWire * const neighbor(unsigned) const;

    /// returns true if a given wire is consective in a layer.
    bool consective(const CDCTriggerWire &) const;

    /// returns true if a given wire is adjacent.
    bool adjacent(const CDCTriggerWire &) const;

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
    int localIdDifference(const CDCTriggerWire &) const;

    /// returns name.
    std::string name(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

  public:// Modifiers
    /// sets a pointer to CDCTriggerWireHit.
    const CDCTriggerWireHit * const hit(const CDCTriggerWireHit * const);

    /// appends a pointer to CDCTriggerWireHitMC.
    const CDCTriggerWireHitMC * const hit(CDCTriggerWireHitMC * const);

    /// sets state.
    unsigned state(unsigned newState);

    /// clears information.
    void clear(void);

  private:
    unsigned _state;
    const CDCTriggerWireHit * _hit;
    std::vector<CDCTriggerWireHitMC *> _mcHits;

    unsigned _id;
    unsigned _localId;
    const CDCTriggerLayer * _layer;
    HepGeom::Point3D<double>  _xyPosition;
    HepGeom::Point3D<double>  _forwardPosition;
    HepGeom::Point3D<double>  _backwardPosition;
    Vector3D _direction;
};

//-----------------------------------------------------------------------------

#ifdef CDCTriggerWire_NO_INLINE
#define inline
#else
#undef inline
#define CDCTriggerWire_INLINE_DEFINE_HERE
#endif

#ifdef CDCTriggerWire_INLINE_DEFINE_HERE

inline
unsigned
CDCTriggerWire::id(void) const {
    return _id;
}

inline
unsigned
CDCTriggerWire::localId(void) const {
    return _localId;
}

inline
unsigned
CDCTriggerWire::layerId(void) const {
    return _layer->id();
}

inline
unsigned
CDCTriggerWire::superLayerId(void) const {
    return _layer->superLayerId();
}

inline
unsigned
CDCTriggerWire::localLayerId(void) const {
    return _layer->localLayerId();
}

inline
const CDCTriggerLayer &
CDCTriggerWire::layer(void) const {
    return * _layer;
}

inline
unsigned
CDCTriggerWire::state(void) const {
    return _state;
}

inline
unsigned
CDCTriggerWire::state(unsigned a) {
    return _state = a;
}

inline
const CDCTriggerWireHit * const
CDCTriggerWire::hit(const CDCTriggerWireHit * const h) {
    return _hit = h;
}

inline
const CDCTriggerWireHit * const
CDCTriggerWire::hit(void) const {
    return _hit;
}

inline
const CDCTriggerWireHitMC * const
CDCTriggerWire::hit(CDCTriggerWireHitMC * const a) {
    _mcHits.push_back(a);
    return a;
}

inline
const HepGeom::Point3D<double>  &
CDCTriggerWire::forwardPosition(void) const {
    return _forwardPosition;
}

inline
const HepGeom::Point3D<double>  &
CDCTriggerWire::backwardPosition(void) const {
    return _backwardPosition;
}

inline
double *
CDCTriggerWire::backwardPosition(double p[3]) const {
    p[0] = _backwardPosition.x();
    p[1] = _backwardPosition.y();
    p[2] = _backwardPosition.z();
    return p;
}

inline
const HepGeom::Point3D<double>  &
CDCTriggerWire::xyPosition(void) const {
    return _xyPosition;
}

inline
double *
CDCTriggerWire::xyPosition(double a[3]) const {
    a[0] = _xyPosition.x();
    a[1] = _xyPosition.y();
    a[2] = 0.;
    return a;
}

inline
const Vector3D &
CDCTriggerWire::direction(void) const {
    return _direction;
}

inline
bool
CDCTriggerWire::axial(void) const {
    return _layer->axial();
}

inline
bool
CDCTriggerWire::stereo(void) const {
    return _layer->stereo();
}

inline
unsigned
CDCTriggerWire::axialStereoLayerId(void) const {
    return _layer->axialStereoLayerId();
}

inline
unsigned
CDCTriggerWire::axialStereoSuperLayerId(void) const {
    return _layer->axialStereoSuperLayerId();
}

inline
bool
CDCTriggerWire::innerPart(void) const {
    std::cout << "CDCTriggerWire::innerPart ... position not defined" << std::endl;
    if (layerId() < 14) return true;
    return false;
}

inline
bool
CDCTriggerWire::mainPart(void) const {
    std::cout << "CDCTriggerWire::mainPart ... position not defined" << std::endl;
    if (layerId() > 13) return true;
    return false;
}

inline
float
CDCTriggerWire::cellSize(void) const {
    if (_layer) {
	return _layer->cellSize();
    }
    else {
#ifdef TRASAN_DEBUG
	std::cout << "CDCTriggerWire::cellSize !!! can't return cell size";
	std::cout << " because no pointer to a layer" << std::endl;
#endif
	return 0.;
    }
}

inline
bool
CDCTriggerWire::consective(const CDCTriggerWire & w) const {
    if (neighbor(2) == & w) return true;
    else if (neighbor(3) == & w) return true;
    return false;
}

inline
bool
CDCTriggerWire::adjacent(const CDCTriggerWire & w) const {
    for (unsigned i = 0; i < 7; i++)
	if (neighbor(i) == & w)
	    return true;
    return false;
}

#endif

#undef inline

} // namespace Belle2

#endif /* CDCTriggerWire_FLAG_ */

