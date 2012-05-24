//-----------------------------------------------------------------------------
// $Id: TCircle.cc 10660 2008-09-25 04:27:48Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TCircle.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a circle in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.26  2005/11/03 23:20:11  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.25  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.24  2004/03/26 06:07:03  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.23  2003/12/25 12:03:32  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.22  2001/12/23 09:58:46  katayama
// removed Strings.h
//
// Revision 1.21  2001/12/19 02:59:45  katayama
// Uss find,istring
//
// Revision 1.20  2000/10/05 23:54:21  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.19  2000/04/11 13:05:42  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.18  2000/02/17 13:24:19  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.17  1999/10/30 10:12:15  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.16  1999/01/11 03:03:08  yiwasaki
// Fitters added
//
// Revision 1.15  1998/11/10 09:09:03  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.14  1998/11/06 02:31:49  yiwasaki
// 0 div protection from Katayama san
//
// Revision 1.13  1998/09/29 01:24:22  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.12  1998/07/29 04:34:49  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.10  1998/07/06 15:48:46  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.9  1998/06/15 09:58:14  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.8  1998/06/14 11:09:48  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.7  1998/06/11 08:14:05  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.6  1998/05/22 08:23:17  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.5  1998/05/08 09:45:43  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.4  1998/04/23 17:21:36  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.3  1998/04/16 16:49:32  yiwasaki
// minor changes
//
// Revision 1.2  1998/04/10 09:36:26  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:50:14  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TCircle.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TLink.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  const TCircleFitter
  TCircle::_fitter = TCircleFitter("TCircle Default Circle Fitter");

  TCircle::TCircle(const TCircle& a)
    : TTrackBase((TTrackBase&) a),
//  _circle(a._circle),
      _radius(a._radius),
      _center(a._center)
  {
  }

  TCircle::TCircle(const AList<TLink> & a)
    : TTrackBase(a), _charge(0.), _radius(0.)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TCircle";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "making a circle:#links=" << a.length() << std::endl;
#endif

    //...Set a defualt fitter...
    fitter(& TCircle::_fitter);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  TCircle::TCircle(const TTrack& t)
//    : TTrackBase((TTrackBase &) t),
    : TTrackBase(t.links()),
      _charge(t.charge()),
      _radius(fabs(t.radius())),
      _center(t.center().x(), t.center().y(), 0)
  {

    //...Set a defualt fitter...
    fitter(& TCircle::_fitter);
  }

  TCircle::TCircle(float r, float phi, float charge)
    : TTrackBase(),
      _charge(charge),
      _radius(r),
      _center(r* cos(phi), r* sin(phi), 0)
  {

    //...Set a defualt fitter...
    fitter(& TCircle::_fitter);

    //...Because no link...
    _fitted = true;
  }

  TCircle::TCircle(const TPoint2D& c, float r, float charge)
    : TTrackBase(),
      _charge(charge),
      _radius(r),
      _center(c.y() * cos(c.x()), c.y() * sin(c.x()), 0)
  {

    //...Set a defualt fitter...
    fitter(& TCircle::_fitter);

    //...Because no link...
    _fitted = true;
  }

  TCircle::~TCircle()
  {
  }

  void
  TCircle::dump(const std::string& msg, const std::string& pre) const
  {
    bool def = false;
    if (msg == "") def = true;

    if (def ||
        msg.find("circle") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "#links=" << _links.length();
      if (_fitted) {
        std::cout << ",charge=" << _charge;
        std::cout << ",center=" << _center;
        std::cout << ",radius=" << _radius;
        std::cout << std::endl << pre;
        std::cout << "pt=" << pt();
        std::cout << ",impact=" << impact();
        std::cout << std::endl;
      } else {
        std::cout << ",not fitted yet" << std::endl;
      }
    }
    if (! def) TTrackBase::dump(msg, pre);
  }

//  int
//  TCircle::fitx(void) {
//  #ifdef TRASAN_DEBUG_DETAIL
// std::cout << "    TCircle::fit ..." << std::endl;
//  #endif
//      if (_fitted) return 0;
//      unsigned n = _links.length();
//      if (n < 3) return -1;

//      for (unsigned i = 0; i < n; i++) {
//    TLink * l = _links[i];
//    const TRGCDCWireHit * h = l->hit();

//    //...Check next hit...
//    Point3D point;
//    if (h->state() & WireHitPatternLeft)
//        point = h->position(WireHitLeft);
//    else if (h->state() & WireHitPatternRight)
//        point = h->position(WireHitRight);
//    else
//        point = h->xyPosition();
//    // float weight = 1. / (h->distance() * h->distance());
//    // float weight = 1. / h->distance();

//    _circle.add_point(point.x(), point.y()); //, weight);
//      }

//      if(_circle.fit()<0.0 || _circle.kappa()==0.0) return -1;
//      CLHEP::HepVector v(_circle.center());
//      _center.setX(v(1));
//      _center.setY(v(2));
//      _radius = _circle.radius();

//      //...Determine charge...Better way???
//      int qSum = 0;
//      for (unsigned i = 0; i < n; i++) {
//    TLink * l = _links[i];
//    if (l == 0) continue;

//    const TRGCDCWireHit * h = l->hit();
//    if (h == 0) continue;

//    float q = (_center.cross(h->xyPosition())).z();
//    if (q > 0.) qSum += 1;
//    else        qSum -= 1;
//      }
//      if (qSum >= 0) _charge = +1.;
//      else           _charge = -1.;
//      _radius *= _charge;

//      _fitted = true;
//      return 0;
//  }

  double
  TCircle::weight(const TLink& l) const
  {

    //...Axial Wires
    int maxLink = 0;
    int localID[7];
    int layerID[7];
    int LayerID = l.hit()->wire().layerId();
    for (int i = 0; i < 7; i++) {
      if (l.neighbor(i)) {
        maxLink = i;
        localID[i] = l.neighbor(i)->hit()->wire().localId();
        layerID[i] = l.neighbor(i)->hit()->wire().layerId();
      } else {
        break;
      }
    }
    if (maxLink != 1)return 1.0;
    if (layerID[0] == LayerID &&
        layerID[1] == LayerID)return 0.5;
    if (layerID[0] + layerID[1] != LayerID * 2)return 1.0;
    if (localID[0] != localID[1])return 1.5; //1.0 or 2.0 ??
    return 1.0;
  }

  int
  TCircle::fitForCurl(int ipConst)
  {
    unsigned n = _links.length();
    if (n < 3) return -1;

    //IP check
    unsigned flagIP = 1;
    unsigned layerID = _links[0]->hit()->wire().layerId();
    for (unsigned i = 0; i < n; i++) {
      if (layerID != _links[i]->hit()->wire().layerId()) {
        flagIP = 0;
        break;
      }
    }
    if (ipConst != 0)flagIP = 1;
    if (flagIP == 1) {
      _circle.add_point(0., 0., 0.5);
      //++_nPointsForFit;
    }
    for (unsigned i = 0; i < n; i++) {
      TLink* l = _links[i];
      if (l == 0) continue;

      const Belle2::TRGCDCWireHit* h = l->hit();
      if (h == 0) continue;

      //...Check next hit...
      HepGeom::Point3D<double> point;
      point = h->xyPosition();

      double weight = 1.0;
      weight = this->weight(* l);

      _circle.add_point(point.x(), point.y(), weight);
      //++_nPointsForFit;
    }

    //if (_nPointsForFit < 3) return -1;

    if (_circle.fit() < 0.0 || _circle.kappa() == 0.0) return -1;
    CLHEP::HepVector v(_circle.center());
    _center.setX(v(1));
    _center.setY(v(2));
    _radius = _circle.radius();

    //...Determine charge...Better way???
    int qSum = 0;
    for (unsigned i = 0; i < n; i++) {
      TLink* l = _links[i];
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

    _fitted = true;
    return 0;
  }

} // namespace Belle

