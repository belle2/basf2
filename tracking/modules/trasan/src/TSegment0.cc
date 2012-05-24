//-----------------------------------------------------------------------------
// $Id: TSegment0.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSegment0.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to manage a group of TLink's.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.6  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.5  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.4  2000/10/05 23:54:25  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.3  2000/04/11 13:05:46  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.2  2000/02/15 13:46:42  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.1  2000/01/28 06:30:25  yiwasaki
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
#include "tracking/modules/trasan/TSegment0.h"

#include "tracking/modules/trasan/TLink.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/Range.h"


namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  TSegment0::TSegment0()
    : TTrackBase(),
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

  TSegment0::TSegment0(const AList<TLink> & a)
    : TTrackBase(a),
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

  TSegment0::~TSegment0()
  {
  }

  void
  TSegment0::dump(const std::string& msg, const std::string& pre) const
  {
    if (! _fitted) update();
    bool def = false;
    if (msg == "") def = true;

    if (def || msg.find("cluster") != std::string::npos || msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "#links=" << _links.length();
      std::cout << "(" << _innerWidth << "," << _outerWidth << ":";
      std::cout << clusterType() << ")," << _nDual << "," << _duality << ",";
      std::cout << _angle << std::endl;
    }
    if (def || msg.find("vector") != std::string::npos || msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "pos" << _position << "," << "dir" << _direction;
      std::cout << std::endl;
    }
    if (! def) TTrackBase::dump(msg, pre);
  }

  void
  TSegment0::update(void) const
  {
    _clusterType = 0;
    _position = ORIGIN;
    _direction = ORIGIN;
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

    double centerX = _links[0]->position().x();
    HepGeom::Point3D<double> inner = ORIGIN;
    HepGeom::Point3D<double> outer = ORIGIN;
    unsigned nInner = 0;
    unsigned nOuter = 0;
    for (unsigned i = 0; i < (unsigned) _links.length(); i++) {
      Point3D tmp = _links[i]->position();
      double diff = tmp.x() - centerX;
      if (diff > M_PI) {
        tmp.setX(centerX - 2. * M_PI + diff);
      } else if (diff < - M_PI) {
        tmp.setX(centerX + 2. * M_PI + diff);
      }

      _links[i]->conf(tmp);
      _position += tmp;
      unsigned id = _links[i]->wire()->layerId();
      if (id == _innerMostLayer) {
        inner += tmp;
        ++nInner;
        _inners.append(_links[i]);
      }
      if (id == _outerMostLayer) {
        outer += tmp;
        ++nOuter;
        _outers.append(_links[i]);
      }
    }
    _innerWidth = TLink::width(_inners);
    _outerWidth = TLink::width(_outers);
    _position *= (1. / (float) _links.length());

    inner *= (1. / (float) nInner);
    outer *= (1. / (float) nOuter);
    _direction = (inner - outer).unit();

    _fitted = true;
  }

  double
  TSegment0::distance(const TSegment0& c) const
  {
    Vector3D dir = c.position() - _position;
    if (dir.x() > M_PI) dir.setX(dir.x() - 2. * M_PI);
    else if (dir.x() < - M_PI) dir.setX(2. * M_PI + dir.x());

    float radial = fabs(_direction.dot(dir));
    float radial2 = radial * radial;

    return (dir.mag2() - radial2) > 0.0 ? sqrt(dir.mag2() - radial2) : 0.;
  }

  double
  TSegment0::distance(const HepGeom::Point3D<double> & p, const Vector3D& v) const
  {
    Vector3D dir = _position - p;
    if (dir.x() > M_PI) dir.setX(dir.x() - 2. * M_PI);
    else if (dir.x() < - M_PI) dir.setX(2. * M_PI + dir.x());

    float radial = fabs(v.unit().dot(dir));
    float radial2 = radial * radial;

    return (dir.mag2() - radial2) > 0.0 ? sqrt(dir.mag2() - radial2) : 0.;
  }

  Range
  TSegment0::rangeX(double min, double max) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    if (min > max) {
      std::cout << "TSegment0::range !!! bad arguments:min,max=";
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
  TSegment0::updateType(void) const
  {
    if (! nLinks()) return;
    if (! _fitted) update();

    //...Parameter...
    unsigned fat = 3;
    unsigned tall = 3;

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

  AList<TSegment0>
  TSegment0::split(void) const
  {
    AList<TSegment0> list;

    //...Do not split if cluster type is 1, 2, or 7...
    unsigned t = clusterType();
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... splitting : type=" << t << std::endl;
#endif
    if (t == 0) return list;
    else if (t == 2) {
      // beta 5 if (_nDual > 2 && _duality > 0.7 && _angle > 0.7)
      //          return splitDual();
      if (_nDual > 2 && _duality > 0.7) return splitDual();
      return list;
    } else if (t == 1) return list;
    else if (t == 7) return list;

    //...Parallel...
    else if (t == 6) return splitParallel();

    //...Avoid splitting of X or parallel...(future implementation)...
    else if (t > 4) return splitComplicated();

    //...A or V...
    return splitAV();
  }

  AList<TSegment0>
  TSegment0::splitAV(void) const
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
    std::cout << "    corner:" << corner << std::endl;
    std::cout << "    edge:" << edgePos[0] << "(";
    std::cout << edge[0]->wire()->layerId() << "-";
    std::cout << edge[0]->wire()->localId() << ")";
    std::cout << v[0] << std::endl;
    std::cout << "         ";
    std::cout << edgePos[1] << "(";
    std::cout << edge[1]->wire()->layerId() << "-";
    std::cout << edge[1]->wire()->localId() << ")";
    std::cout << v[1] << std::endl;
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
      std::cout << "        p:" << p << std::endl;
      std::cout << "    " << l->wire()->layerId() << "-";
      std::cout << l->wire()->localId() << ":" << dist[0];
      std::cout << "," << dist[1] << std::endl;
#endif
    }

    AList<TSegment0> list;
    for (unsigned i = 0; i < 2; i++) {
      if (seeds[i].length()) {
        TSegment0* nc = new TSegment0(seeds[i]);
        AList<TSegment0> ncx = nc->split();
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

  AList<TSegment0>
  TSegment0::splitComplicated(void) const
  {

    //...Select best hits...
    AList<TSegment0> newClusters;
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
      std::cout << "        seed is " << seed->wire()->name() << std::endl;
      std::cout << "            ";
      for (unsigned i = 0; i < (unsigned) used.length(); i++) {
        std::cout << used[i]->wire()->name() << ",";
      }
      std::cout << std::endl;
#endif

      //...Create new cluster...
      if (used.length() == 0) continue;
      if ((unsigned) used.length() == nLinks()) return newClusters;
      TSegment0* c = new TSegment0(used);
      AList<TSegment0> cx = c->split();
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
      TSegment0* c = new TSegment0(original);
      AList<TSegment0> cx = c->split();
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

  AList<TSegment0>
  TSegment0::splitParallel(void) const
  {
    AList<TLink> seeds[2];
    AList<TSegment0> newClusters;
    for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
      AList<TLink> list = TLink::sameLayer(_links, i);
      AList<TLink> outerList = TLink::edges(list);

#ifdef TRASAN_DEBUG_DETAIL
      if (outerList.length() != 2) {
        std::cout << "TSegment0::splitParallel !!! ";
        std::cout << "This is not a parallel cluster" << std::endl;
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
      TSegment0* c0 = new TSegment0(seeds[0]);
      AList<TSegment0> c0x = c0->split();
      if (c0x.length()) {
        newClusters.append(c0x);
        delete c0;
      } else {
        c0->solveDualHits();
        newClusters.append(c0);
      }
    }

    if ((seeds[1].length()) && ((unsigned) seeds[1].length() < nLinks())) {
      TSegment0* c1 = new TSegment0(seeds[1]);
      AList<TSegment0> c1x = c1->split();
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
  TSegment0::updateDuality(void) const
  {
    _duality = 0.;
    _nDual = 0;
    HepGeom::Point3D<double> x[2];
    for (unsigned i = _innerMostLayer; i <= _outerMostLayer; i++) {
      AList<TLink> list = TLink::sameLayer(_links, i);
      if (i == _innerMostLayer) {
        for (unsigned j = 0; j < (unsigned) list.length(); j++)
          x[0] += list[j]->hit()->xyPosition();
        x[0] *= 1. / double(list.length());
      } else if (i == _outerMostLayer) {
        for (unsigned j = 0; j < (unsigned) list.length(); j++)
          x[1] += list[j]->hit()->xyPosition();
        x[1] *= 1. / double(list.length());
      }

      if (list.length() == 2) {
        if (TLink::width(list) != 2) continue;
        const Belle2::TRGCDCWireHit* h0 = list[0]->hit();
        const Belle2::TRGCDCWireHit* h1 = list[1]->hit();

        double distance = (h0->xyPosition() - h1->xyPosition()).mag();
        distance = fabs(distance - h0->drift() - h1->drift());
        _duality += distance;
        ++_nDual;
      }
    }
    if (_nDual > 0) _duality /= float(_nDual);
    _angle = (x[1] - x[0]).unit().dot(x[0].unit());
  }

  AList<TSegment0>
  TSegment0::splitDual(void) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TSegment0::splitDual called" << std::endl;
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
        std::cout << "TSegment0::splitDual !!! no TLink for seed 0" << std::endl;
      if (seeds[1].length() == 0)
        std::cout << "TSegment0::splitDual !!! no TLink for seed 1" << std::endl;
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

    AList<TSegment0> newClusters;
    newClusters.append(new TSegment0(seeds[0]));
    newClusters.append(new TSegment0(seeds[1]));
    return newClusters;
  }

  int
  TSegment0::solveDualHits(void)
  {
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
          distance = fabs(distance - h0->drift() - h1->drift());
          if (distance > 0.5) duals.append(list);
#ifdef TRASAN_DEBUG_DETAIL
//    h0->dump();
//    h1->dump();
//    std::cout << "duality distance = " << distance << std::endl;
//    std::cout << "i = " << i << std::endl;
#endif
        }
      } else if (list.length() == 0) {
        continue;
      }
#ifdef TRASAN_DEBUG_DETAIL
      else {
        std::cout << "TSegment0::solveDualHits !!! this is not expected 2";
        std::cout << std::endl;
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
    return n;
  }

  void
  CheckSegments(const CAList<TSegment0> & list)
  {
//cnv     static bool first = true;
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

//  std::cout << "CheckSegments ... initialized" << std::endl;
//  return;
//     }

//     unsigned n = list.length();
//     for (unsigned i = 0; i < n; i++) {
//  const TSegment0 & s = * list[i];
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
  }

  void
  CheckSegmentLink(const TSegment0& base,
                   const TSegment0& next,
                   float distance,
                   float dirAngle)
  {
//cnv     static bool first = true;
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

//  std::cout << "CheckSegmentLink ... initialized" << std::endl;
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
  }

  unsigned
  NCoreLinks(const CAList<TSegment0> & list)
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
  Links(const TSegment0& s, const TTrack& t)
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

} // namespace Belle

