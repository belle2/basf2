/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/cdc/CircleFitter.h"
#include "trg/cdc/TrackBase.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Lpav.h"

using namespace std;

namespace Belle2 {

  TCCFitter::TCCFitter(const std::string& name)
    : TCFitter(name), _charge(0.), _radius(0.), _center(Point3D(0., 0., 0.))
  {
  }

  TCCFitter::~TCCFitter()
  {
  }

  int
  TCCFitter::fit(TCTBase& t) const
  {

    TRGDebug::enterStage("TCCFitter::fit");
//     int oldLevel = TRGDebug::level();
//     TRGDebug::level(10);

    //...Already fitted ?...
    if (t.fitted()) {
      if (TRGDebug::level() > 1)
        cout << TRGDebug::tab() << "Circle is fitted already" << endl;
      TRGDebug::leaveStage("TCCFitter::fit");
      return TRGCDCFitAlreadyFitted;
    }

    //...Check # of hits...
    if (t.links().size() < 3) {
      if (TRGDebug::level() > 1)
        cout << TRGDebug::tab() << "#links is less than 3" << endl;
      TRGDebug::leaveStage("TCCFitter::fit");
      return TRGCDCFitErrorFewHits;
    }

    //...Hit loop...
    TCLpav circle;
    const unsigned n = t.links().size();
    for (unsigned i = 0; i < n; i++) {
      const TCLink* l = t.links()[i];
      const Belle2::TRGCDCCellHit* h = l->hit();

      //...Check next hit...
      Point3D point;
      if (h->state() & CellHitPatternLeft)
        point = h->position(CellHitLeft);
      else if (h->state() & CellHitPatternRight)
        point = h->position(CellHitRight);
      else
        point = h->xyPosition();

      //...Presently weight is not used.
      // float weight = 1. / (h->distance() * h->distance());
      // float weight = 1. / h->distance();

      circle.add_point(point.x(), point.y()); //, weight);

      if (TRGDebug::level() > 2) {
        cout << TRGDebug::tab() << "point " << i;
        cout << point << endl;
      }
    }

    if (circle.fit() < 0.0 || circle.kappa() == 0.0) {
      if (TRGDebug::level() > 1)
        cout << TRGDebug::tab() << "fit failed" << endl;
      TRGDebug::leaveStage("TCCFitter::fit");
      return TRGCDCFitFailed;
    }
    CLHEP::HepVector v(circle.center());
    _center.setX(v(1));
    _center.setY(v(2));
    _radius = circle.radius();

    //...Determine charge...Better way???
    int qSum = 0;
    for (unsigned i = 0; i < n; i++) {
      const TCLink* l = t.links()[i];
      if (l == 0) continue;

      const Belle2::TRGCDCCellHit* h = l->hit();
      if (h == 0) continue;

      float q = (_center.cross(h->xyPosition())).z();
      if (q > 0.) qSum += 1;
      else        qSum -= 1;
    }
    if (qSum >= 0) _charge = +1.;
    else           _charge = -1.;
    _radius *= _charge;

    if (t.objectType() == TRGCDCCircleType)
      ((TRGCDCCircle&) t).property(_charge, _radius, _center);
    fitDone(t);

    //...Update link information...
    for (unsigned i = 0; i < n; i++) {
      TCLink* l = t.links()[i];
      if (l == 0) continue;

      t.approach2D(* l);
    }

    if (TRGDebug::level() > 1) {
      cout << TRGDebug::tab() << "fitted successfully" << endl;
      cout << TRGDebug::tab() << "    charge=" << _charge
           << ",radius=" << _radius << ",center=" << _center << endl;
    }
    TRGDebug::leaveStage("TCCFitter::fit");

    return 0;
  }

} // namespace Belle

