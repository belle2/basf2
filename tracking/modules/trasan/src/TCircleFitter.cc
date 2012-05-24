//-----------------------------------------------------------------------------
// $Id: TCircleFitter.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TCircleFitter.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a circle.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2005/11/03 23:20:11  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.6  2001/12/23 09:58:46  katayama
// removed Strings.h
//
// Revision 1.5  2001/12/19 02:59:45  katayama
// Uss find,istring
//
// Revision 1.4  2001/04/11 01:09:09  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.3  1999/10/30 10:12:16  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.2  1999/03/11 23:27:12  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.1  1999/01/11 03:03:10  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TCircleFitter.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/Lpav.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  TCircleFitter::TCircleFitter(const std::string& name)
    : TFitter(name), _charge(0.), _radius(0.), _center(Point3D(0., 0., 0.))
  {
  }

  TCircleFitter::~TCircleFitter()
  {
  }

  int
  TCircleFitter::fit(TTrackBase& t) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TCrclFitter::fit";
    EnterStage(stage);
#endif

    //...Already fitted ?...
    if (t.fitted()) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return TFitAlreadyFitted;
    }

    //...Check # of hits...
    if (t.links().length() < 3) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return TFitErrorFewHits;
    }

    //...Hit loop...
    Lpav circle;
    unsigned n = t.links().length();
    for (unsigned i = 0; i < n; i++) {
      TLink* l = t.links()[i];
      const Belle2::TRGCDCWireHit* h = l->hit();

      //...Check next hit...
      Point3D point;
      if (h->state() & CellHitPatternLeft)
        point = h->position(CellHitLeft);
      else if (h->state() & CellHitPatternRight)
        point = h->position(CellHitRight);
      else
        point = h->xyPosition();
      // float weight = 1. / (h->distance() * h->distance());
      // float weight = 1. / h->distance();

      circle.add_point(point.x(), point.y()); //, weight);
    }

    if (circle.fit() < 0.0 || circle.kappa() == 0.0) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return TFitFailed;
    }
    CLHEP::HepVector v(circle.center());
    _center.setX(v(1));
    _center.setY(v(2));
    _radius = circle.radius();

    //...Determine charge...Better way???
    int qSum = 0;
    for (unsigned i = 0; i < n; i++) {
      TLink* l = t.links()[i];
      if (l == 0) continue;

      const Belle2::TRGCDCWireHit* h = l->hit();
      if (h == 0) continue;

      float q = (_center.cross(h->xyPosition())).z();
      if (q > 0.) qSum += 1;
      else        qSum -= 1;
    }
    if (qSum >= 0) _charge = +1.;
    else           _charge = -1.;
    _radius *= _charge;

    if (t.objectType() == Circle)
      ((TCircle&) t).property(_charge, _radius, _center);
    fitDone(t);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return 0;
  }

} // namespace Belle

