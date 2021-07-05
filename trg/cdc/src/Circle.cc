/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a circle
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/trg/Constants.h"
#include "trg/trg/Point2D.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/CircleFitter.h"
#include "trg/cdc/HoughPlane.h"

using namespace std;

namespace Belle2 {

  const TRGCDCCircleFitter
  TRGCDCCircle::_fitter = TRGCDCCircleFitter("TRGCDCCircle Default Fitter");

  TRGCDCCircle::TRGCDCCircle(const std::vector<TCLink*>& links)
    : TCTBase("UnknownCircle", 0),
      _center(ORIGIN),
      _radius(0),
      _plane(0)
  {
    fitter(& _fitter);
    append(links);
  }

  TRGCDCCircle::TRGCDCCircle(double r,
                             double phi,
                             double charge,
                             const TCHPlane& plane)
    : TCTBase("unknown", charge),
      _center(r * cos(phi), r * sin(phi)),
      _radius(r),
      _plane(& plane)
  {
    fitter(& _fitter);
  }

  TRGCDCCircle::~TRGCDCCircle()
  {
  }

  void
  TRGCDCCircle::dump(const string& cmd, const string& pre) const
  {
    cout << pre;
    cout << "c=" << _center;
    cout << ":r=" << _radius;
    if (_plane) cout << ":hp=" << _plane->name();
    cout << endl;
    if (cmd.find("detail") != string::npos)
      TRGCDCTrackBase::dump(cmd, pre);
  }

  int
  TRGCDCCircle::approach2D(TCLink& l) const
  {
    HepGeom::Point3D<double> xw = l.cell()->xyPosition();
    HepGeom::Point3D<double> xc(_center.x(), _center.y(), 0.);

    xw.setZ(0.);
    const HepGeom::Point3D<double> xv
      = _charge * _radius * (xw - xc).unit() + xc;
    l.positionOnTrack(xv);
    l.positionOnWire(xw);
    l.dPhi(0.);
    return 0;
  }

} // namespace Belle2
