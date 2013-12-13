//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWireHitMC.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a MC wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TWireHitMC_FLAG_
#define TWireHitMC_FLAG_

#include <string>
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

namespace Belle {

class TWire;
class TWireHit;
class TTrackMC;

/// A class to represent a MC wire hit in CDC.
class TWireHitMC {

  public:
    /// Constructor.
    TWireHitMC(const TWire *, const TWireHit *);

    /// Destructor
    virtual ~TWireHitMC();

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

    /// returns a pointer to a TWire.
    const TWire * wire(void) const;

    /// returns a pointer to a GEN_HEPEVT.
    const TTrackMC * hep(void) const;

    /// returns a pointer to a TWireHit.
    const TWireHit * hit(void) const;

  private:
    const TWire * _wire;
    const TWireHit * _hit;
    HepGeom::Point3D<double>  _position;
    HepGeom::Point3D<double>  _entrance;
    HepGeom::Vector3D<double>  _direction;
    CLHEP::Hep3Vector _momentum;
    float _distance;
    float _energy;
    int _leftRight;
    const TTrackMC * _hep;

    friend class TCDC;
};

//-----------------------------------------------------------------------------

inline
float
TWireHitMC::distance(void) const {
    return _distance;
}

inline
const HepGeom::Point3D<double>  &
TWireHitMC::hitPosition(void) const {
    return _position;
}

inline
const HepGeom::Point3D<double>  &
TWireHitMC::entrance(void) const {
    return _entrance;
}

inline
const HepGeom::Vector3D<double>  &
TWireHitMC::direction(void) const {
    return _direction;
}

inline
int
TWireHitMC::leftRight(void) const {
    return _leftRight;
}

inline
const TWire *
TWireHitMC::wire(void) const {
    return _wire;
}

inline
const TTrackMC *
TWireHitMC::hep(void) const {
    return _hep;
}

inline
const TWireHit *
TWireHitMC::hit(void) const {
    return _hit;
}

inline
const CLHEP::Hep3Vector &
TWireHitMC::momentum(void) const {
    return _momentum;
}

} // namespace Belle

#endif /* TWireHitMC_FLAG_ */
