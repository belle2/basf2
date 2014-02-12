/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTSegment_H
#define FTSegment_H

#include "skim/hlt/modules/level3/FTList.h"

namespace Belle2 {

  class FTFinder;
  class FTWire;
  class FTLayer;
  class FTSuperLayer;
  class FTTrack;
  class FTSegment {
  public:

    //! constructors
    FTSegment(FTSuperLayer* super, FTList<FTWire*>& hits);

    //! destructors
    ~FTSegment();

  public: // public member functions
    //! examine segment
    int examine(void);

    //! update information for axial segment
    void update(void);

    //! update information for stereo segment
    int update3D(const FTTrack* track);

    //! link stereo segments by tanLambda
    void linkStereoSegments(void);

    //! connect short segments
    void connectOuter(const FTList<FTWire*>&, const FTList<FTWire*>&);

    //! connect single hit
    void connectOuter(const FTWire*);

    //! connect single hit
    void connectInner(const FTWire*);

  public: // Selectors
    //! returns wire-hit list
    FTList<FTWire*>& getWireHits(void) const;

    //! returns super-layer
    FTSuperLayer& superLayer(void) const;

    //! returns innerBoundHits
    FTList<FTWire*>& getInnerBoundHits(void) const;

    //! returns outerBoundHits
    FTList<FTWire*>& getOuterBoundHits(void)  const;

    //! returns x of outgoing position
    double outgoingX(void) const;

    //! returns y of outgoing position
    double outgoingY(void) const;

    //! returns x of incoming position
    double incomingX(void) const;

    //! returns y of incoming position
    double incomingY(void) const;

    //! returns phi of incoming position
    double incomingPhi(void) const;

    //! returns phi of outgoing position
    double outgoingPhi(void) const;

    //! returns track
    FTTrack* track(void) const;

    //! returns kappa(axial)
    double kappa(void) const;

    //! returns r from origin
    double r(void) const;

    //! returns s for matched 2D track
    double s(void) const;

    //! returns z for matched 2D track
    double z(void) const;

  public: // Modifiers
    //! set track
    FTTrack* track(FTTrack*);

  private: // private data members
    FTList<FTWire*>& m_wireHits;
    FTSuperLayer& m_superLayer;
    FTList<FTWire*>& m_innerBoundHits;
    FTList<FTWire*>& m_outerBoundHits;
    FTList<double>* m_sList;
    union {
      double m_kappa;
      FTList<double>* m_zList;
    };
    union {
      FTTrack* m_track;
      FTList<FTTrack*>* m_trackList;
    };
    double m_r;
    double m_outgoing_x;
    double m_outgoing_y;
    double m_incoming_x;
    double m_incoming_y;
#ifdef FZISAN_DEBUG
    Gen_hepevt* m_hep;
#endif
  };


  //----------------------------------------------
#ifdef FTSegment_NO_INLINE
#define inline
#else
#undef inline
#define FTSegment_INLINE_DEFINE_HERE
#endif

#ifdef FTSegment_INLINE_DEFINE_HERE

  inline
  FTSegment::FTSegment(FTSuperLayer* super, FTList<FTWire*>& hits)
    : m_wireHits(hits),
      m_superLayer(*super),
      m_innerBoundHits(*(new FTList<FTWire*>(3))),
      m_outerBoundHits(*(new FTList<FTWire*>(3))),
      m_sList(NULL),
      m_zList(NULL),
      m_track(NULL)
  {
  }

  inline
  FTSegment::~FTSegment()
  {
    delete &m_wireHits;
    delete &m_innerBoundHits;
    delete &m_outerBoundHits;
    if (m_sList) {
      delete m_sList;
      delete m_zList;
      delete m_trackList;
    }
  }

  inline
  void
  FTSegment::connectOuter(const FTList<FTWire*>& outerHits,
                          const FTList<FTWire*>& outerBound)
  {
    m_wireHits.append((FTList<FTWire*>&)outerHits);
    m_outerBoundHits.clear();
    m_outerBoundHits.append((FTList<FTWire*>&)outerBound);
  }

  inline
  void
  FTSegment::connectOuter(const FTWire* h)
  {
    m_wireHits.append((FTWire*)h);
    m_outerBoundHits.clear();
    m_outerBoundHits.append((FTWire*)h);
  }

  inline
  void
  FTSegment::connectInner(const FTWire* h)
  {
    m_wireHits.append((FTWire*)h);
    m_innerBoundHits.clear();
    m_innerBoundHits.append((FTWire*)h);
  }

  inline
  FTSuperLayer&
  FTSegment::superLayer(void) const
  {
    return m_superLayer;
  }

  inline
  FTList<FTWire*>&
  FTSegment::getWireHits(void) const
  {
    return m_wireHits;
  }

  inline
  FTList<FTWire*>&
  FTSegment::getInnerBoundHits(void) const
  {
    return m_innerBoundHits;
  }

  inline
  FTList<FTWire*>&
  FTSegment::getOuterBoundHits(void) const
  {
    return m_outerBoundHits;
  }

  inline
  double
  FTSegment::outgoingX(void) const
  {
    return m_outgoing_x;
  }

  inline
  double
  FTSegment::outgoingY(void) const
  {
    return m_outgoing_y;
  }

  inline
  double
  FTSegment::incomingX(void) const
  {
    return m_incoming_x;
  }

  inline
  double
  FTSegment::incomingY(void) const
  {
    return m_incoming_y;
  }

  inline
  double
  FTSegment::r(void) const
  {
    return m_r;
  }

  inline
  double
  FTSegment::kappa(void) const
  {
    return m_kappa;
  }

  inline
  double
  FTSegment::s(void) const
  {
    return m_sList->first();
  }

  inline
  double
  FTSegment::z(void) const
  {
    return m_zList->first();
  }

  inline
  FTTrack*
  FTSegment::track(void) const
  {
    return m_track;
  }

  inline
  FTTrack*
  FTSegment::track(FTTrack* src)
  {
    return m_track = src;
  }

#endif

#undef inline

}

#endif /* FTSegment_FLAG_ */

