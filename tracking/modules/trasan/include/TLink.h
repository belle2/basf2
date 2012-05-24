//-----------------------------------------------------------------------------
// $Id: TLink.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TLink.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to relate TRGCDCWireHit and TTrack objects.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.51  2005/11/03 23:20:36  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.50  2005/04/18 23:42:05  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.49  2005/03/11 03:58:35  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.48  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.47  2003/12/25 12:04:43  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.46  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.45  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.44  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.43  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.42  2001/04/25 02:36:08  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.41  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.40  2000/10/05 23:54:31  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.39  2000/04/15 13:41:02  katayama
// Add/remove const so that they compile
//
// Revision 1.38  2000/04/11 13:05:56  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.37  2000/03/30 08:30:36  katayama
// mods for CC5.0
//
// Revision 1.36  2000/03/17 07:00:52  katayama
// Module function modified
//
// Revision 1.35  2000/02/25 08:10:00  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.34  2000/02/23 08:45:08  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.33  2000/02/15 13:46:50  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.32  2000/01/30 08:17:15  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.31  1999/11/19 09:13:16  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.30  1999/10/30 10:12:50  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.29  1999/09/21 02:01:38  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.28  1999/06/16 08:29:56  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.27  1999/06/09 15:09:57  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.26  1999/03/12 13:12:04  yiwasaki
// Trasan 1.26 : bug fix in RECCDC_TRK output
//
// Revision 1.25  1999/03/11 23:27:28  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.24  1999/03/10 12:55:14  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.23  1999/02/09 06:24:01  yiwasaki
// Trasan 1.17 release : cathode codes updated by T.Matsumoto, FPE error fixed by J.Tanaka
//
// Revision 1.22  1999/02/03 06:23:20  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.21  1999/01/25 03:16:18  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.20  1998/11/27 08:15:47  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.19  1998/11/10 09:09:26  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.18  1998/09/28 16:11:15  yiwasaki
// fitter with cathode added
//
// Revision 1.17  1998/08/12 16:33:02  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.16  1998/07/29 04:35:23  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.13  1998/06/17 20:23:05  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.12  1998/06/17 20:12:43  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.11  1998/06/14 11:09:58  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.10  1998/06/11 08:15:47  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.9  1998/06/08 14:39:21  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.8  1998/06/03 17:16:55  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TCluster::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.7  1998/05/26 05:09:18  yiwasaki
// cvs repair
//
// Revision 1.6  1998/05/22 08:21:52  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.5  1998/05/08 09:47:06  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.4  1998/04/23 17:25:06  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.3  1998/04/16 16:51:09  yiwasaki
// minor changes
//
// Revision 1.2  1998/04/10 09:38:20  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:51:15  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef TLink_FLAG_
#define TLink_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#include <strings.h>
#include <iostream>

#define HEP_SHORT_NAMES
#include <string>
#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/ConstAList.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
//cnv #include "trg/cdc/Clust.h"

namespace Belle2 {
  class TRGCDC;
  class TRGCDCTrackMC;
}

namespace Belle {

  typedef HepGeom::Point3D<double>  Point3D;
  template <class T> class CAList;
  class TTrack;

/// A class to relate TRGCDCWireHit and TTrack objects.
  class TLink {

  public:
    /// Constructor.
    TLink(TTrack* track = 0,
          const Belle2::TRGCDCWireHit* hit = 0,
          const HepGeom::Point3D<double> & position = Point3D());

    /// Copy constructor.
    TLink(const TLink&);

    /// Destructor
    virtual ~TLink();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a pointer to a hit.
    const Belle2::TRGCDCWireHit* hit(void) const;

    /// returns a pointer to a wire.
    const Belle2::TRGCDCWire* const wire(void) const;

    /// returns a pointer to a track.
    TTrack* track(void) const;

    /// returns pull.
    double pull(void) const;

    /// returns middle position of a wire. z componet is 0.
    const HepGeom::Point3D<double> & xyPosition(void) const;

    /// returns the closest point on wire to a track.
    const HepGeom::Point3D<double> & positionOnWire(void) const;

    /// returns the closest point on track to wire.
    const HepGeom::Point3D<double> & positionOnTrack(void) const;

    /// returns dPhi to the closest point.
    double dPhi(void) const;

    /// returns left-right. 0:left, 1:right, 2:wire
    unsigned leftRight(void) const;

    /// returns stauts of stereo hit
    int zStatus(void) const;

    /// returns id\#  of the pair, if zStatus == 20 (2 consective hits).
    int zPair(void) const;

    /// returns position.
    const HepGeom::Point3D<double> & position(void) const;

    /// returns distance between point on wire and on track.
    double distance(void) const;

    /// returns neighbor TLink.
    TLink* neighbor(unsigned int) const;

    /// returns a pointer to a TLink.
    TLink* link(void) const;

// S.Suzuki added
    /// returns UseCathode flag
    int usecathode(void) const;

    /// returns pointer to the cluster to be fit
//cnv    Belle2::TRGCDCClust * getmclust(void) const;
// end S.Suzuki

    /// returns arc and Z for the curl finder.
    const HepGeom::Point3D<double> & arcZ(const unsigned = 0) const;

  public:// tmp
//    const HepGeom::Point3D<double> & conf(void) const;
    const HepGeom::Point3D<double> & conf(const HepGeom::Point3D<double> & conf);

  public:// Modifiers
    /// sets results of fitting.
    void update(const HepGeom::Point3D<double> & onTrack,
                const HepGeom::Point3D<double> & onWire,
                unsigned leftRight,
                double pull);

    /// sets a pointer to a hit.
    const Belle2::TRGCDCWireHit* hit(const Belle2::TRGCDCWireHit*);

    /// sets a pointer to a track.
    TTrack* track(TTrack*);

    /// sets and returns the closest point on wire to a track.
    const HepGeom::Point3D<double> & positionOnWire(const HepGeom::Point3D<double> &);
    const HepGeom::Point3D<double> & positionOnWire(double p[3]);

    /// sets and returns the closest point on track to wire.
    const HepGeom::Point3D<double> & positionOnTrack(const HepGeom::Point3D<double> &);
    const HepGeom::Point3D<double> & positionOnTrack(double p[3]);

    /// sets and returns dPhi to the closest point.
    double dPhi(double);

    /// sets and returns position.
    const HepGeom::Point3D<double> & position(const HepGeom::Point3D<double> &);

    /// sets neighbor TLink.
    void neighbor(unsigned int, TLink*);

    /// sets a pointer to a TLink.
    TLink* link(TLink*);

    /// sets left-right. 0:left, 1:right, 2:wire
    unsigned leftRight(unsigned);

    /// sets pull.
    double pull(double);

    /// sets stereo Hit status
    int zStatus(int);

    /// sets id\# of the pair
    int zPair(int);

// S.Suzuki added

    /// sets usecathode flag
    void setusecathode(int);

    /// sets pointer to the cluster to be fit
//cnv    void setmclust(Belle2::TRGCDCClust *);

    /// sets z(phi) before cathode fit
    void setZphiBeforeCathode(float);

// end S.Suzuki

    /// sets and returns arc and Z for the curl finder.
    const HepGeom::Point3D<double> & arcZ(const HepGeom::Point3D<double> &, const unsigned = 0);
    const unsigned fit2D(const unsigned&);
    unsigned fit2D(void);

  public:// Drift distance
    /// returns/sets drift distance.
    float drift(void) const;
    float drift(unsigned) const;
    float drift(float, unsigned);

    /// returns/sets drift distance error.
    float dDrift(void) const;
    float dDrift(unsigned) const;
    float dDrift(float, unsigned);

  public:// Static utility functions
    /// returns TRGCDCTrackMC
    static const Belle2::TRGCDCTrackMC& links2HEP(const AList<TLink> & links);

    /// returns \# of layers.
    static unsigned nSuperLayers(const AList<TLink> & links);
    static unsigned nSuperLayers(const AList<TLink> & links,
                                 unsigned minNHits);

    /// returns \# of missing axial super layers. Stereo super layers are not taken into accout.
    static unsigned nMissingAxialSuperLayers(const AList<TLink> & links);

    /// returns \# of missing stereo super layers.
    static unsigned nMissingStereoSuperLayers(const AList<TLink> & links);

    /// returns \# of layers.
    static unsigned nLayers(const AList<TLink> & links);

    /// returns \# of hits per layer.
    static void nHits(const AList<TLink> & links, unsigned* nHits);

    /// returns \# of hits per super layer.
    static void nHitsSuperLayer(const AList<TLink> & links,
                                unsigned* nHits);

    /// returns hits list per super layer.
    static void nHitsSuperLayer(const AList<TLink> & links,
                                AList<TLink> * list);

    /// returns axial hits.
    static AList<TLink> axialHits(const AList<TLink> & links);

    /// returns stereo hits.
    static AList<TLink> stereoHits(const AList<TLink> & links);

    /// returns \# of axial hits.
    static unsigned nAxialHits(const AList<TLink> & links);

    /// returns \# of stereo hits.
    static unsigned nStereoHits(const AList<TLink> & links);

    /// returns width(wire cell unit) of given AList<TLink>. This function assumes that all TLink's are in the same super layer.
    static unsigned width(const AList<TLink> &);

    /// returns links which are edges. This function assumes that all TLink's are in the same super layer.
    static AList<TLink> edges(const AList<TLink> &);

    /// returns links which are in the same layer as 'a' or 'id'.
    static AList<TLink> sameLayer(const AList<TLink> & list, const TLink& a);
    static AList<TLink> sameLayer(const AList<TLink> & list, unsigned id);

    /// returns links which are in the same super layer as 'a' or 'id'.
    static AList<TLink> sameSuperLayer(const AList<TLink> & list,
                                       const TLink& a);
    static AList<TLink> sameSuperLayer(const AList<TLink> & list, unsigned id);

    /// returns super layer pattern.
    static unsigned superLayer(const AList<TLink> & list);
    static unsigned superLayer(const AList<TLink> & list, unsigned minNHits);

    /// returns the inner(outer)-most link.
    static TLink* innerMost(const AList<TLink> & links);
    static TLink* outerMost(const AList<TLink> & links);

    /// returns links which are in the inner most and outer most layer. This function assumes that all TLink's are in the same super layer.
    static AList<TLink> inOut(const AList<TLink> &);

    /// separate cores and non-cores.
    static void separateCores(const AList<TLink> & input,
                              AList<TLink> & cores,
                              AList<TLink> & nonCores);
    static AList<TLink> cores(const AList<TLink> & input);

    /// removes links from list if wire is same
    static void remove(AList<TLink> & list, const AList<TLink> & links);

    /// dumps TLinks.
    static void dump(const CAList<TLink> & links,
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));
    static void dump(const TLink& link,
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));
    static std::string layerUsage(const AList<TLink> & links);

    /// Sorters
#if defined(__GNUG__)
    static int sortByWireId(const TLink** a, const TLink** b);
    static int sortByX(const TLink** a, const TLink** b);
#else
//     extern "C" int
//  SortByWireId(const void* a, const void* b);
//     extern "C" int
//  SortByX(const void* a, const void* b);
#endif

  private:
    static void set_smallcell(bool s) {
      ms_smallcell = s;
    }
    static void set_superb(bool s) {
      ms_superb = s;
    }
    friend class Belle2::TRGCDC;

  public:
    static void initializeBuffers(void);

  private:
    static void clearBufferSL(void);

  private:
    TTrack* _track;
    const Belle2::TRGCDCWireHit* _hit;
    HepGeom::Point3D<double> _onTrack;
    HepGeom::Point3D<double> _onWire;
    HepGeom::Point3D<double> _position;
    double _dPhi;
    unsigned _leftRight;
    float _drift[2];
    float _dDrift[2];

    int _zStatus;
    int _zPair;
    double _pull;
    TLink* _neighbor[7];
    TLink* _link;

    //...tmp...
    HepGeom::Point3D<double> _conf;

    //-- S. Suzuki added -------
    int _usecathode;
    float _ZphiBeforeCathode;
//cnv    Belle2::TRGCDCClust * _mclust;
    //-- S. Suzuki added end ---

    HepGeom::Point3D<double> _arcZ[4];
    unsigned _fit2D;
    //
    // for quick dicision
    //
    static bool ms_smallcell;
    static bool ms_superb;

    //...Buffers...
    static unsigned _nL;
    static unsigned _nSL;
    static unsigned _nSLA;
    static unsigned* _nHitsSL;

#ifdef TRASAN_DEBUG
  public:
    static unsigned nTLinks(void);
    static unsigned nTLinksMax(void);
  private:
    static unsigned _nTLinks;
    static unsigned _nTLinksMax;
#endif
  };

//-----------------------------------------------------------------------------

#ifdef TLink_NO_INLINE
#define inline
#else
#undef inline
#define TLink_INLINE_DEFINE_HERE
#endif

#ifdef TLink_INLINE_DEFINE_HERE

  inline
  const Belle2::TRGCDCWireHit*
  TLink::hit(void) const
  {
    return _hit;
  }

  inline
  TTrack*
  TLink::track(void) const
  {
    return _track;
  }

  inline
  const Belle2::TRGCDCWireHit*
  TLink::hit(const Belle2::TRGCDCWireHit* a)
  {
    return _hit = a;
  }

  inline
  TTrack*
  TLink::track(TTrack* a)
  {
    return _track = a;
  }

  inline
  void
  TLink::update(const HepGeom::Point3D<double> & onTrack,
                const HepGeom::Point3D<double> & onWire,
                unsigned leftRight,
                double pull)
  {
    _onTrack = onTrack;
    _onWire = onWire;
    _leftRight = leftRight;
    _pull = pull;
  }

  inline
  double
  TLink::pull(void) const
  {
    return _pull;
  }

  inline
  double
  TLink::pull(double a)
  {
    return _pull = a;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::positionOnWire(void) const
  {
    return _onWire;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::positionOnTrack(void) const
  {
    return _onTrack;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::positionOnWire(const HepGeom::Point3D<double> & a)
  {
    return _onWire = a;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::positionOnWire(double p[3])
  {
    _onWire.setX(p[0]);
    _onWire.setY(p[1]);
    _onWire.setZ(p[2]);
    return _onWire;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::positionOnTrack(const HepGeom::Point3D<double> & a)
  {
    return _onTrack = a;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::positionOnTrack(double p[3])
  {
    _onTrack.setX(p[0]);
    _onTrack.setY(p[1]);
    _onTrack.setZ(p[2]);
    return _onTrack;
  }

  inline
  unsigned
  TLink::leftRight(void) const
  {
    return _leftRight;
  }

  inline
  unsigned
  TLink::leftRight(unsigned a)
  {
    return _leftRight = a;
  }

  inline
  double
  TLink::dPhi(void) const
  {
    return _dPhi;
  }

  inline
  double
  TLink::dPhi(double a)
  {
    return _dPhi = a;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::position(void) const
  {
    return _position;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::position(const HepGeom::Point3D<double> & a)
  {
    return _position = a;
  }

// inline
// const HepGeom::Point3D<double> &
// TLink::conf(void) const {
//     return _conf;
// }

  inline
  const HepGeom::Point3D<double> &
  TLink::conf(const HepGeom::Point3D<double> & a)
  {
    return _conf = a;
  }

  inline
  void
  TLink::neighbor(unsigned n, TLink* neighbor)
  {
    if (n < 7)
      _neighbor[n] = neighbor;
  }

  inline
  TLink*
  TLink::neighbor(unsigned n) const
  {
    if (n < 7)
      return _neighbor[n];
    return NULL;
  }

  inline
  TLink*
  TLink::link(void) const
  {
    return _link;
  }

  inline
  TLink*
  TLink::link(TLink* a)
  {
    return _link = a;
  }

  inline
  double
  TLink::distance(void) const
  {
    return (_onTrack - _onWire).mag();
  }

  inline
  const Belle2::TRGCDCWire* const
  TLink::wire(void) const
  {
    if (_hit) return & _hit->wire();
    return NULL;
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::xyPosition(void) const
  {
    return _hit->wire().xyPosition();
  }

  inline
  int
  TLink::zStatus(void) const
  {
    return _zStatus;
  }

  inline
  int
  TLink::zStatus(int a)
  {
    return _zStatus = a;
  }

  inline
  int
  TLink::zPair(void) const
  {
    return _zPair;
  }

  inline
  int
  TLink::zPair(int a)
  {
    return _zPair = a;
  }

//-- S. Suzuki added -------
  inline
  int
  TLink::usecathode(void) const
  {
    return _usecathode;
  }

  inline
  void
  TLink::setusecathode(int a)
  {
    _usecathode = a ;
  }

// inline
// Belle2::TRGCDCClust *
// TLink::getmclust(void) const {
//     return _mclust;
// }

// inline
// void
// TLink::setmclust(Belle2::TRGCDCClust * a) {
//     _mclust = a ;
// }

//-- S. Suzuki added end ----

  inline
  const HepGeom::Point3D<double> &
  TLink::arcZ(const unsigned i) const
  {
    if (i < 4)return _arcZ[i];
    std::cerr << "Error!! Please stop!!.....arcZ of TLink!!! in Trasan." << std::endl;
    return _arcZ[0];
  }

  inline
  const HepGeom::Point3D<double> &
  TLink::arcZ(const HepGeom::Point3D<double> &az, const unsigned i)
  {
    if (i < 4)return _arcZ[i] = az;
    std::cerr << "Error!! Please stop!!.....arcZ of TLink!!! in Trasan." << std::endl;
    return _arcZ[0];
  }

  inline
  void
  TLink::setZphiBeforeCathode(float a)
  {
    _ZphiBeforeCathode = a;
  }

  inline
  const unsigned
  TLink::fit2D(const unsigned& f)
  {
    return _fit2D = f;
  }

  inline
  unsigned
  TLink::fit2D(void)
  {
    return _fit2D;
  }

  inline
  float
  TLink::drift(unsigned a) const
  {
    return _drift[a];
  }

  inline
  float
  TLink::drift(float b, unsigned a)
  {
    return _drift[a] = b;
  }

  inline
  float
  TLink::dDrift(unsigned a) const
  {
    return _dDrift[a];
  }

  inline
  float
  TLink::dDrift(float b, unsigned a)
  {
    return _dDrift[a] = b;
  }

  inline
  float
  TLink::drift(void) const
  {
    return (_drift[0] + _drift[1]) / 2.;
  }

  inline
  float
  TLink::dDrift(void) const
  {
    return (_dDrift[0] + _dDrift[1]) / 2.;
  }

  inline
  void
  TLink::clearBufferSL(void)
  {
    bzero(_nHitsSL, sizeof(unsigned) * _nSL);
  }

#endif

#undef inline

} // namespace Belle

#endif /* TLink_FLAG_ */

