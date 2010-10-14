//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerWireHitMC.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a MC wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef CDCTriggerWireHitMC_FLAG_
#define CDCTriggerWireHitMC_FLAG_

#include <string>

#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#define CTWHitMC CDCTriggerWireHitMC
#endif

namespace Belle2 {

class CDCTriggerWire;
class CDCTriggerWireHit;
class CDCTriggerTrackMC;

/// A class to represent a MC wire hit in CDC.
class CDCTriggerWireHitMC {

  public:
    /// Constructor.
    CDCTriggerWireHitMC(const CDCTriggerWire *, const CDCTriggerWireHit *);

    /// Destructor
    virtual ~CDCTriggerWireHitMC();

  public:// Selectors
    /// returns drift distance.
    float distance(void) const;

    /// returns hit position.
    const HepGeom::Point3D<double>  & hitPosition(void) const;

    /// returns an entrance point.
    const HepGeom::Point3D<double>  & entrance(void) const;

    /// returns vector from entrance to exit point.
    const HepGeom::Vector3D<double>  & direction(void) const;

    /// returns momentum vector at the entrance.
    const CLHEP::Hep3Vector & momentum(void) const;

    /// returns left or right.
    int leftRight(void) const;

    /// returns a pointer to a CDCTriggerWire.
    const CDCTriggerWire * const wire(void) const;

    /// returns a pointer to a GEN_HEPEVT.
    const CDCTriggerTrackMC * const hep(void) const;

    /// returns a pointer to a CDCTriggerWireHit.
    const CDCTriggerWireHit * const hit(void) const;

  private:
    const CDCTriggerWire * _wire;
    const CDCTriggerWireHit * _hit;
    HepGeom::Point3D<double>  _position;
    HepGeom::Point3D<double>  _entrance;
    HepGeom::Vector3D<double>  _direction;
    CLHEP::Hep3Vector _momentum;
    float _distance;
    float _energy;
    int _leftRight;
    const CDCTriggerTrackMC * _hep;

  friend class CDCTrigger;
};

//-----------------------------------------------------------------------------

#ifdef CDCTriggerWireHitMC_NO_INLINE
#define inline
#else
#undef inline
#define CDCTriggerWireHitMC_INLINE_DEFINE_HERE
#endif

#ifdef CDCTriggerWireHitMC_INLINE_DEFINE_HERE

inline
float
CDCTriggerWireHitMC::distance(void) const {
    return _distance;
}

inline
const HepGeom::Point3D<double>  &
CDCTriggerWireHitMC::hitPosition(void) const {
    return _position;
}

inline
const HepGeom::Point3D<double>  &
CDCTriggerWireHitMC::entrance(void) const {
    return _entrance;
}

inline
const HepGeom::Vector3D<double>  &
CDCTriggerWireHitMC::direction(void) const {
    return _direction;
}

inline
int
CDCTriggerWireHitMC::leftRight(void) const {
    return _leftRight;
}

inline
const CDCTriggerWire * const
CDCTriggerWireHitMC::wire(void) const {
    return _wire;
}

inline
const CDCTriggerTrackMC * const
CDCTriggerWireHitMC::hep(void) const {
    return _hep;
}

inline
const CDCTriggerWireHit * const 
CDCTriggerWireHitMC::hit(void) const {
    return _hit;
}

inline
const CLHEP::Hep3Vector &
CDCTriggerWireHitMC::momentum(void) const {
    return _momentum;
}

#endif

#undef inline

} // namespace Belle2

#endif /* CDCTriggerWireHitMC_FLAG_ */
