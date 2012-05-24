//-----------------------------------------------------------------------------
// $Id: TSegment0.h 10021 2007-03-03 05:43:02Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSegment0.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to manage a group of TLink's.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.6  2002/01/03 11:04:59  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.5  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.4  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.3  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.2  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.1  2000/01/28 06:30:32  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.8  2000/01/23 08:23:10  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.7  1999/10/30 10:12:51  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.6  1999/10/21 15:45:20  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.5  1999/03/11 23:27:29  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.4  1999/01/11 03:03:27  yiwasaki
// Fitters added
//
// Revision 1.3  1998/07/29 04:35:24  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.1  1998/07/02 10:15:32  yiwasaki
// TSegment.cc,h added
//
//-----------------------------------------------------------------------------

#ifndef TSegment0_FLAG_
#define TSegment0_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

#include "CLHEP/Geometry/Vector3D.h"

#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLink.h"

namespace Belle {

  typedef HepGeom::Point3D<double>  Point3D;
  typedef HepGeom::Vector3D<double>  Vector3D;
  class TTrack;
  class Range;
  template <class T> class CAList;

/// A class to relate TRGCDCWireHit and TTrack objects.
  class TSegment0 : public TTrackBase {

  public:
    /// Constructor.
    TSegment0();
    TSegment0(const AList<TLink> &);

    /// Destructor
    virtual ~TSegment0();

  public:// Selectors
    /// returns type.
    virtual unsigned objectType(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns super layer id.
    unsigned superLayerId() const;

    /// returns position.
    const HepGeom::Point3D<double>  & position(void) const;

    /// returns direction.
    const Vector3D& direction(void) const;

    /// calculates distance between two clusters. Smaller value indicates closer.
    double distance(const TSegment0&) const;
    double distance(const HepGeom::Point3D<double>  &, const Vector3D&) const;

    /// returns Range of x-coordinate of TLinks.
    Range rangeX(double min, double max) const;

    /// returns inner width.
    unsigned innerWidth(void) const;

    /// returns outer width.
    unsigned outerWidth(void) const;

    /// returns inner most layer.
    unsigned innerMostLayer(void) const;

    /// returns outer most layer.
    unsigned outerMostLayer(void) const;

    /// returns cluster type. 0:empty, 1:short line, 2:long line, 3:V shage(from outside), 4:A shape, 5:X shape, 6:parallel, 7:complicated.
    unsigned clusterType(void) const;

    /// returns a list of sub TSegments in this cluster. If cluster type is 1, 2, or 7, no cluster is returned.
    AList<TSegment0> split(void) const;

    int solveDualHits(void);
    double duality(void) const;

  public:// TTrack relations
    AList<TTrack> & tracks(void);

  public:// Segment links
    AList<TSegment0> & innerLinks(void);

  private:
    AList<TSegment0> splitAV(void) const;
    AList<TSegment0> splitParallel(void) const;
    AList<TSegment0> splitComplicated(void) const;
    AList<TSegment0> splitDual(void) const;

  private:
    /// updates cache.
    void update(void) const;

    /// updates type.
    void updateType(void) const;

    /// updates duality.
    void updateDuality(void) const;

  private:
    // always updated.
    AList<TTrack> _tracks;
    AList<TSegment0> _innerLinks;

  private:
    mutable HepGeom::Point3D<double>  _position;
    mutable Vector3D _direction;
    mutable unsigned _innerWidth;
    mutable unsigned _outerWidth;
    mutable unsigned _innerMostLayer;
    mutable unsigned _outerMostLayer;
    mutable AList<TLink> _inners;
    mutable AList<TLink> _outers;
    mutable unsigned _nLayer;
    mutable unsigned _clusterType;
    mutable double _duality;
    mutable unsigned _nDual;
    mutable double _angle;
  };

// Utility functions
/// returns \# of core links in segments.
  unsigned NCoreLinks(const CAList<TSegment0> & list);

/// returns AList of TLink used for a track.
  AList<TLink> Links(const TSegment0&, const TTrack&);

/// checks property of segments.
  void
  CheckSegments(const CAList<TSegment0> & segmentList);

/// checks to link segments.
  void
  CheckSegmentLink(const TSegment0& base,
                   const TSegment0& next,
                   float distance,
                   float dirAngle);

//-----------------------------------------------------------------------------

#ifdef TSegment0_NO_INLINE
#define inline
#else
#undef inline
#define TSegment0_INLINE_DEFINE_HERE
#endif

#ifdef TSegment0_INLINE_DEFINE_HERE

  inline
  const HepGeom::Point3D<double>  &
  TSegment0::position(void) const
  {
    if (! _fitted) update();
    return _position;
  }

  inline
  const Vector3D&
  TSegment0::direction(void) const
  {
    if (! _fitted) update();
    return _direction;
  }

  inline
  unsigned
  TSegment0::innerWidth(void) const
  {
    if (! _fitted) update();
    return _innerWidth;
  }

  inline
  unsigned
  TSegment0::outerWidth(void) const
  {
    if (! _fitted) update();
    return _outerWidth;
  }

  inline
  unsigned
  TSegment0::innerMostLayer(void) const
  {
    if (! _fitted) update();
    return _innerMostLayer;
  }

  inline
  unsigned
  TSegment0::outerMostLayer(void) const
  {
    if (! _fitted) update();
    return _outerMostLayer;
  }

  inline
  unsigned
  TSegment0::clusterType(void) const
  {
    if (! nLinks()) return 0;
    if (_clusterType == 0) updateType();
    return _clusterType;
  }

  inline
  double
  TSegment0::duality(void) const
  {
    return _duality;
  }

  inline
  unsigned
  TSegment0::objectType(void) const
  {
    return Segment;
  }

  inline
  unsigned
  TSegment0::superLayerId(void) const
  {
    unsigned id = (links())[0]->wire()->superLayerId();
#ifdef TRASAN_DEBUG
//     {
//  const AList<TLink> & list = links();
//  unsigned n = list.length();
//  for (unsigned i = 1; i < n; i++) {
//      if (list[i]->hit()->wire()->superLayerId() != id) {
//    std::cout << "TSegment0::superLayerId !!! strange segment found";
//    std::cout << std::endl;
//    dump();
//    break;
//      }
//  }
//     }
#endif
    return id;
  }

  inline
  AList<TTrack> &
  TSegment0::tracks(void)
  {
    return _tracks;
  }

  inline
  AList<TSegment0> &
  TSegment0::innerLinks(void)
  {
    return _innerLinks;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TSegment0_FLAG_ */

