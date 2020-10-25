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
/// type of the wire
#define WireInnerLeft 0
/// type of the wire
#define WireInnerRight 1
/// type of the wire
#define WireLeft 2
/// type of the wire
#define WireRight 3
/// type of the wire
#define WireOuterLeft 4
/// type of the wire
#define WireOuterRight 5
/// Max number of neighbors
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
               const TRGCDCLayer&,
               const HepGeom::Point3D<double>& forwardPosition,
               const HepGeom::Point3D<double>& backwardPosition,
               const TRGClock& clock);

    /// Destructor
    virtual ~TRGCDCWire();

  public:// Selectors

    /// returns a pointer to a TRGCDCWireHit.
    const TRGCDCWireHit* hit(void) const;

//     /// returns a pointer to a TRGCDCTrackSegment.
//     const TRGCDCTrackSegment * const segment(void) const;

    /// returns a pointer to a neighbor wire. This function is expensive.
    const TRGCDCWire* neighbor(unsigned) const;

    /// returns true if a given wire is consective in a layer.
    bool consective(const TRGCDCWire&) const;

    /// returns true if a given wire is adjacent.
    bool adjacent(const TRGCDCWire&) const;

    /// returns localId but if maxLocalId, return -1.
    int localIdForPlus(void) const;

    /// returns localId but if 0, return maxLocalId + 1.
    int localIdForMinus(void) const;

    /// returns name.
    std::string name(void) const override;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const override;

  public:// Modifiers

    /// clears information.
    void clear(void) override;

    /// returns a pointer to a TRGCDCWireHit.
    const TRGCDCWireHit* hit(const TRGCDCWireHit*);

    /// appends a pointer to TRGCDCWireHitMC.
    const TRGCDCWireHitMC* hit(TRGCDCWireHitMC*);

    /// set signal |= newSignal
    void addSignal(const TRGSignal& newSignal);

  public:// TRG

    /// returns an input to the trigger. This is sync'ed to 1GHz clock.
    const TRGSignal& signal(void) const override;

  private:

    /// MC wire hit.
//  std::vector<const TRGCDCWireHitMC * const> _mcHits;
    std::vector<const TRGCDCWireHitMC*> _mcHits;

    /// Trigger output.
    mutable TRGSignal _signal;

    /// Complete access from TRGCDC.
    friend class TRGCDC;
  };

//-----------------------------------------------------------------------------

  inline
  const TRGCDCWireHit*
  TRGCDCWire::hit(const TRGCDCWireHit* h)
  {
    //TODO only this strong retyping work, is it OK?
    return reinterpret_cast<const TRGCDCWireHit*>(TRGCDCCell::hit(reinterpret_cast<const TRGCDCCellHit*>(h)));
  }

  inline
  const TRGCDCWireHit*
  TRGCDCWire::hit(void) const
  {
    //TODO only this strong retyping work, is it OK?
    return reinterpret_cast<const TRGCDCWireHit*>(TRGCDCCell::hit());
  }

  inline
  const TRGCDCWireHitMC*
  TRGCDCWire::hit(TRGCDCWireHitMC* const a)
  {
    _mcHits.push_back(a);
    return a;
  }

  inline
  void
  TRGCDCWire::addSignal(const TRGSignal& newSignal)
  {
    _signal |= newSignal;
  }

  inline
  bool
  TRGCDCWire::consective(const TRGCDCWire& w) const
  {
    if (neighbor(2) == & w) return true;
    else if (neighbor(3) == & w) return true;
    return false;
  }

  inline
  bool
  TRGCDCWire::adjacent(const TRGCDCWire& w) const
  {
    for (unsigned i = 0; i < 7; i++)
      if (neighbor(i) == & w)
        return true;
    return false;
  }

  inline
  int
  TRGCDCWire::localIdForPlus(void) const
  {
    //
    // used in curl finder
    //
    // new version by NK
    //
    //ho  const TCDCLayer &l = *layer();
    //ho  const int nw = l.nWires();

    const TRGCDCLayer& l = this->layer();
    const unsigned nw = l.nCells();
    if (localId() + 1 == nw)
      return -1;
    else
      return localId();
  }

  inline
  int
  TRGCDCWire::localIdForMinus(void) const
  {
    //
    // used in curl finder
    //
    // new version by NK
    //
    //ho  const TRGCDCLayer &l = *layer();
    //ho const int nw = l.nWires();

    const TRGCDCLayer& l = this->layer();
    const unsigned nw = l.nCells();
    if (0 == localId())
      return nw;
    else
      return localId();
  }

  inline
  const TRGSignal&
  TRGCDCWire::signal(void) const
  {
    return _signal;
  }

} // namespace Belle2

#endif /* TRGCDCWire_FLAG_ */

