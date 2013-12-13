//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWire.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TWire_FLAG_
#define TWire_FLAG_

#include <vector>
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "tracking/modules/trasan/TCell.h"
#include "tracking/modules/trasan/TLayer.h"

namespace Belle {

typedef HepGeom::Point3D<double> Point3D;
typedef HepGeom::Vector3D<double> Vector3D;
#define WireInnerLeft 0
#define WireInnerRight 1
#define WireLeft 2
#define WireRight 3
#define WireOuterLeft 4
#define WireOuterRight 5
#define MaxNeighbors 6

class TCDC;
class TWireHit;
class TWireHitMC;

/// A class to represent a wire in CDC.
class TWire : public TCell {

  public:
    /// Constructor.
    TWire(unsigned id,
	  unsigned localId,
	  const TLayer &,
	  const HepGeom::Point3D<double> & forwardPosition,
	  const HepGeom::Point3D<double> & backwardPosition);

    /// Destructor
    virtual ~TWire();

  public:// Selectors

    /// returns a pointer to a TWireHit.
    const TWireHit * hit(void) const;

    /// returns a pointer to a neighbor wire. This function is expensive.
    const TWire * neighbor(unsigned) const;

    /// returns true if a given wire is consective in a layer.
    bool consective(const TWire &) const;

    /// returns true if a given wire is adjacent.
    bool adjacent(const TWire &) const;

    /// returns localId but if maxLocalId, return -1.
    int localIdForPlus(void) const;

    /// returns localId but if 0, return maxLocalId + 1.
    int localIdForMinus(void) const;

    /// returns name.
    std::string name(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
              const std::string & prefix = std::string("")) const;

  public:// Modifiers

    /// clears information.
    void clear(void);

    /// returns a pointer to a TWireHit.
    const TWireHit * hit(const TWireHit *);

    /// appends a pointer to TWireHitMC.
    const TWireHitMC * hit(TWireHitMC *);

  private:

    /// MC wire hit.
//  std::vector<const TWireHitMC * const> _mcHits;
    std::vector<const TWireHitMC *> _mcHits;

    /// Complete access from T.
    friend class TCDC;
};

//-----------------------------------------------------------------------------

inline
const TWireHit *
TWire::hit(const TWireHit * h) {
    return (const TWireHit *) TCell::hit((const TCellHit *) h);
}

inline
const TWireHit *
TWire::hit(void) const {
    return (const TWireHit *) TCell::hit();
}

inline
const TWireHitMC *
TWire::hit(TWireHitMC * const a) {
    _mcHits.push_back(a);
    return a;
}

inline
bool
TWire::consective(const TWire & w) const {
    if (neighbor(2) == & w) return true;
    else if (neighbor(3) == & w) return true;
    return false;
}

inline
bool
TWire::adjacent(const TWire & w) const {
    for (unsigned i = 0; i < 7; i++)
        if (neighbor(i) == & w)
            return true;
    return false;
}

inline 
int
TWire::localIdForPlus(void) const {
    //
    // used in curl finder
    //
    // new version by NK
    //
    //ho  const TCDCLayer &l = *layer();
    //ho  const int nw = l.nWires();

    const TLayer & l = this->layer();
    const unsigned nw = l.nCells();
    if (localId() + 1 == nw)
	return -1;
    else
	return localId();
}
 
inline
int
TWire::localIdForMinus(void) const {
    //
    // used in curl finder
    //
    // new version by NK
    //
    //ho  const TLayer &l = *layer();
    //ho const int nw = l.nWires();

    const TLayer &l = this->layer();
    const unsigned nw = l.nCells();
    if (0 == localId())
        return nw;
    else
        return localId();
}

} // namespace Belle

#endif /* TWire_FLAG_ */

