//-----------------------------------------------------------------------------
// $Id: TSegment.h 10016 2007-02-28 04:27:03Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TSegment.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to manage a group of TLink's.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.19  2002/01/03 11:04:59  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.18  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.17  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.16  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.15  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.14  2000/02/23 08:45:08  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.13  2000/02/15 13:46:51  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.12  2000/02/10 13:11:43  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.11  2000/02/01 11:24:46  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.10  2000/01/30 08:17:16  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.9  2000/01/28 06:30:32  yiwasaki
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

#ifndef TSegment_FLAG_
#define TSegment_FLAG_

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

#define TSegmentCrowd 8

/// A class to relate TRGCDCWireHit and TTrack objects.
  class TSegment : public TTrackBase {

  public:
    /// Constructor.
    TSegment();
    TSegment(const AList<TLink> &);

    /// Destructor
    virtual ~TSegment();

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
    const HepGeom::Point3D<double>  & outerPosition(void) const;

    /// returns direction.
    const Vector3D& direction(void) const;

    /// calculates distance between two clusters. Smaller value indicates closer.
    double distance(const TSegment&) const;
    double distance(const HepGeom::Point3D<double>  &, const Vector3D&) const;

    /// returns Range of x-coordinate of TLinks.
    Range rangeX(double min, double max) const;

    /// returns TLinks in inner/outer-most layer.
    const AList<TLink> & inners(void) const;
    const AList<TLink> & outers(void) const;

    /// returns a TLink which is the closest to the center.
    const TLink& center(void) const;

    /// returns width.
    unsigned width(void) const;

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
    AList<TSegment> split(void) const;

    int solveDualHits(void);
    double duality(void) const;

  public:// TTrack relations
    AList<TTrack> & tracks(void);

  public:// Segment links
    AList<TSegment> & innerLinks(void);
    const AList<TSegment> & innerLinks(void) const;
    AList<TSegment> & outerLinks(void);
    const AList<TSegment> & outerLinks(void) const;
    unsigned state(void) const;
    unsigned state(unsigned);

  private:
    AList<TSegment> splitAV(void) const;
    AList<TSegment> splitParallel(void) const;
    AList<TSegment> splitComplicated(void) const;
    AList<TSegment> splitDual(void) const;

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
    AList<TSegment> _innerLinks;
    AList<TSegment> _outerLinks;
    unsigned _state;

  private:
    mutable HepGeom::Point3D<double>  _position;
    mutable HepGeom::Point3D<double>  _outerPosition;
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
  unsigned NCoreLinks(const CAList<TSegment> & list);

/// returns AList of TLink used for a track.
  AList<TLink> Links(const TSegment&, const TTrack&);

/// returns AList of TLink.
  AList<TLink> Links(const AList<TSegment> & list);

/// checks property of segments.
  void
  CheckSegments(const CAList<TSegment> & segmentList);

/// checks to link segments.
  void
  CheckSegmentLink(const TSegment& base,
                   const TSegment& next,
                   float distance,
                   float dirAngle);

/// returns \# of unique segment links.
  unsigned
  NUniqueLinks(const TSegment& a);

/// returns a list of unique segments in links.
  AList<TSegment>
  UniqueLinks(const TSegment& a);

/// returns a segment to the outer-most unique segment.
  TSegment*
  OuterMostUniqueLink(const TSegment& a);

/// returns \# of links in the major link.
  unsigned
  NMajorLinks(const TSegment& a);

/// returns a list of segments in major links.
  AList<TSegment>
  MajorLinks(const TSegment& a);

/// returns \# of link branches in the major link.
  unsigned
  NLinkBranches(const TSegment& a);

/// returns isolated and crowded list.
  void
  SeparateCrowded(const AList<TSegment> & input,
                  AList<TSegment> & isolated,
                  AList<TSegment> & crowded);

/// returns super layer pattern.
  unsigned
  SuperLayer(const AList<TSegment> & list);

//-----------------------------------------------------------------------------

#ifdef TSegment_NO_INLINE
#define inline
#else
#undef inline
#define TSegment_INLINE_DEFINE_HERE
#endif

#ifdef TSegment_INLINE_DEFINE_HERE

  inline
  const AList<TLink> &
  TSegment::inners(void) const
  {
    if (! _fitted) update();
    return _inners;
  }

  inline
  const AList<TLink> &
  TSegment::outers(void) const
  {
    if (! _fitted) update();
    return _outers;
  }

  inline
  const HepGeom::Point3D<double>  &
  TSegment::position(void) const
  {
    if (! _fitted) update();
    return _position;
  }

  inline
  const Vector3D&
  TSegment::direction(void) const
  {
    if (! _fitted) update();
    return _direction;
  }

  inline
  unsigned
  TSegment::innerWidth(void) const
  {
    if (! _fitted) update();
    return _innerWidth;
  }

  inline
  unsigned
  TSegment::outerWidth(void) const
  {
    if (! _fitted) update();
    return _outerWidth;
  }

  inline
  unsigned
  TSegment::innerMostLayer(void) const
  {
    if (! _fitted) update();
    return _innerMostLayer;
  }

  inline
  unsigned
  TSegment::outerMostLayer(void) const
  {
    if (! _fitted) update();
    return _outerMostLayer;
  }

  inline
  unsigned
  TSegment::clusterType(void) const
  {
    if (! nLinks()) return 0;
    if (_clusterType == 0) updateType();
    return _clusterType;
  }

  inline
  double
  TSegment::duality(void) const
  {
    return _duality;
  }

  inline
  unsigned
  TSegment::objectType(void) const
  {
    return Segment;
  }

  inline
  AList<TTrack> &
  TSegment::tracks(void)
  {
    return _tracks;
  }

  inline
  AList<TSegment> &
  TSegment::innerLinks(void)
  {
    return _innerLinks;
  }

  inline
  const AList<TSegment> &
  TSegment::innerLinks(void) const
  {
    return _innerLinks;
  }

  inline
  AList<TSegment> &
  TSegment::outerLinks(void)
  {
    return _outerLinks;
  }

  inline
  const AList<TSegment> &
  TSegment::outerLinks(void) const
  {
    return _outerLinks;
  }

  inline
  unsigned
  TSegment::state(void) const
  {
    return _state;
  }

  inline
  unsigned
  TSegment::state(unsigned a)
  {
    return _state = a;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TSegment_FLAG_ */

