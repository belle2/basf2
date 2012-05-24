//-----------------------------------------------------------------------------
// $Id: TSegment.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSegment.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to manage a group of TLink's.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.21  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.20  2003/12/19 07:36:03  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.19  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.18  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.17  2000/10/05 23:54:25  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.16  2000/04/11 13:05:46  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.15  2000/02/29 07:16:16  yiwasaki
// Trasan 2.00RC13 : default stereo param. changed
//
// Revision 1.14  2000/02/23 08:45:06  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.13  2000/02/15 13:46:42  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.12  2000/02/10 13:11:39  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.11  2000/01/30 08:17:07  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.10  2000/01/28 06:30:24  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.9  1999/11/19 09:13:10  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.8  1999/10/30 10:12:21  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.7  1999/10/21 15:45:15  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.6  1999/01/11 03:03:13  yiwasaki
// Fitters added
//
// Revision 1.5  1998/11/10 23:09:20  katayama
// Fix it right
//
// Revision 1.4  1998/11/10 09:09:07  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.3  1998/07/29 04:34:54  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.1  1998/07/02 10:15:31  yiwasaki
// TSegment.cc,h added
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TSegment.h"

#include "tracking/modules/trasan/TLink.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/Range.h"

#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  TSegment::TSegment()
    : TTrackBase(),
      _state(0),
      _innerWidth(0),
      _outerWidth(0),
      _nLayer(0),
      _clusterType(0),
      _duality(0.),
      _nDual(0),
      _angle(0.)
  {
    _fitted = false;
  }

  TSegment::TSegment(const AList<TLink> & a)
    : TTrackBase(a),
      _state(0),
      _innerWidth(0),
      _outerWidth(0),
      _nLayer(0),
      _clusterType(0),
      _duality(0.),
      _nDual(0),
      _angle(0.)
  {
    _links.sort(TLink::sortByWireId);
    _fitted = false;
  }

  TSegment::~TSegment()
  {
  }

  void
  TSegment::dump(const std::string& msg, const std::string& pre0) const
  {
    if (! _fitted) update();
    bool def = false;
    if (msg == "") def = true;
    std::string pre = pre0;

    if (def ||
        msg.find("cluster") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "#links=" << _links.length()
                << "(inWidth=" << _innerWidth
                << ",outWidth" << _outerWidth << ":type="
                << clusterType() << "),_nDual=" << _nDual
                << ",_duality=" << _duality << ",_angle="
                << _angle << std::endl;
      pre = "";
    }
    if (def ||
        msg.find("vector") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "pos" << _position << "," << "dir" << _direction;
      std::cout << std::endl;
      pre = "";
    }
    if (def ||
        msg.find("state") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "state=" << _state << std::endl;
      pre = "";
    }
    if (def ||
        msg.find("link") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "unique links=" << NUniqueLinks(* this);
      std::cout << ",major links=" << NMajorLinks(* this);
      std::cout << ",branches=" << NLinkBranches(* this);
      std::cout << std::endl;
      pre = "";
    }
    TTrackBase::dump(msg, pre);
  }

  void
  TSegment::update(void) const
  {
    _clusterType = 0;
    _position = ORIGIN;
    _direction = ORIGIN;
    _outerPosition = ORIGIN;
    _inners.removeAll();
    _outers.removeAll();

    if (_links.length() == 0) return;

    _innerMostLayer = _links[0]->wire()->layerId();
    _outerMostLayer = _innerMostLayer;
    for (unsigned i = 1; i < (unsigned) _links.length(); i++) {
      unsigned id = _links[i]->wire()->layerId();
      if (id < _innerMostLayer) _innerMostLayer = id;
      else if (id > _outerMostLayer) _outerMostLayer = id;
    }
    _nLayer = _outerMostLayer - _innerMostLayer + 1;

//  double centerX = _links[0]->position().x();
    HepGeom::Point3D<double> inner = ORIGIN;
    unsigned nInner = 0;
    unsigned nOuter = 0;
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      const HepGeom::Point3D<double> & tmp = _links[i]->position();
      _position += tmp;
      unsigned id = _links[i]->wire()->layerId();
      if (id == _innerMostLayer) {
        inner += tmp;
        ++nInner;
        _inners.append(_links[i]);
      }
      if (id == _outerMostLayer) {
        _outerPosition += tmp;
        ++nOuter;
        _outers.append(_links[i]);
      }
    }
//      TTrackBase::dump("hits");
// std::cout << "0:nin,nout=" << nInner << "," << nOuter << std::endl;
// std::cout << "0:in,out=" << inner << ", " << _outerPosition << std::endl;
// std::cout << "0:npos=" << n << std::endl;
// std::cout << "0:pos=" << _position << std::endl;

    _innerWidth = TLink::width(_inners);
    _outerWidth = TLink::width(_outers);
    _position *= (1. / float(n));

    inner *= (1. / (float) nInner);
    _outerPosition *= (1. / (float) nOuter);
    _direction = (inner - _outerPosition).unit();

// std::cout << "1:in,out=" << inner << ", " << _outerPosition << std::endl;
// std::cout << "1:dir=" << _direction << std::endl;
// std::cout << "1:pos=" << _position << std::endl;

    _fitted = true;
  }

  double
  TSegment::distance(const TSegment& c) const
  {
    Vector3D dir = c.position() - _position;
    if (dir.x() > M_PI) dir.setX(dir.x() - 2. * M_PI);
    else if (dir.x() < - M_PI) dir.setX(2. * M_PI + dir.x());

    float radial = fabs(_direction.dot(dir));
    float radial2 = radial * radial;

    return (dir.mag2() - radial2) > 0.0 ? sqrt(dir.mag2() - radial2) : 0.;
  }

  double
  TSegment::distance(const HepGeom::Point3D<double> & p, const Vector3D& v) const
  {
    Vector3D dir = _position - p;
    if (dir.x() > M_PI) dir.setX(dir.x() - 2. * M_PI);
    else if (dir.x() < - M_PI) dir.setX(2. * M_PI + dir.x());

    float radial = fabs(v.unit().dot(dir));
    float radial2 = radial * radial;

    return (dir.mag2() - radial2) > 0.0 ? sqrt(dir.mag2() - radial2) : 0.;
  }

  Range
  TSegment::rangeX(double min, double max) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    if (min > max) {
      std::cout << "TSegment::range !!! bad arguments:min,max=";
      std::cout << min << "," << max << std::endl;
    }
#endif

    unsigned n = _links.length();
    if (n == 0) return Range(0., 0.);

    //...Search for a center...
    bool found = false;
    double center;
    for (unsigned i = 0; i < n; i++) {
      double x = _links[i]->position().x();
      if (x < min || x > max) continue;
      center = x;
      found = true;
      break;
    }
    if (! found) return Range(0., 0.);

#ifdef TRASAN_DEBUG_DETAIL
// std::cout << "    center=" << center << std::endl;
#endif

    double distanceR = 0.;
    double distanceL = 0.;
    double distanceMax = max - min;
    for (unsigned i = 0; i < n; i++) {
      double x = _links[i]->position().x();
      if (x < min || x > max) continue;

      double distance0, distance1;
      if (x > center) {
        distance0 = x - center;
        distance1 = distanceMax - distance0;
      } else {
        distance1 = center - x;
        distance0 = distanceMax - distance1;
      }

      if (distance0 < distance1) {
        if (distance0 > distanceR) distanceR = distance0;
      } else {
        if (distance1 > distanceL) distanceL = distance1;
      }

#ifdef TRASAN_DEBUG_DETAIL
//  std::cout << "    ";
//  std::cout << _links[i]->wire()->layerId() << "-";
//  std::cout << _links[i]->wire()->localId() << ",";
//  std::cout << _links[i]->position().x();
//  std::cout << ",0,1=" << distance0 << "," << distance1;
//  std::cout << ",l,r=" << distanceL << "," << distanceR;
//  std::cout << std::endl;
#endif
    }

    double right = center + distanceR;
    double left = center - distanceL;

    return Range(left, right);
  }

  void
  TSegment::updateType(void) const
  {
    if (! nLinks()) return;
    if (! _fitted) update();

    //...Parameter...
    const unsigned fat = 3;
    const unsigned tall = 3;

    //...Calculate
    updateDuality();

    //...Long or short...
    if ((_innerWidth < fat) && (_outerWidth < fat)) {
      _clusterType = 1;

      //...Check length across a super layer...
      if (_nLayer > tall) _clusterType = 2;
      return;
    }

    //...A...
    else if (_outerWidth < fat) {
      _clusterType = 3;
      return;
    }

    //...V...
    else if (_innerWidth < fat) {
      _clusterType = 4;
      return;
    }

    //...X or parallel...
    else {
      bool space = true;
      for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
        unsigned n = 0;
        AList<TLink> tmp;
        for (unsigned j = 0; j < (unsigned) _links.length(); j++)
          if (_links[j]->wire()->layerId() == i) {
            ++n;
            tmp.append(_links[j]);
          }

        if (n == TLink::width(tmp)) {
          space = false;
          break;
        }
      }

      _clusterType = 5;
      if (space) _clusterType = 6;
      return;
    }
  }

  AList<TSegment>
  TSegment::split(void) const
  {
    AList<TSegment> list;

    //...Do not split if cluster type is 1, 2, or 7...
    const unsigned t = clusterType();
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "    ... splitting : type=" << t << std::endl;
#endif
    if (t == 0) return list;
    else if (t == 2) {
      // beta 5
      //if (_nDual > 2 && _duality > 0.7 && _angle > 0.7) return splitDual();

      // 1.67g
      // if (_nDual > 2 && _duality > 0.7) return splitDual();

      if (_nDual > 2 && _duality > 0.7 && _angle > 0.7) return splitDual();
      return list;
    } else if (t == 1) return list;
    else if (t == 7) return list;

    //...Parallel...
    else if (t == 6) return splitParallel();
    // else if (t == 6) return list;

    //...Avoid splitting of X or parallel...(future implementation)...
    else if (t > 4) return splitComplicated();

    //...A or V...
    return splitAV();
  }

  AList<TSegment>
  TSegment::splitAV(void) const
  {
    unsigned t = clusterType();
    AList<TLink> seeds[2];

    //...Calculate corner of V or A...
    const AList<TLink> * corners;
    if (t == 3) corners = & _outers;
    else        corners = & _inners;
    HepGeom::Point3D<double> corner;
    for (unsigned i = 0; i < (unsigned) corners->length(); i++)
      corner += (* corners)[i]->wire()->xyPosition();
    corner *= 1. / (float) corners->length();
    seeds[0].append(* corners);
    seeds[1].append(* corners);

    //...Calculdate edges...
    const AList<TLink> * links;
    if (t == 3) links = & _inners;
    else        links = & _outers;
    AList<TLink> edge = TLink::edges(* links);
    HepGeom::Point3D<double> edgePos[2];
    Vector3D v[2];
    for (unsigned i = 0; i < 2; i++) {
      edgePos[i] = edge[i]->wire()->xyPosition();
      v[i] = (edgePos[i] - corner).unit();
    }
    seeds[0].append(edge[0]);
    seeds[1].append(edge[1]);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "    corner:" << corner << std::endl;
    std::cout << Tab() << "    edge:" << edgePos[0] << "(";
    std::cout << Tab() << edge[0]->wire()->layerId() << "-";
    std::cout << Tab() << edge[0]->wire()->localId() << ")";
    std::cout << Tab() << v[0] << std::endl;
    std::cout << Tab() << "         ";
    std::cout << Tab() << edgePos[1] << "(";
    std::cout << Tab() << edge[1]->wire()->layerId() << "-";
    std::cout << Tab() << edge[1]->wire()->localId() << ")";
    std::cout << Tab() << v[1] << std::endl;
#endif

    //...Examine each hits...
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      TLink* l = _links[i];
      if (edge.member(l)) continue;
      if (corners->member(l)) continue;

      Vector3D p = l->wire()->xyPosition() - corner;
      double p2 = p.mag2();

      double dist[2];
      for (unsigned j = 0; j < 2; j++) {
        dist[j] = v[j].dot(p);
        double d2 = dist[j] * dist[j];
        dist[j] = (p2 - d2) > 0. ? sqrt(p2 - d2) : 0.;
      }
      if (dist[0] < dist[1]) seeds[0].append(l);
      else                   seeds[1].append(l);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "        p:" << p << std::endl;
      std::cout << Tab() << "    " << l->wire()->layerId() << "-";
      std::cout << Tab() << l->wire()->localId() << ":" << dist[0];
      std::cout << Tab() << "," << dist[1] << std::endl;
#endif
    }

    AList<TSegment> list;
    for (unsigned i = 0; i < 2; i++) {
      if (seeds[i].length()) {
        TSegment* nc = new TSegment(seeds[i]);
        AList<TSegment> ncx = nc->split();
        if (ncx.length() == 0) {
          nc->solveDualHits();
          list.append(nc);
        } else {
          list.append(ncx);
          delete nc;
        }
      }
    }
    return list;
  }

  AList<TSegment>
  TSegment::splitComplicated(void) const
  {

    //...Select best hits...
    AList<TSegment> newClusters;
    AList<TLink> goodHits;
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWireHit* h = _links[i]->hit();
      unsigned state = h->state();
      if (!(state & CellHitContinuous)) continue;
      if (!(state & CellHitIsolated)) continue;
      if ((!(state & CellHitPatternLeft)) &&
          (!(state & CellHitPatternRight))) continue;
      goodHits.append(_links[i]);
    }
    if (goodHits.length() == 0) return newClusters;

    //...Main loop...
    goodHits.sort(TLink::sortByWireId);
    AList<TLink> original(_links);
    while (goodHits.length()) {

      //...Select an edge hit...
      TLink* seed = goodHits.last();
      const Belle2::TRGCDCWire* wire = seed->wire();
      unsigned localId = wire->localId();
      AList<TLink> used;
      unsigned nn = _links.length();
      for (unsigned i = 0; i < nn; i++) {
        TLink* t = _links[i];
        const Belle2::TRGCDCWire* w = t->wire();

        //...Straight?...
        if (abs((int) w->localId() - (int) localId) < 2) used.append(t);
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "        seed is " << seed->wire()->name() << std::endl;
      std::cout << Tab() << "            ";
      for (unsigned i = 0; i < (unsigned) used.length(); i++) {
        std::cout << Tab() << used[i]->wire()->name() << ",";
      }
      std::cout << Tab() << std::endl;
#endif

      //...Create new cluster...
      if (used.length() == 0) continue;
      if ((unsigned) used.length() == nLinks()) return newClusters;
      TSegment* c = new TSegment(used);
      AList<TSegment> cx = c->split();
      if (cx.length() == 0) {
        c->solveDualHits();
        newClusters.append(c);
      } else {
        newClusters.append(cx);
        delete c;
      }
      goodHits.remove(used);
      original.remove(used);
    }

    //...Remainings...
    if ((original.length()) && ((unsigned) original.length() < nLinks())) {
      TSegment* c = new TSegment(original);
      AList<TSegment> cx = c->split();
      if (cx.length() == 0) {
        c->solveDualHits();
        newClusters.append(c);
      } else {
        newClusters.append(cx);
        delete c;
      }
    }

    return newClusters;
  }

  AList<TSegment>
  TSegment::splitParallel(void) const
  {
    AList<TLink> seeds[2];
    AList<TSegment> newClusters;
    for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
      AList<TLink> list = TLink::sameLayer(_links, i);
      AList<TLink> outerList = TLink::edges(list);

#ifdef TRASAN_DEBUG_DETAIL
      if (outerList.length() != 2) {
        std::cout << Tab() << "TSegment::splitParallel !!! ";
        std::cout << Tab() << "This is not a parallel cluster" << std::endl;
      }
#endif

      seeds[0].append(outerList[0]);
      seeds[1].append(outerList[1]);
      if (list.length() == 2) continue;

      const Belle2::TRGCDCWire& wire0 = * outerList[0]->wire();
      const Belle2::TRGCDCWire& wire1 = * outerList[1]->wire();
      for (unsigned k = 0; k < (unsigned) list.length(); k++) {
        TLink* t = list[k];
        if (t == outerList[0]) continue;
        if (t == outerList[1]) continue;

        if (abs(wire0.localIdDifference(* t->wire())) <
            abs(wire1.localIdDifference(* t->wire())))
          seeds[0].append(t);
        else
          seeds[1].append(t);
      }
    }

    if ((seeds[0].length()) && ((unsigned) seeds[0].length() < nLinks())) {
      TSegment* c0 = new TSegment(seeds[0]);
      AList<TSegment> c0x = c0->split();
      if (c0x.length()) {
        newClusters.append(c0x);
        delete c0;
      } else {
        c0->solveDualHits();
        newClusters.append(c0);
      }
    }

    if ((seeds[1].length()) && ((unsigned) seeds[1].length() < nLinks())) {
      TSegment* c1 = new TSegment(seeds[1]);
      AList<TSegment> c1x = c1->split();
      if (c1x.length()) {
        newClusters.append(c1x);
        delete c1;
      } else {
        c1->solveDualHits();
        newClusters.append(c1);
      }
    }

    return newClusters;
  }

  void
  TSegment::updateDuality(void) const
  {
    _duality = 0.;
    _nDual = 0;
    HepGeom::Point3D<double> x[2];
    for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
      AList<TLink> list = TLink::sameLayer(_links, i);
      if (i == _innerMostLayer) {
        for (unsigned j = 0; j < (unsigned) list.length(); j++)
          x[0] += list[j]->hit()->xyPosition();
        x[0] *= 1. / float(list.length());
      } else if (i == _outerMostLayer) {
        for (unsigned j = 0; j < (unsigned) list.length(); j++)
          x[1] += list[j]->hit()->xyPosition();
        x[1] *= 1. / float(list.length());
      }

      if (list.length() == 2) {
        if (TLink::width(list) != 2) continue;
        const Belle2::TRGCDCWireHit* h0 = list[0]->hit();
        const Belle2::TRGCDCWireHit* h1 = list[1]->hit();

        double distance = (h0->xyPosition() - h1->xyPosition()).mag();
        distance = fabs(distance - list[0]->drift() - list[1]->drift());
        _duality += distance;
        ++_nDual;
      }
    }
    if (_nDual > 0) _duality /= float(_nDual);
    _angle = (x[1] - x[0]).unit().dot(x[0].unit());
  }

  AList<TSegment>
  TSegment::splitDual(void) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "    TSegment::splitDual called" << std::endl;
#endif
    AList<TLink> seeds[2];
    AList<TLink> unknown;
    for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
      AList<TLink> list = TLink::sameLayer(_links, i);

      if (list.length() == 2) {
        if (TLink::width(list) == 2) {
          seeds[0].append(list[0]);
          seeds[1].append(list[1]);
          continue;
        }
      }
      unknown.append(list);
    }

    if (unknown.length() > 0) {
#ifdef TRASAN_DEBUG_DETAIL
      if (seeds[0].length() == 0)
        std::cout << Tab() << "TSegment::splitDual !!! no TLink for seed 0" << std::endl;
      if (seeds[1].length() == 0)
        std::cout << Tab() << "TSegment::splitDual !!! no TLink for seed 1" << std::endl;
#endif
      const HepGeom::Point3D<double> & p0 = seeds[0][0]->xyPosition();
      const HepGeom::Point3D<double> & p1 = seeds[1][0]->xyPosition();
      Vector3D v0 = (seeds[0].last()->xyPosition() - p0).unit();
      Vector3D v1 = (seeds[1].last()->xyPosition() - p1).unit();

      for (unsigned i = 0; i < (unsigned) unknown.length(); i++) {
        TLink* t = unknown[i];
        Vector3D x0 = t->xyPosition() - p0;
        double d0 = (x0 - (x0.dot(v0) * v0)).mag();
        Vector3D x1 = t->xyPosition() - p1;
        double d1 = (x1 - (x1.dot(v0) * v0)).mag();

        if (d0 < d1) seeds[0].append(t);
        else         seeds[1].append(t);
      }
    }

    AList<TSegment> newClusters;
    newClusters.append(new TSegment(seeds[0]));
    newClusters.append(new TSegment(seeds[1]));
    return newClusters;
  }

  int
  TSegment::solveDualHits(void)
  {
    updateType();
    if (_clusterType == 0) return 0;
    if (_nDual == 0) return 0;
    update();
    return 0;

    updateType();
    if (_clusterType == 0) return 0;
    if (_nDual == 0) return 0;

    AList<TLink> seeds;
    AList<TLink> duals;
    for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
      AList<TLink> list = TLink::sameLayer(_links, i);

      if (list.length() == 1) {
        seeds.append(list[0]);
      } else if (list.length() == 2) {
        if (TLink::width(list) > 1) {
          const Belle2::TRGCDCWireHit* h0 = list[0]->hit();
          const Belle2::TRGCDCWireHit* h1 = list[1]->hit();
          double distance = (h0->xyPosition() - h1->xyPosition()).mag();
          distance = fabs(distance -
                          list[0]->drift() -
                          list[1]->drift());
          if (distance > 0.5) duals.append(list);
#ifdef TRASAN_DEBUG_DETAIL
          h0->dump("", "        ");
          h1->dump("", "        ");
          std::cout << Tab() << "        lyr=" << i;
          std::cout << Tab() << ", duality distance = " << distance << std::endl;
#endif
        }
      } else if (list.length() == 0) {
        continue;
      }
#ifdef TRASAN_DEBUG_DETAIL
      else {
        std::cout << Tab() << "TSegment::solveDualHits !!! this is not expected 2";
        std::cout << Tab() << std::endl;
        this->dump("cluster hits mc", "    ");
      }
#endif
    }

    //...Solve them...
    if (seeds.length() < 2) return -1;
    AList<TLink> outers = TLink::inOut(seeds);
    const HepGeom::Point3D<double> & p = outers[0]->xyPosition();
    Vector3D v = (outers[1]->xyPosition() - p).unit();
    unsigned n = duals.length() / 2;
    for (unsigned i = 0; i < n; i++) {
      TLink* t0 = duals[i * 2 + 0];
      TLink* t1 = duals[i * 2 + 1];
      Vector3D x0 = t0->xyPosition() - p;
      Vector3D x1 = t1->xyPosition() - p;
      double d0 = (x0 - (x0.dot(v) * v)).mag();
      double d1 = (x1 - (x1.dot(v) * v)).mag();
      if (d0 < d1) _links.remove(t1);
      else         _links.remove(t0);
    }
    update();
    return n;
  }



// void
// CheckSegments(const CAList<TSegment> & list) {
//     static bool first = true;
//     static BelleHistogram * hError;
//     static BelleHistogram * hNHeps[11];
//     static BelleHistogram * hNHits[3];

//     if (first) {
//  first = false;
//  extern BelleTupleManager * BASF_Histogram;
//  BelleTupleManager * m = BASF_Histogram;

//  hError = m->histogram("segment errors", 10, 0, 10, 20000);

//  hNHeps[0] = m->histogram("segment nheps sl 0", 10, 0., 10.);
//  hNHeps[1] = m->histogram("segment nheps sl 1", 10, 0., 10.);
//  hNHeps[2] = m->histogram("segment nheps sl 2", 10, 0., 10.);
//  hNHeps[3] = m->histogram("segment nheps sl 3", 10, 0., 10.);
//  hNHeps[4] = m->histogram("segment nheps sl 4", 10, 0., 10.);
//  hNHeps[5] = m->histogram("segment nheps sl 5", 10, 0., 10.);
//  hNHeps[6] = m->histogram("segment nheps sl 6", 10, 0., 10.);
//  hNHeps[7] = m->histogram("segment nheps sl 7", 10, 0., 10.);
//  hNHeps[8] = m->histogram("segment nheps sl 8", 10, 0., 10.);
//  hNHeps[9] = m->histogram("segment nheps sl 9", 10, 0., 10.);
//  hNHeps[10] = m->histogram("segment nheps sl 10", 10, 0., 10.);

//  hNHits[0] = m->histogram("segment nhits, nheps = 1", 20, 0., 20.);
//  hNHits[1] = m->histogram("segment nhits, nheps = 2", 20, 0., 20.);
//  hNHits[2] = m->histogram("segment nhits, nheps >= 3", 20, 0., 20.);

// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << "CheckSegments ... initialized" << std::endl;
// #endif

//  return;
//     }

//     unsigned n = list.length();
//     for (unsigned i = 0; i < n; i++) {
//  const TSegment & s = * list[i];
//  const AList<TLink> & links = s.links();
//  unsigned nLinks = links.length();
//  if (nLinks == 0) {
//      hError->accumulate(0.5);
//      continue;
//  }

//  unsigned sl = links[0]->wire()->superLayerId();
//  unsigned nHeps = s.nHeps();
//  hNHeps[sl]->accumulate((float) nHeps + .5);
//  if (nHeps == 1)      hNHits[0]->accumulate((float) nLinks + .5);
//  else if (nHeps == 2) hNHits[1]->accumulate((float) nLinks + .5);
//  else                 hNHits[2]->accumulate((float) nLinks + .5);
//     }
// }

// void
// CheckSegmentLink(const TSegment & base,
//     const TSegment & next,
//     float distance,
//     float dirAngle) {
//     static bool first = true;
//     static BelleHistogram * hAngle[2];
//     static BelleHistogram * hAngle1[2];
//     static BelleHistogram * hDistance[2];
//     static BelleHistogram * hDirAngle[2];
//     static BelleHistogram * hDirAngle1[2];

//     if (first) {
//  first = false;
//  extern BelleTupleManager * BASF_Histogram;
//  BelleTupleManager * m = BASF_Histogram;

//  hAngle[0] = m->histogram("segment correct link, angle",
//         50, -1., 1., 21000);
//  hAngle[1] = m->histogram("segment wrong link, angle", 50, -1., 1.);
//  hAngle1[0] = m->histogram("segment correct link, angle, wide",
//          50, .8, 1.);
//  hAngle1[1] = m->histogram("segment wrong link, angle, wide",
//          50, .8, 1.);
//  hDistance[0] = m->histogram("segment correct link, dist", 50, 0., 1.);
//  hDistance[1] = m->histogram("segment wrong link, dist", 50, 0., 1.);
//  hDirAngle[0] = m->histogram("segment correct link, dir angle",
//            50, -1, 1.);
//  hDirAngle[1] = m->histogram("segment wrong link, dir angle",
//            50, -1, 1.);
//  hDirAngle1[0] = m->histogram("segment correct link, dir angle, wide",
//            50, .8, 1.);
//  hDirAngle1[1] = m->histogram("segment wrong link, dir angle, wide",
//            50, .8, 1.);

// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << "CheckSegmentLink ... initialized" << std::endl;
// #endif

//  return;
//     }

//     const TTrackHEP * const hep0 = base.hep();
//     const TTrackHEP * const hep1 = next.hep();

//     float angle = base.direction().dot(next.direction());

//     unsigned id = 0;
//     if (hep0 != hep1) id = 1;
//     hAngle[id]->accumulate(angle);
//     hAngle1[id]->accumulate(angle);
//     hDistance[id]->accumulate(distance);
//     hDirAngle[id]->accumulate(dirAngle);
//     hDirAngle1[id]->accumulate(dirAngle);
// }

  unsigned
  NCoreLinks(const CAList<TSegment> & list)
  {
    unsigned n = 0;
    unsigned nList = list.length();
    for (unsigned i = 0; i < nList; i++) {
      const AList<TLink> & links = list[i]->links();
      for (unsigned j = 0; j < (unsigned) links.length(); j++) {
        unsigned state = links[j]->hit()->state();
        if ((!(state & CellHitPatternLeft)) &&
            (!(state & CellHitPatternRight)))
          ++n;
      }
    }
    return n;
  }

  AList<TLink>
  Links(const TSegment& s, const TTrack& t)
  {
    AList<TLink> a;

    const AList<TLink> & links = s.links();
    const AList<TLink> & trackLinks = t.links();
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (trackLinks.hasMember(links[i]))
        a.append(links[i]);
    }

    return a;
  }

  unsigned
  NUniqueLinks(const TSegment& a)
  {
    unsigned n = 0;
    const TSegment* s = & a;
    while (s) {
      unsigned nLinks = s->innerLinks().length();
      if (nLinks != 1) return n;
      ++n;
      s = s->innerLinks()[0];
    }
    return n;
  }

  AList<TSegment>
  UniqueLinks(const TSegment& a)
  {
    AList<TSegment> links;
    const TSegment* s = & a;
    while (s) {
      unsigned nLinks = s->innerLinks().length();
      if (nLinks != 1) return links;
      const TSegment* t = s->innerLinks()[0];
      links.append((TSegment*) t);
      s = t;
    }
    return links;
  }

  unsigned
  NMajorLinks(const TSegment& a)
  {
    unsigned n = 0;
    const TSegment* s = & a;
    while (s) {
      unsigned nLinks = s->innerLinks().length();
      if (nLinks == 0) return n;
      ++n;
      s = s->innerLinks()[0];
    }
    return n;
  }

  AList<TSegment>
  MajorLinks(const TSegment& a)
  {
    AList<TSegment> links;
    const TSegment* s = & a;
    while (s) {
      unsigned nLinks = s->innerLinks().length();
      if (nLinks == 0) return links;
      const TSegment* t = s->innerLinks()[0];
      links.append((TSegment*) t);
      s = t;
    }
    return links;
  }

  unsigned
  NLinkBranches(const TSegment& a)
  {
    unsigned n = 0;
    const TSegment* s = & a;
    while (s) {
      unsigned nLinks = s->innerLinks().length();
      if (nLinks == 0) return n;
      if (nLinks > 1) ++n;
      s = s->innerLinks()[0];
    }
    return n;
  }

  void
  SeparateCrowded(const AList<TSegment> & in,
                  AList<TSegment> & isolated,
                  AList<TSegment> & crowded)
  {
    unsigned n = in.length();
    if (n == 0) return;

    for (unsigned i = 0; i < n; i++) {
      TSegment& s = * in[i];
      if (s.state() & TSegmentCrowd) crowded.append(s);
      else                           isolated.append(s);
    }
  }

  unsigned
  SuperLayer(const AList<TSegment> & list)
  {
    unsigned sl = 0;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++)
      sl |= (1 << (list[i]->superLayerId()));
    return sl;
  }

  TSegment*
  OuterMostUniqueLink(const TSegment& a)
  {
    const TSegment* o = & a;
    while (o->outerLinks().length() == 1)
      o = o->outerLinks()[0];
    return (TSegment*) o;
  }

  AList<TLink>
  Links(const AList<TSegment> & list)
  {
    AList<TLink> links;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++)
      links.append(list[i]->links());
    return links;
  }

  unsigned
  TSegment::width(void) const
  {
    unsigned maxWidth = 0;
    for (unsigned i = innerMostLayer(); i <= outerMostLayer(); i++) {
      AList<TLink> tmp = TLink::sameLayer(links(), i);
      unsigned w = TLink::width(tmp);
      if (w > maxWidth) maxWidth = w;
    }
    return maxWidth;
  }

  unsigned
  TSegment::superLayerId(void) const
  {
    unsigned id = (links())[0]->wire()->superLayerId();
#ifdef TRASAN_DEBUG
    {
//  const AList<Belle::TLink> & list = links();
      const AList<TLink> & list = links();
      unsigned n = list.length();
      for (unsigned i = 1; i < n; i++) {
        if (list[i]->hit()->wire().superLayerId() != id) {
          std::cout << Tab() << "TSegment::superLayerId !!! strange segment found";
          std::cout << Tab() << std::endl;
          dump();
          break;
        }
      }
    }
#endif
    return id;
  }

} // namespace Belle

