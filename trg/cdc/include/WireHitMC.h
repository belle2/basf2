/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : WireHitMC.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a MC wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCWireHitMC_FLAG_
#define TRGCDCWireHitMC_FLAG_

#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCWHitMC TRGCDCWireHitMC
#endif

namespace Belle2 {

  class TRGCDCWire;
  class TRGCDCWireHit;
  class TRGCDCTrackMC;

/// A class to represent a MC wire hit in CDC.
  class TRGCDCWireHitMC {

  public:
    /// Constructor.
    TRGCDCWireHitMC(const TRGCDCWire*, const TRGCDCWireHit*);

    /// Destructor
    virtual ~TRGCDCWireHitMC();

  public:// Selectors
    /// returns drift distance.
    float distance(void) const;

    /// returns hit position.
    const HepGeom::Point3D<double>&   hitPosition(void) const;

    /// returns an entrance point.
    const HepGeom::Point3D<double>&   entrance(void) const;

    /// returns vector from entrance to exit point.
    const HepGeom::Vector3D<double>&   direction(void) const;

    /// returns momentum vector at the entrance.
    const CLHEP::Hep3Vector& momentum(void) const;

    /// returns left or right.
    int leftRight(void) const;

    /// returns a pointer to a TRGCDCWire.
    const TRGCDCWire* wire(void) const;

    /// returns a pointer to a GEN_HEPEVT.
    const TRGCDCTrackMC* hep(void) const;

    /// returns a pointer to a TRGCDCWireHit.
    const TRGCDCWireHit* hit(void) const;

  private:
    /// pointer to TRGCDCWire
    const TRGCDCWire* _wire;
    /// pointer to TRGCDCWireHit
    const TRGCDCWireHit* _hit;
    /// position vector
    HepGeom::Point3D<double>  _position;
    /// entrance vector
    HepGeom::Point3D<double>  _entrance;
    /// direction vector
    HepGeom::Vector3D<double>  _direction;
    /// momentum
    CLHEP::Hep3Vector _momentum;
    /// distance
    float _distance;
    /// energy
    float _energy;
    /// pointer to TRGCDCTrackMC
    const TRGCDCTrackMC* _hep;
    /// leftright info
    int _leftRight;

    friend class TRGCDC;
  };

//-----------------------------------------------------------------------------

  inline
  float
  TRGCDCWireHitMC::distance(void) const
  {
    return _distance;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCWireHitMC::hitPosition(void) const
  {
    return _position;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCWireHitMC::entrance(void) const
  {
    return _entrance;
  }

  inline
  const HepGeom::Vector3D<double>&
  TRGCDCWireHitMC::direction(void) const
  {
    return _direction;
  }

  inline
  int
  TRGCDCWireHitMC::leftRight(void) const
  {
    return _leftRight;
  }

  inline
  const TRGCDCWire*
  TRGCDCWireHitMC::wire(void) const
  {
    return _wire;
  }

  inline
  const TRGCDCTrackMC*
  TRGCDCWireHitMC::hep(void) const
  {
    return _hep;
  }

  inline
  const TRGCDCWireHit*
  TRGCDCWireHitMC::hit(void) const
  {
    return _hit;
  }

  inline
  const CLHEP::Hep3Vector&
  TRGCDCWireHitMC::momentum(void) const
  {
    return _momentum;
  }

} // namespace Belle2

#endif /* TRGCDCWireHitMC_FLAG_ */
