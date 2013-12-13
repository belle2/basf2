//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TCell.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell (a wire or a track
// segment) in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TCell_FLAG_
#define TCell_FLAG_

#include <vector>
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "tracking/modules/trasan/TLayer.h"

namespace Belle {

typedef HepGeom::Point3D<double>  Point3D;
typedef HepGeom::Vector3D<double>  Vector3D;

class TCDC;
class TCellHit;

/// A class to represent a wire in CDC.
class TCell {

  public:
    /// Constructor.
    TCell(unsigned id,
          unsigned localId,
          const TLayer & layer,
          const HepGeom::Point3D<double> & forwardPosition,
          const HepGeom::Point3D<double> & backwardPosition);

    /// Destructor
    virtual ~TCell();

  public:// Selectors

    /// returns id.
    unsigned id(void) const;

    /// returns local id in a layer.
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
    const TLayer & layer(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns true if this wire is in an axial layer.
    bool axial(void) const;

    /// returns true if this wire is in a stereo layer.
    bool stereo(void) const;

//     /// returns a pointer to a neighbor wire. This function is expensive.
//     virtual const TCell * const neighbor(unsigned) const = 0;

    /// returns local id difference.
    int localIdDifference(const TCell &) const;

    /// returns name.
    virtual std::string name(void) const = 0;

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
		      const std::string & prefix = std::string("")) const;

  public:// Geometry

    /// returns position in forward endplate.
    const HepGeom::Point3D<double> & forwardPosition(void) const;

    /// returns position in backward endplate.
    const HepGeom::Point3D<double> & backwardPosition(void) const;
    double * backwardPosition(double p[3]) const;

    /// returns middle position of a wire. z componet is 0.
    const HepGeom::Point3D<double> & xyPosition(void) const;
    double * xyPosition(double p[3]) const;

    /// returns direction vector of the wire.
    const Vector3D & direction(void) const;

    /// calculates position and direction vector with sag correction.
//     void wirePosition(float zPosition,
//                       HepGeom::Point3D<double>  & xyPosition,
//                       HepGeom::Point3D<double>  & backwardPosition,
//                       Vector3D & direction) const;

    /// returns cell size in phi.
    float cellSize(void) const;

  public:// Utility functions

    /// returns true this has member named a.
    virtual bool hasMember(const std::string & a) const;

  public:// event by event information.

    /// returns a pointer to a TCellHit.
    const TCellHit * hit(void) const;

  public:// Obsolete functions from Belle

    /// returns true if this wire is in the inner part.
    bool innerPart(void) const;

    /// returns true if this wire is in the main part.
    bool mainPart(void) const;

  public:// Modifiers

    /// sets state.
    unsigned state(unsigned newState);

    /// clears information.
    virtual void clear(void);

    /// sets a pointer to TWireHit.
    const TCellHit * hit(const TCellHit *);

  private:

    /// ID
    const unsigned _id;

    /// Local ID
    const unsigned _localId;

    /// Layer.
    const TLayer & _layer;

    /// Wire center(?) position.
    const HepGeom::Point3D<double> _xyPosition;

    /// Wire forward position.
    const HepGeom::Point3D<double> _forwardPosition;

    /// Wire backward position.
    const HepGeom::Point3D<double> _backwardPosition;

    /// Direction vector.
    const Vector3D _direction;

    /// Status in this event.
    unsigned _state;

    /// Cell hit.
    const TCellHit * _hit;

    /// Complete access from TCDC.
    friend class TCDC;
};

//-----------------------------------------------------------------------------

inline
unsigned
TCell::id(void) const {
    return _id;
}

inline
unsigned
TCell::localId(void) const {
    return _localId;
}

inline
unsigned
TCell::layerId(void) const {
    return _layer.id();
}

inline
unsigned
TCell::superLayerId(void) const {
    return _layer.superLayerId();
}

inline
unsigned
TCell::localLayerId(void) const {
    return _layer.localLayerId();
}

inline
const TLayer &
TCell::layer(void) const {
    return _layer;
}

inline
unsigned
TCell::state(void) const {
    return _state;
}

inline
unsigned
TCell::state(unsigned a) {
    return _state = a;
}

inline
bool
TCell::axial(void) const {
    return _layer.axial();
}

inline
bool
TCell::stereo(void) const {
    return _layer.stereo();
}

inline
unsigned
TCell::axialStereoLayerId(void) const {
    return _layer.axialStereoLayerId();
}

inline
unsigned
TCell::axialStereoSuperLayerId(void) const {
    return _layer.axialStereoSuperLayerId();
}

inline
bool
TCell::innerPart(void) const {
    std::cout << "TCell::innerPart ... position not defined" << std::endl;
    if (layerId() < 14) return true;
    return false;
}

inline
bool
TCell::mainPart(void) const {
    std::cout << "TCell::mainPart ... position not defined" << std::endl;
    if (layerId() > 13) return true;
    return false;
}

inline
float
TCell::cellSize(void) const {
    return _layer.cellSize();
}

inline
void
TCell::clear(void) {
    _state = 0;
    _hit = 0;
}

inline
const HepGeom::Point3D<double>  &
TCell::forwardPosition(void) const {
    return _forwardPosition;
}

inline
const HepGeom::Point3D<double>  &
TCell::backwardPosition(void) const {
    return _backwardPosition;
}

inline
double *
TCell::backwardPosition(double p[3]) const {
    p[0] = _backwardPosition.x();
    p[1] = _backwardPosition.y();
    p[2] = _backwardPosition.z();
    return p;
}

inline
const HepGeom::Point3D<double>  &
TCell::xyPosition(void) const {
    return _xyPosition;
}

inline
double *
TCell::xyPosition(double a[3]) const {
    a[0] = _xyPosition.x();
    a[1] = _xyPosition.y();
    a[2] = 0.;
    return a;
}

inline
const Vector3D &
TCell::direction(void) const {
    return _direction;
}

inline
const TCellHit *
TCell::hit(void) const {
    return _hit;
}

inline
const TCellHit *
TCell::hit(const TCellHit * a) {
    return _hit = a;
}

inline
bool
TCell::hasMember(const std::string & a) const {
    return name() == a;
}

} // namespace Belle

#endif /* TCell_FLAG_ */

