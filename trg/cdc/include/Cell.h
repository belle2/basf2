/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a cell (a wire or a track segment) in CDC.
//-----------------------------------------------------------------------------

#ifndef TRGCDCCell_FLAG_
#define TRGCDCCell_FLAG_

#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "trg/trg/Signal.h"
#include "trg/cdc/Layer.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCCell TRGCDCCell
#endif

namespace Belle2 {

  //! 3D point
  typedef HepGeom::Point3D<double>  Point3D;
  //! 3D Vector
  typedef HepGeom::Vector3D<double>  Vector3D;

  class TRGCDC;
  class TRGCDCCellHit;

/// A class to represent a wire in CDC.
  class TRGCDCCell {

  public:
    /// Constructor.
    TRGCDCCell(unsigned id,
               unsigned localId,
               const TRGCDCLayer& layer,
               const HepGeom::Point3D<double>& forwardPosition,
               const HepGeom::Point3D<double>& backwardPosition);

    /// Destructor
    virtual ~TRGCDCCell();

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
    const TRGCDCLayer& layer(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns true if this wire is in an axial layer.
    bool axial(void) const;

    /// returns true if this wire is in a stereo layer.
    bool stereo(void) const;

//     /// returns a pointer to a neighbor wire. This function is expensive.
//     virtual const TRGCDCCell * const neighbor(unsigned) const = 0;

    /// returns local id difference.
    int localIdDifference(const TRGCDCCell&) const;

    /// returns name.
    virtual std::string name(void) const = 0;

    /// dumps debug information.
    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const;

    /// returns trigger output. Null will returned if no signal.
    virtual const TRGSignal& signal(void) const = 0;

  public:// Geometry

    /// returns position in forward endplate.
    const HepGeom::Point3D<double>& forwardPosition(void) const;

    /// returns position in backward endplate.
    const HepGeom::Point3D<double>& backwardPosition(void) const;
    /// position in backward endplate.
    double* backwardPosition(double p[3]) const;

    /// returns middle position of a wire. z componet is 0.
    const HepGeom::Point3D<double>& xyPosition(void) const;
    /// middle position of a wire. z componet is 0.
    double* xyPosition(double p[3]) const;

    /// returns direction vector of the wire.
    const Vector3D& direction(void) const;

    /// calculates position and direction vector with sag correction.
//     void wirePosition(float zPosition,
//                       HepGeom::Point3D<double>  & xyPosition,
//                       HepGeom::Point3D<double>  & backwardPosition,
//                       Vector3D & direction) const;

    /// returns cell size in phi.
    float cellSize(void) const;

  public:// Utility functions

    /// returns true this has member named a.
    virtual bool hasMember(const std::string& a) const;

  public:// event by event information.

    /// returns a pointer to a TRGCDCCellHit.
    const TRGCDCCellHit* hit(void) const;

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

    /// sets a pointer to TRGCDCWireHit.
    const TRGCDCCellHit* hit(const TRGCDCCellHit*);

  private:

    /// ID
    const unsigned _id;

    /// Local ID
    const unsigned _localId;

    /// Layer.
    const TRGCDCLayer& _layer;

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
    const TRGCDCCellHit* _hit;

    /// Complete access from TRGCDC.
    friend class TRGCDC;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TRGCDCCell::id(void) const
  {
    return _id;
  }

  inline
  unsigned
  TRGCDCCell::localId(void) const
  {
    return _localId;
  }

  inline
  unsigned
  TRGCDCCell::layerId(void) const
  {
    return _layer.id();
  }

  inline
  unsigned
  TRGCDCCell::superLayerId(void) const
  {
    return _layer.superLayerId();
  }

  inline
  unsigned
  TRGCDCCell::localLayerId(void) const
  {
    return _layer.localLayerId();
  }

  inline
  const TRGCDCLayer&
  TRGCDCCell::layer(void) const
  {
    return _layer;
  }

  inline
  unsigned
  TRGCDCCell::state(void) const
  {
    return _state;
  }

  inline
  unsigned
  TRGCDCCell::state(unsigned a)
  {
    return _state = a;
  }

  inline
  bool
  TRGCDCCell::axial(void) const
  {
    return _layer.axial();
  }

  inline
  bool
  TRGCDCCell::stereo(void) const
  {
    return _layer.stereo();
  }

  inline
  unsigned
  TRGCDCCell::axialStereoLayerId(void) const
  {
    return _layer.axialStereoLayerId();
  }

  inline
  unsigned
  TRGCDCCell::axialStereoSuperLayerId(void) const
  {
    return _layer.axialStereoSuperLayerId();
  }

  inline
  bool
  TRGCDCCell::innerPart(void) const
  {
    std::cout << "TRGCDCCell::innerPart ... position not defined" << std::endl;
    if (layerId() < 14) return true;
    return false;
  }

  inline
  bool
  TRGCDCCell::mainPart(void) const
  {
    std::cout << "TRGCDCCell::mainPart ... position not defined" << std::endl;
    if (layerId() > 13) return true;
    return false;
  }

  inline
  float
  TRGCDCCell::cellSize(void) const
  {
    return _layer.cellSize();
  }

  inline
  void
  TRGCDCCell::clear(void)
  {
    _state = 0;
    _hit = 0;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCCell::forwardPosition(void) const
  {
    return _forwardPosition;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCCell::backwardPosition(void) const
  {
    return _backwardPosition;
  }

  inline
  double*
  TRGCDCCell::backwardPosition(double p[3]) const
  {
    p[0] = _backwardPosition.x();
    p[1] = _backwardPosition.y();
    p[2] = _backwardPosition.z();
    return p;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCCell::xyPosition(void) const
  {
    return _xyPosition;
  }

  inline
  double*
  TRGCDCCell::xyPosition(double a[3]) const
  {
    a[0] = _xyPosition.x();
    a[1] = _xyPosition.y();
    a[2] = 0.;
    return a;
  }

  inline
  const Vector3D&
  TRGCDCCell::direction(void) const
  {
    return _direction;
  }

  inline
  const TRGCDCCellHit*
  TRGCDCCell::hit(void) const
  {
    return _hit;
  }

  inline
  const TRGCDCCellHit*
  TRGCDCCell::hit(const TRGCDCCellHit* a)
  {
    return _hit = a;
  }

  inline
  bool
  TRGCDCCell::hasMember(const std::string& a) const
  {
    return name() == a;
  }

} // namespace Belle2

#endif /* TRGCDCCell_FLAG_ */

