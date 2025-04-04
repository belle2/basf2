/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to relate TRGCDCCellHit and TRGCDCTrack objects.
//-----------------------------------------------------------------------------

#ifndef TRGCDCLink_FLAG_
#define TRGCDCLink_FLAG_

#include <cstring>
#include <vector>
#include "CLHEP/Geometry/Point3D.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/CellHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCLink TRGCDCLink
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCTrack;
  class TRGCDCTrackMC;
  class TRGCDCWire;
  class TRGCDCSegment;

/// A class to relate TRGCDCCellHit and TRGCDCTrack objects.
  class TRGCDCLink {

  public:

    /// Constructor.
    TRGCDCLink(TRGCDCTrack* track = 0,
               const TRGCDCCellHit* hit = 0,
               const HepGeom::Point3D<double>& position = Point3D());

    /// Copy constructor.
    TRGCDCLink(const TRGCDCLink&);

    /// Destructor
    virtual ~TRGCDCLink();

    /// destructs all TRGCDCLink objects. (Called by TRGCDC)
    static void removeAll(void);

    /// new operator.
    static void* operator new (size_t);

    /// delete operator.
    static void operator delete (void*);

  public:// Selectors

    /// returns a pointer to a track.
    TRGCDCTrack* track(void) const;

    /// returns a pointer to a hit.
    const TRGCDCCellHit* hit(void) const;

    /// returns a pointer to a cell.
    const TRGCDCCell* cell(void) const;

    /// returns a pointer to a wire.
    const TRGCDCWire* wire(void) const;

    /// returns a pointer to a track segment.
    const TRGCDCSegment* segment(void) const;

    /// returns pull.
    double pull(void) const;

    /// returns middle position of a wire. z component is 0.
    const HepGeom::Point3D<double>& xyPosition(void) const;

    /// returns the closest point on wire to a track.
    const HepGeom::Point3D<double>& positionOnWire(void) const;

    /// returns the closest point on track to wire.
    const HepGeom::Point3D<double>& positionOnTrack(void) const;

    /// returns dPhi to the closest point.
    double dPhi(void) const;

    /// returns left-right. 0:left, 1:right, 2:wire
    unsigned leftRight(void) const;

    /// returns position.
    const HepGeom::Point3D<double>& position(void) const;

    /// returns distance between point on wire and on track.
    double distance(void) const;

    /// returns neighbor TRGCDCLink.
    TRGCDCLink* neighbor(unsigned n) const;

    /// returns a pointer to a TRGCDCLink.
    TRGCDCLink* link(void) const;

  public:// tmp
//    const HepGeom::Point3D<double> & conf(void) const;
    /// conf 3D point
    const HepGeom::Point3D<double>& conf(const HepGeom::Point3D<double>& conf);

  public:// Modifiers
    /// sets results of fitting.
    void update(const HepGeom::Point3D<double>& onTrack,
                const HepGeom::Point3D<double>& onWire,
                unsigned leftRight,
                double pull);

    /// sets a pointer to a hit.
    const TRGCDCCellHit* hit(const TRGCDCCellHit*);

    /// sets a pointer to a track.
    TRGCDCTrack* track(TRGCDCTrack*);

    /// sets and returns the closest point on wire to a track.
    const HepGeom::Point3D<double>& positionOnWire(const HepGeom::Point3D<double>&);
    /// sets and returns the closest point on wire to a track.
    const HepGeom::Point3D<double>& positionOnWire(double p[3]);

    /// sets and returns the closest point on track to wire.
    const HepGeom::Point3D<double>& positionOnTrack(const HepGeom::Point3D<double>&);
    /// sets and returns the closest point on track to wire.
    const HepGeom::Point3D<double>& positionOnTrack(double p[3]);

    /// sets and returns dPhi to the closest point.
    double dPhi(double);

    /// sets and returns position.
    const HepGeom::Point3D<double>& position(const HepGeom::Point3D<double>&);

    /// sets neighbor TRGCDCLink.
    void neighbor(unsigned n, TRGCDCLink* neighbor);

    /// sets a pointer to a TRGCDCLink.
    TRGCDCLink* link(TRGCDCLink*);

    /// sets left-right. 0:left, 1:right, 2:wire
    unsigned leftRight(unsigned);

    /// sets pull.
    double pull(double);

//    const unsigned fit2D(const unsigned &);

//    unsigned fit2D(void);

  public:// Drift distance

    /// returns drift distance.
    float drift(void) const;

    /// returns drift distance of left or right.
    float drift(unsigned) const;

    /// sets and returns drift distance of left or right.
    float drift(float, unsigned);

    /// returns drift distance error.
    float dDrift(void) const;

    /// returns drift distance error of left or right.
    float dDrift(unsigned) const;

    /// sets and returns drift distance error of left or right.
    float dDrift(float, unsigned);

  public:// Utility functions

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Static utility functions (for Belle2)

    /// separates into layers.
    static void separate(const std::vector<TRGCDCLink*>& links,
                         unsigned nLayers,
                         std::vector<TRGCDCLink*>* layers);

  public:// Static utility functions

    /// returns TRGCDCTrackMC
    static const TRGCDCTrackMC& links2HEP(
      const std::vector<TRGCDCLink*>& links);

    /// returns \# of layers.
    static unsigned nSuperLayers(const std::vector<TRGCDCLink*>& links);

    /// returns \# of layers.
    static unsigned nSuperLayers(const std::vector<TRGCDCLink*>& links,
                                 unsigned minNHits);

    /// returns \# of missing axial super layers.
    /// Stereo super layers are not taken into account.
    static unsigned nMissingAxialSuperLayers(
      const std::vector<TRGCDCLink*>& links);

    /// returns \# of missing stereo super layers.
    static unsigned nMissingStereoSuperLayers(
      const std::vector<TRGCDCLink*>& links);

    /// returns \# of layers.
    static unsigned nLayers(const std::vector<TRGCDCLink*>& links);

    /// returns \# of hits per layer.
    static void nHits(const std::vector<TRGCDCLink*>& links,
                      unsigned* nHits);

    /// returns \# of hits per super layer.
    static void nHitsSuperLayer(const std::vector<TRGCDCLink*>& links,
                                unsigned* nHits);

    /// returns hits list per super layer.
    static void nHitsSuperLayer(const std::vector<TRGCDCLink*>& links,
                                std::vector<TRGCDCLink*>* list);

    /// returns axial hits.
    static std::vector<TRGCDCLink*> axialHits(
      const std::vector<TRGCDCLink*>& links);

    /// returns stereo hits.
    static std::vector<TRGCDCLink*> stereoHits(
      const std::vector<TRGCDCLink*>& links);

    /// returns \# of axial hits.
    static unsigned nAxialHits(const std::vector<TRGCDCLink*>& links);

    /// returns \# of stereo hits.
    static unsigned nStereoHits(const std::vector<TRGCDCLink*>& links);

    /// returns width(wire cell unit) of given std::vector<TRGCDCLink *>.
    /// This function assumes that all TRGCDCLink's are in the same super
    /// layer.
    static unsigned width(const std::vector<TRGCDCLink*>&);

    /// returns links which are edges. This function assumes that all
    /// TRGCDCLink's are in the same super layer.
    static std::vector<TRGCDCLink*> edges(const std::vector<TRGCDCLink*>&);

    /// returns links which are in the same layer as 'a' or 'id'.
    static std::vector<TRGCDCLink*> sameLayer(
      const std::vector<TRGCDCLink*>& list,
      const TRGCDCLink& a);

    /// returns links which are in the same layer as 'a' or 'id'.
    static std::vector<TRGCDCLink*> sameLayer(
      const std::vector<TRGCDCLink*>& list,
      unsigned id);

    /// returns links which are in the same super layer as 'a' or 'id'.
    static std::vector<TRGCDCLink*> sameSuperLayer(
      const std::vector<TRGCDCLink*>& list,
      const TRGCDCLink& a);

    /// returns links which are in the same super layer as 'a' or 'id'.
    static std::vector<TRGCDCLink*> sameSuperLayer(
      const std::vector<TRGCDCLink*>& list,
      unsigned id);

    /// returns super layer pattern.
    static unsigned superLayer(const std::vector<TRGCDCLink*>& list);

    /// returns super layer pattern.
    static unsigned superLayer(const std::vector<TRGCDCLink*>& list,
                               unsigned minNHits);

    /// returns the inner-most link.
    static TRGCDCLink* innerMost(const std::vector<TRGCDCLink*>& links);

    /// returns the outer-most link.
    static TRGCDCLink* outerMost(const std::vector<TRGCDCLink*>& links);

    /// returns links which are in the inner most and outer most
    /// layer. This function assumes that all TRGCDCLink's are in the
    /// same super layer.
    static std::vector<TRGCDCLink*> inOut(const std::vector<TRGCDCLink*>&);

    /// separate cores and non-cores.
    static void separateCores(const std::vector<TRGCDCLink*>& input,
                              std::vector<TRGCDCLink*>& cores,
                              std::vector<TRGCDCLink*>& nonCores);

    /// separate cores and non-cores.
    static std::vector<TRGCDCLink*> cores(
      const std::vector<TRGCDCLink*>& input);

    /// removes links from list if wire is same
    static void remove(std::vector<TRGCDCLink*>& list,
                       const std::vector<TRGCDCLink*>& links);

    /// dumps TRGCDCLinks.
    static void dump(const std::vector<const TRGCDCLink*>& links,
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));

    /// dumps TRGCDCLinks.
    static void dump(const std::vector<TRGCDCLink*>& links,
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));

    /// dumps TRGCDCLinks.
    static void dump(const TRGCDCLink& link,
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));

    /// usage of each layer
    static std::string layerUsage(const std::vector<TRGCDCLink*>& links);

  public:// Sorters

    /// sorts by ID.
    static bool sortById(const TRGCDCLink* a, const TRGCDCLink* b);

    /// sorts by X position.
    static int sortByX(const TRGCDCLink* a, const TRGCDCLink* b);

  private:
    friend class TRGCDC;

  public:
    /// initialize the Buffers
    static void initializeBuffers(void);

  private:
    /// clear buffers
    static void clearBufferSL(void);

    /// dumps debug information.
    void dump_base(const std::string& message = std::string(""),
                   const std::string& prefix = std::string("")) const;

  private:

    /// Keeps all TRGCDCLinks created by new().
    static std::vector<TRGCDCLink*> m_all;

    /// Track object.
    TRGCDCTrack* m_track;

    /// Cell hit object.
    const TRGCDCCellHit* m_hit;

    /// position on track
    HepGeom::Point3D<double> m_onTrack;
    /// position on wire
    HepGeom::Point3D<double> m_onWire;
    /// position
    HepGeom::Point3D<double> m_position;
    /// dPhi
    double m_dPhi;
    /// left right
    unsigned m_leftRight;
    /// drift distance
    float m_drift[2];
    /// drift distance error
    float m_dDrift[2];

    /// z status of stereo
    int m_zStatus;
    /// not useds?
    int m_zPair;
    /// pull
    double m_pull;
    /// neighbor TRGCDCLink
    TRGCDCLink* m_neighbor[7];
    /// TRGCDCLink
    TRGCDCLink* m_link;

    /// ...tmp...
    HepGeom::Point3D<double> m_conf;

    /// arcZ
    HepGeom::Point3D<double> m_arcZ[4];
    ///  2D fitted or not
    unsigned m_fit2D;
    //
    // for quick decision
    //
    /// ms_smallcell
    static bool ms_smallcell;
    /// ms_superb
    static bool ms_superb;

    /// ...Buffers...
    static unsigned m_nL;
    /// ...Buffers...
    static unsigned m_nSL;
    /// ...Buffers...
    static unsigned m_nSLA;
    /// ...Buffers...
    static unsigned* m_nHitsSL;

#ifdef TRASAN_DEBUG
  public:
    static unsigned nTRGCDCLinks(void);
    static unsigned nTRGCDCLinksMax(void);
  private:
    static unsigned _nTRGCDCLinks;
    static unsigned _nTRGCDCLinksMax;
#endif
  };

//-----------------------------------------------------------------------------

  inline
  const TRGCDCCellHit*
  TRGCDCLink::hit(void) const
  {
    return m_hit;
  }

  inline
  TRGCDCTrack*
  TRGCDCLink::track(void) const
  {
    return m_track;
  }

  inline
  const TRGCDCCellHit*
  TRGCDCLink::hit(const TRGCDCCellHit* a)
  {
    return m_hit = a;
  }

  inline
  TRGCDCTrack*
  TRGCDCLink::track(TRGCDCTrack* a)
  {
    return m_track = a;
  }

  inline
  void
  TRGCDCLink::update(const HepGeom::Point3D<double>& onTrack,
                     const HepGeom::Point3D<double>& onWire,
                     unsigned leftRight,
                     double pull)
  {
    m_onTrack = onTrack;
    m_onWire = onWire;
    m_leftRight = leftRight;
    m_pull = pull;
  }

  inline
  double
  TRGCDCLink::pull(void) const
  {
    return m_pull;
  }

  inline
  double
  TRGCDCLink::pull(double a)
  {
    return m_pull = a;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::positionOnWire(void) const
  {
    return m_onWire;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::positionOnTrack(void) const
  {
    return m_onTrack;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::positionOnWire(const HepGeom::Point3D<double>& a)
  {
    return m_onWire = a;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::positionOnWire(double p[3])
  {
    m_onWire.setX(p[0]);
    m_onWire.setY(p[1]);
    m_onWire.setZ(p[2]);
    return m_onWire;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::positionOnTrack(const HepGeom::Point3D<double>& a)
  {
    return m_onTrack = a;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::positionOnTrack(double p[3])
  {
    m_onTrack.setX(p[0]);
    m_onTrack.setY(p[1]);
    m_onTrack.setZ(p[2]);
    return m_onTrack;
  }

  inline
  unsigned
  TRGCDCLink::leftRight(void) const
  {
    return m_leftRight;
  }

  inline
  unsigned
  TRGCDCLink::leftRight(unsigned a)
  {
    return m_leftRight = a;
  }

  inline
  double
  TRGCDCLink::dPhi(void) const
  {
    return m_dPhi;
  }

  inline
  double
  TRGCDCLink::dPhi(double a)
  {
    return m_dPhi = a;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::position(void) const
  {
    return m_position;
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::position(const HepGeom::Point3D<double>& a)
  {
    return m_position = a;
  }

// inline
// const HepGeom::Point3D<double> &
// TRGCDCLink::conf(void) const {
//     return m_conf;
// }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::conf(const HepGeom::Point3D<double>& a)
  {
    return m_conf = a;
  }

  inline
  void
  TRGCDCLink::neighbor(unsigned n, TRGCDCLink* neighbor)
  {
    if (n < 7)
      m_neighbor[n] = neighbor;
  }

  inline
  TRGCDCLink*
  TRGCDCLink::neighbor(unsigned n) const
  {
    if (n < 7)
      return m_neighbor[n];
    return NULL;
  }

  inline
  TRGCDCLink*
  TRGCDCLink::link(void) const
  {
    return m_link;
  }

  inline
  TRGCDCLink*
  TRGCDCLink::link(TRGCDCLink* a)
  {
    return m_link = a;
  }

  inline
  double
  TRGCDCLink::distance(void) const
  {
    return (m_onTrack - m_onWire).mag();
  }

  inline
  const HepGeom::Point3D<double>&
  TRGCDCLink::xyPosition(void) const
  {
    return m_hit->cell().xyPosition();
  }

// inline
// const unsigned
// TRGCDCLink::fit2D(const unsigned &f) {
//   return m_fit2D = f;
// }

// inline
// unsigned
// TRGCDCLink::fit2D(void) {
//   return m_fit2D;
// }

  inline
  float
  TRGCDCLink::drift(unsigned a) const
  {
    return m_drift[a];
  }

  inline
  float
  TRGCDCLink::drift(float b, unsigned a)
  {
    return m_drift[a] = b;
  }

  inline
  float
  TRGCDCLink::dDrift(unsigned a) const
  {
    return m_dDrift[a];
  }

  inline
  float
  TRGCDCLink::dDrift(float b, unsigned a)
  {
    return m_dDrift[a] = b;
  }

  inline
  float
  TRGCDCLink::drift(void) const
  {
    return (m_drift[0] + m_drift[1]) / 2.;
  }

  inline
  float
  TRGCDCLink::dDrift(void) const
  {
    return (m_dDrift[0] + m_dDrift[1]) / 2.;
  }

  inline
  void
  TRGCDCLink::clearBufferSL(void)
  {
    bzero(m_nHitsSL, sizeof(unsigned) * m_nSL);
  }

  inline
  const TRGCDCCell*
  TRGCDCLink::cell(void) const
  {
    if (m_hit)
      return & m_hit->cell();
    return 0;
  }

} // namespace Belle2

#endif /* TRGCDCLink_FLAG_ */
